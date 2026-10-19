// fwMSX -- Mamute: os comandos de disco e de notas do SUPER-X (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/mamute-spec.md, "Fase 4".
//
// Disco: o "disco corrente" e' uma imagem .dsk (XDK escolhe; XFS, XCI, XTP, XL% e XS% usam). No Mamute original o primeiro comando de
// disco abria um dialogo de arquivo; aqui nao ha' dialogo, entao sem disco corrente eles pedem o XDK. XSV, XLD, XS# e XL# gravam e leem
// arquivos do computador (nao do disco). Notas: um arquivo de texto "AAAA;slot;tipo;texto" que documenta enderecos da BIOS.
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>

#include "../../msxdisk/core/disk_image.h"
#include "mamute_config.h"
#include "mamute_monitor.h"
#include "mamute_util.h"

namespace mamute {

using util::Hex;
using util::Join;
using util::ParseHexN;
using util::SplitCommas;
using util::Trim;
using util::Upper;

// ---------------------------------------------------------------------------------------------------------------------
// o arquivo de notas
// ---------------------------------------------------------------------------------------------------------------------
std::string DefaultNotesPath() {
    std::string cfg = DefaultConfigPath(); // ".../mamute.cfg"
    const std::string tail = "mamute.cfg";
    if (cfg.size() >= tail.size()) cfg.erase(cfg.size() - tail.size());
    return cfg + "SUPER-X-PT.notas";
}

bool LoadNotes(const std::string &path, std::vector<Note> &out, std::string &error) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        error = "NAO FOI POSSIVEL ABRIR " + path;
        return false;
    }
    std::vector<Note> notes;
    std::string line;
    bool first = true;
    unsigned bad = 0;
    while (std::getline(f, line)) {
        if (first && line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF && static_cast<unsigned char>(line[1]) == 0xBB && static_cast<unsigned char>(line[2]) == 0xBF) line.erase(0, 3);
        first = false;
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
        if (Trim(line).empty()) continue;
        const size_t a = line.find(';');
        const size_t b = a == std::string::npos ? a : line.find(';', a + 1);
        const size_t c = b == std::string::npos ? b : line.find(';', b + 1);
        Note n;
        uint32_t addr = 0;
        int slot = 0, type = 0;
        if (c == std::string::npos || !ParseHexN(line.substr(0, a), addr, 4) || !util::ParseDec(line.substr(a + 1, b - a - 1), slot) || slot > 4 ||
            !util::ParseDec(line.substr(b + 1, c - b - 1), type) || type > 7) {
            ++bad;
            continue;
        }
        n.addr = addr;
        n.slot = slot;
        n.type = type;
        n.text = line.substr(c + 1);
        notes.push_back(n);
    }
    if (notes.empty()) {
        error = "ARQUIVO DE NOTAS INVALIDO (" + std::to_string(bad) + " LINHAS RECUSADAS)";
        return false;
    }
    out = std::move(notes);
    return true;
}

bool SaveNotes(const std::string &path, const std::vector<Note> &notes) {
    std::ofstream o(path, std::ios::trunc);
    if (!o) return false;
    for (const Note &n : notes) o << Hex(n.addr, 4) << ";" << n.slot << ";" << n.type << ";" << n.text << "\n";
    return static_cast<bool>(o);
}

namespace {

std::string Unquote(std::string s) {
    s = Trim(s);
    if (s.size() >= 2 && (s.front() == '"' || s.front() == '\'') && s.back() == s.front()) s = s.substr(1, s.size() - 2);
    return s;
}

bool ReadFileBytes(const std::string &path, std::vector<uint8_t> &data) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    data.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    return true;
}

bool WriteFileBytes(const std::string &path, const std::vector<uint8_t> &data) {
    std::ofstream o(path, std::ios::binary | std::ios::trunc);
    if (!o) return false;
    o.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
    return static_cast<bool>(o);
}

std::string BaseName(const std::string &p) {
    const size_t s = p.find_last_of("/\\");
    return s == std::string::npos ? p : p.substr(s + 1);
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------------
// disco
// ---------------------------------------------------------------------------------------------------------------------
std::string Monitor::NeedDisk(std::string &error) {
    if (sx_.disk.empty()) {
        error = "NENHUM DISCO CORRENTE: escolha um com XDK <arquivo.dsk>";
        return "";
    }
    return sx_.disk;
}

std::string Monitor::CmdXdisk(const std::string &cmd, const std::string &rest) {
    IMemory &mem = Mem();
    std::string err;

    // ---- XDK: o unico que troca o disco corrente ----
    if (cmd == "XDK") {
        const std::string name = Unquote(rest);
        if (name.empty()) return sx_.disk.empty() ? "SEM DISCO CORRENTE (XDK <arquivo.dsk> escolhe)" : "DISCO: " + BaseName(sx_.disk);
        msxdisk::DiskError de = msxdisk::DiskError::None;
        const auto img = msxdisk::DiskImage::Load(name, &de);
        if (!img) return "NAO E' UMA IMAGEM DE DISCO MSX (OU NAO ABRIU): " + name;
        sx_.disk = name;
        return "DISCO: " + BaseName(name) + " (" + std::to_string(img->geometry().ImageSizeBytes() / 1024) + " KB)";
    }

    // ---- os de disco que usam a imagem corrente ----
    if (cmd == "XFS" || cmd == "XCI" || cmd == "XTP") {
        const std::string disk = NeedDisk(err);
        if (disk.empty()) return err;
        msxdisk::DiskError de = msxdisk::DiskError::None;
        const auto img = msxdisk::DiskImage::Load(disk, &de);
        if (!img) return "NAO FOI POSSIVEL LER O DISCO " + disk;
        if (cmd == "XFS") {
            const std::vector<msxdisk::FileEntry> files = img->ListDirectory("");
            std::vector<std::string> lines{"DISCO: " + BaseName(disk)};
            uint64_t bytes = 0;
            for (const msxdisk::FileEntry &e : files) {
                std::string name = e.name;
                while (name.size() < 13) name += ' ';
                const bool dir = (e.attr & msxdisk::kAttrDirectory) != 0;
                std::string size = dir ? "<DIR>" : std::to_string(e.size);
                while (size.size() < 8) size = " " + size;
                lines.push_back(name + size);
                if (!dir) bytes += e.size;
            }
            int64_t free_bytes = 0;
            float pct = 0;
            img->CapacityStats(&free_bytes, &pct);
            lines.push_back(std::to_string(files.size()) + " ENTRADAS, " + std::to_string(bytes) + " BYTES EM ARQUIVOS, " + std::to_string(free_bytes) + " BYTES LIVRES");
            return Join(lines);
        }
        if (cmd == "XCI") {
            int64_t free_bytes = 0;
            float pct = 0;
            img->CapacityStats(&free_bytes, &pct);
            const uint32_t per = img->geometry().BytesPerCluster();
            return BaseName(disk) + ": " + std::to_string(per ? free_bytes / per : 0) + " / " + std::to_string(img->geometry().TotalClusters()) + " CLUSTERS LIVRES";
        }
        // XTP <arquivo>: um arquivo de dentro do disco, como texto
        const std::string name = Unquote(rest);
        if (name.empty()) return "SINTAXE: XTP <arquivo>  (o nome de um arquivo DENTRO do disco corrente: XFS lista)";
        static std::atomic<int> counter{0};
        const std::filesystem::path tmp = std::filesystem::temp_directory_path() / ("mamute_xtp_" + std::to_string(++counter) + ".tmp");
        msxdisk::DiskError xe = msxdisk::DiskError::None;
        if (!img->ExtractFile(name, tmp, &xe)) return "ARQUIVO NAO ENCONTRADO NO DISCO: " + name;
        std::vector<uint8_t> raw;
        ReadFileBytes(tmp.string(), raw);
        std::error_code ec;
        std::filesystem::remove(tmp, ec);
        std::vector<std::string> lines;
        std::string cur;
        for (size_t k = 0; k < raw.size(); ++k) {
            const uint8_t c = raw[k];
            if (c == 0x1A) break; // o fim de texto do CP/M e do MSX-DOS
            if (c == '\r' || c == '\n') {
                if (c == '\r' && k + 1 < raw.size() && raw[k + 1] == '\n') ++k;
                lines.push_back(cur);
                cur.clear();
            } else if (c == '\t') {
                cur += std::string(8 - cur.size() % 8, ' ');
            } else {
                cur += (c >= 0x20 && c < 0x7F) ? static_cast<char>(c) : '.';
            }
        }
        if (!cur.empty() || lines.empty()) lines.push_back(cur);
        if (panels_) return OpenPanel(MakeTextPanel(name, lines));
        constexpr size_t kMax = 200;
        std::vector<std::string> shown(lines.begin(), lines.begin() + static_cast<std::ptrdiff_t>(std::min(lines.size(), kMax)));
        if (lines.size() > kMax) shown.push_back("... (" + std::to_string(lines.size() - kMax) + " LINHAS A MAIS: o painel da TUI rola e busca)");
        return Join(shown);
    }

    if (cmd == "XL%" || cmd == "XS%") {
        const std::string disk = NeedDisk(err);
        if (disk.empty()) return err;
        const std::vector<std::string> f = SplitCommas(rest);
        const std::string usage = cmd + " <setorinic>[,<setorfim>],<end>[#alvo]   (setores em hexa, 512 bytes cada; sem o setorfim, um so')";
        if (f.size() != 2 && f.size() != 3) return usage;
        uint32_t s1 = 0, s2 = 0;
        SxAddr a;
        if (!ParseHexN(f[0], s1, 6) || !ParseSx(mem, sx_.vars, f.back(), a)) return usage;
        s2 = s1;
        if (f.size() == 3 && (!ParseHexN(f[1], s2, 6) || s2 < s1)) return usage;
        std::fstream io(disk, std::ios::in | std::ios::out | std::ios::binary);
        if (!io) return "NAO FOI POSSIVEL ABRIR " + disk;
        io.seekg(0, std::ios::end);
        const uint64_t size = static_cast<uint64_t>(io.tellg());
        if ((static_cast<uint64_t>(s2) + 1) * 512 > size) return "O SETOR " + Hex(s2, 1) + " ESTA' ALEM DO FIM DO DISCO (" + std::to_string(size / 512) + " SETORES)";
        const uint32_t n = (s2 - s1 + 1) * 512;
        if (static_cast<uint64_t>(a.addr) + n - 1 > TargetMax(mem, a.t)) return "OS BYTES NAO CABEM A PARTIR DE " + FormatSx(a) + " (NUNCA DA A VOLTA)";
        if (cmd == "XL%") {
            std::vector<uint8_t> buf(n);
            io.seekg(static_cast<std::streamoff>(static_cast<uint64_t>(s1) * 512));
            io.read(reinterpret_cast<char *>(buf.data()), static_cast<std::streamsize>(n));
            uint32_t ok = 0;
            for (uint32_t k = 0; k < n; ++k)
                if (TWrite(mem, a.t, a.addr + k, buf[k])) ++ok;
            std::string out = "LIDOS " + std::to_string(s2 - s1 + 1) + " SETORES (" + Hex(s1, 1) + "-" + Hex(s2, 1) + ") PARA " + FormatSx(a);
            if (ok < n) out += " - " + std::to_string(n - ok) + " IGNORADOS (NAO E' RAM)";
            return out;
        }
        std::vector<uint8_t> buf(n);
        for (uint32_t k = 0; k < n; ++k) buf[k] = TRead(mem, a.t, a.addr + k);
        io.seekp(static_cast<std::streamoff>(static_cast<uint64_t>(s1) * 512));
        io.write(reinterpret_cast<const char *>(buf.data()), static_cast<std::streamsize>(n));
        if (!io) return "ERRO AO GRAVAR " + disk;
        return "GRAVADOS " + std::to_string(s2 - s1 + 1) + " SETORES (" + Hex(s1, 1) + "-" + Hex(s2, 1) + ") DE " + FormatSx(a) + " NO DISCO";
    }

    // ---- arquivos do computador: XSV / XLD (com o cabecalho do BSAVE) e XS# / XL# (so' os bytes) ----
    const std::vector<std::string> f = SplitCommas(rest);
    if (cmd == "XSV" || cmd == "XS#") {
        const bool bsave = cmd == "XSV";
        const std::string usage = bsave ? "SINTAXE: XSV <arquivo>,<ini>[#slot[-sub]|#S],<fim>[,<exec>[,<desloc>]]   (cabecalho do BSAVE; <desloc> soma so' nos enderecos do cabecalho)"
                                        : "SINTAXE: XS# <arquivo>,<ini>[#alvo],<fim>   (so' os bytes, sem cabecalho)";
        if (f.size() < 3 || f.size() > (bsave ? 5u : 3u) || f[0].empty()) return usage;
        SxAddr s;
        uint32_t e = 0;
        if (!ParseBlock(f[1], f[2], s, e) || e < s.addr) return usage;
        if (bsave && s.t.vram) return "O BSAVE NUNCA SALVA DA VRAM (USE XS#)";
        uint32_t exec = s.addr, off = 0;
        if (bsave && f.size() >= 4 && !f[3].empty() && !ParseHexN(f[3], exec, 4)) return usage;
        if (bsave && f.size() == 5 && !ParseHexN(f[4], off, 4)) return usage;
        std::vector<uint8_t> data;
        if (bsave) {
            data.push_back(0xFE);
            for (const uint32_t v : {s.addr + off, e + off, exec + off}) {
                data.push_back(static_cast<uint8_t>(v & 0xFF));
                data.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
            }
        }
        for (uint32_t a = s.addr; a <= e; ++a) data.push_back(TRead(mem, s.t, a));
        const std::string path = Unquote(f[0]);
        if (!WriteFileBytes(path, data)) return "NAO FOI POSSIVEL GRAVAR " + path;
        std::string out = "SALVO \"" + path + "\" - " + FormatRange(s, e) + " - TAMANHO " + Hex(e - s.addr + 1, 4);
        if (bsave) out += " - EXEC " + Hex((exec + off) & 0xFFFF, 4) + (off ? " (CABECALHO DESLOCADO " + Hex(off, 4) + ")" : "");
        return out;
    }
    if (cmd == "XLD" || cmd == "XL#") {
        const bool bload = cmd == "XLD";
        const std::string usage = bload ? "SINTAXE: XLD <arquivo>[,<desloc>[#slot[-sub]|#S]]   (arquivo com cabecalho do BSAVE; <desloc> ignora o endereco do cabecalho)"
                                        : "SINTAXE: XL# <arquivo>,<ini>[#alvo]   (carrega o arquivo inteiro, sem cabecalho)";
        if (f.empty() || f[0].empty() || f.size() > (bload ? 2u : 2u)) return usage;
        if (!bload && f.size() != 2) return usage;
        std::vector<uint8_t> data;
        const std::string path = Unquote(f[0]);
        if (!ReadFileBytes(path, data)) return "NAO FOI POSSIVEL ABRIR " + path;
        SxAddr dest;
        size_t skip = 0;
        uint32_t hstart = 0, hend = 0, hexec = 0;
        if (bload) {
            if (data.size() < 7 || data[0] != 0xFE) return "ARQUIVO INVALIDO: NAO COMECA COM O CABECALHO DO BSAVE (FE)";
            hstart = data[1] | (data[2] << 8);
            hend = data[3] | (data[4] << 8);
            hexec = data[5] | (data[6] << 8);
            skip = 7;
            dest.addr = hstart;
            if (f.size() == 2 && !f[1].empty()) {
                if (!ParseSx(mem, sx_.vars, f[1], dest) || dest.t.vram) return usage;
            }
        } else if (!ParseSx(mem, sx_.vars, f[1], dest)) {
            return usage;
        }
        size_t n = data.size() - skip;
        if (bload && hend >= hstart) n = std::min<size_t>(n, static_cast<size_t>(hend - hstart + 1));
        if (n == 0) return "O ARQUIVO NAO TEM DADOS";
        if (static_cast<uint64_t>(dest.addr) + n - 1 > TargetMax(mem, dest.t)) return "O ARQUIVO NAO CABE A PARTIR DE " + FormatSx(dest) + " (NUNCA DA A VOLTA)";
        uint32_t ok = 0;
        for (size_t k = 0; k < n; ++k)
            if (TWrite(mem, dest.t, dest.addr + static_cast<uint32_t>(k), data[skip + k])) ++ok;
        const uint32_t last = dest.addr + static_cast<uint32_t>(n) - 1;
        // as variaveis @B/@E/@S guardam o inicio, o fim e (BLOAD) o endereco de execucao ou (cru) o tamanho
        SxAddr vb = dest, ve = dest, vs = dest;
        ve.addr = last;
        vs.addr = bload ? static_cast<uint32_t>((static_cast<int64_t>(hexec) - hstart + dest.addr) & 0xFFFF) : static_cast<uint32_t>(n & 0xFFFF);
        for (const auto &kv : {std::make_pair('B', vb), std::make_pair('E', ve), std::make_pair('S', vs)}) {
            Vars::Slot *slot = sx_.vars.Find(kv.first);
            slot->has = true;
            slot->v = kv.second;
        }
        std::string out = "CARREGADO \"" + path + "\" EM " + FormatSx(dest) + " - TAMANHO " + Hex(static_cast<unsigned>(n), 4) + " - FIM " + Hex(last, TargetDigits(dest.t));
        if (bload) out += " - EXEC " + Hex(vs.addr, 4) + " (O XLD NAO EXECUTA; use XGO)";
        if (ok < n) out += " - " + std::to_string(n - ok) + " IGNORADOS (NAO E' RAM)";
        return out;
    }
    return "COMANDO DE DISCO DESCONHECIDO: " + cmd;
}

// ---------------------------------------------------------------------------------------------------------------------
// notas
// ---------------------------------------------------------------------------------------------------------------------
std::string Monitor::CmdXnotes(const std::string &cmd, const std::string &rest) {
    if (cmd == "XIM") { // XIM <end>,<slot 0-4>,<tipo 0-7>,<texto>  (o texto e' tudo depois da terceira virgula)
        const size_t a = rest.find(',');
        const size_t b = a == std::string::npos ? a : rest.find(',', a + 1);
        const size_t c = b == std::string::npos ? b : rest.find(',', b + 1);
        Note n;
        uint32_t addr = 0;
        int slot = 0, type = 0;
        const char *usage = "SINTAXE: XIM <end>,<slot 0-4>,<tipo 0-7>,<texto>   (slot: GERAL MAIN SUB FDC RAM; tipo: GERAL BIOS WORK DATA PORT MATH KEY HOOK)";
        if (c == std::string::npos || !ParseHexN(rest.substr(0, a), addr, 4) || !util::ParseDec(rest.substr(a + 1, b - a - 1), slot) || slot > 4 ||
            !util::ParseDec(rest.substr(b + 1, c - b - 1), type) || type > 7 || Trim(rest.substr(c + 1)).empty())
            return usage;
        n.addr = addr;
        n.slot = slot;
        n.type = type;
        n.text = Trim(rest.substr(c + 1)).substr(0, 60);
        sx_.notes.push_back(n);
        return "NOTA ADICIONADA EM " + Hex(addr, 4) + " (" + std::to_string(sx_.notes.size()) + " NA MEMORIA)";
    }
    if (cmd == "XIC") {
        uint32_t addr = 0;
        if (!ParseHexN(rest, addr, 4)) return "SINTAXE: XIC <end>";
        std::vector<std::string> lines;
        for (const Note &n : sx_.notes)
            if (n.addr == addr) lines.push_back(FormatNote(n)); // pode haver mais de uma nota no mesmo endereco
        if (lines.empty()) return sx_.notes.empty() ? "NOTA NAO ENCONTRADA (NENHUMA NOTA NA MEMORIA: XIL CARREGA UM ARQUIVO)" : "NOTA NAO ENCONTRADA";
        return Join(lines);
    }
    if (cmd == "XIL") {
        const std::string path = Trim(rest).empty() ? DefaultNotesPath() : Unquote(rest);
        std::string err;
        std::vector<Note> notes;
        if (!LoadNotes(path, notes, err)) return err;
        sx_.notes = std::move(notes); // substitui TODAS as notas em memoria
        return "CARREGADO: " + BaseName(path) + " (" + std::to_string(sx_.notes.size()) + " NOTAS)";
    }
    if (cmd == "XIS") {
        const std::string path = Trim(rest).empty() ? std::string("mamute.notas") : Unquote(rest);
        if (!SaveNotes(path, sx_.notes)) return "NAO FOI POSSIVEL GRAVAR " + path;
        return "GRAVADO: " + path + " (" + std::to_string(sx_.notes.size()) + " NOTAS)";
    }
    // XIR [<end>] -- o visualizador. Na TUI abre o painel (com busca); no console lista as notas (XIR <texto> filtra).
    if (panels_) {
        int first = 0;
        uint32_t addr = 0;
        if (ParseHexN(rest, addr, 4)) {
            for (size_t k = 0; k < sx_.notes.size(); ++k)
                if (sx_.notes[k].addr == addr) {
                    first = static_cast<int>(k);
                    break;
                }
        }
        return OpenPanel(MakeNotesPanel(&sx_.notes, first));
    }
    if (sx_.notes.empty()) return "NENHUMA NOTA NA MEMORIA (XIL carrega um arquivo, XIM adiciona uma)";
    std::string needle = Upper(Trim(rest));
    std::vector<std::string> lines;
    size_t hits = 0;
    for (const Note &n : sx_.notes) {
        const std::string line = FormatNote(n);
        if (!needle.empty() && Upper(line).find(needle) == std::string::npos) continue;
        ++hits;
        if (lines.size() < 40) lines.push_back(line);
    }
    if (hits > lines.size()) lines.push_back("... (" + std::to_string(hits - lines.size()) + " NOTAS A MAIS; XIR <texto> filtra, XIC <end> consulta, o painel da TUI navega e busca)");
    if (!hits) return "NAO ENCONTRADO";
    return Join(lines);
}

} // namespace mamute
