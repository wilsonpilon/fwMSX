// Teste do MSX2: mapper de RAM (portas FCh-FFh), relogio RTC (B4h/B5h) e a
// maquina completa com a BIOS MSX2 real -- ver doc/msx2-spec.md.
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "../../src/machine/machine.h"
#include "../../src/memmap/cpp/memory_system.h"
#include "../../src/memmap/cpp/ram_mapper.h"
#include "../../src/memmap/cpp/slot_memory_bus.h"
#include "../../src/rtc/rtc_device.h"

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    if (cond) {
        std::printf("[PASS] %s\n", what.c_str());
    } else {
        std::printf("[FAIL] %s\n", what.c_str());
        ++g_failures;
    }
}

bool Contains(const std::string &h, const std::string &n) { return h.find(n) != std::string::npos; }

bool VramHas(const machine::Machine &m, const std::string &text) {
    const uint8_t *v = m.vdp_state().vram;
    for (int i = 0; i + static_cast<int>(text.size()) <= 0x4000; ++i)
        if (std::equal(text.begin(), text.end(), v + i)) return true;
    return false;
}

void Frames(machine::Machine &m, int n) {
    for (int i = 0; i < n; ++i) m.RunFrame();
}

// Digita um caractere (SHIFT quando preciso, layout internacional).
void TypeChar(machine::Machine &m, char c) {
    std::string key;
    bool shift = false;
    if (c >= 'a' && c <= 'z') key = std::string(1, c);
    else if (c >= 'A' && c <= 'Z') { key = std::string(1, static_cast<char>(c - 'A' + 'a')); shift = true; }
    else if (c >= '0' && c <= '9') key = std::string(1, c);
    else {
        switch (c) {
        case ' ': key = "space"; break;
        case '|': key = "enter"; break;
        case ',': case '-': case '=': case '.': key = std::string(1, c); break;
        case '(': key = "9"; shift = true; break;
        case ')': key = "0"; shift = true; break;
        case ':': key = ";"; shift = true; break;
        case '$': key = "4"; shift = true; break;
        case '"': key = "'"; shift = true; break;
        default: std::printf("caractere sem tecla: %c\n", c); return;
        }
    }
    if (shift) m.KeyDown("shift");
    m.KeyDown(key);
    Frames(m, 6);
    m.KeyUp(key);
    if (shift) m.KeyUp("shift");
    Frames(m, 6);
}

void Type(machine::Machine &m, const std::string &text) {
    for (char c : text) TypeChar(m, c);
}

// Pixel (x,y) de SCREEN 5 lido da VRAM da maquina.
int Pix5(const machine::Machine &m, int x, int y) {
    const uint8_t b = m.vdp_state().vram[(y << 7) + (x >> 1)];
    return (x & 1) ? (b & 0x0F) : (b >> 4);
}

} // namespace

int main() {
    // --- 1. MemorySystem: RAM com mapper -------------------------------------------
    {
        memmap::MemorySystem mem;
        mem.AllocateMapperRam(3, 2, 8);
        check(mem.MapperSegments(3, 2) == 8 && mem.MapperSegments(0, 0) == 0, "AllocateMapperRam: 8 segmentos de 16KB em 3:2 (e 0 nos outros slots)");

        // Mapeamento explicito (o padrao agora e' 0,1,2,3): pagina 0 -> segmento 3, 1 -> 2, 2 -> 1, 3 -> 0
        mem.SetMapperSegment(3, 2, 0, 3);
        mem.SetMapperSegment(3, 2, 1, 2);
        mem.SetMapperSegment(3, 2, 2, 1);
        mem.SetMapperSegment(3, 2, 3, 0);
        mem.PokeSlot(3, 2, 0x0000, 0x30); // pagina 0 -> segmento 3
        mem.PokeSlot(3, 2, 0x4000, 0x20); // pagina 1 -> segmento 2
        mem.PokeSlot(3, 2, 0x8000, 0x10); // pagina 2 -> segmento 1
        mem.PokeSlot(3, 2, 0xC000, 0x00); // pagina 3 -> segmento 0
        // Trocar a pagina 3 para o segmento 3 mostra o que a pagina 0 escreveu
        mem.SetMapperSegment(3, 2, 3, 3);
        check(mem.PeekSlot(3, 2, 0xC000) == 0x30, "pagina 3 -> segmento 3 mostra o byte gravado pela pagina 0 (mesmo segmento fisico)");
        mem.SetMapperSegment(3, 2, 3, 1);
        check(mem.PeekSlot(3, 2, 0xC000) == 0x10, "pagina 3 -> segmento 1 mostra o byte da pagina 2");
        mem.SetMapperSegment(3, 2, 3, 5);
        check(mem.PeekSlot(3, 2, 0xC000) == 0x00, "pagina 3 -> segmento 5 (nunca escrito): zeros");
        mem.SetMapperSegment(3, 2, 3, 13); // 13 & 7 = 5
        mem.PokeSlot(3, 2, 0xC123, 0x77);
        mem.SetMapperSegment(3, 2, 0, 5);
        check(mem.PeekSlot(3, 2, 0x0123) == 0x77, "o numero do segmento e' mascarado pelo tamanho (13 -> 5)");

        // A vista da CPU acompanha a troca na hora (slot 3:2 visivel nas 4 paginas)
        memmap::SlotMemoryBus bus(mem);
        bus.out(0xA8, 0xFF);     // todas as paginas no slot primario 3
        bus.write(0xFFFF, 0xAA); // e no subslot 2
        bus.write(0x8000, 0x5A);
        mem.SetMapperSegment(3, 2, 2, 6);
        check(bus.read(0x8000) == 0x00, "vista ativa: trocar o segmento da pagina visivel muda o que a CPU le");
        mem.SetMapperSegment(3, 2, 2, 1);
        check(bus.read(0x8000) == 0x5A, "... e voltar ao segmento anterior devolve o dado");
        bus.write(0x8001, 0x6B);
        check(mem.PeekSlot(3, 2, 0x8001) == 0x6B, "a escrita pela vista vai para o segmento da pagina (RAM gravavel)");

        // Regras de subslot do MSX2 (valor 2): so' os slots de cartucho (1 e 2) nao expandem
        memmap::MemorySystem m2;
        m2.state().msx1_subslot_rules = 2;
        memmap::SlotMemoryBus b2(m2);
        b2.out(0xA8, 0x00); // pagina 3 = slot 0
        b2.write(0xFFFF, 0x55);
        check(b2.read(0xFFFF) == static_cast<uint8_t>(~0x55), "regras MSX2: o slot 0 aceita subslot (FFFFh devolve o complemento)");
        b2.out(0xA8, 0x40 | 0x80 | 0x00); // paginas 2 e 3 = slot 1... (3 = slot 2, 2 = slot 1)
        b2.out(0xA8, 0xC0);               // pagina 3 = slot 3
        b2.out(0xA8, 0x40);               // pagina 3 = slot 1
        b2.write(0xFFFF, 0x55);
        check(m2.state().ssl_reg[1] == 0, "regras MSX2: o slot 1 (cartucho) nunca tem subslot");
        memmap::MemorySystem m1;
        m1.state().msx1_subslot_rules = 1;
        memmap::SlotMemoryBus b1(m1);
        b1.out(0xA8, 0x00);
        b1.write(0xFFFF, 0x55);
        check(b1.read(0xFFFF) == 0xFF, "regras MSX1 (valor 1): o slot 0 NAO tem subslot (continua como antes)");
    }

    // --- 2. RamMapperDevice (portas FCh-FFh) -------------------------------------------
    {
        memmap::MemorySystem mem;
        mem.AllocateMapperRam(3, 2, 8);
        memmap::RamMapperDevice dev(mem, 3, 2);
        check(dev.in(0xFC) == (0xF8 | 0) && dev.in(0xFD) == (0xF8 | 1) && dev.in(0xFE) == (0xF8 | 2) && dev.in(0xFF) == (0xF8 | 3),
              "reset: FCh..FFh = segmentos 0,1,2,3 e os bits acima da mascara leem 1 (F8h|n)");
        mem.PokeSlot(3, 2, 0x4000, 0xA2);
        dev.out(0xFC, 1);
        check(dev.segment(0) == 1 && mem.PeekSlot(3, 2, 0x0000) == 0xA2, "OUT (FCh),1: a pagina 0 passa a mostrar o segmento 1");
        dev.out(0xFF, 0xFD); // so' os 3 bits baixos: 5
        check(dev.segment(3) == 5 && dev.in(0xFF) == 0xFD, "escrita mascarada (FDh -> 5) e leitura com os bits altos em 1");
        dev.Reset();
        check(dev.segment(0) == 0 && dev.segment(3) == 3, "Reset() volta a 0,1,2,3 (os 4 primeiros segmentos)");
    }

    // --- 3. RTC (RP5C01) ------------------------------------------------------------------
    {
        rtc::RtcDevice rtc;
        std::tm tm{};
        tm.tm_year = 126; // 2026
        tm.tm_mon = 9;    // outubro
        tm.tm_mday = 2;
        tm.tm_hour = 12;
        tm.tm_min = 34;
        tm.tm_sec = 56;
        tm.tm_isdst = -1;
        const std::time_t fixed = std::mktime(&tm);
        rtc.SetClock([&] { return fixed; });
        auto rd = [&](int reg) {
            rtc.out(0xB4, static_cast<uint8_t>(reg));
            return static_cast<int>(rtc.in(0xB5));
        };
        check((rd(0) & 0x0F) == 6 && (rd(1) & 0x0F) == 5, "RTC: segundos 56 (digitos 6 e 5)");
        check((rd(2) & 0x0F) == 4 && (rd(3) & 0x0F) == 3, "RTC: minutos 34");
        check((rd(4) & 0x0F) == 2 && (rd(5) & 0x0F) == 1, "RTC: horas 12");
        check((rd(7) & 0x0F) == 2 && (rd(8) & 0x0F) == 0, "RTC: dia 02");
        check((rd(9) & 0x0F) == 0 && (rd(10) & 0x0F) == 1, "RTC: mes 10");
        check((rd(11) & 0x0F) == 6 && (rd(12) & 0x0F) == 4, "RTC: ano 46 (2026 - 1980)");
        check((rd(0) & 0xF0) == 0xF0, "RTC: os 4 bits altos sempre leem 1");
        check(rd(14) == 0xFF && rd(15) == 0xFF, "RTC: registradores 14/15 leem FFh");

        // Registrador de modo (13): escolhe o banco; a CMOS do banco 2 tem os padroes da BIOS
        rtc.out(0xB4, 13);
        rtc.out(0xB5, 2);
        check((rd(13) & 0x0F) == 2, "RTC: o registrador 13 devolve o modo escrito (com os bits altos em 1)");
        check(rd(4) == (0xF0 | 40 % 16 | 0) || (rd(4) & 0x0F) == (40 & 0x0F), "RTC: CMOS banco 2, registrador 4 = 40 (padrao do fMSX)");
        check((rd(6) & 0x0F) == 15 && (rd(7) & 0x0F) == 4 && (rd(8) & 0x0F) == 4, "RTC: CMOS banco 2: cores 15/4/4 (frente/fundo/borda padrao do BASIC)");
        // Escrita na CMOS do banco 3 nao vaza para os outros
        rtc.out(0xB4, 13);
        rtc.out(0xB5, 3);
        rtc.out(0xB4, 1);
        rtc.out(0xB5, 0x0A);
        check((rd(1) & 0x0F) == 0x0A, "RTC: escrita na CMOS do banco 3 e leitura de volta");
        rtc.out(0xB4, 13);
        rtc.out(0xB5, 1);
        check((rd(1) & 0x0F) == 0, "RTC: o banco 1 nao foi afetado");
        rtc.out(0xB4, 13);
        rtc.out(0xB5, 0);
        check((rd(1) & 0x0F) == 5, "RTC: de volta ao banco 0, o relogio continua dando a hora (a escrita nao altera o relogio)");
    }

#ifdef FWMSX_SOURCE_DIR
    const std::string roms = std::string(FWMSX_SOURCE_DIR) + "/resource/fMSX/ROMs/";
    machine::MachineConfig config;
    config.model = machine::Model::MSX2;
    config.bios_path = roms + "MSX2.ROM";
    std::string error;

    // --- 4. Erros de criacao ---------------------------------------------------------------
    {
        machine::MachineConfig bad = config;
        bad.ext_rom_path = "/nao/existe/MSX2EXT.ROM";
        check(machine::Machine::Create(bad, error) == nullptr && Contains(error, "sub-ROM"), "MSX2 sem a sub-ROM: Create() falha citando a sub-ROM (" + error + ")");
        machine::MachineConfig badsize = config;
        badsize.ext_rom_path = roms + "MSX.ROM"; // 32KB: tamanho errado para a sub-ROM
        error.clear();
        check(machine::Machine::Create(badsize, error) == nullptr && Contains(error, "tamanho"), "sub-ROM com tamanho errado: erro explicando");
    }

    std::unique_ptr<machine::Machine> m = machine::Machine::Create(config, error);
    if (!m) {
        std::printf("[SKIP] MSX2.ROM/MSX2EXT.ROM nao encontradas (%s)\n", error.c_str());
    } else {
        // --- 5. Boot do MSX BASIC 2.1 ---------------------------------------------------------
        check(m->is_msx2() && m->vdp_state().model == VDP_MODEL_MSX2 && m->vdp_state().vram_mask == 0x1FFFF, "maquina MSX2: VDP V9938 com 128KB de VRAM");
        check(m->mapper() != nullptr && m->rtc() != nullptr && m->memory().MapperSegments(3, 2) == 8, "RAM de 128KB com mapper em 3:2 e RTC ligados");
        Frames(*m, 500);
        check(VramHas(*m, "MSX BASIC version 2.1") && VramHas(*m, "Copyright 1986 by Microsoft"), "boot: a BIOS MSX2 + sub-ROM sobem ate' o MSX BASIC 2.1");
        check(m->vdp_state().scr_mode == 0 && (m->vdp_state().regs[1] & 0x40), "o BASIC sobe em SCREEN 0 com a tela ligada");
        check(m->mapper() != nullptr, "a BIOS MSX2 sobe com o mapper de RAM presente");
        std::vector<uint32_t> rgba;
        machine::FrameSize fs = m->RenderFrame(rgba);
        check(fs.width == 544 && fs.height == 228 && fs.y_scale == 2 && rgba.size() == 544u * 228u, "MSX2: imagem de 512 + borda de 16 com linhas dobradas na exibicao (y_scale=2)");

        // --- 6. BASIC desenhando pelo motor de comandos do V9938 -----------------------------
        // SCREEN 5 + LINE ,bf (LMMV) + a espera de uma tecla para o prompt nao voltar ao texto.
        Type(*m, "screen 5:line (20,20)-(120,80),9,bf:line (10,100)-(200,100),12:a$=input$(1)|");
        Frames(*m, 1500);
        const VdpState &v = m->vdp_state();
        check(v.scr_mode == 5, "SCREEN 5 no BASIC: o VDP entrou no modo 5");
        check(Pix5(*m, 50, 50) == 9 && Pix5(*m, 20, 20) == 9 && Pix5(*m, 120, 80) == 9, "LINE ,bf: retangulo preenchido com a cor 9 (cantos incluidos)");
        check(Pix5(*m, 19, 50) == 4 && Pix5(*m, 121, 50) == 4 && Pix5(*m, 50, 19) == 4 && Pix5(*m, 50, 81) == 4, "fora do retangulo: o fundo (cor 4) intacto");
        check(Pix5(*m, 10, 100) == 12 && Pix5(*m, 100, 100) == 12 && Pix5(*m, 200, 100) == 12 && Pix5(*m, 100, 101) == 4, "LINE (10,100)-(200,100): 191 pixels da cor 12 numa linha so'");
        fs = m->RenderFrame(rgba);
        check(fs.width == 544 && fs.height == 228 && fs.y_scale == 2, "SCREEN 5 do BASIC usa 212 linhas (R#9 bit 7) dentro do quadro 544x228");
        // o pixel (50,50) aparece dobrado na horizontal em (100,50)/(101,50) com a cor 9 da paleta
        const uint32_t px9 = 0xFF000000u | (static_cast<uint32_t>(v.palette_b[9]) << 16) | (static_cast<uint32_t>(v.palette_g[9]) << 8) | v.palette_r[9];
        // 212 linhas: borda superior de 8; pixel (50,50) de 256 -> colunas 100/101 do bloco de 512 (+16 de borda)
        check(rgba[(8 + 50) * 544 + 16 + 100] == px9 && rgba[(8 + 50) * 544 + 16 + 101] == px9, "RenderFrame: os pixels de 256 saem dobrados em largura com a cor da paleta");

        // qualquer tecla termina o INPUT$ e o BASIC volta ao texto
        Type(*m, "x");
        Frames(*m, 300);
        check(m->vdp_state().scr_mode == 0, "depois da tecla, o prompt volta ao modo texto");

        // --- 7. SCREEN 7 (512 pixels) e SCREEN 8 --------------------------------------------------
        Type(*m, "screen 7:line (0,0)-(100,50),5,bf:a$=input$(1)|");
        Frames(*m, 1200);
        check(m->vdp_state().scr_mode == 7, "SCREEN 7 no BASIC");
        check(m->vdp_state().vram[0] == 0x55 && m->vdp_state().vram[49] == 0x55 && m->vdp_state().vram[50] == 0x54 && m->vdp_state().vram[51] == 0x44,
              "SCREEN 7: LINE ,bf pinta os pixels 0..100 (50 bytes + o nibble alto do 51o) e o fundo continua cor 4");
        fs = m->RenderFrame(rgba);
        check(fs.width == 544 && fs.height == 228, "SCREEN 7: imagem 512x212 dentro do quadro 544x228");
        Type(*m, "x");
        Frames(*m, 300);
        Type(*m, "screen 8:line (0,0)-(9,9),28,bf:a$=input$(1)|");
        Frames(*m, 1200);
        check(m->vdp_state().scr_mode == 8 && m->vdp_state().vram[0] == 28 && m->vdp_state().vram[9] == 28 && m->vdp_state().vram[10] != 28 && m->vdp_state().vram[(9 << 8)] == 28 &&
                  m->vdp_state().vram[(10 << 8)] != 28,
              "SCREEN 8: LINE ,bf com a cor 28 (1Ch = vermelho puro GRB) em 10x10 bytes");

        // TEXT80: WIDTH 80 coloca o VDP no modo 13; a imagem sai 512x192 (80 colunas = 480 pixels centralizados)
        Type(*m, "x");
        Frames(*m, 300);
        Type(*m, "screen 0:width 80:print string$(80,\"M\"):a$=input$(1)|");
        Frames(*m, 1200);
        check(m->vdp_state().scr_mode == VDP_MAXSCREEN + 1, "WIDTH 80: o VDP entra em TEXT80 (scr_mode 13)");
        fs = m->RenderFrame(rgba);
        check(fs.width == 544 && fs.height == 228 && fs.y_scale == 2, "TEXT80: imagem de 480 pixels de texto dentro do quadro 544x228");
        Type(*m, "x");
        Frames(*m, 300);

        // --- 8. Reset ---------------------------------------------------------------------------
        m->Reset();
        Frames(*m, 500);
        check(m->is_msx2() && m->vdp_state().model == VDP_MODEL_MSX2 && VramHas(*m, "MSX BASIC version 2.1"), "Reset(): continua MSX2 e volta ao BASIC 2.1");
    }

    // --- 8b. Disco no MSX2: sub-ROM e DISK.ROM dividem o slot 3:1 ----------------------------------
    {
        const std::string dos_src = std::string(FWMSX_SOURCE_DIR) + "/msxdos1.dsk";
        const char *tmp_env = std::getenv("TEMP");
        const std::string dos_tmp = (tmp_env ? std::string(tmp_env) : std::string("/tmp")) + "/fwmsx_msx2_dos.dsk";
        {
            std::ifstream in(dos_src, std::ios::binary);
            std::vector<char> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            std::ofstream out(dos_tmp, std::ios::binary);
            out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        }
        machine::MachineConfig dc = config;
        dc.disk_a = dos_tmp;
        std::unique_ptr<machine::Machine> dm = machine::Machine::Create(dc, error);
        if (!dm) {
            check(false, "MSX2 com disco: Create() falhou: " + error);
        } else {
            Frames(*dm, 700);
            check(VramHas(*dm, "MSX-DOS version 1.8") && VramHas(*dm, "COMMAND version 1.12"),
                  "MSX2 + disco: o MSX-DOS 1.8 boota (sub-ROM em 3:1 pagina 0, DISK.ROM na pagina 1)");
            check(!VramHas(*dm, "Enter new date"), "o MSX2 nao pergunta a data (o RTC ja' a fornece)");
            Type(*dm, "|dir|");
            Frames(*dm, 200);
            check(VramHas(*dm, "COMMAND  COM") && VramHas(*dm, "MSXDOS   SYS") && VramHas(*dm, "2 files"), "MSX2: dir lista os dois arquivos do disco");
        }
        std::remove(dos_tmp.c_str());
    }

    // --- 9. MSX1 continua igual ----------------------------------------------------------------
    {
        machine::MachineConfig c1;
        c1.bios_path = roms + "MSX.ROM";
        std::unique_ptr<machine::Machine> m1 = machine::Machine::Create(c1, error);
        if (m1) {
            Frames(*m1, 250);
            std::vector<uint32_t> rgba;
            const machine::FrameSize fs = m1->RenderFrame(rgba);
            check(!m1->is_msx2() && m1->mapper() == nullptr && fs.width == 272 && fs.height == 228 && fs.y_scale == 1 && m1->vdp_state().vram_mask == 0x3FFF,
                  "MSX1: VDP de 16KB, sem mapper/RTC, quadro 272x228 (256x192 + borda) sem dobrar linhas");
            check(VramHas(*m1, "MSX BASIC version 1.0"), "MSX1: continua subindo no BASIC 1.0");
        }
    }
#endif

    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
