// Teste da maquina MSX1 completa (src/machine/machine.{h,cpp}) -- ver
// doc/machine-spec.md. Roda tudo sem janela: a mesma Machine que a janela usa.
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "../../src/machine/machine.h"
#include "../../src/fdc/cpp/disk_format.h"
#include "../../src/psg/core/psg_state.h"

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

bool Contains(const std::string &haystack, const std::string &needle) {
    return haystack.find(needle) != std::string::npos;
}

std::string TempPath(const char *name) {
    const char *t = std::getenv("TEMP");
    return (t ? std::string(t) : std::string("/tmp")) + "/" + name;
}

bool VramHas(const machine::Machine &m, const std::string &text) {
    const uint8_t *v = m.vdp_state().vram;
    for (int i = 0; i + static_cast<int>(text.size()) <= VDP_VRAM_SIZE; ++i)
        if (std::equal(text.begin(), text.end(), v + i)) return true;
    return false;
}

void Frames(machine::Machine &m, int n) {
    for (int i = 0; i < n; ++i) m.RunFrame();
}

void Type(machine::Machine &m, const std::string &text) {
    for (char c : text) {
        std::string key;
        if (c == ' ') key = "space";
        else if (c == '|') key = "enter";
        else key = std::string(1, c);
        m.KeyDown(key);
        Frames(m, 6);
        m.KeyUp(key);
        Frames(m, 6);
    }
}

} // namespace

int main() {
    using machine::Machine;
    using machine::MachineConfig;

    // --- 1. Constantes de temporizacao --------------------------------------
    check(Machine::kFrameCycles == 59736 && Machine::kFrameRate > 59.9 && Machine::kFrameRate < 59.95,
          "quadro NTSC: 262 linhas x 228 ciclos = 59736 ciclos, ~59.92 Hz");

    // --- 2. Erros de criacao ---------------------------------------------------
    {
        std::string error;
        MachineConfig none;
        check(Machine::Create(none, error) == nullptr && !error.empty(), "sem BIOS: Create() falha com mensagem");
        MachineConfig missing;
        missing.bios_path = "/nao/existe/MSX.ROM";
        error.clear();
        check(Machine::Create(missing, error) == nullptr && Contains(error, "BIOS"), "BIOS inexistente: Create() falha citando a BIOS");
    }

    // --- 2b. SetCartridge()/CartridgePath()/CartridgeMapper(): mapper explicito pela janela --
    // (menu Midia > Cartucho > Mapper, ou a Configuracao de slots -- ver
    // src/machine/gui/emu_window.cpp). Nao precisa de BIOS real: so' mexe no
    // MachineConfig/SlotLayout.
    {
        MachineConfig cfg;
        check(machine::CartridgePath(cfg).empty() && machine::CartridgeMapper(cfg) == MEMMAP_MAPPER_NONE,
              "sem cartucho: CartridgePath/CartridgeMapper vazios");

        machine::SetCartridge(cfg, "jogo.rom"); // sem mapper explicito = auto (NONE), como sempre
        check(machine::CartridgePath(cfg) == "jogo.rom" && machine::CartridgeMapper(cfg) == MEMMAP_MAPPER_NONE,
              "SetCartridge sem mapper: auto/deteccao (comportamento de sempre preservado)");

        machine::SetCartridge(cfg, "msxdos2.rom", MEMMAP_MAPPER_MSXDOS2);
        check(machine::CartridgePath(cfg) == "msxdos2.rom" && machine::CartridgeMapper(cfg) == MEMMAP_MAPPER_MSXDOS2,
              "SetCartridge com mapper explicito (msxdos2): CartridgeMapper devolve o mesmo");

        machine::SetCartridge(cfg, ""); // retirar cartucho zera tudo, inclusive o mapper
        check(machine::CartridgePath(cfg).empty() && machine::CartridgeMapper(cfg) == MEMMAP_MAPPER_NONE,
              "retirar cartucho (path vazio): mapper volta a NONE tambem");
    }

#ifdef FWMSX_SOURCE_DIR
    const std::string rom = std::string(FWMSX_SOURCE_DIR) + "/resource/fMSX/ROMs/MSX.ROM";
    MachineConfig config;
    config.bios_path = rom;
    std::string error;
    std::unique_ptr<Machine> m = Machine::Create(config, error);
    if (!m) {
        std::printf("[SKIP] BIOS real '%s' nao encontrada (%s)\n", rom.c_str(), error.c_str());
    } else {
        // --- 3. Boot ate o prompt, so' com RunFrame() -------------------------
        Frames(*m, 250);
        check(m->frame_count() == 250, "frame_count() conta os quadros executados");
        check(VramHas(*m, "MSX BASIC version 1.0") && VramHas(*m, "Bytes free"), "boot em quadros: o prompt do MSX BASIC aparece na VRAM");
        check(m->psg().state().r[7] == 0xB8, "o PSG foi avancado/programado pela BIOS (R7=B8h)");

        // --- 3b. Chip FM (MSX-MUSIC, portas 7Ch/7Dh) ligado na maquina ----------
        {
            m->fm().out(0x7C, 0x30);
            m->fm().out(0x7D, 0x00);                // canal 0: timbre 0 (usuario), volume 0
            m->fm().out(0x7C, 0x10);
            m->fm().out(0x7D, 0x80);                // fnum baixo
            m->fm().out(0x7C, 0x20);
            m->fm().out(0x7D, 0x00);                // sem nota
            // Timbre do usuario: portadora senoidal de ataque rapido e sustentacao
            // (mesmo ajuste do fmtest).
            m->fm().out(0x7C, 0x00); m->fm().out(0x7D, 0x21);
            m->fm().out(0x7C, 0x01); m->fm().out(0x7D, 0x21);
            m->fm().out(0x7C, 0x02); m->fm().out(0x7D, 0x3F);
            m->fm().out(0x7C, 0x03); m->fm().out(0x7D, 0x00);
            m->fm().out(0x7C, 0x04); m->fm().out(0x7D, 0xF0);
            m->fm().out(0x7C, 0x05); m->fm().out(0x7D, 0xF0);
            m->fm().out(0x7C, 0x06); m->fm().out(0x7D, 0x0F);
            m->fm().out(0x7C, 0x07); m->fm().out(0x7D, 0x0F);
            m->fm().out(0x7C, 0x20);
            m->fm().out(0x7D, 0x18);                // KEY-ON, bloco 4, fnum bit 8 = 0 (fnum 128)
            check(m->fm().in(0x7C) == 0x00 && m->fm().state().ch[0].key == 1,
                  "FM na maquina: as portas 7Ch/7Dh gravam e ligam o canal 0");

            m->EnableLiveAudio(true);
            Frames(*m, 10);
            std::vector<int16_t> samples;
            m->TakeLiveAudio(samples);
            int nonzero = 0;
            for (int16_t v : samples) nonzero += v != 0;
            check(nonzero > 1000, "FM na maquina: o audio ao vivo traz as amostras do chip (" +
                                      std::to_string(nonzero) + " nao nulas)");
            m->EnableLiveAudio(false);
        }

        // --- 3c. FM-PAC em 2:0 (ROM de 16KB com assinatura "AB"), boot continua ok --
        {
            MachineConfig fmpac = config;
            fmpac.fmpac_rom_path = std::string(FWMSX_SOURCE_DIR) + "/resource/fMSX/FMPAC.ROM";
            std::string fmpac_error;
            std::unique_ptr<Machine> mf = Machine::Create(fmpac, fmpac_error);
            if (!mf) {
                std::printf("[SKIP] FM-PAC nao carregado (%s)\n", fmpac_error.c_str());
            } else {
                check(mf->memory().PeekSlot(2, 0, 0x4000) == 'A' && mf->memory().PeekSlot(2, 0, 0x4001) == 'B',
                      "FM-PAC: assinatura 'AB' aparece em 4000h do slot 2:0");
                check(mf->memory().HasSram(2, 0), "FM-PAC: a maquina tem a SRAM de 8KB do mapper");
                Frames(*mf, 250);
                check(VramHas(*mf, "MSX BASIC version 1.0") && VramHas(*mf, "Bytes free"),
                      "FM-PAC na maquina: o boot continua e chega ao prompt do BASIC");
            }
        }

        // --- 3d. Layout de slots (doc/slots-spec.md): BIOS e BASIC em arquivos
        //         separados (16KB + 16KB), RAM em 2:0 e mapper de 1024KB em 3:1 ----
        {
            auto read_bytes = [](const std::string &path) {
                std::ifstream f(path, std::ios::binary | std::ios::ate);
                std::vector<uint8_t> bytes(static_cast<size_t>(f.tellg()));
                f.seekg(0);
                f.read(reinterpret_cast<char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
                return bytes;
            };
            auto write_bytes = [](const std::string &path, const std::vector<uint8_t> &bytes) {
                std::ofstream f(path, std::ios::binary | std::ios::trunc);
                f.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
            };
            const std::vector<uint8_t> whole = read_bytes(rom);
            const std::string bios16 = TempPath("fwmsx_layout_bios.rom");
            const std::string basic16 = TempPath("fwmsx_layout_basic.rom");
            write_bytes(bios16, std::vector<uint8_t>(whole.begin(), whole.begin() + 0x4000));
            write_bytes(basic16, std::vector<uint8_t>(whole.begin() + 0x4000, whole.begin() + 0x8000));

            MachineConfig split = config;
            split.layout = machine::DefaultLayout(config);
            split.layout_set = true;
            split.layout.cell[0][0].path = bios16;
            split.layout.cell[0][0].path2 = basic16;
            split.layout.cell[2][0].kind = machine::SlotKind::Ram;
            split.layout.cell[2][0].size_kb = 64;
            split.layout.cell[3][1].kind = machine::SlotKind::Mapper;
            split.layout.cell[3][1].size_kb = 1024;

            std::string split_error;
            std::unique_ptr<Machine> ms = Machine::Create(split, split_error);
            check(ms != nullptr, "layout: BIOS 16KB (pagina 0) + BASIC 16KB (pagina 1) em arquivos separados monta (" + split_error + ")");
            if (ms) {
                check(ms->memory().PeekSlot(0, 0, 0x0000) == whole[0x0000] && ms->memory().PeekSlot(0, 0, 0x4000) == whole[0x4000],
                      "layout: pagina 0 da BIOS e pagina 1 com o BASIC, na mesma celula 0:0");
                check(ms->memory().MapperSegments(3, 1) == 64, "layout: mapper de 1024KB em 3:1 (64 segmentos de 16KB)");
                Frames(*ms, 250);
                check(VramHas(*ms, "MSX BASIC version 1.0"), "layout: a maquina montada pelo layout chega ao prompt do BASIC");
            }
            std::remove(bios16.c_str());
            std::remove(basic16.c_str());

            // Quatro bancos de RAM de 64KB no slot 2, um em cada subslot (2:0 a 2:3).
            MachineConfig four_ram = config;
            four_ram.layout = machine::DefaultLayout(config);
            four_ram.layout_set = true;
            for (int sec = 0; sec < 4; ++sec) {
                four_ram.layout.cell[2][sec].kind = machine::SlotKind::Ram;
                four_ram.layout.cell[2][sec].size_kb = 64;
            }
            std::string e4;
            std::unique_ptr<Machine> mr = Machine::Create(four_ram, e4);
            check(mr != nullptr && mr->memory().Describe(2, 3).kind == MEMMAP_KIND_RAM,
                  "layout: quatro bancos de RAM no slot 2 (subslots 2:0 a 2:3) montam (" + e4 + ")");
            if (mr) {
                Frames(*mr, 250);
                check(VramHas(*mr, "MSX BASIC version 1.0"), "layout: com RAM em 2:x a maquina chega ao prompt do BASIC");
            }

            // Layouts que nao montam sao recusados com a razao.
            MachineConfig no_bios = config;
            no_bios.layout = machine::DefaultLayout(config);
            no_bios.layout_set = true;
            no_bios.layout.cell[0][0] = machine::SlotItem{};
            std::string e1;
            check(Machine::Create(no_bios, e1) == nullptr && Contains(e1, "BIOS"), "layout: sem BIOS em 0:0 e recusado (" + e1 + ")");

            MachineConfig two_mappers = split;
            two_mappers.layout.cell[0][0] = machine::DefaultLayout(config).cell[0][0];  // os arquivos de 16KB ja foram apagados
            two_mappers.layout.cell[1][1].kind = machine::SlotKind::Mapper;
            two_mappers.layout.cell[1][1].size_kb = 128;
            std::string e2;
            std::unique_ptr<Machine> mm = Machine::Create(two_mappers, e2);
            check(mm != nullptr && mm->memory().MapperSegments(1, 1) == 8 && mm->memory().MapperSegments(3, 1) == 64,
                  "layout: dois mappers (128KB em 1:1 e 1024KB em 3:1) montam, cada um com seus segmentos (" + e2 + ")");

            // RAM de 32KB: 16KB na pagina 2 da celula (2:0) e 16KB na pagina 3 da celula seguinte (2:1).
            MachineConfig ram32 = config;
            ram32.layout = machine::DefaultLayout(config);
            ram32.layout_set = true;
            ram32.layout.cell[2][0].kind = machine::SlotKind::Ram;
            ram32.layout.cell[2][0].size_kb = 32;
            std::string e5;
            std::unique_ptr<Machine> m32 = Machine::Create(ram32, e5);
            check(m32 != nullptr, "layout: RAM de 32KB em 2:0 monta (" + e5 + ")");
            if (m32) {
                m32->memory().PokeSlot(2, 0, 0x8000, 0x5A);
                m32->memory().PokeSlot(2, 1, 0xC000, 0x6B);
                check(m32->memory().PeekSlot(2, 0, 0x8000) == 0x5A && m32->memory().PeekSlot(2, 1, 0xC000) == 0x6B,
                      "layout: 8000h-BFFFh em 2:0 e C000h-FFFFh em 2:1 (32KB seguidos)");
                check(m32->memory().PeekSlot(2, 0, 0x0000) == MEMMAP_EMPTY_BYTE,
                      "layout: 0000h da celula 2:0 continua vazia (a RAM de 32KB nao ocupa a pagina 0)");
            }
            MachineConfig ram32_last = ram32;
            ram32_last.layout.cell[2][0].kind = machine::SlotKind::Empty;
            ram32_last.layout.cell[3][3].kind = machine::SlotKind::Ram;
            ram32_last.layout.cell[3][3].size_kb = 32;
            std::string e6;
            check(Machine::Create(ram32_last, e6) == nullptr && Contains(e6, "celula seguinte"),
                  "layout: RAM de 32KB em 3:3 e recusada (nao ha celula seguinte) (" + e6 + ")");

            MachineConfig bad_ram = split;
            bad_ram.layout.cell[2][1].kind = machine::SlotKind::Ram;
            bad_ram.layout.cell[2][1].size_kb = 48;
            std::string e3;
            check(Machine::Create(bad_ram, e3) == nullptr && Contains(e3, "RAM"), "layout: RAM de 48KB (fora de 16/32/64) e recusada (" + e3 + ")");
        }

        // --- 3e. Controladora de disco por portas e formato dos drives (doc/fdc-spec.md, secao 6) --
        {
            auto write_blank = [](const std::string &path, size_t size) {
                std::ofstream f(path, std::ios::binary | std::ios::trunc);
                std::vector<uint8_t> zeros(size, 0);
                f.write(reinterpret_cast<const char *>(zeros.data()), static_cast<std::streamsize>(zeros.size()));
            };
            const std::string disk720 = TempPath("fwmsx_disk720.dsk");
            const std::string disk360 = TempPath("fwmsx_disk360.dsk");
            write_blank(disk720, 737280);
            write_blank(disk360, 368640);

            MachineConfig port_cfg = config;
            port_cfg.disk_access = machine::DiskAccess::Port;
            port_cfg.disk_port = 0xD0;
            port_cfg.disk_a = disk720;
            std::string port_error;
            std::unique_ptr<Machine> mp = Machine::Create(port_cfg, port_error);
            check(mp != nullptr && mp->port_fdc() != nullptr, "disco por portas: a controladora fica em D0h (" + port_error + ")");
            if (mp) {
                check(mp->port_fdc() && mp->port_fdc()->base() == 0xD0 && mp->disk(0).loaded(),
                      "disco por portas: disco A: montado na controladora por portas");
                check(mp->fdc() == nullptr && mp->memory().Describe(3, 1).kind == MEMMAP_KIND_ROM,
                      "disco por portas: sem controladora por memoria; a ROM do driver fica em 3:1");
            }

            MachineConfig fmt_cfg = config;
            fmt_cfg.disk_format = fdc::DiskFormat::Ds35Dd720;
            fmt_cfg.disk_access = machine::DiskAccess::Port;
            fmt_cfg.disk_a = disk360;
            std::string fmt_error;
            check(Machine::Create(fmt_cfg, fmt_error) == nullptr && Contains(fmt_error, "720"),
                  "formato 3 1/2 DS (720 KB) recusa imagem de 360 KB (" + fmt_error + ")");

            MachineConfig bad_port = config;
            bad_port.disk_access = machine::DiskAccess::Port;
            bad_port.disk_port = 0x98;   // VDP
            bad_port.disk_a = disk720;
            std::string bp_error;
            check(Machine::Create(bad_port, bp_error) == nullptr && Contains(bp_error, "ja"),
                  "base de disco em cima do VDP (98h) e' recusada (" + bp_error + ")");

            // Boot do MSX-DOS pelas portas com o driver real DDX 3.0 (a ROM nao e' do repositorio:
            // o teste so' roda se ela estiver em dist/roms). O disco de teste e' uma copia.
            const std::string ddx = std::string(FWMSX_SOURCE_DIR) + "/dist/roms/filehunter/15-08-2026/extensions/ddx_3.0.rom";
            const std::string dos_src = std::string(FWMSX_SOURCE_DIR) + "/msxdos1.dsk";
            std::ifstream ddx_check(ddx, std::ios::binary), dos_check(dos_src, std::ios::binary);
            if (!ddx_check || !dos_check) {
                std::printf("[SKIP] boot por portas: driver DDX 3.0 ou msxdos1.dsk nao encontrado\n");
            } else {
                const std::string dos_copy = TempPath("fwmsx_ddx_boot.dsk");
                {
                    std::ifstream in(dos_src, std::ios::binary);
                    std::ofstream out(dos_copy, std::ios::binary | std::ios::trunc);
                    out << in.rdbuf();
                }
                MachineConfig boot = config;
                boot.disk_access = machine::DiskAccess::Port;
                boot.disk_rom_path = ddx;
                boot.disk_a = dos_copy;
                std::string boot_error;
                std::unique_ptr<Machine> mb = Machine::Create(boot, boot_error);
                check(mb != nullptr, "boot por portas: a maquina monta com o driver DDX 3.0 (" + boot_error + ")");
                if (mb) {
                    Frames(*mb, 1500);
                    check(VramHas(*mb, "DDX-DRIVE") && VramHas(*mb, "MSX-DOS version 1.8"),
                          "boot por portas: o driver DDX 3.0 aparece e o MSX-DOS 1.8 sobe pelas portas D0h");
                }
                std::remove(dos_copy.c_str());
            }
            std::remove(disk720.c_str());
            std::remove(disk360.c_str());
        }

        // --- 4. RenderFrame ------------------------------------------------------
        std::vector<uint32_t> rgba;
        m->RenderFrame(rgba);
        check(rgba.size() == 272u * 228u, "RenderFrame: 272x228 pixels (256x192 de tela + borda do fMSX)");
        const VdpState &v = m->vdp_state();
        const uint8_t bg = v.regs[7] & 0x0F;
        const uint32_t bg_px = 0xFF000000u | (static_cast<uint32_t>(v.palette_b[bg]) << 16) | (static_cast<uint32_t>(v.palette_g[bg]) << 8) | v.palette_r[bg];
        check(v.scr_mode == 0 && rgba[0] == bg_px && rgba[255] == bg_px, "SCREEN 0 (240px): as bordas laterais de 8px sao a cor de fundo");
        size_t not_bg = 0;
        for (uint32_t px : rgba) not_bg += (px != bg_px);
        check(not_bg > 500 && not_bg < rgba.size() / 2, "o texto do prompt desenha pixels de frente (" + std::to_string(not_bg) + " pixels)");
        check((rgba[0] >> 24) == 0xFF, "alfa opaco (formato R,G,B,A direto para o OpenGL)");

        // --- 5. Teclado: digitar um comando BASIC ------------------------------
        check(!m->KeyDown("naoexiste") && !m->KeyUp("naoexiste") && m->KeyDown("a") && m->KeyUp("a"), "KeyDown/KeyUp: nome invalido = false");
        Type(*m, "print 1234|"); // sem CAPS o MSX digita minusculas
        Frames(*m, 30);
        check(VramHas(*m, "print 1234") && VramHas(*m, " 1234 "), "teclado: 'print 1234' + ENTER e' executado pelo BASIC (resultado na tela)");

        // --- 5b. Save-state: grava, mutila a maquina de proposito, recarrega -------
        {
            const std::string state_path = TempPath("fwmsx_machine_test.sst");
            const uint16_t pc_before = m->cpu().state().pc.w;
            const uint8_t psg_r7_before = m->psg().state().r[7];
            const std::vector<uint8_t> vram_before(m->vdp_state().vram, m->vdp_state().vram + VDP_VRAM_SIZE);

            std::string serr;
            check(m->SaveState(state_path, serr), "SaveState(): grava com sucesso (" + serr + ")");

            // Mutila a maquina de proposito -- se o teste passar depois, e' porque
            // LoadState() de fato restaurou, nao porque "ja' estava assim".
            Frames(*m, 300);
            Type(*m, "cls|");
            Frames(*m, 30);
            m->cpu().state().pc.w = 0x1234;
            m->psg().state().r[7] = 0x00;
            check(m->cpu().state().pc.w != pc_before && !VramHas(*m, "print 1234"),
                  "(mutilacao de proposito: PC e a tela mudaram antes de recarregar)");

            check(m->LoadState(state_path, serr), "LoadState(): recarrega com sucesso (" + serr + ")");
            check(m->cpu().state().pc.w == pc_before, "LoadState(): PC do Z80 restaurado");
            check(m->psg().state().r[7] == psg_r7_before, "LoadState(): registrador do PSG restaurado (R7)");
            check(std::equal(vram_before.begin(), vram_before.end(), m->vdp_state().vram),
                  "LoadState(): VRAM inteira (incl. o texto 'print 1234' na tela) restaurada byte a byte");

            // Fingerprint da midia (CRC32 de BIOS/cartucho): mesma midia = sem aviso; midia
            // diferente = estado carregado MESMO ASSIM, mas com state_warning() preenchido.
            check(m->state_warning().empty(), "LoadState(): mesma BIOS/cartucho -> sem aviso de midia");
            {
                std::string bytes;
                {
                    std::ifstream in(state_path, std::ios::binary);
                    bytes.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
                }
                const size_t meda = bytes.find("MEDA");
                check(meda != std::string::npos, "SaveState(): grava a secao MEDA (CRC32 da midia)");
                if (meda != std::string::npos) {
                    bytes[meda + 8] = static_cast<char>(bytes[meda + 8] ^ 0x5A); // CRC32 da BIOS
                    const std::string other_path = TempPath("fwmsx_machine_test_other.sst");
                    {
                        std::ofstream out(other_path, std::ios::binary | std::ios::trunc);
                        out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
                    }
                    check(m->LoadState(other_path, serr) && Contains(m->state_warning(), "BIOS"),
                          "LoadState(): BIOS diferente da do save -> carrega, mas com aviso");
                    std::remove(other_path.c_str());
                }
            }

            // Arquivo inexistente / corrompido: erro claro, nunca crash.
            check(!m->LoadState("/nao/existe/estado.sst", serr) && !serr.empty(), "LoadState(): arquivo inexistente falha com mensagem");
            const std::string garbage_path = TempPath("fwmsx_machine_test_garbage.sst");
            {
                std::ofstream g(garbage_path, std::ios::binary);
                g << "isso nao e' um estado do fwMSX";
            }
            check(!m->LoadState(garbage_path, serr) && Contains(serr, "nao e'"), "LoadState(): arquivo sem a assinatura certa e' recusado");
            std::remove(garbage_path.c_str());
            std::remove(state_path.c_str());
        }

        // --- 6. Reset ----------------------------------------------------------------
        const uint64_t before = m->frame_count();
        m->Reset();
        Frames(*m, 250);
        check(m->frame_count() == before + 250 && VramHas(*m, "MSX BASIC version 1.0") && !VramHas(*m, "print 1234"),
              "Reset(): reinicia a maquina -- o banner volta e o comando antigo some da tela");

        // --- 7. Cartucho de ROM plana ----------------------------------------------
        // Cabecalho "AB" em 4000h com INIT em 4010h: LD A,42h / LD (E000h),A / RET.
        std::vector<uint8_t> cart(0x4000, 0);
        cart[0] = 'A';
        cart[1] = 'B';
        cart[2] = 0x10;
        cart[3] = 0x40;
        const uint8_t init[] = {0x3E, 0x42, 0x32, 0x00, 0xE0, 0xC9};
        std::copy(init, init + sizeof(init), cart.begin() + 0x10);
        const std::string cart_path = TempPath("fwmsx_machine_test.rom");
        {
            std::ofstream f(cart_path, std::ios::binary);
            f.write(reinterpret_cast<const char *>(cart.data()), static_cast<std::streamsize>(cart.size()));
        }
        check(m->memory().PeekSlot(3, 2, 0xE000) != 0x42, "sem cartucho: o byte de teste em E000h nao esta' setado");

        MachineConfig with_cart = config;
        with_cart.cart_path = cart_path;
        std::unique_ptr<Machine> mc = Machine::Create(with_cart, error);
        check(mc != nullptr, "Create() com cartucho de 16KB: " + error);
        if (mc) {
            Frames(*mc, 250);
            check(mc->memory().PeekSlot(1, 0, 0x4000) == 'A' && mc->memory().PeekSlot(1, 0, 0x4001) == 'B',
                  "cartucho: cabecalho AB visivel em 4000h do slot 1");
            check(mc->memory().PeekSlot(3, 2, 0xE000) == 0x42, "cartucho: a BIOS achou o cabecalho AB e chamou o INIT (E000h = 42h)");
            check(VramHas(*mc, "MSX BASIC version 1.0"), "com cartucho sem TEXT, a BIOS segue para o BASIC");
        }

        // --- 7b. Joystick visto pelo programa do cartucho --------------------------
        // INIT: R15 = 00h (porta A, linhas ligadas) ; OUT (A0h),14 ; IN A,(A2h) ; LD (E001h),A;
        //       LD A,15 / OUT (A0h),A ; LD A,40h / OUT (A1h),A (R15 bit 6 -> porta B);
        //       LD A,14 / OUT (A0h),A ; IN A,(A2h) ; LD (E002h),A ; RET
        std::vector<uint8_t> joy_cart(0x4000, 0);
        joy_cart[0] = 'A';
        joy_cart[1] = 'B';
        joy_cart[2] = 0x10;
        joy_cart[3] = 0x40;
        const uint8_t joy_init[] = {0x3E, 0x0F, 0xD3, 0xA0, 0xAF, 0xD3, 0xA1,                         // R15 = 00h
                                    0x3E, 0x0E, 0xD3, 0xA0, 0xDB, 0xA2, 0x32, 0x01, 0xE0,             // le R14 -> E001h
                                    0x3E, 0x0F, 0xD3, 0xA0, 0x3E, 0x40, 0xD3, 0xA1,                   // R15 = 40h
                                    0x3E, 0x0E, 0xD3, 0xA0, 0xDB, 0xA2, 0x32, 0x02, 0xE0, 0xC9};      // le R14 -> E002h
        std::copy(joy_init, joy_init + sizeof(joy_init), joy_cart.begin() + 0x10);
        const std::string joy_path = TempPath("fwmsx_machine_joy.rom");
        {
            std::ofstream f(joy_path, std::ios::binary);
            f.write(reinterpret_cast<const char *>(joy_cart.data()), static_cast<std::streamsize>(joy_cart.size()));
        }
        MachineConfig joy_config = config;
        joy_config.cart_path = joy_path;
        {
            std::unique_ptr<Machine> jm = Machine::Create(joy_config, error);
            if (jm) {
                // Joystick A: cima + fogo A; joystick B: esquerda -- antes da BIOS chamar o INIT.
                jm->SetJoystick(0, PSG_JOY_UP | PSG_JOY_FIRE_A);
                jm->SetJoystick(1, PSG_JOY_LEFT);
                Frames(*jm, 250);
                check(jm->memory().PeekSlot(3, 2, 0xE001) == 0x7F - 0x01 - 0x10, "joystick A: o programa do cartucho le cima+fogo A em R14 (" +
                                                                               std::to_string(jm->memory().PeekSlot(3, 2, 0xE001)) + ")");
                check(jm->memory().PeekSlot(3, 2, 0xE002) == 0x7F - 0x04, "joystick B (R15 bit 6): o programa le esquerda (" +
                                                                         std::to_string(jm->memory().PeekSlot(3, 2, 0xE002)) + ")");
            } else {
                check(false, "Create() do cartucho de joystick: " + error);
            }
        }
        std::remove(joy_path.c_str());

        // --- 7c. Disco: MSX-DOS 1 de verdade, pela controladora WD2793 ---------------
        check(!m->has_disk_interface(), "sem --disk a maquina nao tem interface de disquete (BASIC puro)");
        {
            std::string derr;
            check(!m->InsertDisk(0, "x.dsk", derr) && Contains(derr, "interface"), "InsertDisk sem interface de disquete: erro explicando");
        }
        {
            MachineConfig dc = config;
            dc.disk_interface = true; // so' a interface, sem disco
            std::unique_ptr<Machine> dm = Machine::Create(dc, error);
            if (dm) {
                Frames(*dm, 300);
                check(dm->has_disk_interface() && VramHas(*dm, "Enter date (M-D-Y):"), "DISK.ROM em 3:1 sem disco: o kernel de disco pergunta a data");
                Type(*dm, "|");
                Frames(*dm, 200);
                check(VramHas(*dm, "Disk BASIC version 1.0"), "depois do ENTER: o MSX BASIC vira o Disk BASIC");
                std::string derr;
                check(!dm->InsertDisk(0, "/nao/existe.dsk", derr) && !derr.empty() && !dm->disk(0).loaded(), "InsertDisk de arquivo inexistente: erro e drive continua vazio");
            } else {
                check(false, "Create() so' com a interface de disco: " + error);
            }
        }
        const std::string dos_src = std::string(FWMSX_SOURCE_DIR) + "/msxdos1.dsk";
        const std::string dos_tmp = TempPath("fwmsx_machine_dos.dsk");
        std::vector<uint8_t> dos_original;
        {
            std::ifstream in(dos_src, std::ios::binary);
            dos_original.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
            std::ofstream out(dos_tmp, std::ios::binary);
            out.write(reinterpret_cast<const char *>(dos_original.data()), static_cast<std::streamsize>(dos_original.size()));
        }
        if (dos_original.size() != 737280) {
            std::printf("[SKIP] msxdos1.dsk nao encontrado em '%s'\n", dos_src.c_str());
        } else {
            MachineConfig dc = config;
            dc.disk_a = dos_tmp;
            std::unique_ptr<Machine> dm = Machine::Create(dc, error);
            if (!dm) {
                check(false, "Create() com disco: " + error);
            } else {
                check(dm->disk(0).loaded() && dm->disk(0).disk()->sides == 2 && dm->disk(0).disk()->tracks == 80, "disco A: 720KB (80 trilhas, 2 lados) carregado");
                Frames(*dm, 500);
                check(VramHas(*dm, "MSX-DOS version 1.8") && VramHas(*dm, "COMMAND version 1.12"),
                      "boot do disco: o MSX-DOS 1.8 carrega MSXDOS.SYS e COMMAND.COM (leitura de setores pela controladora)");
                check(VramHas(*dm, "Enter new date:"), "o COMMAND.COM pergunta a data");
                Type(*dm, "|dir|");
                Frames(*dm, 120);
                check(VramHas(*dm, "A>dir") && VramHas(*dm, "COMMAND  COM") && VramHas(*dm, "MSXDOS   SYS") && VramHas(*dm, "2 files"),
                      "dir: lista COMMAND.COM e MSXDOS.SYS e '2 files'");

                // Escrita: copy cria um arquivo novo; confere o ARQUIVO .dsk.
                Type(*dm, "copy command.com x.com|");
                Frames(*dm, 900);
                check(VramHas(*dm, "1 file copied"), "copy command.com x.com: 'file copied' na tela");
                std::ifstream in(dos_tmp, std::ios::binary);
                std::vector<uint8_t> after((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
                check(after.size() == dos_original.size() && after != dos_original, "o arquivo .dsk foi alterado pelas escritas do MSX-DOS");
                const std::string entry = std::string("X       COM");
                size_t dir_pos = std::string::npos;
                for (size_t i = 0; i + 32 <= after.size() && dir_pos == std::string::npos; i += 32)
                    if (std::equal(entry.begin(), entry.end(), after.begin() + static_cast<long>(i))) dir_pos = i;
                const uint32_t size_field = dir_pos == std::string::npos
                                                ? 0
                                                : (after[dir_pos + 28] | (after[dir_pos + 29] << 8) | (after[dir_pos + 30] << 16) | (after[dir_pos + 31] << 24));
                check(dir_pos != std::string::npos && size_field == 7168, "diretorio do .dsk: X.COM com 7168 bytes (" + std::to_string(size_field) + ")");
                // Integridade: os 14 setores de COMMAND.COM (7168 bytes) agora existem duas vezes no disco.
                size_t twice = 0;
                for (size_t a = 0; a + 512 <= after.size(); a += 512) {
                    if (std::all_of(after.begin() + static_cast<long>(a), after.begin() + static_cast<long>(a) + 512, [](uint8_t v) { return v == 0; })) continue;
                    size_t copies = 0;
                    for (size_t b = 0; b + 512 <= after.size(); b += 512)
                        if (std::equal(after.begin() + static_cast<long>(a), after.begin() + static_cast<long>(a) + 512, after.begin() + static_cast<long>(b))) ++copies;
                    // setores de dados duplicados (so' conta os de COMMAND.COM: nao-nulos e que aparecem 2 vezes)
                    if (copies == 2 && a >= 10 * 512) ++twice;
                }
                check(twice >= 28, "integridade: os setores de dados de COMMAND.COM aparecem em dobro, byte a byte (" + std::to_string(twice) + " ocorrencias)");

                // Reset de maquina: reinicia pelo disco de novo.
                dm->Reset();
                Frames(*dm, 500);
                check(VramHas(*dm, "MSX-DOS version 1.8"), "Reset(): a maquina reinicia pelo disco");
                dm->EjectDisk(0);
                check(!dm->disk(0).loaded(), "EjectDisk(): o drive fica vazio");
                std::string ierr;
                check(dm->InsertDisk(0, dos_tmp, ierr) && dm->disk(0).loaded(), "InsertDisk(): reinsere o disco");
            }
        }
        std::remove(dos_tmp.c_str());

        // --- 7d. Disco somente leitura (--disk-ro): o MSX-DOS le, recusa gravar, o arquivo nao muda
        {
            std::ofstream out(dos_tmp, std::ios::binary);
            out.write(reinterpret_cast<const char *>(dos_original.data()), static_cast<std::streamsize>(dos_original.size()));
        }
        MachineConfig ro = config;
        ro.disk_a = dos_tmp;
        ro.disk_read_only = true;
        std::unique_ptr<Machine> rm = Machine::Create(ro, error);
        if (!rm) {
            check(false, "Create() com disco somente leitura: " + error);
        } else {
            check(rm->disk(0).loaded() && rm->disk(0).disk()->write_protected == 1, "disk_read_only: o disco A: entra protegido contra gravacao");
            Frames(*rm, 500);
            check(VramHas(*rm, "COMMAND version 1.12"), "disco somente leitura: o MSX-DOS boota e le normalmente");
            Type(*rm, "|copy command.com x.com|");
            Frames(*rm, 600);
            check(VramHas(*rm, "Write protect error writing drive A"), "copy em disco protegido: 'Write protect error writing drive A'");
            std::ifstream in(dos_tmp, std::ios::binary);
            std::vector<uint8_t> after((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            check(after == dos_original, "o arquivo .dsk ficou byte a byte identico (nenhuma gravacao)");
            std::string ierr;
            rm->EjectDisk(0);
            check(rm->InsertDisk(0, dos_tmp, ierr) && rm->disk(0).disk()->write_protected == 1, "InsertDisk() depois (menu) tambem entra protegido");
        }
        std::remove(dos_tmp.c_str());

        MachineConfig big = config;
        std::vector<uint8_t> huge(0x10000, 0);
        const std::string big_path = TempPath("fwmsx_machine_big.rom");
        {
            std::ofstream f(big_path, std::ios::binary);
            f.write(reinterpret_cast<const char *>(huge.data()), static_cast<std::streamsize>(huge.size()));
        }
        big.cart_path = big_path;
        error.clear();
        std::unique_ptr<Machine> guessed = Machine::Create(big, error);
        check(guessed != nullptr && Contains(guessed->cart_info(), "64KB") && Contains(guessed->cart_info(), "detectado"),
              "ROM > 32KB sem mapper: o mapper e' detectado (" + (guessed ? guessed->cart_info() : error) + ")");
        MachineConfig mega = big;
        mega.cart_mapper = MEMMAP_MAPPER_ASCII8;
        check(Machine::Create(mega, error) != nullptr, "o mesmo arquivo com mapper ascii8 e' aceito como MegaROM");
        MachineConfig nofile = config;
        nofile.cart_path = "/nao/existe.rom";
        error.clear();
        check(Machine::Create(nofile, error) == nullptr && !error.empty(), "cartucho inexistente: erro");
        std::remove(cart_path.c_str());
        std::remove(big_path.c_str());
    }
#endif

    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
