// Teste do MSX2: mapper de RAM (portas FCh-FFh), relogio RTC (B4h/B5h) e a
// maquina completa com a BIOS MSX2 real -- ver doc/msx2-spec.md.
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
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
        case '#': key = "3"; shift = true; break;
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

    // --- 8a. RAM mapeada de 64 a 4096 KB e dois mappers (interno 3:2 + externo 3:3) ----------------------------------------------------
    {
        check(machine::MapperRamKb(config, false) == 128 && machine::MapperRamKb(config, true) == 0, "MSX2 de fabrica: mapper interno de 128 KB, sem externo");
        machine::MachineConfig rc = config;
        std::string rerr;
        check(!machine::SetMapperRam(rc, 100, 0, rerr) && !machine::SetMapperRam(rc, 128, 8192, rerr) && !machine::SetMapperRam(rc, 32, 0, rerr),
              "SetMapperRam recusa 100 KB, 8192 KB e 32 KB (o registrador do mapper tem 8 bits: ate' 4096 KB)");
        for (const int kb : {64, 128, 256, 512, 1024, 2048, 4096}) {
            machine::MachineConfig c = config;
            c.mapper_kb = kb;
            std::unique_ptr<machine::Machine> mm = machine::Machine::Create(c, rerr);
            check(mm && mm->memory().MapperSegments(3, 2) == kb / 16 && mm->mapper_count() == 1, "mapper interno de " + std::to_string(kb) + " KB: " + std::to_string(kb / 16) + " segmentos");
        }

        // 512 KB interno + 1024 KB externo: o valor escrito na porta e' mascarado por cada mapper
        machine::MachineConfig two = config;
        two.mapper_kb = 512;
        two.mapper2_kb = 1024;
        std::unique_ptr<machine::Machine> tm = machine::Machine::Create(two, rerr);
        check(tm != nullptr && tm->mapper_count() == 2, "dois mappers (512 KB em 3:2 e 1024 KB em 3:3) criados (" + rerr + ")");
        if (tm) {
            check(tm->memory().MapperSegments(3, 2) == 32 && tm->memory().MapperSegments(3, 3) == 64, "32 segmentos no interno e 64 no externo");
            for (size_t i = 0; i < tm->mapper_count(); ++i) tm->mapper_at(i)->out(0xFE, 0x2B); // a porta escreve nos dois
            memmap::RamMapperDevice *d0 = tm->mapper_at(0), *d1 = tm->mapper_at(1);
            const bool inner_first = d0->secondary() == 2;
            memmap::RamMapperDevice *in_dev = inner_first ? d0 : d1, *out_dev = inner_first ? d1 : d0;
            check(in_dev->segment(2) == (0x2B & 31) && out_dev->segment(2) == 0x2B, "OUT FEh,2Bh: o interno fica com o segmento 11 (mascara 31) e o externo com o 43 (mascara 63)");
            check(in_dev->in(0xFE) == static_cast<uint8_t>((0x2B & 31) | ~31) && out_dev->in(0xFE) == static_cast<uint8_t>(0x2B | ~63), "IN FEh: os bits acima da mascara de cada mapper leem 1");
            // estado salvo: os registradores voltam (e o da porta acompanha a memoria)
            const std::string sst = (std::filesystem::temp_directory_path() / "fwmsx_msx2_mapper.sst").string();
            check(tm->SaveState(sst, rerr), "SaveState com dois mappers (" + rerr + ")");
            std::unique_ptr<machine::Machine> t2 = machine::Machine::Create(two, rerr);
            check(t2 && t2->LoadState(sst, rerr), "LoadState com dois mappers (" + rerr + ")");
            if (t2) {
                const bool f2 = t2->mapper_at(0)->secondary() == 2;
                check(t2->mapper_at(f2 ? 0 : 1)->segment(2) == 11 && t2->mapper_at(f2 ? 1 : 0)->segment(2) == 43, "estado carregado: registradores dos dois mappers de volta (11 e 43)");
            }
            std::remove(sst.c_str());
            Frames(*tm, 600);
            check(VramHas(*tm, "MSX BASIC version 2.1"), "a BIOS MSX2 sobe com dois mappers (512 KB + 1024 KB)");
        }
        // layout ja' fixado (SetCartridge): SetMapperRam mexe nas celulas
        machine::MachineConfig fz = config;
        machine::SetCartridge(fz, "", MEMMAP_MAPPER_NONE);
        check(machine::SetMapperRam(fz, 256, 512, rerr) && machine::MapperRamKb(fz, false) == 256 && machine::MapperRamKb(fz, true) == 512, "SetMapperRam num layout fixado: 256 KB + 512 KB");
        check(machine::SetMapperRam(fz, 256, 0, rerr) && machine::MapperRamKb(fz, true) == 0, "SetMapperRam com externo 0 tira o 2o mapper");
        // slot do externo ocupado = erro claro
        machine::MachineConfig bad2 = config;
        bad2.mapper2_kb = 256;
        bad2.mapper2_slot = 13; // 3:1 e' o da sub-ROM/DISK.ROM
        error.clear();
        check(machine::Machine::Create(bad2, error) == nullptr && Contains(error, "3:1"), "2o mapper num slot ocupado: Create() recusa citando o slot (" + error + ")");
    }

    // --- 8b. MSX-DOS 2: o kernel de 64KB no slot 1 (mapper msxdos2) + a interface de disquete + um disco de sistema do MSX-DOS 2 ------------------
    // A ROM vem do Full Set System ROMs (nao fica no repositorio) e o disco de sistema e' o de emulator/ (gerado na build); sem eles o bloco e' pulado.
    {
        namespace fsys = std::filesystem;
        const fsys::path rom = fsys::path(FWMSX_SOURCE_DIR) / "dist/roms/filehunter/15-08-2026/extensions/msxdos22.rom";
        const fsys::path dsk = fsys::path(FWMSX_SOURCE_DIR) / "dist/emulator/msxdos2.dsk";
        std::error_code ec;
        if (!fsys::is_regular_file(rom, ec) || !fsys::is_regular_file(dsk, ec)) {
            std::printf("[SKIP] MSX-DOS 2: msxdos22.rom ou emulator/msxdos2.dsk nao encontrados\n");
        } else {
            const fsys::path tmp = fsys::temp_directory_path() / "fwmsx_msx2_dos2.dsk";
            fsys::copy_file(dsk, tmp, fsys::copy_options::overwrite_existing, ec);
            machine::MachineConfig dc = config;
            machine::EnableDiskInterface(dc);
            machine::SetCartridge(dc, rom.string(), MEMMAP_MAPPER_MSXDOS2);
            dc.disk_a = tmp.string();
            machine::SetMapperRam(dc, 512, 1024, error); // o alvo do usuario: MSX2 com 512 KB internos + 1024 KB externos
            std::unique_ptr<machine::Machine> dm = machine::Machine::Create(dc, error);
            check(dm != nullptr, "MSX-DOS 2: Create() com o cartucho msxdos2 e o disco (" + error + ")");
            if (dm) {
                Frames(*dm, 900);
                check(VramHas(*dm, "A:\\>"), "MSX-DOS 2: o prompt e' o do DOS 2 (A:\\>), com o relogio da MSX2 (sem perguntar a data)");
                Type(*dm, "ver|");
                Frames(*dm, 200);
                check(VramHas(*dm, "MSX-DOS kernel version 2.") && VramHas(*dm, "COMMAND2.COM version 2."), "ver: kernel 2.xx do cartucho e COMMAND2.COM do disco");
                Type(*dm, "mkdir sub|");
                Frames(*dm, 400);
                Type(*dm, "dir|");
                Frames(*dm, 300);
                check(VramHas(*dm, "SUB") && VramHas(*dm, "<dir>"), "mkdir/dir: subdiretorios (so' o MSX-DOS 2 tem) gravados e listados");
            }
            fsys::remove(tmp, ec);
        }
    }

    // --- 8c. Disco rigido: Sunrise IDE + Nextor sobre uma imagem criada pelo fwMSX (MBR + FAT16) --------------------------------------------
    // A ROM do Nextor vem do Full Set System ROMs (nao fica no repositorio); sem ela o bloco e' pulado.
    {
        namespace fsys = std::filesystem;
        const fsys::path nextor = fsys::path(FWMSX_SOURCE_DIR) / "dist/roms/filehunter/15-08-2026/extensions/Nextor-2.1.1.SunriseIDE.ROM";
        std::error_code ec;
        if (!fsys::is_regular_file(nextor, ec)) {
            std::printf("[SKIP] Nextor: Nextor-2.1.1.SunriseIDE.ROM nao encontrada\n");
        } else {
            const fsys::path img = fsys::temp_directory_path() / "fwmsx_msx2_nextor.hdd";
            std::string herr;
            check(hd::CreateHdImage(img.string(), {16, 16}, herr), "Nextor: imagem de HD criada (" + herr + ")");
            machine::MachineConfig hc = config;
            hc.ide_master = img.string();
            hc.nextor_rom = nextor.string();
            hc.mapper_kb = 512;
            std::unique_ptr<machine::Machine> hm = machine::Machine::Create(hc, herr);
            check(hm != nullptr && hm->ide() != nullptr, "Nextor: Create() com a Sunrise IDE em 3:0 (" + herr + ")");
            if (hm) {
                Frames(*hm, 900);
                check(VramHas(*hm, "Nextor BASIC version 2."), "Nextor: o kernel sobe e o BASIC mostra 'Nextor BASIC version 2.xx'");
                check(hm->ide()->disk(0).sectors_read() >= 3, "Nextor: leu a tabela de particoes e o setor de boot do HD (" + std::to_string(hm->ide()->disk(0).sectors_read()) + " setores)");
                Type(*hm, "open \"a:ola.txt\" for output as #1:print #1,\"oi\":close #1|");
                Frames(*hm, 900);
                check(hm->ide()->disk(0).sectors_written() >= 2, "Nextor: gravou arquivo na particao A: (" + std::to_string(hm->ide()->disk(0).sectors_written()) + " setores gravados)");
                std::ifstream f(img, std::ios::binary);
                std::vector<char> data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                const std::string needle = "OLA     TXT";
                check(std::search(data.begin(), data.end(), needle.begin(), needle.end()) != data.end(), "Nextor: o diretorio do arquivo .hdd tem OLA.TXT");
            }
            fsys::remove(img, ec);
        }
    }

    // --- 8d. Cartao SD: MegaSD + Nextor 2.1.4 com o driver MegaSDHC do OCM (pacote do KdL) sobre uma imagem do fwMSX ---------------------------
    // A ROM vem de dist/roms/ocm/roms (copiada do pacote SDBIOS; fica fora do git); sem ela o bloco e' pulado.
    {
        namespace fsys = std::filesystem;
        const fsys::path rom = fsys::path(FWMSX_SOURCE_DIR) / "dist/roms/ocm/roms/nextsd15.rom";
        std::error_code ec;
        if (!fsys::is_regular_file(rom, ec)) {
            std::printf("[SKIP] MegaSD: dist/roms/ocm/roms/nextsd15.rom nao encontrada\n");
        } else {
            const fsys::path img = fsys::temp_directory_path() / "fwmsx_msx2_megasd.hdd";
            std::string herr;
            check(hd::CreateHdImage(img.string(), {16, 16}, herr), "MegaSD: imagem de cartao criada (" + herr + ")");
            machine::MachineConfig sc = config;
            sc.sd_image = img.string();
            sc.megasd_rom = rom.string();
            std::unique_ptr<machine::Machine> sm = machine::Machine::Create(sc, herr);
            check(sm != nullptr && sm->megasd() != nullptr, "MegaSD: Create() com o MegaSD em 3:3 (" + herr + ")");
            if (sm) {
                Frames(*sm, 1500);
                check(VramHas(*sm, "Nextor BASIC version 2."), "MegaSD: o kernel sobe e o BASIC mostra 'Nextor BASIC version 2.xx'");
                check(sm->megasd()->card().sectors_read() >= 1, "MegaSD: o driver inicializou o cartao (CMD0/8/55/41/58) e leu a tabela de particoes (" + std::to_string(sm->megasd()->card().sectors_read()) + " setores)");
                Type(*sm, "open \"a:sd.txt\" for output as #1:print #1,\"oi\":close #1|");
                Frames(*sm, 1500);
                check(sm->megasd()->card().sectors_written() >= 2, "MegaSD: gravou arquivo na particao A: (" + std::to_string(sm->megasd()->card().sectors_written()) + " setores gravados)");
                std::ifstream f(img, std::ios::binary);
                std::vector<char> data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                const std::string needle = "SD      TXT";
                check(std::search(data.begin(), data.end(), needle.begin(), needle.end()) != data.end(), "MegaSD: o diretorio do arquivo da imagem tem SD.TXT");
            }
            fsys::remove(img, ec);
        }
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
