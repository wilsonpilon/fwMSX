// Teste da maquina MSX1 completa (src/machine/machine.{h,cpp}) -- ver
// doc/machine-spec.md. Roda tudo sem janela: a mesma Machine que a janela usa.
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
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

        MachineConfig big = config;
        std::vector<uint8_t> huge(0x10000, 0);
        const std::string big_path = TempPath("fwmsx_machine_big.rom");
        {
            std::ofstream f(big_path, std::ios::binary);
            f.write(reinterpret_cast<const char *>(huge.data()), static_cast<std::streamsize>(huge.size()));
        }
        big.cart_path = big_path;
        error.clear();
        check(Machine::Create(big, error) == nullptr && Contains(error, "32KB"), "ROM plana > 32KB sem mapper: erro explicando");
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
