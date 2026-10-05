// Teste de fumaca do nucleo Z80 (Fase 1) -- ver doc/z80-core-spec.md,
// secao 6. Roda pequenos trechos de codigo Z80 escritos a mao contra uma
// RAM plana de 64KB e confere registradores/memoria/flags esperados.
// Nao e' um test-suite de conformidade (tipo ZEXDOC) -- so o suficiente
// para pegar erros grosseiros de transcricao (uniao de par de registro,
// flags basicas, salto condicional, wraparound de 16 bits) antes de
// seguir para as proximas fases.
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>

#include "../../src/z80/common/z80_state.h"
#include "../../src/z80/core/z80_tables.h"
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
    // Quando false, ram_ptr() sempre devolve nullptr -- forca o core a
    // usar o loop byte-a-byte original mesmo tendo RAM plana disponivel.
    // Usado pelo teste diferencial de LDIR/LDDR (Fase 3) para comparar o
    // caminho rapido contra o lento a partir do MESMO estado inicial.
    bool allow_fast_path = true;

    uint8_t read(uint16_t addr) override { return ram[addr]; }
    void write(uint16_t addr, uint8_t value) override { ram[addr] = value; }
    uint8_t in(uint16_t) override { return 0; }
    void out(uint16_t, uint8_t) override {}

    uint8_t *ram_ptr(uint16_t addr, uint16_t len) override {
        if (!allow_fast_path) return nullptr;
        // addr+len pode estourar 16 bits (regiao cruzando 0xFFFF); nao
        // implementamos esse caso (recusamos o caminho rapido, o core cai
        // pro loop byte-a-byte) -- ver contrato em src/z80/core/z80_bus.h.
        if (static_cast<uint32_t>(addr) + len > 0x10000u) return nullptr;
        return &ram[addr];
    }
};

void load(FlatRamBus &bus, uint16_t addr, std::initializer_list<uint8_t> bytes) {
    uint16_t a = addr;
    for (uint8_t b : bytes) bus.ram[a++] = b;
}

// Recalcula o valor esperado de zs_table[i]/pzs_table[i] de forma
// TOTALMENTE independente da rotina Fortran (z80_build_flag_tables) e da
// antiga tabela literal em C -- uma terceira implementacao, em C++, bit a
// bit sem nenhum intrinseco de contagem de bits, para que um eventual
// erro conceitual compartilhado entre as outras duas nao passe
// despercebido. Ver doc/z80-core-spec.md, secao 3.5/6 (Fase 2).
uint8_t expected_zs(int i) {
    if (i == 0) return Z80_Z_FLAG;
    if (i & 0x80) return Z80_S_FLAG;
    return 0;
}

uint8_t expected_pzs(int i) {
    int ones = 0;
    for (int bit = 0; bit < 8; ++bit) {
        if (i & (1 << bit)) ++ones;
    }
    const bool even_parity = (ones % 2) == 0;
    return static_cast<uint8_t>(expected_zs(i) | (even_parity ? Z80_P_FLAG : 0));
}

// --- Teste diferencial LDIR/LDDR (Fase 3, caminho rapido em Assembly) --
//
// Monta um programinha LD HL,nn / LD DE,nn / LD BC,nn / <LDIR ou LDDR> /
// HALT e roda o MESMO programa, a partir do MESMO estado inicial de
// memoria, em duas instancias de Z80Cpu -- uma com o caminho rapido
// (bus->ram_ptr) habilitado, outra forcada a usar so o loop byte-a-byte
// original -- chamando run(budget) repetidamente com um orcamento de
// ciclos pequeno (para forcar o caminho rapido a parar no meio do bloco e
// retomar na chamada seguinte, exercitando o "rebobinamento" de PC, nao
// so o caso de completar tudo de uma vez). Ao final, os dois tem que
// bater em HL/DE/BC/AF (flags inclusas) e na RAM inteira.
struct BlockCase {
    uint16_t src;
    uint16_t dst;
    uint16_t len;
    int budget;
    bool reverse; // false = LDIR, true = LDDR
};

struct BlockResult {
    bool halted;
    uint16_t af, bc, de, hl;
};

BlockResult run_block_case_once(const BlockCase &c, bool allow_fast_path, FlatRamBus &bus,
                                 const std::array<uint8_t, 0x10000> &initial_ram) {
    bus.ram = initial_ram;
    bus.allow_fast_path = allow_fast_path;

    // LDIR usa HL/DE apontando pro INICIO da regiao; LDDR usa HL/DE
    // apontando pro FIM (ver semantica real do Z80 -- LDDR decrementa).
    const uint16_t hl_start = c.reverse ? static_cast<uint16_t>(c.src + c.len - 1) : c.src;
    const uint16_t de_start = c.reverse ? static_cast<uint16_t>(c.dst + c.len - 1) : c.dst;

    load(bus, 0x0000,
         {0x21, static_cast<uint8_t>(hl_start & 0xFF), static_cast<uint8_t>(hl_start >> 8), // LD HL,nn
          0x11, static_cast<uint8_t>(de_start & 0xFF), static_cast<uint8_t>(de_start >> 8), // LD DE,nn
          0x01, static_cast<uint8_t>(c.len & 0xFF), static_cast<uint8_t>(c.len >> 8),       // LD BC,nn
          0xED, static_cast<uint8_t>(c.reverse ? 0xB8 : 0xB0),                              // LDDR / LDIR
          0x76});                                                                            // HALT

    z80::Z80Cpu cpu(bus);
    cpu.reset();

    // Limite generoso de chamadas run(budget) -- cada byte custa no
    // maximo 21 ciclos no loop lento, entao (21*len + folga da parte
    // fixa do programa) / budget chamadas bastam de sobra; +64 de
    // margem de seguranca contra arredondamento.
    const int max_iters = (21 * static_cast<int>(c.len) + 64) / (c.budget > 0 ? c.budget : 1) + 64;
    for (int i = 0; i < max_iters && !(cpu.iff() & Z80_IFF_HALT); ++i) {
        cpu.run(c.budget);
    }

    BlockResult r;
    r.halted = (cpu.iff() & Z80_IFF_HALT) != 0;
    r.af = cpu.af();
    r.bc = cpu.bc();
    r.de = cpu.de();
    r.hl = cpu.hl();
    return r;
}

void run_block_case(const BlockCase &c, int case_index) {
    std::array<uint8_t, 0x10000> initial{};
    std::mt19937 rng(1000u + static_cast<unsigned>(case_index)); // seed fixo -- reprodutivel
    std::uniform_int_distribution<int> byte_dist(0, 255);

    // Preenche a regiao afetada (origem e destino, que podem se
    // sobrepor de proposito) com bytes aleatorios reprodutiveis; o resto
    // da RAM fica zerado nas duas instancias, entao comparar a RAM
    // inteira ao final pega qualquer vazamento fora do intervalo
    // esperado.
    const uint16_t lo = std::min(c.src, c.dst);
    const uint16_t hi = static_cast<uint16_t>(std::max(c.src, c.dst) + c.len);
    for (uint32_t a = lo; a < static_cast<uint32_t>(hi); ++a) {
        initial[a] = static_cast<uint8_t>(byte_dist(rng));
    }

    FlatRamBus bus_fast, bus_slow;
    const BlockResult fast = run_block_case_once(c, /*allow_fast_path=*/true, bus_fast, initial);
    const BlockResult slow = run_block_case_once(c, /*allow_fast_path=*/false, bus_slow, initial);

    char label[192];
    std::snprintf(label, sizeof(label),
                  "%s caso #%d (len=%u budget=%d src=%04X dst=%04X): caminho rapido == lento",
                  c.reverse ? "LDDR" : "LDIR", case_index, c.len, c.budget, c.src, c.dst);

    const bool ok = fast.halted && slow.halted && fast.af == slow.af && fast.bc == slow.bc &&
                    fast.de == slow.de && fast.hl == slow.hl && bus_fast.ram == bus_slow.ram;
    check(ok, label);
}

// --- Teste diferencial CPIR/CPDR (caminho rapido em Assembly vs. loop lento) --
//
// Mesma tecnica de run_block_case(): LD HL,nn / LD BC,nn / LD A,n / CPIR ou
// CPDR / HALT, rodado com e sem o caminho rapido, com orcamento pequeno (para
// forcar o rebobinamento de PC) ou grande. `present` escolhe se o valor
// procurado existe na regiao (e em qual posicao) ou nao existe de jeito nenhum.
struct SearchCase {
    uint16_t src;
    uint16_t len;
    int budget;
    bool reverse; // false = CPIR, true = CPDR
    bool present;
    uint16_t pos;  // posicao do valor procurado dentro da regiao, se present
};

void run_search_case(const SearchCase &c, int case_index) {
    std::array<uint8_t, 0x10000> initial{};
    std::mt19937 rng(7000u + static_cast<unsigned>(case_index));
    std::uniform_int_distribution<int> byte_dist(0, 255);
    for (uint32_t a = c.src; a < static_cast<uint32_t>(c.src) + c.len; ++a) {
        initial[a] = static_cast<uint8_t>(byte_dist(rng));
    }

    uint8_t value = 0;
    if (c.present) {
        value = initial[c.src + c.pos];
    } else {
        // Escolhe um valor que nao aparece na regiao (len < 256 garante que existe).
        for (int v = 0; v < 256; ++v) {
            bool seen = false;
            for (uint32_t a = c.src; a < static_cast<uint32_t>(c.src) + c.len && !seen; ++a) seen = initial[a] == v;
            if (!seen) { value = static_cast<uint8_t>(v); break; }
        }
    }

    const uint16_t hl_start = c.reverse ? static_cast<uint16_t>(c.src + c.len - 1) : c.src;
    auto run_once = [&](bool allow_fast, FlatRamBus &bus) {
        bus.ram = initial;
        bus.allow_fast_path = allow_fast;
        load(bus, 0x0000,
             {0x21, static_cast<uint8_t>(hl_start & 0xFF), static_cast<uint8_t>(hl_start >> 8), // LD HL,nn
              0x01, static_cast<uint8_t>(c.len & 0xFF), static_cast<uint8_t>(c.len >> 8),       // LD BC,nn
              0x3E, value,                                                                      // LD A,n
              0xED, static_cast<uint8_t>(c.reverse ? 0xB9 : 0xB1),                              // CPDR / CPIR
              0x76});                                                                           // HALT
        z80::Z80Cpu cpu(bus);
        cpu.reset();
        const int max_iters = (21 * static_cast<int>(c.len) + 64) / (c.budget > 0 ? c.budget : 1) + 64;
        for (int i = 0; i < max_iters && !(cpu.iff() & Z80_IFF_HALT); ++i) cpu.run(c.budget);
        struct Out { bool halted; uint16_t af, bc, hl; };
        return Out{(cpu.iff() & Z80_IFF_HALT) != 0, cpu.af(), cpu.bc(), cpu.hl()};
    };

    FlatRamBus bus_fast, bus_slow;
    const auto fast = run_once(true, bus_fast);
    const auto slow = run_once(false, bus_slow);

    char label[200];
    std::snprintf(label, sizeof(label),
                  "%s caso (len=%u budget=%d src=%04X present=%d pos=%u): caminho rapido == lento",
                  c.reverse ? "CPDR" : "CPIR", c.len, c.budget, c.src, c.present ? 1 : 0, c.pos);
    const bool ok = fast.halted && slow.halted && fast.af == slow.af && fast.bc == slow.bc &&
                    fast.hl == slow.hl && bus_fast.ram == bus_slow.ram;
    check(ok, label);
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

    // --- Teste 4: tabelas de flag geradas em Fortran (Fase 2) ----------
    // cpu.reset() acima ja' disparou z80_tables_init() (via z80_reset())
    // pelo menos uma vez -- confere as 256 posicoes de g_z80_zs_table/
    // g_z80_pzs_table contra um calculo independente (ver expected_zs/
    // expected_pzs acima).
    {
        bool zs_ok = true, pzs_ok = true;
        for (int i = 0; i < 256; ++i) {
            if (g_z80_zs_table[i] != expected_zs(i)) zs_ok = false;
            if (g_z80_pzs_table[i] != expected_pzs(i)) pzs_ok = false;
        }
        check(zs_ok, "Fortran z80_build_flag_tables: g_z80_zs_table bate com calculo independente (256/256)");
        check(pzs_ok, "Fortran z80_build_flag_tables: g_z80_pzs_table bate com calculo independente (256/256)");
    }

    // --- Teste 5: LDIR/LDDR, caminho rapido (Assembly) vs. lento -------
    // (Fase 3) -- ver run_block_case acima para o metodo. Casos fixos
    // cobrindo os cenarios exigidos pelo design (doc/z80-core-spec.md,
    // secao 6): completar em uma unica chamada de run(), completar so
    // depois de varias chamadas com orcamento minusculo (forcando o
    // rebobinamento de PC no meio do bloco), len==1, e as duas direcoes.
    int case_index = 0;
    run_block_case({0x2000, 0x2100, 1, 1000, false}, case_index++);   // LDIR, len=1, 1 chamada
    run_block_case({0x2000, 0x2100, 1, 1000, true}, case_index++);    // LDDR, len=1, 1 chamada
    run_block_case({0x2000, 0x2100, 200, 1000, false}, case_index++); // LDIR, completa numa chamada
    run_block_case({0x2000, 0x2100, 200, 1000, true}, case_index++);  // LDDR, completa numa chamada
    run_block_case({0x2000, 0x2100, 200, 5, false}, case_index++);    // LDIR, orcamento minusculo -> varias chamadas
    run_block_case({0x2000, 0x2100, 200, 5, true}, case_index++);     // LDDR, orcamento minusculo -> varias chamadas
    run_block_case({0x2000, 0x2010, 64, 21, false}, case_index++);    // LDIR sobreposto (dst > src, smear)
    run_block_case({0x2010, 0x2000, 64, 21, true}, case_index++);     // LDDR sobreposto

    // Varredura aleatoria (semente fixa -- reprodutivel) por cima dos
    // casos fixos acima, para cobertura mais ampla de combinacoes de
    // tamanho/orcamento/enderecos/sobreposicao.
    std::mt19937 case_rng(42);
    std::uniform_int_distribution<int> len_dist(1, 300);
    std::uniform_int_distribution<int> budget_dist(1, 60);
    std::uniform_int_distribution<int> addr_dist(0x1000, 0x8000);
    std::uniform_int_distribution<int> offset_dist(-40, 300); // pode gerar sobreposicao
    std::uniform_int_distribution<int> bool_dist(0, 1);
    for (int i = 0; i < 150; ++i) {
        const uint16_t len = static_cast<uint16_t>(len_dist(case_rng));
        const int budget = budget_dist(case_rng);
        const bool reverse = bool_dist(case_rng) != 0;
        const uint16_t src = static_cast<uint16_t>(addr_dist(case_rng));
        uint16_t dst = static_cast<uint16_t>(static_cast<int>(src) + offset_dist(case_rng));
        // Mantem dentro de um intervalo seguro de 64KB (longe de 0x0000,
        // onde mora o programa de teste, e sem estourar 0xFFFF).
        if (dst < 0x0800) dst = static_cast<uint16_t>(dst + 0x1000);
        if (static_cast<uint32_t>(dst) + len > 0xF000u) dst = static_cast<uint16_t>(0xF000 - len);
        run_block_case({src, dst, len, budget, reverse}, case_index++);
    }

    // --- Teste 6: CPIR/CPDR, caminho rapido (Assembly) vs. lento ----------
    int search_index = 0;
    run_search_case({0x2000, 1, 1000, false, true, 0}, search_index++);     // CPIR, 1 byte, achou
    run_search_case({0x2000, 1, 1000, true, false, 0}, search_index++);     // CPDR, 1 byte, nao achou
    run_search_case({0x2000, 200, 1000, false, true, 0}, search_index++);   // CPIR, acha no 1o byte
    run_search_case({0x2000, 200, 1000, false, true, 199}, search_index++); // CPIR, acha no ultimo byte
    run_search_case({0x2000, 200, 1000, false, false, 0}, search_index++);  // CPIR, nao acha: BC zera
    run_search_case({0x2000, 200, 1000, true, true, 0}, search_index++);    // CPDR, acha no ultimo (1o pelo topo)
    run_search_case({0x2000, 200, 1000, true, true, 199}, search_index++);  // CPDR, acha no 1o byte
    run_search_case({0x2000, 200, 1000, true, false, 0}, search_index++);   // CPDR, nao acha
    run_search_case({0x2000, 200, 21, false, true, 150}, search_index++);   // CPIR, orcamento minusculo
    run_search_case({0x2000, 200, 21, true, true, 50}, search_index++);     // CPDR, orcamento minusculo
    run_search_case({0x2000, 200, 5, false, false, 0}, search_index++);     // CPIR, orcamento < 21 ciclos
    run_search_case({0x2000, 200, 5, true, true, 120}, search_index++);     // CPDR, orcamento < 21 ciclos

    // Varredura aleatoria (semente fixa), mesma ideia de run_block_case.
    std::mt19937 search_rng(4242);
    std::uniform_int_distribution<int> s_len_dist(1, 200);
    std::uniform_int_distribution<int> s_budget_dist(1, 120);
    std::uniform_int_distribution<int> s_addr_dist(0x1000, 0x8000);
    std::uniform_int_distribution<int> s_bool_dist(0, 1);
    for (int i = 0; i < 120; ++i) {
        const uint16_t len = static_cast<uint16_t>(s_len_dist(search_rng));
        const bool present = s_bool_dist(search_rng) != 0;
        const uint16_t pos = static_cast<uint16_t>(std::uniform_int_distribution<int>(0, len - 1)(search_rng));
        run_search_case({static_cast<uint16_t>(s_addr_dist(search_rng)), len, s_budget_dist(search_rng),
                         s_bool_dist(search_rng) != 0, present, pos},
                        search_index++);
    }

    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
