// Teste do mapa de memoria MSX (slots/subslots, Fase 1) -- ver
// doc/memory-map-spec.md, secao 6. Foco no requisito "vital" do autor:
// o depurador precisa enxergar qualquer combinacao de slot, nao so o que
// esta visivel para a CPU no momento.
#include <cstdio>
#include <string>
#include <vector>

#include "../../src/memmap/cpp/memory_system.h"
#include "../../src/memmap/cpp/slot_memory_bus.h"
#include "../../src/z80/debug/flat_memory_bus.h"
#include "../../src/z80/debug/z80_debug_session.h"

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

} // namespace

int main() {
    // --- 1. O teste central: PeekSlot enxerga uma combinacao que NAO --
    //     esta na vista ativa da CPU, com conteudo diferente do que
    //     IBus::read ve. Isso e' o que prova que a visibilidade "por
    //     fora" de fato funciona, nao so que a troca de slot muda a
    //     vista da CPU. ---------------------------------------------------
    {
        memmap::MemorySystem mem;
        mem.AllocateRam(0, 0, 0x4000);
        mem.AllocateRam(1, 0, 0x4000);
        mem.PokeSlot(0, 0, 0x0000, 0xAA);
        mem.PokeSlot(1, 0, 0x0000, 0xBB);

        memmap::SlotMemoryBus bus(mem);

        // psl_reg=0 (default apos construcao) -> pagina 0 mostra slot 0:0.
        check(bus.read(0x0000) == 0xAA, "vista ativa inicial (slot 0:0) le 0xAA");
        check(mem.PeekSlot(1, 0, 0x0000) == 0xBB,
              "PeekSlot no slot 1:0 (fora da vista ativa) le 0xBB -- nao afetado pelo que a CPU enxerga");

        // Troca o slot primario da pagina 0 para 1 (bits 0-1 do valor
        // escrito na porta A8h -- ver memmap_switch_primary()).
        bus.out(0xA8, 0x01);
        check(bus.read(0x0000) == 0xBB, "apos trocar pagina 0 para slot 1:0, vista ativa le 0xBB");
        check(mem.PeekSlot(0, 0, 0x0000) == 0xAA,
              "PeekSlot no slot 0:0 (agora fora da vista ativa) continua lendo 0xAA -- "
              "o requisito vital: nada muda 'por baixo' so porque a CPU trocou de slot");
    }

    // --- 2. Round-trip Poke/Peek, incluindo um slot ainda vazio --------
    {
        memmap::MemorySystem mem;
        mem.AllocateRam(2, 3, 0x2000);
        mem.PokeSlot(2, 3, 0x0010, 0x42);
        check(mem.PeekSlot(2, 3, 0x0010) == 0x42, "PokeSlot/PeekSlot: round-trip em slot 2:3");

        check(mem.PeekSlot(3, 3, 0x0000) == 0xFF,
              "PeekSlot num slot ainda vazio (3:3) devolve 0xFF (mesma convencao NORAM do fMSX)");
        mem.PokeSlot(3, 3, 0x0000, 0x99); // deve ser descartado silenciosamente, sem crash
        check(mem.PeekSlot(3, 3, 0x0000) == 0xFF, "PokeSlot num slot vazio e' descartado (continua 0xFF)");
    }

    // --- 3. Describe(): Empty vs Ram ------------------------------------
    {
        memmap::MemorySystem mem;
        check(mem.Describe(0, 0).kind == MEMMAP_KIND_EMPTY, "Describe: slot 0:0 comeca vazio");
        mem.AllocateRam(0, 0, 0x8000);
        check(mem.Describe(0, 0).kind == MEMMAP_KIND_RAM, "Describe: slot 0:0 vira RAM apos AllocateRam");
        check(mem.Describe(0, 0).size == 0x8000, "Describe: tamanho reportado bate com o alocado (0x8000)");
        check(mem.Describe(1, 1).kind == MEMMAP_KIND_EMPTY, "Describe: slot 1:1 (nunca tocado) continua vazio");
    }

    // --- 4. CurrentView() reflete o estado apos troca de slot -----------
    {
        memmap::MemorySystem mem;
        mem.AllocateRam(0, 0, 0x10000);
        mem.AllocateRam(2, 0, 0x10000);
        memmap::SlotMemoryBus bus(mem);

        auto view = mem.CurrentView();
        check(view[0].primary == 0 && view[0].secondary == 0, "CurrentView inicial: pagina 0 = slot 0:0");
        check(view[0].writable, "CurrentView inicial: pagina 0 e' gravavel (RAM alocada)");

        bus.out(0xA8, 0x02); // pagina 0 -> slot 2 (bits 0-1 = 10b = 2)
        view = mem.CurrentView();
        check(view[0].primary == 2, "CurrentView apos troca: pagina 0 = slot 2:x");
    }

    // --- 5. Quirk do registrador de slot secundario: um por SLOT --------
    //     PRIMARIO, nao um registrador global unico. -----------------------
    {
        memmap::MemorySystem mem;
        mem.AllocateRam(0, 0, 0x10000);
        mem.AllocateRam(0, 1, 0x10000);
        mem.AllocateRam(1, 0, 0x10000);
        mem.AllocateRam(1, 2, 0x10000);
        mem.PokeSlot(0, 1, 0x0000, 0x11);
        mem.PokeSlot(1, 2, 0x0000, 0x22);

        memmap::SlotMemoryBus bus(mem);

        // Poe o slot primario 0 em TODAS as paginas e troca o subslot da
        // PAGINA 0 (endereco 0x0000, que le os bits 0-1 do valor escrito
        // em FFFFh -- ver memmap_switch_secondary, bit_shift = pagina*2)
        // para 1.
        bus.out(0xA8, 0x00); // psl_reg=0x00 -> psl[pagina]=0 para as 4 paginas
        bus.write(0xFFFF, 0x01); // ssl_reg[0] = 0x01 -> ssl[0] = 0x01&3 = 1
        check(bus.read(0x0000) == 0x11, "slot primario 0, apos FFFFh<-0x01: pagina 0 mostra subslot 1 (0x11)");

        // Agora troca o slot primario pra 1 em todas as paginas e grava
        // um valor DIFERENTE em FFFFh -- isso deve afetar so
        // ssl_reg[1], sem mexer no ssl_reg[0] que acabamos de gravar.
        bus.out(0xA8, 0x55); // 0b01010101 -> psl[pagina]=1 para as 4 paginas
        bus.write(0xFFFF, 0x02); // ssl_reg[1] = 0x02 -> ssl[0] = 0x02&3 = 2
        check(bus.read(0x0000) == 0x22, "slot primario 1, apos FFFFh<-0x02: pagina 0 mostra subslot 2 (0x22)");

        // Volta pro slot primario 0 -- o subslot escolhido antes (1) tem
        // que continuar valendo, provando que ssl_reg[0] nao foi
        // sobrescrito pela escrita em FFFFh feita enquanto o slot
        // primario era 1.
        bus.out(0xA8, 0x00);
        check(bus.read(0x0000) == 0x11,
              "volta pro slot primario 0: subslot 1 continua valendo (ssl_reg e' por slot primario, nao global)");
    }

    // --- 6. ram_ptr nunca atravessa fronteira de chunk de 8KB -----------
    {
        memmap::MemorySystem mem;
        mem.AllocateRam(0, 0, 0x10000); // um unico buffer contiguo de 64KB
        memmap::SlotMemoryBus bus(mem);

        check(bus.ram_ptr(0x1000, 0x100) != nullptr,
              "ram_ptr: intervalo dentro de um unico chunk (0x1000..0x10FF) devolve ponteiro");
        check(bus.ram_ptr(0x1FFE, 4) == nullptr,
              "ram_ptr: intervalo atravessando a fronteira 0x2000 (0x1FFE..0x2001) devolve nullptr, "
              "mesmo com os dois chunks vindo do MESMO buffer alocado");
        check(bus.ram_ptr(0x2000, 0x100) != nullptr, "ram_ptr: intervalo no chunk seguinte, sozinho, funciona");
    }

    // --- 7. Sessao de depuracao: comandos slots/pages/slotmem/slotpeek/ --
    //     slotpoke, incluindo slotmem enxergando algo diferente de mem/
    //     peek (a vista da CPU) no mesmo endereco. --------------------------
    {
        memmap::MemorySystem mem;
        mem.AllocateRam(0, 0, 0x10000);
        mem.AllocateRam(1, 0, 0x10000);
        mem.PokeSlot(0, 0, 0x5000, 0x10);
        mem.PokeSlot(1, 0, 0x5000, 0x20);

        memmap::SlotMemoryBus bus(mem);
        z80::debug::Z80DebugSession session(bus, &mem);

        const std::string slots = session.ProcessCommand({"slots"});
        check(slots.find("0:0 -> RAM") != std::string::npos, "comando 'slots': lista slot 0:0 como RAM");
        check(slots.find("2:0 -> vazio") != std::string::npos, "comando 'slots': lista slot 2:0 como vazio");

        const std::string pages = session.ProcessCommand({"pages"});
        check(pages.find("slot 0:0") != std::string::npos, "comando 'pages': mostra slot 0:0 visivel por padrao");

        const std::string peek_cpu = session.ProcessCommand({"peek", "0x5000"});
        check(peek_cpu.find("10") != std::string::npos, "peek (vista da CPU) em 0x5000 mostra 0x10 (slot 0:0 ativo)");

        const std::string slotmem_other = session.ProcessCommand({"slotmem", "1", "0", "0x5000", "1"});
        check(slotmem_other.find("20") != std::string::npos,
              "slotmem no slot 1:0 mostra 0x20, DIFERENTE do que 'peek' (vista da CPU) mostrou -- "
              "o comando central do requisito vital");

        const std::string slotpoke_result = session.ProcessCommand({"slotpoke", "1", "0", "0x6000", "0x77"});
        check(slotpoke_result.find("77") != std::string::npos, "slotpoke: confirma o valor escrito");
        const std::string slotpeek_result = session.ProcessCommand({"slotpeek", "1", "0", "0x6000"});
        check(slotpeek_result.find("77") != std::string::npos, "slotpeek: le de volta o que slotpoke escreveu");

    }

    // --- 8. Sem MemorySystem associado (sessao "--z80dbg" sem "--slots"), --
    //     os comandos de slot avisam em vez de travar. -----------------------
    {
        z80::debug::FlatMemoryBus flat_bus;
        z80::debug::Z80DebugSession session(flat_bus); // memory_system = nullptr (default)

        const std::string slots = session.ProcessCommand({"slots"});
        check(slots.find("--slots") != std::string::npos,
              "comando 'slots' sem MemorySystem: avisa que precisa de --slots, nao trava");
        const std::string slotpeek = session.ProcessCommand({"slotpeek", "0", "0", "0x0000"});
        check(slotpeek.find("--slots") != std::string::npos,
              "comando 'slotpeek' sem MemorySystem: mesma mensagem de aviso, nao trava");
    }

    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
