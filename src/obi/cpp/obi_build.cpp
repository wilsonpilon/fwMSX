// fwMSX -- obi: a build da receita (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/obi-spec.md.
#include "obi.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>

#include "../../asm/cpp/assembler.h"
#include "../../badig/cpp/badig.h"
#include "../../msxdisk/core/disk_image.h"
#include "../asm/obi_crc.h"
#include "../c/obi_name.h"
#include "../fortran/obi_fit.h"

namespace fs = std::filesystem;

namespace obi {

namespace {

// Um arquivo pronto para o disco (depois de montado/convertido).
struct Staged {
    std::string name; // 8.3 no disco
    std::vector<uint8_t> data;
    std::string from;
    std::string host; // so' os arquivos de apoio do Dignified (disk_files) ja' estao em disco: o caminho; senao vazio
};

std::string Upper(std::string s) {
    for (char &c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

std::string JoinPath(const std::string &dir, const std::string &rel) {
    const fs::path r(rel);
    if (r.is_absolute()) return r.lexically_normal().string();
    return (fs::path(dir) / r).lexically_normal().string();
}

// Le pelo leitor (o projeto) e, se ele nao tem, do disco.
bool ReadAny(const Reader &reader, const std::string &path, std::vector<uint8_t> &data) {
    if (reader && reader(path, data)) return true;
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    data.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    return true;
}

// .include / .incbin do fonte Assembly, relativos a pasta do fonte.
class AsmFiles : public fwasm::FileProvider {
public:
    AsmFiles(std::string dir, Reader reader) : dir_(std::move(dir)), reader_(std::move(reader)) {}
    bool Read(const std::string &path, std::vector<uint8_t> &data) override { return ReadAny(reader_, JoinPath(dir_, path), data); }

private:
    std::string dir_;
    Reader reader_;
};

// O leitor de .include do Basic Dignified vale so' durante a build.
struct BadigReaderScope {
    explicit BadigReaderScope(const Reader &r) {
        if (!r) return;
        badig::SetSourceReader([r](const std::string &path, std::string &data) {
            std::vector<uint8_t> d;
            if (!r(path, d)) return false;
            data.assign(d.begin(), d.end());
            return true;
        });
        set_ = true;
    }
    ~BadigReaderScope() {
        if (set_) badig::SetSourceReader(badig::SourceReader());
    }
    bool set_ = false;
};

std::string DiskErr(msxdisk::DiskError e);

std::string g_emulator_dir;

// A pasta com os arquivos do MSX-DOS `version`: emulator/ ao lado do executavel, a "emulator" da pasta atual e, na arvore de desenvolvimento,
// resource/msxdosN. Vazia se nao achou.
std::string SystemDir(int version) {
    const std::string leaf = "msxdos" + std::to_string(version);
    std::vector<fs::path> cands;
    if (!g_emulator_dir.empty()) cands.push_back(fs::path(g_emulator_dir) / leaf);
    cands.push_back(fs::path("emulator") / leaf);
    cands.push_back(fs::path("resource") / leaf);
    cands.push_back(fs::path("..") / "resource" / leaf);
    for (const fs::path &c : cands) {
        std::error_code ec;
        if (fs::is_directory(c, ec)) return c.string();
    }
    return {};
}

// Copia a pasta `dir` (recursiva) para a imagem, com os nomes em maiusculas.
bool AddTree(msxdisk::DiskImage &img, const fs::path &dir, const std::string &prefix, std::string &err) {
    std::error_code ec;
    for (const fs::directory_entry &e : fs::directory_iterator(dir, ec)) {
        const std::string name = Upper(e.path().filename().string());
        msxdisk::DiskError de = msxdisk::DiskError::None;
        if (e.is_directory(ec)) {
            if (!img.MakeDirectory(prefix + name, &de)) {
                err = "cannot create the directory " + prefix + name + " on the disk (" + DiskErr(de) + ")";
                return false;
            }
            if (!AddTree(img, e.path(), prefix + name + "\\", err)) return false;
        } else if (!img.AddFile(e.path(), prefix + name, 0x20, &de)) {
            err = "cannot put " + prefix + name + " on the disk (" + DiskErr(de) + ")";
            return false;
        }
    }
    return true;
}

// Poe o sistema do MSX-DOS `version` num disco em branco: o essencial (MSXDOS.SYS + COMMAND.COM; no 2 tambem MSXDOS2.SYS + COMMAND2.COM) e,
// com `tools`, o resto da pasta. O setor de boot do CreateBlank carrega o MSXDOS.SYS.
bool LoadSystem(msxdisk::DiskImage &img, int version, bool tools, std::string &from, std::string &err) {
    from = SystemDir(version);
    if (from.empty()) {
        err = "MSX-DOS " + std::to_string(version) + " files not found: put them in emulator/msxdos" + std::to_string(version) + " next to the program";
        return false;
    }
    const std::vector<std::string> core = version == 1 ? std::vector<std::string>{"MSXDOS.SYS", "COMMAND.COM"}
                                                       : std::vector<std::string>{"MSXDOS.SYS", "MSXDOS2.SYS", "COMMAND2.COM"};
    std::error_code ec;
    std::vector<std::string> done;
    for (const fs::directory_entry &e : fs::directory_iterator(from, ec)) {
        const std::string name = Upper(e.path().filename().string());
        if (std::find(core.begin(), core.end(), name) == core.end()) continue;
        msxdisk::DiskError de = msxdisk::DiskError::None;
        if (!img.AddFile(e.path(), name, 0x20, &de)) {
            err = "cannot put " + name + " on the disk (" + DiskErr(de) + ")";
            return false;
        }
        done.push_back(name);
    }
    for (const std::string &c : core) {
        if (std::find(done.begin(), done.end(), c) == done.end()) {
            err = c + " is missing in " + from;
            return false;
        }
    }
    if (!tools) return true;
    for (const fs::directory_entry &e : fs::directory_iterator(from, ec)) {
        const std::string name = Upper(e.path().filename().string());
        if (std::find(core.begin(), core.end(), name) != core.end()) continue;
        msxdisk::DiskError de = msxdisk::DiskError::None;
        if (e.is_directory(ec)) {
            if (!img.MakeDirectory(name, &de) || !AddTree(img, e.path(), name + "\\", err)) {
                if (err.empty()) err = "cannot create the directory " + name + " on the disk";
                return false;
            }
        } else if (!img.AddFile(e.path(), name, 0x20, &de)) {
            err = "cannot put " + name + " on the disk (" + DiskErr(de) + ")";
            return false;
        }
    }
    return true;
}

const char *NameError(int rc) {
    switch (rc) {
    case OBI_NAME_EMPTY: return "empty name";
    case OBI_NAME_BASE_LONG: return "more than 8 characters before the dot";
    case OBI_NAME_EXT_LONG: return "more than 3 characters after the dot";
    default: return "character not allowed in an MSX-DOS name";
    }
}

// O nome 8.3 de um arquivo vindo de `src` com a extensao `ext` ("BMX"), ou o que a receita pediu em `name`.
bool DiskName(const FileSpec &f, const std::string &ext, std::string &out, std::string &err) {
    std::string want = f.name;
    if (want.empty()) {
        const std::string stem = fs::path(f.source).stem().string();
        want = ext.empty() ? fs::path(f.source).filename().string() : stem + "." + ext;
    }
    char buf[16];
    const int rc = obi_msx_name(want.c_str(), buf, sizeof(buf));
    if (rc != OBI_NAME_OK) {
        err = "Obifile, line " + std::to_string(f.line) + ": '" + want + "' is not a valid MSX-DOS file name (" + NameError(rc) + "); use 'name:' to choose one";
        return false;
    }
    out = buf;
    return true;
}

std::string TempDir() {
    static std::atomic<int> counter{0};
    const fs::path p = fs::temp_directory_path() / ("fwmsx_obi_" + std::to_string(static_cast<long long>(fs::file_time_type::clock::now().time_since_epoch().count() % 1000000000LL)) +
                                                    "_" + std::to_string(counter++));
    std::error_code ec;
    fs::create_directories(p, ec);
    return p.string();
}

struct TempCleanup {
    std::string dir;
    ~TempCleanup() {
        std::error_code ec;
        if (!dir.empty()) fs::remove_all(dir, ec);
    }
};

std::string DiskErr(msxdisk::DiskError e) {
    switch (e) {
    case msxdisk::DiskError::DiskFull: return "disk full";
    case msxdisk::DiskError::DirectoryFull: return "directory full";
    default: return "write error";
    }
}

} // namespace

std::string FindObifile(const std::string &dir) {
    std::error_code ec;
    const fs::path d = dir.empty() ? fs::current_path(ec) : fs::path(dir);
    if (fs::is_regular_file(d / "Obifile", ec)) return (d / "Obifile").string();
    for (const fs::directory_entry &e : fs::directory_iterator(d, ec)) {
        if (e.is_regular_file(ec) && Upper(e.path().extension().string()) == ".OBI") return e.path().string();
    }
    return {};
}

void SetEmulatorDir(const std::string &dir) { g_emulator_dir = dir; }
const std::string &EmulatorDir() { return g_emulator_dir; }

bool BuildSystemDisk(int version, bool tools, const std::string &out, std::string &msg) {
    if (version != 1 && version != 2) {
        msg = "MSX-DOS version must be 1 or 2";
        return false;
    }
    msxdisk::DiskImage img = msxdisk::DiskImage::CreateBlank();
    std::string from;
    if (!LoadSystem(img, version, tools, from, msg)) return false;
    msxdisk::DiskError de = msxdisk::DiskError::None;
    std::error_code ec;
    if (fs::path(out).has_parent_path()) fs::create_directories(fs::path(out).parent_path(), ec);
    if (!img.Save(out, &de)) {
        msg = "cannot write " + out;
        return false;
    }
    int64_t free_bytes = 0;
    float pct = 0;
    img.CapacityStats(&free_bytes, &pct);
    msg = out + ": MSX-DOS " + std::to_string(version) + " system disk from " + from + ", " + std::to_string(free_bytes / 1024) + " KiB free";
    return true;
}

BuildResult Build(const BuildOptions &o) {
    BuildResult res;
    const std::string dir = fs::path(o.obifile).parent_path().string();
    // Cada mensagem vai para o log (texto) e, com arquivo e linha, para `msgs` (o editor leva o cursor la'). Dois formatos chegam como texto:
    // "Obifile, line N: ..." (a receita) e "arq.asm, line N: ..." (o montador, com o arquivo relativo a pasta do Obifile).
    auto note = [&](const std::string &m) {
        res.log.push_back(m);
        Message msg;
        msg.text = m;
        const size_t at = m.find(", line ");
        if (at != std::string::npos && at > 0) {
            size_t p = at + 7;
            int n = 0;
            while (p < m.size() && std::isdigit(static_cast<unsigned char>(m[p]))) n = n * 10 + (m[p++] - '0');
            if (n > 0 && p < m.size() && m[p] == ':') {
                const std::string name = m.substr(0, at);
                msg.file = name == "Obifile" ? o.obifile : JoinPath(dir, name);
                msg.line = n;
                msg.text = m.substr(p + 1);
                while (!msg.text.empty() && msg.text.front() == ' ') msg.text.erase(msg.text.begin());
            }
        }
        msg.warning = msg.text.find("warning") != std::string::npos || msg.text.rfind("Warning", 0) == 0;
        res.msgs.push_back(msg);
    };
    auto fail = [&](const std::string &m) {
        note(m);
        res.ok = false;
        return res;
    };

    std::vector<uint8_t> raw;
    if (!ReadAny(o.reader, o.obifile, raw)) return fail("Cannot read " + o.obifile);
    Recipe rc;
    std::string err;
    if (!ParseRecipe(std::string(raw.begin(), raw.end()), rc, err)) return fail(err);

    const BadigReaderScope reader_scope(o.reader);

    // 1) converte cada arquivo da receita no que vai para o disco
    std::vector<Staged> staged;
    auto add = [&](Staged s) -> bool {
        for (const Staged &p : staged) {
            if (p.name == s.name) {
                note("Obifile: two files would be '" + s.name + "' on the disk (" + p.from + " and " + s.from + "); use 'name:' on one of them");
                return false;
            }
        }
        staged.push_back(std::move(s));
        return true;
    };
    bool bad = false;
    for (const FileSpec &f : rc.files) {
        const std::string path = JoinPath(dir, f.source);
        std::vector<uint8_t> data;
        if (!ReadAny(o.reader, path, data)) {
            note("Obifile, line " + std::to_string(f.line) + ": cannot read " + f.source);
            bad = true;
            continue;
        }
        const int kind = obi_kind(f.source.c_str());
        Staged s;
        s.from = f.source;
        if (kind == OBI_KIND_DMX) {
            const std::string text(data.begin(), data.end());
            const badig::Options base = badig::LoadDefaults();
            const badig::Result r = badig::Convert(base, {path}, &text, false);
            for (const badig::Diag &d : r.diags) {
                res.log.push_back(d.formatted.empty() ? d.file + ", line " + std::to_string(d.line) + ": " + d.text : d.formatted);
                Message m;
                m.file = d.file;
                m.line = d.line;
                m.warning = d.level == 2;
                m.text = d.text;
                res.msgs.push_back(m);
            }
            if (!r.ok || r.bmx.empty()) {
                note(f.source + ": the Basic Dignified conversion failed");
                bad = true;
                continue;
            }
            s.data = r.bmx;
            if (!DiskName(f, "BMX", s.name, err)) {
                note(err);
                bad = true;
                continue;
            }
            // os arquivos de apoio do proprio fonte (disk_files, NBASIC.BIN) vao junto, como no F5
            std::vector<badig::DiskFile> extra;
            std::string cerr;
            if (!badig::CollectDiskFiles(r, extra, cerr)) {
                note(f.source + ": " + cerr);
                bad = true;
            } else {
                for (const badig::DiskFile &x : extra) {
                    bool have = false;
                    for (const FileSpec &g : rc.files) have = have || Upper(g.name) == x.msx;
                    for (const Staged &p : staged) have = have || p.name == x.msx;
                    if (have) continue;
                    Staged a;
                    a.name = x.msx;
                    a.from = f.source + " (disk_files)";
                    if (!ReadAny(o.reader, x.host, a.data)) {
                        note(f.source + ": cannot read the support file " + x.host);
                        bad = true;
                        continue;
                    }
                    staged.push_back(std::move(a));
                }
            }
        } else if (kind == OBI_KIND_ASM) {
            fwasm::AsmOptions ao;
            AsmFiles files(fs::path(path).parent_path().string(), o.reader);
            const fwasm::AsmResult r = fwasm::Assemble(f.source, std::string(data.begin(), data.end()), ao, &files);
            for (const std::string &m : r.msgs) note(m);
            if (r.rc != 0) {
                note(f.source + ": assembly failed");
                bad = true;
                continue;
            }
            // o formato do programa decide o arquivo: .COM (MSX-DOS), .BIN (BLOAD) ou .ROM; so' os bytes crus (sem diretiva) vao como .BIN
            std::string ext, key;
            for (const char *k : {"com", "bin", "rom"}) {
                if (r.outs.count(k)) {
                    key = k;
                    ext = Upper(k);
                    break;
                }
            }
            if (key.empty() && r.outs.count("z80")) {
                key = "z80";
                ext = "BIN";
                note(f.source + ": warning: no output format directive (.BIN, .ROM, .MSXDOS...): the raw bytes go to the disk as .BIN without the BLOAD header");
            }
            if (key.empty()) {
                note(f.source + ": the assembler produced no program");
                bad = true;
                continue;
            }
            s.data = r.outs.at(key);
            if (!DiskName(f, ext, s.name, err)) {
                note(err);
                bad = true;
                continue;
            }
        } else {
            s.data = std::move(data);
            if (!DiskName(f, "", s.name, err)) {
                note(err);
                bad = true;
                continue;
            }
        }
        if (!add(std::move(s))) bad = true;
    }
    if (bad) return fail("obi: the build stopped (see above)");

    // 2) quem roda sozinho
    std::string boot = Upper(rc.boot);
    if (boot.empty()) {
        for (const Staged &s : staged) {
            if (s.name.size() > 4 && s.name.compare(s.name.size() - 4, 4, ".BMX") == 0) {
                boot = s.name;
                break;
            }
        }
    }
    std::string autoexec_bas, autoexec_bat;
    if (!boot.empty()) {
        const bool listed = std::any_of(staged.begin(), staged.end(), [&](const Staged &s) { return s.name == boot; });
        if (!listed) return fail("Obifile: 'boot: " + rc.boot + "' is not one of the files on the disk");
        const std::string ext = boot.find('.') == std::string::npos ? "" : boot.substr(boot.find('.') + 1);
        if (ext == "BMX" || ext == "BAS") autoexec_bas = "10 RUN \"" + boot + "\"\r\n";
        else if (ext == "BIN") autoexec_bas = "10 BLOAD \"" + boot + "\",R\r\n";
        else if (ext == "COM") {
            if (!rc.dos) return fail("Obifile: a .COM runs under MSX-DOS: add 'dos: 1' or 'dos: 2' or boot a .BMX / .BIN");
            autoexec_bat = boot.substr(0, boot.find('.')) + "\r\n";
        } else {
            return fail("Obifile: do not know how to start a ." + ext + " (use .BMX, .BAS, .BIN or .COM)");
        }
        if (rc.dos && !autoexec_bas.empty()) autoexec_bat = "BASIC\r\n"; // o MSX-DOS 1 cai no COMMAND.COM; o BASIC roda o AUTOEXEC.BAS
        for (const Staged &s : staged) {
            if ((!autoexec_bas.empty() && s.name == "AUTOEXEC.BAS") || (!autoexec_bat.empty() && s.name == "AUTOEXEC.BAT"))
                return fail("Obifile: " + s.name + " is generated from 'boot:'; remove it from the list or drop 'boot:'");
        }
    }
    if (!autoexec_bas.empty()) staged.push_back({"AUTOEXEC.BAS", std::vector<uint8_t>(autoexec_bas.begin(), autoexec_bas.end()), "boot", {}});
    if (!autoexec_bat.empty()) staged.push_back({"AUTOEXEC.BAT", std::vector<uint8_t>(autoexec_bat.begin(), autoexec_bat.end()), "boot", {}});

    // 3) o disco: em branco ou o de sistema do MSX-DOS
    msxdisk::DiskError derr = msxdisk::DiskError::None;
    msxdisk::DiskImage img = msxdisk::DiskImage::CreateBlank();
    if (rc.dos_version > 0) {
        std::string from;
        if (!LoadSystem(img, rc.dos_version, rc.dos_tools, from, err)) return fail("Obifile: " + err);
        res.log.push_back("MSX-DOS " + std::to_string(rc.dos_version) + " system files: " + from);
    } else if (rc.dos) {
        std::string sys;
        if (!rc.dos_disk.empty()) {
            sys = JoinPath(dir, rc.dos_disk);
        } else {
            for (const char *c : {"msxdos1.dsk", "dos/msxdos1.dsk", "disks/msxdos1.dsk"}) {
                for (const std::string &base : {dir, std::string()}) {
                    const std::string cand = base.empty() ? std::string(c) : JoinPath(base, c);
                    std::error_code ec;
                    if (fs::exists(cand, ec)) {
                        sys = cand;
                        break;
                    }
                }
                if (!sys.empty()) break;
            }
        }
        if (sys.empty()) return fail("Obifile: 'dos: true' needs a MSX-DOS system disk: set 'dos_disk: msxdos1.dsk' (or use 'dos: 1' / 'dos: 2')");
        std::optional<msxdisk::DiskImage> loaded = msxdisk::DiskImage::Load(sys, &derr);
        if (!loaded) return fail("Obifile: cannot open the system disk " + sys);
        img = std::move(*loaded);
        res.log.push_back("system disk: " + sys);
    }

    // 4) cabe? (clusters inteiros por arquivo)
    int64_t free_bytes = 0;
    float free_pct = 0;
    img.CapacityStats(&free_bytes, &free_pct);
    std::vector<int> sizes;
    for (const Staged &s : staged) sizes.push_back(static_cast<int>(s.data.size()));
    const int cluster = static_cast<int>(msxdisk::Geometry::Disk720KB().BytesPerCluster());
    const int need = obi_clusters_needed(sizes.data(), static_cast<int>(sizes.size()), cluster);
    if (static_cast<int64_t>(need) * cluster > free_bytes)
        return fail("obi: the files need " + std::to_string((static_cast<int64_t>(need) * cluster + 1023) / 1024) + " KiB and the disk has " + std::to_string(free_bytes / 1024) + " KiB free");

    // 5) grava cada arquivo (o DiskImage le de um arquivo do computador: um temporario)
    TempCleanup tmp{TempDir()};
    int n = 0;
    for (const Staged &s : staged) {
        const std::string host = (fs::path(tmp.dir) / ("f" + std::to_string(n++) + ".tmp")).string();
        {
            std::ofstream f(host, std::ios::binary | std::ios::trunc);
            f.write(reinterpret_cast<const char *>(s.data.data()), static_cast<std::streamsize>(s.data.size()));
            if (!f) return fail("obi: cannot write the temporary file " + host);
        }
        img.DeleteFile(s.name); // o disco de sistema pode ja' ter um com o mesmo nome: a receita manda
        if (!img.AddFile(host, s.name, 0x20, &derr)) return fail("obi: cannot put " + s.name + " on the disk (" + DiskErr(derr) + ")");
        BuiltFile b;
        b.name = s.name;
        b.size = s.data.size();
        b.crc = obi_crc32(s.data.data(), static_cast<int>(s.data.size()));
        b.from = s.from;
        res.files.push_back(b);
    }
    img.CapacityStats(&free_bytes, &free_pct);
    res.free_bytes = static_cast<long>(free_bytes);

    res.dsk = JoinPath(dir, rc.target);
    res.dos_version = rc.dos_version;
    if (o.write) {
        std::error_code ec;
        const fs::path parent = fs::path(res.dsk).parent_path();
        if (!parent.empty()) fs::create_directories(parent, ec);
        if (!img.Save(res.dsk, &derr)) return fail("obi: cannot write " + res.dsk);
    }
    res.ok = true;
    return res;
}

} // namespace obi
