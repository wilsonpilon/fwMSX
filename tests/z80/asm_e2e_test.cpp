// Teste de ponta a ponta do fwasm: monta programas de verdade (ROM, os quatro mappers da MegaROM e a fita .cas) e RODA o resultado
// na maquina emulada, conferindo o texto na tela. Pega o que os casos dourados (que comparam com o asmsx.exe) nao pegam: se os bytes
// gerados funcionam no emulador do fwMSX -- registradores dos mappers, janelas, formato da fita.
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "../../src/asm/cpp/assembler.h"
#include "../../src/control/command.h"
#include "../../src/machine/machine.h"
#include "../../src/vdp/core/vdp_state.h"

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

std::string TempPath(const std::string &name) {
    const char *t = std::getenv("TEMP");
    return (t ? std::string(t) : std::string("/tmp")) + "/" + name;
}

bool Write(const std::string &path, const std::vector<uint8_t> &data) {
    std::ofstream f(path, std::ios::binary);
    f.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
    return static_cast<bool>(f);
}

bool VramHas(const machine::Machine &m, const std::string &text) {
    const uint8_t *v = m.vdp_state().vram;
    for (int i = 0; i + static_cast<int>(text.size()) <= VDP_VRAM_SIZE; ++i)
        if (std::equal(text.begin(), text.end(), v + i)) return true;
    return false;
}

struct NoFiles : fwasm::FileProvider {
    bool Read(const std::string &, std::vector<uint8_t> &) override { return false; }
};

fwasm::AsmResult Assemble(const std::string &src) {
    NoFiles nf;
    fwasm::AsmOptions opts;
    return fwasm::Assemble("x.asm", src, opts, &nf);
}

// Um programa que escreve `text` na tela com CHPUT e fica parado.
std::string PrintLoop(const std::string &text) {
    return "lp: ld a,[hl]\n or a\n jr z,fim\n call CHPUT\n inc hl\n jr lp\nfim: jr fim\nmsg: db \"" + text + "\",0\n";
}

void TestCartridges(const std::string &bios) {
    // ROM simples de 16 KB, sem .search (so' a pagina 1)
    {
        const fwasm::AsmResult r = Assemble(".bios\n.page 1\n.start init\n.rom\ninit: ld hl,msg\n" + PrintLoop("ROM do fwasm"));
        check(r.rc == 0 && r.outs.count("rom") && r.outs.at("rom").size() == 8192, "ROM: montou (8192 bytes)");
        const std::string path = TempPath("fwasm_e2e.rom");
        check(Write(path, r.outs.at("rom")), "ROM: gravou o arquivo");
        machine::MachineConfig cfg;
        cfg.bios_path = bios;
        cfg.cart_path = path;
        std::string error;
        std::unique_ptr<machine::Machine> m = machine::Machine::Create(cfg, error);
        check(m != nullptr, "ROM: maquina criada " + error);
        if (m) {
            for (int i = 0; i < 250; ++i) m->RunFrame();
            check(VramHas(*m, "ROM do fwasm"), "ROM: o cartucho montado pelo fwasm escreveu o texto na tela");
        }
        std::remove(path.c_str());
    }
    // MegaROM: .search liga a pagina 2 ao cartucho, .select troca a subpagina 1 na janela de 8000h (o registrador depende do mapper)
    struct Case {
        const char *directive;
        MemMapMapperType mapper;
    } cases[] = {{"konami", MEMMAP_MAPPER_KONAMI4}, {"konamiscc", MEMMAP_MAPPER_KONAMI5}, {"ascii8", MEMMAP_MAPPER_ASCII8}, {"ascii16", MEMMAP_MAPPER_ASCII16}};
    for (const Case &c : cases) {
        const std::string text = std::string("mega ") + c.directive + " ok";
        const std::string src = std::string(".bios\n.megarom ") + c.directive + "\n.start init\ninit: .search\n .select 1 at 8000h\n jp sub1\n.subpage 1 at 8000h\nsub1: ld hl,msg\n" +
                                PrintLoop(text);
        const fwasm::AsmResult r = Assemble(src);
        check(r.rc == 0 && r.outs.count("rom") && r.outs.at("rom").size() >= 16384, std::string("MegaROM ") + c.directive + ": montou");
        if (r.rc != 0 || !r.outs.count("rom")) continue;
        const std::string path = TempPath("fwasm_e2e_mega.rom");
        Write(path, r.outs.at("rom"));
        machine::MachineConfig cfg;
        cfg.bios_path = bios;
        cfg.cart_path = path;
        cfg.cart_mapper = c.mapper;
        std::string error;
        std::unique_ptr<machine::Machine> m = machine::Machine::Create(cfg, error);
        check(m != nullptr, std::string("MegaROM ") + c.directive + ": maquina criada " + error);
        if (m) {
            for (int i = 0; i < 250; ++i) m->RunFrame();
            check(VramHas(*m, text), std::string("MegaROM ") + c.directive + ": a troca de subpagina do fwasm funciona no emulador (\"" + text + "\")");
        }
        std::remove(path.c_str());
    }
}

void TestTape(const std::string &bios) {
    // programa do BASIC (.bin) e a fita .cas: BLOAD"CAS:",R carrega e executa
    const fwasm::AsmResult r = Assemble(".bios\n.org 0d000h\n.basic\n.cas \"HELLO\"\n ld hl,msg\nlp: ld a,[hl]\n or a\n ret z\n call CHPUT\n inc hl\n jr lp\nmsg: db \"carregado da fita\",0\n");
    check(r.rc == 0 && r.outs.count("cas") && r.outs.count("bin") && r.outs.count("wav") == 0, "fita: montou o .bin e o .cas");
    if (r.rc != 0 || !r.outs.count("cas")) return;
    const std::string path = TempPath("fwasm_e2e.cas");
    Write(path, r.outs.at("cas"));
    machine::MachineConfig cfg;
    cfg.bios_path = bios;
    cfg.tape_path = path;
    std::string error;
    std::unique_ptr<machine::Machine> m = machine::Machine::Create(cfg, error);
    check(m != nullptr, "fita: maquina criada " + error);
    if (m) {
        for (int i = 0; i < 300; ++i) m->RunFrame();
        std::vector<control::Commander::TypedKey> keys;
        std::string kerr;
        check(control::Commander::ParseTypeSpec(*m, "bload\"cas:\",r{enter}", true, keys, kerr), "fita: digitou BLOAD\"CAS:\",R");
        for (const auto &tk : keys) {
            for (const std::string &k : tk.keys) m->KeyDown(k);
            for (int i = 0; i < 6; ++i) m->RunFrame();
            for (auto it = tk.keys.rbegin(); it != tk.keys.rend(); ++it) m->KeyUp(*it);
            for (int i = 0; i < 6; ++i) m->RunFrame();
        }
        for (int i = 0; i < 900; ++i) m->RunFrame();
        check(VramHas(*m, "Found:HELLO"), "fita: o BASIC achou o arquivo HELLO na fita gerada pelo fwasm");
        check(VramHas(*m, "carregado da fita"), "fita: o programa foi carregado e executado");
    }
    std::remove(path.c_str());
}

unsigned Crc32(const std::vector<uint8_t> &d) {
    unsigned c = 0xFFFFFFFFu;
    for (uint8_t b : d) {
        c ^= b;
        for (int k = 0; k < 8; ++k) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u)));
    }
    return ~c;
}

// Le os arquivos do .include/.incbin do disco, relativos a uma pasta.
struct DiskFiles : fwasm::FileProvider {
    std::string dir;
    explicit DiskFiles(std::string d) : dir(std::move(d)) {}
    bool Read(const std::string &path, std::vector<uint8_t> &data) override {
        std::ifstream f(dir + "/" + path, std::ios::binary);
        if (!f) return false;
        data.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        return true;
    }
};

// Programa real: o replayer de PT3 do SapphiRe (tests/asm/real/pt3) -- ROM + .sym iguais ao asMSX v1.2.0 (CRC do oraculo) e
// a musica AMAZONIA.PT3 realmente toca no PSG da maquina emulada.
void TestPt3Replayer(const std::string &bios, const std::string &src_dir) {
    const std::string dir = src_dir + "/tests/asm/real/pt3";
    std::ifstream sf(dir + "/example.asm", std::ios::binary);
    if (!sf) {
        std::printf("[SKIP] fontes do replayer PT3 nao encontrados\n");
        return;
    }
    const std::string text((std::istreambuf_iterator<char>(sf)), std::istreambuf_iterator<char>());
    DiskFiles files(dir);
    fwasm::AsmOptions opts;
    const fwasm::AsmResult r = fwasm::Assemble("example.asm", text, opts, &files);
    check(r.rc == 0 && r.outs.count("rom") && r.outs.count("sym"), "PT3: montou example.asm + PT3-ROM.ASM + PT3-RAM.ASM");
    if (r.rc != 0 || !r.outs.count("rom") || !r.outs.count("sym")) return;
    check(r.outs.at("rom").size() == 8192 && Crc32(r.outs.at("rom")) == 0x898867E9u, "PT3: o .rom e' igual ao do asMSX v1.2.0");
    check(Crc32(r.outs.at("sym")) == 0x5E6BB47Bu, "PT3: o .sym e' igual ao do asMSX v1.2.0");
    const std::string path = TempPath("fwasm_e2e_pt3.rom");
    Write(path, r.outs.at("rom"));
    machine::MachineConfig cfg;
    cfg.bios_path = bios;
    cfg.cart_path = path;
    std::string error;
    std::unique_ptr<machine::Machine> m = machine::Machine::Create(cfg, error);
    check(m != nullptr, "PT3: maquina criada " + error);
    if (m) {
        std::vector<int> tons;
        bool amp = false;
        for (int i = 0; i < 600; ++i) {
            m->RunFrame();
            const uint8_t *rg = m->psg().state().r;
            const int ton = rg[0] | (rg[1] << 8);
            if (tons.empty() || tons.back() != ton) tons.push_back(ton);
            if ((rg[8] & 15) || (rg[9] & 15) || (rg[10] & 15)) amp = true;
        }
        check(amp, "PT3: o replayer liga o volume de algum canal do PSG");
        check(tons.size() >= 3, "PT3: o tom do canal A muda ao longo da musica (" + std::to_string(tons.size()) + " valores)");
    }
    std::remove(path.c_str());
}

// Programa real: o jogo Amazonia (Renato Degiovani) em versao cartucho de 48 KB, fonte adaptado por Wilson Pilon. Cobre o que o replayer nao cobre: fonte numa
// subpasta incluindo "source/x.asm" relativo ao diretorio ATUAL, .incbin "64k/x" idem, constantes com #hex e .size com MegaROM-like de 48 KB.
void TestAmazonia(const std::string &src_dir) {
    const std::string dir = src_dir + "/tests/asm/real/amazonia";
    std::ifstream sf(dir + "/source/amazonia.asm", std::ios::binary);
    if (!sf) {
        std::printf("[SKIP] fontes do Amazonia nao encontrados\n");
        return;
    }
    const std::string text((std::istreambuf_iterator<char>(sf)), std::istreambuf_iterator<char>());
    DiskFiles files(dir);
    fwasm::AsmOptions opts;
    const fwasm::AsmResult r = fwasm::Assemble("source/amazonia.asm", text, opts, &files);
    check(r.rc == 0 && r.outs.count("rom") && r.outs.count("sym"), "Amazonia: montou source/amazonia.asm (includes e incbin relativos ao diretorio atual, #hex)");
    if (r.rc != 0 || !r.outs.count("rom") || !r.outs.count("sym")) return;
    check(r.outs.at("rom").size() == 49152 && Crc32(r.outs.at("rom")) == 0x0254F631u, "Amazonia: o .rom (48 KB) e' igual ao do asMSX v1.2.0");
    check(Crc32(r.outs.at("sym")) == 0xB036A9F4u, "Amazonia: o .sym e' igual ao do asMSX v1.2.0");
}

} // namespace

int main() {
#ifdef FWMSX_SOURCE_DIR
    const std::string bios = std::string(FWMSX_SOURCE_DIR) + "/resource/fMSX/ROMs/MSX.ROM";
    {
        std::ifstream probe(bios, std::ios::binary);
        if (!probe) {
            std::printf("[SKIP] BIOS nao encontrada em '%s'\n", bios.c_str());
            return 0;
        }
    }
    TestCartridges(bios);
    TestTape(bios);
    TestPt3Replayer(bios, FWMSX_SOURCE_DIR);
    TestAmazonia(FWMSX_SOURCE_DIR);
#endif
    std::printf("\n%s (%d falha(s))\n", g_failures ? "FALHOU" : "OK", g_failures);
    return g_failures ? 1 : 0;
}
