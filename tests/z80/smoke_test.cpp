// Teste de fumaca do nucleo Z80 (Fase 1) -- ver doc/z80-core-spec.md,
// secao 6. Roda pequenos trechos de codigo Z80 escritos a mao contra uma
// RAM plana de 64KB e confere registradores/memoria/flags esperados.
// Nao e' um test-suite de conformidade (tipo ZEXDOC) -- so o suficiente
// para pegar erros grosseiros de transcricao (uniao de par de registro,
// flags basicas, salto condicional, wraparound de 16 bits) antes de
// seguir para as proximas fases.
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "../../src/z80/common/z80_state.h"
#include "../../src/z80/cpp/z80_bus.h"
#include "../../src/z80/cpp/z80_cpu.h"

namespace {

int g_failures = 0;

void check(bool cond, const char *what) {
    if (cond) {
        std::printf("[PASS] %s\n", what);
    } else {
        std::printf("[FAIL] %s\n", what);
        ++g_failures;
    }
}

class FlatRamBus : public z80::IBus {
public:
    std::array<uint8_t, 0x10000> ram{};

    uint8_t read(uint16_t addr) override { return ram[addr]; }
    void write(uint16_t addr, uint8_t value) override { ram[addr] = value; }
    uint8_t in(uint16_t) override { return 0; }
    void out(uint16_t, uint8_t) override {}
};

void load(FlatRamBus &bus, uint16_t addr, std::initializer_list<uint8_t> bytes) {
    uint16_t a = addr;
    for (uint8_t b : bytes) bus.ram[a++] = b;
}

} // namespace

int main() {
    FlatRamBus bus;
    z80::Z80Cpu cpu(bus);

    // --- Teste 1: aritmetica de 8 bits + armazenamento em memoria -----
    // LD A,5 / LD B,3 / ADD A,B / LD (1000H),A / HALT
    load(bus, 0x0000, {0x3E, 0x05, 0x06, 0x03, 0x80, 0x32, 0x00, 0x10, 0x76});
    cpu.reset();
    cpu.run(100);

    check((cpu.af() >> 8) == 8, "ADD A,B: A = 5 + 3 = 8");
    check(bus.ram[0x1000] == 8, "LD (1000H),A: memoria recebeu o valor de A");
    check(!(cpu.af() & Z80_Z_FLAG), "ADD A,B: Z_FLAG limpa (resultado != 0)");
    check(!(cpu.af() & Z80_S_FLAG), "ADD A,B: S_FLAG limpa (resultado positivo)");
    check((cpu.iff() & Z80_IFF_HALT) != 0, "HALT: flip-flop de HALT setado");

    // --- Teste 2a: JR NZ nao tomado (Z_FLAG setada) --------------------
    // XOR A / JR NZ,+5 / LD A,7 / HALT
    load(bus, 0x2000, {0xAF, 0x20, 0x05, 0x3E, 0x07, 0x76});
    cpu.reset();
    cpu.set_pc(0x2000);
    cpu.run(100);
    check((cpu.af() >> 8) == 7, "JR NZ nao tomado: cai no fallthrough (A = 7)");

    // --- Teste 2b: JR NZ tomado (Z_FLAG limpa) -------------------------
    // LD A,1 / OR A / JR NZ,+2 / LD A,0FFH (pulado) / LD A,9 / HALT
    load(bus, 0x3000, {0x3E, 0x01, 0xB7, 0x20, 0x02, 0x3E, 0xFF, 0x3E, 0x09, 0x76});
    cpu.reset();
    cpu.set_pc(0x3000);
    cpu.run(100);
    check((cpu.af() >> 8) == 9, "JR NZ tomado: pula o LD A,0FFH e cai em LD A,9");

    // --- Teste 3: wraparound de 16 bits (INC HL 0xFFFF -> 0x0000) ------
    load(bus, 0x4000, {0x21, 0xFF, 0xFF, 0x23, 0x76});
    cpu.reset();
    cpu.set_pc(0x4000);
    cpu.run(100);
    check(cpu.hl() == 0x0000, "INC HL: 0xFFFF + 1 = 0x0000 (wraparound)");

    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
