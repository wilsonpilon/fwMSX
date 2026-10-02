// Teste do PPI i8255 + teclado (portas A8h-ABh) -- ver doc/ppi-spec.md.
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "../../src/memmap/cpp/memory_system.h"
#include "../../src/memmap/cpp/slot_memory_bus.h"
#include "../../src/ppi/cpp/ppi_device.h"
#include "../../src/ppi/core/ppi_state.h"
#include "../../src/vdp/core/vdp_state.h"
#include "../../src/z80/common/z80_state.h"
#include "../../src/z80/cpp/composite_bus.h"
#include "../../src/z80/debug/z80_debug_session.h"
#include "../../src/z80/debug/z80_debug_shell_startup.h"

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

// Coloca o chip no modo que a BIOS do MSX programa: A saida, B entrada,
// C saida (controle 82h).
void ConfigureLikeBios(PpiState &p) { ppi_write(&p, 3, 0x82); }

bool KeyAt(const char *name, int expect_row, int expect_mask) {
    int row = -1;
    uint8_t mask = 0;
    const int id = ppi_key_lookup(name);
    return id != PPI_KEY_NONE && ppi_key_position(id, &row, &mask) && row == expect_row && mask == expect_mask;
}

} // namespace

int main() {
    // --- 1. Reset do chip ---------------------------------------------------
    {
        PpiState p;
        ppi_reset(&p);
        check(p.r[3] == 0x9B && p.r[0] == 0 && p.r[1] == 0 && p.r[2] == 0, "reset: controle 9Bh (tudo entrada), registradores zerados");
        bool all_released = true;
        for (int i = 0; i < PPI_KEY_ROWS; ++i) all_released = all_released && p.key_state[i] == 0xFF;
        check(all_released, "reset: nenhuma tecla pressionada (matriz toda 0xFF)");
    }

    // --- 2. Porta A so' dirige os pinos em modo saida -----------------------
    {
        PpiState p;
        ppi_reset(&p);
        ppi_write(&p, 0, 0x55);
        check(p.r[0] == 0x55 && p.rout[0] == 0x00, "porta A em modo ENTRADA (reset): escrita guarda o registrador mas os pinos ficam em 0");
        ConfigureLikeBios(p);
        check(p.rout[0] == 0x55, "controle 82h: porta A vira saida e o valor ja' escrito aparece nos pinos");
        ppi_write(&p, 0, 0xA4);
        check(p.rout[0] == 0xA4 && ppi_read(&p, 0) == 0xA4, "porta A em saida: pinos e leitura devolvem o ultimo valor escrito");
    }

    // --- 3. Controle: set/reset de um bit da porta C ------------------------
    {
        PpiState p;
        ppi_reset(&p);
        ConfigureLikeBios(p);
        ppi_write(&p, 2, 0x00);
        ppi_write(&p, 3, 0x0D); // 0000 110 1: bit 6 = 1 (LED de CAPS)
        check(p.r[2] == 0x40 && p.rout[2] == 0x40, "controle (bit 7=0): set do bit 6 da porta C");
        ppi_write(&p, 3, 0x0C); // bit 6 = 0
        check(p.r[2] == 0x00, "controle (bit 7=0): reset do bit 6 da porta C");
        check(p.r[3] == 0x82, "set/reset de bit nao altera o registrador de modo");
    }

    // --- 4. Teclado: leitura da linha selecionada por AAh -------------------
    {
        PpiState p;
        ppi_reset(&p);
        ConfigureLikeBios(p);
        ppi_write(&p, 2, 0x02); // linha 2
        check(ppi_read(&p, 1) == 0xFF, "sem tecla pressionada, a linha lida e' 0xFF");
        ppi_key_set(&p, ppi_key_lookup("a"), 1);
        check(ppi_read(&p, 1) == 0xBF, "'a' pressionada (linha 2, bit 6): A9h le 0xBF na linha 2");
        ppi_write(&p, 2, 0x03);
        check(ppi_read(&p, 1) == 0xFF, "a mesma tecla nao aparece na linha 3");
        ppi_write(&p, 2, 0x02 | 0x40); // linha 2 + LED de CAPS ligado (nibble alto ignorado na selecao)
        check(ppi_read(&p, 1) == 0xBF, "os bits 4-7 de AAh nao interferem na escolha da linha");
        ppi_key_set(&p, ppi_key_lookup("a"), 0);
        check(ppi_read(&p, 1) == 0xFF, "'a' solta: a linha volta a 0xFF");
        ppi_write(&p, 2, 0x0F);
        ppi_key_set(&p, ppi_key_lookup("shift"), 1);
        check(ppi_read(&p, 1) == 0xFF, "linhas 11-15 nao tem teclas: sempre 0xFF");
    }

    // --- 5. Tabela de teclas (Fortran): posicoes conhecidas -----------------
    {
        check(KeyAt("a", 2, 0x40) && KeyAt("b", 2, 0x80), "matriz: a/b na linha 2 (bits 6/7)");
        check(KeyAt("c", 3, 0x01) && KeyAt("j", 3, 0x80), "matriz: c..j na linha 3");
        check(KeyAt("k", 4, 0x01) && KeyAt("r", 4, 0x80), "matriz: k..r na linha 4");
        check(KeyAt("s", 5, 0x01) && KeyAt("z", 5, 0x80), "matriz: s..z na linha 5");
        check(KeyAt("0", 0, 0x01) && KeyAt("7", 0, 0x80) && KeyAt("8", 1, 0x01) && KeyAt("9", 1, 0x02),
              "matriz: digitos 0-7 na linha 0, 8-9 na linha 1");
        check(KeyAt("-", 1, 0x04) && KeyAt(";", 1, 0x80) && KeyAt("'", 2, 0x01) && KeyAt("/", 2, 0x10),
              "matriz: pontuacao (- ; ' /)");
        check(KeyAt("shift", 6, 0x01) && KeyAt("ctrl", 6, 0x02) && KeyAt("caps", 6, 0x08) && KeyAt("f3", 6, 0x80),
              "matriz: shift/ctrl/caps/f3 na linha 6");
        check(KeyAt("f4", 7, 0x01) && KeyAt("esc", 7, 0x04) && KeyAt("bs", 7, 0x20) && KeyAt("enter", 7, 0x80),
              "matriz: f4/esc/bs/enter na linha 7");
        check(KeyAt("space", 8, 0x01) && KeyAt("del", 8, 0x08) && KeyAt("left", 8, 0x10) && KeyAt("right", 8, 0x80),
              "matriz: space/del/left/right na linha 8");
        check(KeyAt("pad0", 9, 0x08) && KeyAt("pad4", 9, 0x80) && KeyAt("pad5", 10, 0x01) && KeyAt("pad9", 10, 0x10),
              "matriz: teclado numerico (pad0-pad9)");
        check(KeyAt("pad*", 9, 0x01) && KeyAt("pad/", 9, 0x04) && KeyAt("pad-", 10, 0x20) && KeyAt("pad.", 10, 0x80),
              "matriz: operadores do teclado numerico");

        // Invariante geral: toda tecla tem posicao unica, de 1 bit, linha <= 10.
        std::set<int> seen;
        bool ok = true;
        for (int id = 0; id < ppi_key_count(); ++id) {
            int row;
            uint8_t mask;
            if (!ppi_key_position(id, &row, &mask) || row < 0 || row >= PPI_KEY_MATRIX_ROWS || mask == 0 ||
                (mask & (mask - 1)) != 0 || !seen.insert(row * 256 + mask).second)
                ok = false;
        }
        check(ok && ppi_key_count() == 87, "todas as 87 teclas tem posicao unica na matriz (1 bit, linha 0-10)");
        check(ppi_key_lookup("SHIFT") == ppi_key_lookup("shift") && ppi_key_lookup("A") == ppi_key_lookup("a"),
              "nomes de tecla nao diferenciam maiusculas");
        check(ppi_key_lookup("xyz") == PPI_KEY_NONE && ppi_key_lookup("") == PPI_KEY_NONE &&
                  ppi_key_lookup(nullptr) == PPI_KEY_NONE,
              "tecla desconhecida / vazia / nula: PPI_KEY_NONE");
    }

    // --- 6. Contagem de teclas pressionadas (Assembly) ----------------------
    {
        PpiState p;
        ppi_reset(&p);
        check(ppi_pressed_count(p.key_state) == 0, "contagem (asm): 0 teclas com a matriz livre");
        ppi_key_set(&p, ppi_key_lookup("a"), 1);
        ppi_key_set(&p, ppi_key_lookup("shift"), 1);
        ppi_key_set(&p, ppi_key_lookup("pad.") , 1); // linha 10: a ultima contada
        check(ppi_pressed_count(p.key_state) == 3, "contagem (asm): 3 teclas pressionadas (linhas 2, 6 e 10)");
        ppi_key_set(&p, ppi_key_lookup("a"), 1);
        check(ppi_pressed_count(p.key_state) == 3, "contagem (asm): pressionar a mesma tecla de novo nao conta duas vezes");
        p.key_state[11] = 0x00; // fora da matriz real: nao deve entrar na conta
        check(ppi_pressed_count(p.key_state) == 3, "contagem (asm): linhas 11-15 sao ignoradas");
        ppi_key_release_all(&p);
        check(ppi_pressed_count(p.key_state) == 0, "release_all solta tudo");
        for (int i = 0; i < 11; ++i) p.key_state[i] = 0x00;
        check(ppi_pressed_count(p.key_state) == 88, "contagem (asm): 11 linhas x 8 bits = 88 (sem estourar o byte)");
    }

    // --- 7. PpiDevice + mapa de memoria: slot primario pelo PPI -------------
    {
        memmap::MemorySystem mem;
        mem.AllocateRam(0, 0, 0x10000);
        mem.AllocateRam(1, 0, 0x10000);
        mem.PokeSlot(0, 0, 0x8000, 0xA0);
        mem.PokeSlot(1, 0, 0x8000, 0xB1);
        memmap::SlotMemoryBus slot_bus(mem);
        ppi::PpiDevice ppi_dev(mem);
        z80::CompositeBus bus(slot_bus);
        bus.RegisterPortRange(0xA8, 0xAB, &ppi_dev);

        check(bus.read(0x8000) == 0xA0, "base: pagina 2 enxerga o slot 0");
        bus.out(0xA8, 0x20); // pagina 2 -> slot 2? (bits 5-4 = 10b) -- chip ainda em modo entrada
        check(bus.read(0x8000) == 0xA0, "porta A em modo entrada (antes da BIOS programar o chip): OUT A8h nao troca slot");
        bus.out(0xAB, 0x82);
        bus.out(0xA8, 0x10); // pagina 2 -> slot 1
        check(bus.read(0x8000) == 0xB1, "apos OUT ABh,82h: OUT A8h,10h coloca o slot 1 na pagina 2");
        check(bus.in(0xA8) == 0x10, "IN A8h devolve o valor escrito");
        bus.out(0xA8, 0x00);
        check(bus.read(0x8000) == 0xA0, "OUT A8h,00h volta ao slot 0");
        bus.out(0xA8, 0x10);
        ppi_dev.Reset();
        check(bus.read(0x8000) == 0xA0 && bus.in(0xAB) == 0x9B, "Reset(): volta ao slot 0 e ao modo de entrada (9Bh)");

        // Teclado pelo barramento.
        bus.out(0xAB, 0x82);
        bus.out(0xAA, 0x08); // linha 8
        ppi_key_set(&ppi_dev.state(), ppi_key_lookup("space"), 1);
        check(bus.in(0xA9) == 0xFE, "IN A9h na linha 8 com 'space' pressionada: 0xFE");
        bus.out(0xAB, 0x0D); // set bit 6 de C pelo controle
        check(bus.in(0xAA) == 0x48, "OUT ABh,0Dh liga o bit 6 de C sem perder a linha selecionada (IN AAh = 48h)");
        ppi_dev.Reset();
        check(ppi_dev.state().key_state[8] == 0xFE, "Reset() de maquina nao solta teclas pressionadas");
    }

    // --- 8. Sessao de depuracao: comandos de teclado ------------------------
    {
        z80::debug::Z80DebugShellStartup startup = z80::debug::BuildZ80DebugShellStartup({"--slots", "--ppi"});
        check(startup.use_slots && startup.use_ppi && startup.ppi_device && startup.composite_bus && !startup.use_vdp,
              "startup: '--slots --ppi' monta PPI + barramento composto (sem VDP)");
        z80::debug::Z80DebugSession session(startup.Bus(), startup.memory_system.get(), nullptr, startup.ppi_device.get());

        std::string r = session.ProcessCommand({"keydown", "shift", "a"});
        check(Contains(r, "shift pressionada") && Contains(r, "a pressionada"), "keydown shift a: confirma as duas teclas");
        r = session.ProcessCommand({"keys"});
        check(Contains(r, "2 tecla(s) pressionada(s): a shift") && Contains(r, "linha  2: 10111111"),
              "keys: matriz em binario + 2 teclas pressionadas (contagem vinda do Assembly)");
        r = session.ProcessCommand({"keyup", "a"});
        check(Contains(r, "a solta") && Contains(session.ProcessCommand({"keys"}), "1 tecla(s)"), "keyup a: solta so' a tecla pedida");
        check(Contains(session.ProcessCommand({"keydown", "naoexiste"}), "tecla desconhecida"), "keydown de tecla invalida: erro claro");
        check(Contains(session.ProcessCommand({"keyup", "all"}), "todas as teclas soltas") &&
                  Contains(session.ProcessCommand({"keys"}), "0 tecla(s)"),
              "keyup all: solta tudo");

        session.ProcessCommand({"keydown", "a"});
        r = session.ProcessCommand({"reset"});
        check(Contains(r, "PPI") && Contains(session.ProcessCommand({"keys"}), "1 tecla(s)"), "reset: reseta o PPI mas mantem as teclas");
        r = session.ProcessCommand({"ppiregs"});
        check(Contains(r, "Controle (ABh) = 0x9B") || Contains(r, "Controle (ABh) = 9B") || Contains(r, "9B"),
              "ppiregs: mostra o registrador de controle");
        check(Contains(r, "porta A ENTRADA"), "ppiregs: decodifica o modo da porta A");

        z80::debug::Z80DebugSession no_ppi(startup.Bus(), startup.memory_system.get());
        check(Contains(no_ppi.ProcessCommand({"keys"}), "requer") && Contains(no_ppi.ProcessCommand({"ppiregs"}), "requer"),
              "sem --ppi, os comandos de PPI pedem a flag em vez de travar");

        z80::debug::Z80DebugShellStartup bad = z80::debug::BuildZ80DebugShellStartup({"--ppi"});
        check(!bad.use_ppi && !bad.ppi_error.empty(), "'--ppi' sem '--slots': ignorado com aviso");
        z80::debug::Z80DebugShellStartup both = z80::debug::BuildZ80DebugShellStartup({"--slots", "--ppi", "--vdp"});
        check(both.use_ppi && both.use_vdp && both.ppi_device && both.vdp_device, "'--slots --ppi --vdp' liga os dois dispositivos");
    }

    // --- 8b. Regras de subslot do MSX1 (SSlot() do fMSX) -----------------------
    {
        memmap::MemorySystem mem;
        mem.AllocateRam(0, 0, 0x10000);
        mem.AllocateRam(3, 2, 0x10000);
        memmap::SlotMemoryBus bus(mem);

        bus.write(0xFFFF, 0xF0); // sem as regras: slot 0 aceita subslot livremente
        check(bus.read(0xFFFF) == 0x0F, "sem msx1_subslot_rules: escrita em FFFFh no slot 0 vale (leitura devolve o complemento)");
        bus.write(0xFFFF, 0x00);

        mem.state().msx1_subslot_rules = 1;
        bus.write(0xFFFF, 0xF0);
        check(bus.read(0xFFFF) == 0xFF, "msx1_subslot_rules: slot 0 nao tem subslot -- FFFFh continua lendo 0xFF (BIOS ve 'nao expandido')");
        bus.out(0xA8, 0xC0); // pagina 3 -> slot 3
        bus.write(0xFFFF, 0xF0);
        check(bus.read(0xFFFF) == 0x0F, "msx1_subslot_rules: o slot 3 continua expandido (FFFFh le o complemento do escrito)");
        bus.out(0xA8, 0x40); // pagina 3 -> slot 1 (cartucho)
        bus.write(0xFFFF, 0xF0);
        check(bus.read(0xFFFF) == 0xFF, "msx1_subslot_rules: slots de cartucho (1/2) nunca tem subslot");
    }

    // --- 8c. Startup: layout MSX1 padrao com BIOS + --ppi -------------------
    {
        const std::string rom_path = std::string(FWMSX_SOURCE_DIR) + "/resource/fMSX/ROMs/MSX.ROM";
        z80::debug::Z80DebugShellStartup s = z80::debug::BuildZ80DebugShellStartup({"--slots", rom_path, "--ppi"});
        if (!s.boot_rom_loaded) {
            std::printf("[SKIP] layout MSX1 com BIOS: '%s' nao encontrada\n", rom_path.c_str());
        } else {
            s.memory_system->PokeSlot(3, 2, 0xC000, 0x5A);
            check(s.memory_system->PeekSlot(3, 2, 0xC000) == 0x5A, "BIOS + --ppi: ha' RAM gravavel no slot 3:2 (layout MSX1 do fMSX)");
            check(s.memory_system->state().msx1_subslot_rules == 1, "BIOS + --ppi: regras de subslot MSX1 ligadas");
            z80::debug::Z80DebugShellStartup plain = z80::debug::BuildZ80DebugShellStartup({"--slots", rom_path});
            check(plain.memory_system->state().msx1_subslot_rules == 0, "BIOS sem --ppi: comportamento anterior preservado (regras desligadas)");
        }
    }

#ifdef FWMSX_SOURCE_DIR
    // --- 9. BIOS real: boot completo ate o prompt do MSX BASIC ----------------
    // Criterio de aceite de verdade (era informativo ate a v1.6.0): com PPI, RAM em
    // 3:2 e o nucleo Z80 inicializado de verdade (ver Z80Cpu::Z80Cpu), a BIOS MSX1
    // sobe, habilita o VBlank, escreve a tela de abertura do MSX BASIC na VRAM e
    // passa a ler o teclado pelo PPI.
    {
        const std::string rom = std::string(FWMSX_SOURCE_DIR) + "/resource/fMSX/ROMs/MSX.ROM";
        z80::debug::Z80DebugShellStartup s = z80::debug::BuildZ80DebugShellStartup({"--slots", rom, "--vdp", "--ppi"});
        if (!s.boot_rom_loaded) {
            std::printf("[SKIP] BIOS real: '%s' nao encontrada\n", rom.c_str());
        } else {
            z80::debug::Z80DebugSession session(s.Bus(), s.memory_system.get(), s.vdp_device.get(), s.ppi_device.get());
            auto run = [&](int cycles) {
                for (int done = 0; done < cycles; done += 100000) session.ProcessCommand({"run", "100000"});
            };
            auto vram_has = [&](const std::string &text) {
                const uint8_t *v = s.vdp_device->state().vram;
                for (int i = 0; i + static_cast<int>(text.size()) <= VDP_VRAM_SIZE; ++i)
                    if (std::equal(text.begin(), text.end(), v + i)) return true;
                return false;
            };

            run(100000000);
            const VdpState &vs = s.vdp_device->state();
            check((vs.regs[1] & 0x20) != 0 && (session.cpu().iff() & Z80_IFF_1) != 0,
                  "BIOS real: habilitou a interrupcao de VBlank do VDP (R#1 bit 5) e liga IFF1");
            check(vs.scr_mode == 0 && vram_has("MSX BASIC version 1.0") && vram_has("Bytes free"),
                  "BIOS real: a tela de abertura 'MSX BASIC version 1.0 ... Bytes free' esta na VRAM (SCREEN 0)");
            check(!vram_has("zzz"), "BIOS real: antes de qualquer tecla nao ha' 'zzz' na tela");

            session.ProcessCommand({"keydown", "z"});
            run(1500000);
            session.ProcessCommand({"keyup", "z"});
            run(2000000);
            check(vram_has("zz"), "BIOS real + PPI: 'keydown z' chega ao BASIC (a BIOS le a matriz de teclado e escreve 'z' na tela)");
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
