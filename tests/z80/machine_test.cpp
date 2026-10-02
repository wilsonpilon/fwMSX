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

        // --- 4. RenderFrame ------------------------------------------------------
        std::vector<uint32_t> rgba;
        m->RenderFrame(rgba);
        check(rgba.size() == 256u * 192u, "RenderFrame: 256x192 pixels");
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
