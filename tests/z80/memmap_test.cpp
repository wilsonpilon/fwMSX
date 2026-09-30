// Teste do mapa de memoria MSX (slots/subslots, Fase 1) -- ver
// doc/memory-map-spec.md, secao 6. Foco no requisito "vital" do autor:
// o depurador precisa enxergar qualquer combinacao de slot, nao so o que
// esta visivel para a CPU no momento.
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "../../src/memmap/cpp/memory_system.h"
#include "../../src/memmap/cpp/slot_memory_bus.h"
#include "../../src/z80/common/z80_state.h"
#include "../../src/z80/cpp/z80_cpu.h"
#include "../../src/z80/debug/flat_memory_bus.h"
#include "../../src/z80/debug/z80_debug_session.h"

// Implementado em src/memmap/fortran/rom_checksum.f90 -- chamado direto
// aqui (Secao 9) pra isolar a funcao do resto de LoadRom().
extern "C" void rom_crc32(const uint8_t *data, int32_t length, uint32_t *crc_out);

namespace {

int g_failures = 0;

// CRC32 de referencia, SEM tabela (bit a bit) -- deliberadamente uma
// implementacao diferente da de rom_checksum.f90 (que usa tabela de 256
// entradas), pra que um erro compartilhado entre as duas nao passe
// despercebido (mesmo principio de "recomputo independente" usado no
// teste das tabelas de flag do nucleo Z80, Fase 2).
uint32_t ReferenceCrc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320u : (crc >> 1);
        }
    }
    return ~crc;
}

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

    // --- 9. CRC32 (Fase 2): vetor de teste padrao -----------------------
    //     "123456789" -> 0xCBF43926 e' o vetor que qualquer implementacao
    //     de CRC32 (zlib/PKZIP/Ethernet) e' checada contra -- chamado
    //     direto aqui (isolado de LoadRom()). ------------------------------
    {
        const char *vector = "123456789";
        uint32_t crc = 0;
        rom_crc32(reinterpret_cast<const uint8_t *>(vector), 9, &crc);
        check(crc == 0xCBF43926u, "CRC32: vetor de teste padrao '123456789' == 0xCBF43926");
    }

    // --- 10. LoadRom com dados sinteticos em tamanhos validos (8/32/64KB) --
    //      Describe() deve reportar Kind::Rom, o tamanho certo, e um CRC32
    //      que bate com uma implementacao INDEPENDENTE (ReferenceCrc32,
    //      sem tabela) sobre os MESMOS bytes. -------------------------------
    {
        const std::size_t sizes[] = {0x2000, 0x8000, 0x10000};
        int primary = 0;
        for (std::size_t size : sizes) {
            std::vector<uint8_t> data(size);
            for (std::size_t i = 0; i < size; ++i) data[i] = static_cast<uint8_t>((i * 37 + 11) & 0xFF);

            memmap::MemorySystem mem;
            std::string error;
            const bool ok = mem.LoadRom(primary, 0, data.data(), data.size(), &error);
            check(ok, "LoadRom aceita tamanho valido " + std::to_string(size) + " bytes");

            const memmap::SlotDescriptor desc = mem.Describe(primary, 0);
            check(desc.kind == MEMMAP_KIND_ROM, "Describe apos LoadRom(" + std::to_string(size) + "): kind == Rom");
            check(desc.size == size, "Describe apos LoadRom(" + std::to_string(size) + "): tamanho bate");
            const uint32_t expected = ReferenceCrc32(data.data(), data.size());
            check(desc.crc32 == expected, "Describe apos LoadRom(" + std::to_string(size) +
                                               "): CRC32 bate com implementacao independente (sem tabela)");
            ++primary;
        }
    }

    // --- 11. LoadRom rejeita tamanho invalido (nao multiplo de 8KB) -------
    {
        memmap::MemorySystem mem;
        std::vector<uint8_t> data(10000, 0xAA);
        std::string error;
        const bool ok = mem.LoadRom(3, 0, data.data(), data.size(), &error);
        check(!ok, "LoadRom rejeita tamanho 10000 (nao multiplo de 0x2000)");
        check(!error.empty(), "LoadRom preenche mensagem de erro para tamanho invalido");
        const memmap::SlotDescriptor desc = mem.Describe(3, 0);
        check(desc.kind == MEMMAP_KIND_EMPTY, "combinacao 3:0 continua vazia apos LoadRom invalido (sem escrita parcial)");
    }

    // --- 12. Permissao de escrita: ROM e' somente-leitura nos DOIS --------
    //      caminhos -- PokeSlot() (API do depurador) E SlotMemoryBus::write
    //      (o caminho que o Z80 usaria) -- Fase 1 mexeu nos dois via
    //      active_writable/chunk_writable, entao os dois precisam ser
    //      checados separadamente. ------------------------------------------
    {
        memmap::MemorySystem mem;
        std::vector<uint8_t> data(0x2000, 0x55);
        mem.LoadRom(0, 0, data.data(), data.size());

        mem.PokeSlot(0, 0, 0x0010, 0x99);
        check(mem.PeekSlot(0, 0, 0x0010) == 0x55, "PokeSlot numa ROM e' descartado (permanece 0x55)");

        memmap::SlotMemoryBus bus(mem);
        // psl_reg=0 por padrao -> pagina 0 (e' onde 0:0 esta' mapeado) mostra a ROM.
        bus.write(0x0020, 0x99);
        check(bus.read(0x0020) == 0x55, "SlotMemoryBus::write numa ROM (vista ativa da CPU) tambem e' descartado");
    }

    // --- 13. Comando 'loadrom' via Z80DebugSession::ProcessCommand --------
    {
        const std::string tmp_path = "memmap_test_loadrom_tmp.bin";
        std::vector<uint8_t> rom_bytes(0x2000);
        for (std::size_t i = 0; i < rom_bytes.size(); ++i) rom_bytes[i] = static_cast<uint8_t>((i * 7 + 3) & 0xFF);
        {
            std::ofstream out(tmp_path, std::ios::binary);
            out.write(reinterpret_cast<const char *>(rom_bytes.data()), static_cast<std::streamsize>(rom_bytes.size()));
        }

        memmap::MemorySystem mem;
        memmap::SlotMemoryBus bus(mem);
        z80::debug::Z80DebugSession session(bus, &mem);

        const std::string result = session.ProcessCommand({"loadrom", "2", "0", tmp_path});
        const uint32_t expected_crc = ReferenceCrc32(rom_bytes.data(), rom_bytes.size());
        char expected_hex[16];
        std::snprintf(expected_hex, sizeof(expected_hex), "%08X", expected_crc);
        check(result.find("8192") != std::string::npos, "loadrom: relata o tamanho carregado (8192 bytes)");
        check(result.find(expected_hex) != std::string::npos, "loadrom: relata o CRC32 correto no texto de resposta");

        const std::string slots = session.ProcessCommand({"slots"});
        check(slots.find("2:0 -> ROM") != std::string::npos, "'slots' apos loadrom mostra 2:0 como ROM");
        check(slots.find(expected_hex) != std::string::npos, "'slots' apos loadrom mostra o CRC32 correto");

        const std::string slotmem = session.ProcessCommand({"slotmem", "2", "0", "0x0000", "1"});
        char first_byte_hex[8];
        std::snprintf(first_byte_hex, sizeof(first_byte_hex), "%02X", rom_bytes[0]);
        check(slotmem.find(first_byte_hex) != std::string::npos, "'slotmem' apos loadrom reflete o conteudo carregado");

        const std::string missing = session.ProcessCommand({"loadrom", "1", "0", "arquivo_que_nao_existe.bin"});
        check(missing.find("loadrom") != std::string::npos && missing.find("nao foi possivel") != std::string::npos,
              "loadrom em arquivo inexistente reporta erro claro, nao trava");

        std::remove(tmp_path.c_str());
    }

    // --- 14. Aceitacao com BIOS real (resource/fMSX/ROMs/MSX.ROM) ----------
    //      So' LE o arquivo ja existente no repositorio (material de
    //      estudo/referencia, ver resource/README.md) -- nunca copia/
    //      redistribui. Resiliente a ausencia do arquivo (nao falha o
    //      suite, so avisa) -- ver doc/memory-map-spec.md, secao 5. -----------
    {
#ifndef FWMSX_SOURCE_DIR
#define FWMSX_SOURCE_DIR "."
#endif
        const std::string bios_path = std::string(FWMSX_SOURCE_DIR) + "/resource/fMSX/ROMs/MSX.ROM";
        std::ifstream bios_file(bios_path, std::ios::binary | std::ios::ate);
        if (!bios_file) {
            std::printf("[SKIP] teste de aceitacao com BIOS real: '%s' nao encontrado (nao e' um erro -- "
                        "material de terceiros, ver resource/README.md)\n",
                        bios_path.c_str());
        } else {
            const std::streamsize size = bios_file.tellg();
            bios_file.seekg(0, std::ios::beg);
            std::vector<uint8_t> bios(static_cast<std::size_t>(size));
            bios_file.read(reinterpret_cast<char *>(bios.data()), size);

            memmap::MemorySystem mem;
            std::string error;
            const bool loaded = mem.LoadRom(0, 0, bios.data(), bios.size(), &error);
            check(loaded, "BIOS real (MSX.ROM, " + std::to_string(size) + " bytes) carrega em 0:0 sem erro");

            memmap::SlotMemoryBus bus(mem);
            z80::Z80Cpu cpu(bus);
            cpu.reset();

            // A BIOS de reset do MSX comeca com um JP para o codigo de
            // inicializacao de verdade -- PC sai de 0x0000 quase
            // imediatamente. Rodamos um orcamento generoso de ciclos e
            // contamos quantos valores DISTINTOS de PC foram visitados --
            // um numero baixo (ex.: <=2) indicaria a CPU travada num loop
            // trivial ou parada logo de cara (opcode invalido, HALT
            // imediato); um numero alto e' evidencia concreta de que
            // instrucoes de verdade da BIOS estao sendo buscadas/
            // executadas em sequencia -- mais forte que so "nao lancou
            // excecao".
            std::vector<uint16_t> pc_history;
            pc_history.reserve(2048);
            int budget_left = 100000;
            while (budget_left > 0) {
                const int leftover = cpu.run(1);
                budget_left -= (1 - leftover);
                pc_history.push_back(cpu.pc());
                if (cpu.state().iff & Z80_IFF_HALT) break; // BIOS/teste parou de proposito (HALT)
            }
            std::sort(pc_history.begin(), pc_history.end());
            pc_history.erase(std::unique(pc_history.begin(), pc_history.end()), pc_history.end());

            check(cpu.pc() != 0x0000, "apos rodar a BIOS, PC nao esta mais parado em 0x0000 (reset vector)");
            check(pc_history.size() > 50, "BIOS real visitou mais de 50 enderecos de PC distintos em 100000 "
                                            "ciclos (" +
                                                std::to_string(pc_history.size()) +
                                                ") -- evidencia de execucao real, nao so nao-crash");
        }
    }

    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
