// fwMSX -- teste do obi (receita Obifile -> disquete .dsk). Ver doc/obi-spec.md.
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>

#include "../../src/msxdisk/core/disk_image.h"
#include "../../src/obi/asm/obi_crc.h"
#include "../../src/obi/c/obi_name.h"
#include "../../src/obi/cpp/obi.h"
#include "../../src/obi/fortran/obi_fit.h"

namespace fs = std::filesystem;

static int g_failures = 0;

static void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

static void spit(const fs::path &p, const std::string &d) {
    fs::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    f.write(d.data(), static_cast<std::streamsize>(d.size()));
}

static std::string slurp(const fs::path &p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

static bool has(const std::vector<std::string> &log, const std::string &needle) {
    for (const std::string &l : log)
        if (l.find(needle) != std::string::npos) return true;
    return false;
}

// O conteudo de `name` dentro de um .dsk ("" se nao existe).
static bool disk_file(const fs::path &dsk, const std::string &name, std::string &out) {
    msxdisk::DiskError e;
    std::optional<msxdisk::DiskImage> img = msxdisk::DiskImage::Load(dsk, &e);
    if (!img) return false;
    const fs::path tmp = fs::temp_directory_path() / "obi_test_extract.bin";
    if (!img->ExtractFile(name, tmp, &e)) return false;
    out = slurp(tmp);
    fs::remove(tmp);
    return true;
}

static obi::BuildResult build(const fs::path &dir, const std::string &recipe, obi::Reader reader = nullptr, bool write = true) {
    spit(dir / "Obifile", recipe);
    obi::BuildOptions o;
    o.obifile = (dir / "Obifile").string();
    o.reader = std::move(reader);
    o.write = write;
    return obi::Build(o);
}

int main() {
    // ---- C: nomes 8.3 e tipos ----
    char n[16];
    check(obi_msx_name("jogo.bmx", n, sizeof n) == OBI_NAME_OK && std::strcmp(n, "JOGO.BMX") == 0, "obi_msx_name: maiusculas");
    check(obi_msx_name("AUTOEXEC", n, sizeof n) == OBI_NAME_OK && std::strcmp(n, "AUTOEXEC") == 0, "obi_msx_name: sem extensao");
    check(obi_msx_name("muitolongo.bin", n, sizeof n) == OBI_NAME_BASE_LONG, "obi_msx_name: mais de 8 antes do ponto");
    check(obi_msx_name("a.json", n, sizeof n) == OBI_NAME_EXT_LONG, "obi_msx_name: mais de 3 depois do ponto");
    check(obi_msx_name("a b.dat", n, sizeof n) == OBI_NAME_BAD_CHAR && obi_msx_name("a.", n, sizeof n) == OBI_NAME_BAD_CHAR && obi_msx_name("a.b.c", n, sizeof n) == OBI_NAME_BAD_CHAR,
          "obi_msx_name: espaco, ponto final e dois pontos recusados");
    check(obi_msx_name("", n, sizeof n) == OBI_NAME_EMPTY && obi_msx_name(".dat", n, sizeof n) != OBI_NAME_OK, "obi_msx_name: vazio e so' extensao");
    check(obi_kind("a/b/x.DMX") == OBI_KIND_DMX && obi_kind("x.asm") == OBI_KIND_ASM && obi_kind("x.spr") == OBI_KIND_RAW && obi_kind("semext") == OBI_KIND_RAW, "obi_kind");

    // ---- Assembly: CRC-32 (valor de referencia do zip/PNG) ----
    const char *nine = "123456789";
    check(obi_crc32(reinterpret_cast<const unsigned char *>(nine), 9) == 0xCBF43926u && obi_crc32(nullptr, 0) == 0u, "obi_crc32 de '123456789' = CBF43926");

    // ---- Fortran: clusters ----
    const int sz[] = {0, 1, 1024, 1025};
    check(obi_clusters_needed(sz, 4, 1024) == 0 + 1 + 1 + 2 && obi_clusters_needed(sz, 0, 1024) == 0 && obi_clusters_needed(sz, 4, 0) == -1, "obi_clusters_needed arredonda por arquivo");

    // ---- parser ----
    obi::Recipe r;
    std::string err;
    check(obi::ParseRecipe("# receita\ntarget: jogo.dsk\nboot: JOGO.BMX\ndos: false\nfiles:\n  - source: jogo.dmx\n  - source: spr.spr\n    name: SPR.DAT\n  - data.bin\n", r, err) && r.target == "jogo.dsk" &&
              r.boot == "JOGO.BMX" && !r.dos && r.files.size() == 3 && r.files[1].name == "SPR.DAT" && r.files[2].source == "data.bin",
          "ParseRecipe: chaves, lista com campo extra e forma curta");
    check(!obi::ParseRecipe("files:\n  - x.dat\n", r, err) && err.find("target") != std::string::npos, "ParseRecipe: sem target");
    check(!obi::ParseRecipe("target: a.dsk\nfiles:\n  - x.dat\nfoo: 1\n", r, err) && err.find("line 4") != std::string::npos && err.find("foo") != std::string::npos, "ParseRecipe: chave desconhecida com a linha");
    check(!obi::ParseRecipe("target: a.dsk\n", r, err) && err.find("no files") != std::string::npos, "ParseRecipe: sem arquivos");
    check(!obi::ParseRecipe("target: a.dsk\nfiles:\n  - source: x\n    cor: 1\n", r, err) && err.find("cor") != std::string::npos, "ParseRecipe: campo de arquivo desconhecido");

    // ---- a build ----
    const fs::path dir = fs::temp_directory_path() / "fwmsx_obi_test";
    fs::remove_all(dir);
    fs::create_directories(dir / "src");
    spit(dir / "src/jogo.dmx", "print \"ola\"\n");
    spit(dir / "src/rotina.asm", ".bios\n.org 0d000h\n.basic\n ret\n");
    spit(dir / "src/dados.dat", std::string("\x01\x02\x03", 3));
    spit(dir / "src/hero.spr", "SPRITES");
    spit(dir / "src/tela.scr", "SCREEN");

    obi::BuildResult b = build(dir, "target: out/jogo.dsk\nfiles:\n  - src/jogo.dmx\n  - src/rotina.asm\n  - src/dados.dat\n  - source: src/hero.spr\n    name: HERO.SPR\n  - src/tela.scr\n");
    check(b.ok && fs::exists(dir / "out/jogo.dsk"), "build: grava o .dsk (e cria a pasta out/)");
    std::map<std::string, size_t> sizes;
    for (const obi::BuiltFile &f : b.files) sizes[f.name] = f.size;
    check(sizes.count("JOGO.BMX") && sizes.count("ROTINA.BIN") && sizes.count("DADOS.DAT") && sizes.count("HERO.SPR") && sizes.count("TELA.SCR") && sizes.count("AUTOEXEC.BAS"),
          ".dmx vira .BMX, .asm vira .BIN, o resto vai como esta', e o AUTOEXEC.BAS e' gerado");
    std::string got;
    check(disk_file(dir / "out/jogo.dsk", "AUTOEXEC.BAS", got) && got == "10 RUN \"JOGO.BMX\"\r\n", "o 1o .BMX roda sozinho (AUTOEXEC.BAS)");
    check(disk_file(dir / "out/jogo.dsk", "DADOS.DAT", got) && got == std::string("\x01\x02\x03", 3), "o .dat esta' no disco byte a byte");
    check(disk_file(dir / "out/jogo.dsk", "HERO.SPR", got) && got == "SPRITES", "name: renomeia o arquivo no disco");
    check(disk_file(dir / "out/jogo.dsk", "JOGO.BMX", got) && got.size() > 4 && static_cast<unsigned char>(got[0]) == 0xFF, "o .BMX e' o BASIC tokenizado (comeca com FF)");
    check(b.free_bytes > 600 * 1024, "sobra espaco no disco de 720K");

    // boot por .BIN e boot explicito
    b = build(dir, "target: b.dsk\nboot: ROTINA.BIN\nfiles:\n  - src/rotina.asm\n  - src/jogo.dmx\n");
    check(b.ok && disk_file(dir / "b.dsk", "AUTOEXEC.BAS", got) && got == "10 BLOAD \"ROTINA.BIN\",R\r\n", "boot: .BIN vira BLOAD ...,R");
    b = build(dir, "target: b.dsk\nfiles:\n  - src/dados.dat\n");
    check(b.ok && !disk_file(dir / "b.dsk", "AUTOEXEC.BAS", got), "sem .BMX e sem boot: nao ha' AUTOEXEC.BAS");

    // erros
    b = build(dir, "target: x.dsk\nfiles:\n  - src/naoexiste.dat\n");
    check(!b.ok && has(b.log, "cannot read src/naoexiste.dat") && has(b.log, "line 3"), "arquivo que nao existe: erro com a linha da receita");
    b = build(dir, "target: x.dsk\nfiles:\n  - src/dados.dat\n  - source: src/tela.scr\n    name: DADOS.DAT\n");
    check(!b.ok && has(b.log, "two files would be 'DADOS.DAT'"), "dois arquivos com o mesmo nome no disco");
    spit(dir / "src/nome_muito_longo.dat", "x");
    b = build(dir, "target: x.dsk\nfiles:\n  - src/nome_muito_longo.dat\n");
    check(!b.ok && has(b.log, "not a valid MSX-DOS file name"), "nome que nao e' 8.3 pede name:");
    b = build(dir, "target: x.dsk\nboot: NADA.BMX\nfiles:\n  - src/dados.dat\n");
    check(!b.ok && has(b.log, "not one of the files"), "boot: de um arquivo que nao esta' na lista");
    b = build(dir, "target: x.dsk\nboot: DADOS.DAT\nfiles:\n  - src/dados.dat\n");
    check(!b.ok && has(b.log, "do not know how to start"), "boot: de um tipo que nao roda");
    spit(dir / "src/prog.asm", ".bios\n.msxdos\n.org 0100h\n ret\n");
    b = build(dir, "target: x.dsk\nboot: PROG.COM\nfiles:\n  - src/prog.asm\n");
    check(!b.ok && has(b.log, "dos: true"), ".COM sem dos: true explica o que falta");
    b = build(dir, "target: x.dsk\ndos: true\ndos_disk: nao_existe.dsk\nfiles:\n  - src/prog.asm\n");
    check(!b.ok && has(b.log, "cannot open the system disk"), "dos_disk que nao abre");
    spit(dir / "src/ruim.dmx", "goto {nowhere}\n");
    b = build(dir, "target: x.dsk\nfiles:\n  - src/ruim.dmx\n");
    check(!b.ok && has(b.log, "nowhere") && has(b.log, "conversion failed") && !fs::exists(dir / "x.dsk"), "erro no .dmx: mensagem do Badig e nada gravado");
    spit(dir / "src/ruim.asm", ".bios\n.org 0d000h\n ld a,\n");
    b = build(dir, "target: x.dsk\nfiles:\n  - src/ruim.asm\n");
    check(!b.ok && has(b.log, "ruim.asm") && has(b.log, "assembly failed"), "erro no .asm: mensagem do montador");
    spit(dir / "src/grande.dat", std::string(800 * 1024, 'g'));
    b = build(dir, "target: x.dsk\nfiles:\n  - src/grande.dat\n");
    check(!b.ok && has(b.log, "need") && has(b.log, "KiB free"), "nao cabe no disco: diz quanto falta");

    // so' conferir (check) nao grava
    fs::remove(dir / "c.dsk");
    b = build(dir, "target: c.dsk\nfiles:\n  - src/dados.dat\n", nullptr, false);
    check(b.ok && !fs::exists(dir / "c.dsk"), "write=false confere sem gravar");

    // o leitor (o projeto): o fonte vem dele, nao da pasta
    std::map<std::string, std::string> proj;
    proj[(dir / "src/viro.dat").lexically_normal().string()] = "DO PROJETO";
    obi::Reader rd = [&](const std::string &path, std::vector<uint8_t> &data) {
        const auto it = proj.find(fs::path(path).lexically_normal().string());
        if (it == proj.end()) return false;
        data.assign(it->second.begin(), it->second.end());
        return true;
    };
    b = build(dir, "target: d.dsk\nfiles:\n  - src/viro.dat\n", rd);
    check(b.ok && disk_file(dir / "d.dsk", "VIRO.DAT", got) && got == "DO PROJETO", "o leitor do projeto serve arquivos que nao estao na pasta");

    // include do .dmx e do .asm vindos do leitor
    proj[(dir / "src/lib.dmx").lexically_normal().string()] = "print \"da lib\"\n";
    proj[(dir / "src/p2.dmx").lexically_normal().string()] = "include \"lib.dmx\"\nprint 2\n";
    b = build(dir, "target: e.dsk\nfiles:\n  - src/p2.dmx\n", rd);
    check(b.ok, "include do Dignified lido pelo leitor do projeto");

    // CLI
    const int rc_bad = obi::RunObiCommand({"build", (dir / "naoexiste" / "Obifile").string()});
    check(rc_bad != 0, "RunObiCommand: Obifile que nao existe falha");
    spit(dir / "ok/Obifile", "target: ok.dsk\nfiles:\n  - a.dat\n");
    spit(dir / "ok/a.dat", "A");
    check(obi::RunObiCommand({"build", (dir / "ok/Obifile").string()}) == 0 && fs::exists(dir / "ok/ok.dsk"), "RunObiCommand build");
    check(fs::path(obi::FindObifile((dir / "ok").string())).lexically_normal() == (dir / "ok" / "Obifile").lexically_normal(), "FindObifile acha o Obifile da pasta");

    fs::remove_all(dir);
    std::printf("\n%s (%d falha%s)\n", g_failures == 0 ? "TODOS OS TESTES PASSARAM" : "HA' FALHAS", g_failures, g_failures == 1 ? "" : "s");
    return g_failures == 0 ? 0 : 1;
}
