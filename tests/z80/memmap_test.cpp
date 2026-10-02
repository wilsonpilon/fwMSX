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
#include "../../src/memmap/cpp/rom_guess.h"
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

    // === Fase 3 -- MegaROM (bank-switch), ver doc/memory-map-spec.md, ======
    //     secao 6. Escopo: so a troca de banco de ROM de GEN8/GEN16/
    //     KONAMI5/KONAMI4/ASCII8/ASCII16 (sem SCC/SRAM/GMASTER2/FMPAC/
    //     MAP_GUESS -- ver a justificativa no design doc). ==================

    // Monta uma ROM sintetica de `banks` bancos de 8KB, cada um preenchido
    // com um byte distinto (0x10+indice do banco) -- basta ler o primeiro
    // byte de uma janela pra saber qual banco esta visivel ali agora.
    auto MakeBankedRom = [](int banks) {
        std::vector<uint8_t> rom(static_cast<size_t>(banks) * 0x2000);
        for (int b = 0; b < banks; ++b) {
            std::fill(rom.begin() + static_cast<long>(b) * 0x2000, rom.begin() + static_cast<long>(b + 1) * 0x2000,
                       static_cast<uint8_t>(0x10 + b));
        }
        return rom;
    };

    // Bank-switch so acontece atraves da VISTA ATIVA da CPU (memmap_write
    // deriva primary/secondary de psl[pagina]/ssl[pagina], nunca de qual
    // combinacao recebeu o LoadRom) -- entao todo teste de bank-switch
    // precisa primeiro tornar a combinacao carregada visivel em TODAS as
    // paginas, ou a escrita de troca de banco (via bus.write) acaba caindo
    // na combinacao 0:0 (a vista ativa default), nao na combinacao
    // pretendida. Helper: forca (primary,secondary) em todas as 4 paginas.
    auto SelectEverywhere = [](memmap::SlotMemoryBus &bus, int primary, int secondary) {
        const uint8_t p = static_cast<uint8_t>(primary | (primary << 2) | (primary << 4) | (primary << 6));
        bus.out(0xA8, p);
        const uint8_t s = static_cast<uint8_t>(secondary | (secondary << 2) | (secondary << 4) | (secondary << 6));
        bus.write(0xFFFF, s);
    };

    // --- 15. MAP_GEN8: qualquer endereco em 4000h-BFFFh escolhe o banco --
    //      do quarto correspondente (J=(A-4000h)>>13); escrita fora dessa
    //      faixa nao troca nada. ------------------------------------------
    {
        const std::vector<uint8_t> rom = MakeBankedRom(4); // mask=3
        memmap::MemorySystem mem;
        std::string error;
        check(mem.LoadRom(0, 0, rom.data(), rom.size(), &error, MEMMAP_MAPPER_GEN8), "GEN8: LoadRom aceita (" + error + ")");

        memmap::SlotMemoryBus bus(mem);
        bus.write(0x4000, 2); // quarto 0 (4000h-5FFFh) -> banco 2
        bus.write(0x6000, 1); // quarto 1 (6000h-7FFFh) -> banco 1
        bus.write(0x9FFF, 3); // quarto 2 (8000h-9FFFh) -> banco 3 (qualquer endereco na janela serve)
        bus.write(0xB000, 0); // quarto 3 (A000h-BFFFh) -> banco 0

        check(mem.PeekSlot(0, 0, 0x4000) == 0x12, "GEN8: quarto 0 mostra banco 2 apos escrita em 4000h");
        check(mem.PeekSlot(0, 0, 0x6000) == 0x11, "GEN8: quarto 1 mostra banco 1 apos escrita em 6000h");
        check(mem.PeekSlot(0, 0, 0x8000) == 0x13, "GEN8: quarto 2 mostra banco 3 apos escrita em 9FFFh");
        check(mem.PeekSlot(0, 0, 0xA000) == 0x10, "GEN8: quarto 3 mostra banco 0 apos escrita em B000h");

        // Negativo: escrita fora de 4000h-BFFFh nao afeta nada (cai no
        // descarte padrao) -- pedacos 0/1/6/7 continuam vazios (0xFF).
        bus.write(0x3FFF, 1);
        bus.write(0xC000, 1);
        check(mem.PeekSlot(0, 0, 0x0000) == MEMMAP_EMPTY_BYTE, "GEN8: escrita em 3FFFh nao cria conteudo em 0000h-3FFFh");
        check(mem.PeekSlot(0, 0, 0xC000) == MEMMAP_EMPTY_BYTE, "GEN8: escrita em C000h nao cria conteudo em C000h-FFFFh");
    }

    // --- 16. MAP_GEN16: J=(A&8000h)>>14 da' 0 OU 2 (granularidade de -----
    //      16KB, dois pedacos de 8KB trocados juntos). ---------------------
    {
        const std::vector<uint8_t> rom = MakeBankedRom(4); // mask=3 (2 "bancos" de 16KB: 0-1 e 2-3)
        memmap::MemorySystem mem;
        std::string error;
        check(mem.LoadRom(0, 1, rom.data(), rom.size(), &error, MEMMAP_MAPPER_GEN16), "GEN16: LoadRom aceita (" + error + ")");

        memmap::SlotMemoryBus bus(mem);
        SelectEverywhere(bus, 0, 1); // 0:1 nao e' a vista ativa default -- precisa disso antes de escrever
        bus.write(0x4000, 1); // metade baixa (4000h-7FFFh) -> banco 16KB #1 = bancos 8KB {2,3}
        check(mem.PeekSlot(0, 1, 0x4000) == 0x12, "GEN16: metade baixa mostra banco 8KB 2 apos escrita com V=1");
        check(mem.PeekSlot(0, 1, 0x6000) == 0x13, "GEN16: metade baixa (segundo pedaco) mostra banco 8KB 3");

        bus.write(0x9000, 0); // metade alta (8000h-BFFFh) -> banco 16KB #0 = bancos 8KB {0,1}
        check(mem.PeekSlot(0, 1, 0x8000) == 0x10, "GEN16: metade alta mostra banco 8KB 0 apos escrita com V=0");
        check(mem.PeekSlot(0, 1, 0xA000) == 0x11, "GEN16: metade alta (segundo pedaco) mostra banco 8KB 1");
    }

    // --- 17. MAP_KONAMI5: SO enderecos exatos 5000h/7000h/9000h/B000h; ---
    //      qualquer outro endereco (ex. 6000h) e' ignorado. ----------------
    {
        const std::vector<uint8_t> rom = MakeBankedRom(4);
        memmap::MemorySystem mem;
        std::string error;
        check(mem.LoadRom(1, 0, rom.data(), rom.size(), &error, MEMMAP_MAPPER_KONAMI5),
              "KONAMI5: LoadRom aceita (" + error + ")");

        memmap::SlotMemoryBus bus(mem);
        SelectEverywhere(bus, 1, 0);
        bus.write(0x5000, 2); // quarto 0 -> banco 2
        bus.write(0x7000, 3); // quarto 1 -> banco 3
        bus.write(0x9000, 1); // quarto 2 -> banco 1
        bus.write(0xB000, 0); // quarto 3 -> banco 0
        check(mem.PeekSlot(1, 0, 0x4000) == 0x12, "KONAMI5: 5000h troca quarto 0 para banco 2");
        check(mem.PeekSlot(1, 0, 0x6000) == 0x13, "KONAMI5: 7000h troca quarto 1 para banco 3");
        check(mem.PeekSlot(1, 0, 0x8000) == 0x11, "KONAMI5: 9000h troca quarto 2 para banco 1");
        check(mem.PeekSlot(1, 0, 0xA000) == 0x10, "KONAMI5: B000h troca quarto 3 para banco 0");

        // Negativo: 6000h nao e' um endereco de controle valido do KONAMI5
        // (nem 5000h/7000h/9000h/B000h) -- nao deve trocar nada.
        bus.write(0x6000, 3);
        check(mem.PeekSlot(1, 0, 0x6000) == 0x13,
              "KONAMI5: escrita em 6000h (endereco invalido) nao troca o quarto 1 (continua banco 3)");
    }

    // --- 18. MAP_KONAMI4: SO 6000h/8000h/A000h (8KB-alinhados); 4000h -----
    //      e' fixo (nao trocavel). ------------------------------------------
    {
        const std::vector<uint8_t> rom = MakeBankedRom(4);
        memmap::MemorySystem mem;
        std::string error;
        check(mem.LoadRom(1, 1, rom.data(), rom.size(), &error, MEMMAP_MAPPER_KONAMI4),
              "KONAMI4: LoadRom aceita (" + error + ")");

        memmap::SlotMemoryBus bus(mem);
        SelectEverywhere(bus, 1, 1);
        bus.write(0x6000, 2); // quarto 1 -> banco 2
        bus.write(0x8000, 3); // quarto 2 -> banco 3
        bus.write(0xA000, 1); // quarto 3 -> banco 1
        check(mem.PeekSlot(1, 1, 0x6000) == 0x12, "KONAMI4: 6000h troca quarto 1 para banco 2");
        check(mem.PeekSlot(1, 1, 0x8000) == 0x13, "KONAMI4: 8000h troca quarto 2 para banco 3");
        check(mem.PeekSlot(1, 1, 0xA000) == 0x11, "KONAMI4: A000h troca quarto 3 para banco 1");

        // Negativo: 4000h e' fixo -- continua banco 0 (estado inicial),
        // mesmo escrevendo la'.
        bus.write(0x4000, 2);
        check(mem.PeekSlot(1, 1, 0x4000) == 0x10, "KONAMI4: quarto 0 (4000h) e' fixo, escrita ali e' ignorada");
    }

    // --- 19. MAP_ASCII8: 4 "portas" em 6000h-7FFFh (6000/6800/7000/7800h) -
    //      escolhem os quartos 0/1/2/3; selecao de SRAM e' reconhecida mas
    //      ignorada (sem crash, sem corrupcao). -----------------------------
    {
        const std::vector<uint8_t> rom = MakeBankedRom(4); // mask=3, mask+1=4 = bit de SRAM
        memmap::MemorySystem mem;
        std::string error;
        check(mem.LoadRom(2, 0, rom.data(), rom.size(), &error, MEMMAP_MAPPER_ASCII8),
              "ASCII8: LoadRom aceita (" + error + ")");

        memmap::SlotMemoryBus bus(mem);
        SelectEverywhere(bus, 2, 0);
        bus.write(0x6000, 2); // porta 0 -> quarto 0
        bus.write(0x6800, 3); // porta 1 -> quarto 1
        bus.write(0x7000, 1); // porta 2 -> quarto 2
        bus.write(0x7800, 0); // porta 3 -> quarto 3
        check(mem.PeekSlot(2, 0, 0x4000) == 0x12, "ASCII8: porta 6000h troca quarto 0 para banco 2");
        check(mem.PeekSlot(2, 0, 0x6000) == 0x13, "ASCII8: porta 6800h troca quarto 1 para banco 3");
        check(mem.PeekSlot(2, 0, 0x8000) == 0x11, "ASCII8: porta 7000h troca quarto 2 para banco 1");
        check(mem.PeekSlot(2, 0, 0xA000) == 0x10, "ASCII8: porta 7800h troca quarto 3 para banco 0");

        // Selecao de SRAM (bit mask+1 = 0x04) reconhecida mas ignorada --
        // quarto 0 continua no banco 2 de antes, sem crash/corrupcao.
        bus.write(0x6000, 0x04);
        check(mem.PeekSlot(2, 0, 0x4000) == 0x12,
              "ASCII8: selecao de SRAM (V=04h) e' ignorada -- quarto 0 continua banco 2");
    }

    // --- 20. MAP_ASCII16: granularidade de 16KB; quirk de rejeicao de -----
    //      escrita "lixo" em endereco nao-alinhado a 4KB (Vauxall/etc.),
    //      portado tal qual do fMSX. -----------------------------------------
    {
        const std::vector<uint8_t> rom = MakeBankedRom(4); // mask=3, mask+1=4
        memmap::MemorySystem mem;
        std::string error;
        check(mem.LoadRom(2, 1, rom.data(), rom.size(), &error, MEMMAP_MAPPER_ASCII16),
              "ASCII16: LoadRom aceita (" + error + ")");

        memmap::SlotMemoryBus bus(mem);
        SelectEverywhere(bus, 2, 1);
        // V=1 (valido, <=mask+1) e' aceito em QUALQUER endereco da janela,
        // alinhado ou nao -- 6001h nao e' 4KB-alinhado, mas o valor e'
        // "plausivel" o bastante pra ser aceito (primeira clausula do OR).
        bus.write(0x6001, 1); // metade baixa -> banco 16KB #1 = bancos 8KB {2,3}
        check(mem.PeekSlot(2, 1, 0x4000) == 0x12, "ASCII16: V=1 em endereco nao-alinhado (6001h) e' aceito");

        // V=200 (bem maior que mask+1=4, "lixo") em endereco NAO-alinhado a
        // 4KB (6001h) e' REJEITADO -- nao muda nada (continua banco 2 de
        // antes).
        bus.write(0x6001, 200);
        check(mem.PeekSlot(2, 1, 0x4000) == 0x12,
              "ASCII16: V=200 (lixo) em endereco nao-alinhado (6001h) e' rejeitado, sem mudanca");

        // O MESMO V=200 numa posicao alinhada a 4KB DENTRO DA MESMA janela
        // de quarto (6000h, ainda quarto 0 -- bit12=0) e' ACEITO (segunda
        // clausula do OR) -- o valor e' entao mascarado pra um banco
        // valido (200<<1=400, 400&mask(3)=0). NAO usar 7000h aqui: 7000h
        // tem bit12=1, controla o OUTRO quarto (metade ALTA, 8000h-BFFFh)
        // -- protocolo real do ASCII16 (dois registradores de controle
        // distintos em 6000h/7000h, cada um pra uma metade diferente),
        // ver o teste em separado logo abaixo.
        bus.write(0x6000, 200);
        check(mem.PeekSlot(2, 1, 0x4000) == 0x10,
              "ASCII16: V=200 (lixo) em endereco alinhado a 4KB (6000h) e' aceito e mascarado para banco 0");

        // Selecao de SRAM (V=mask+1=4, alinhado ou nao) reconhecida mas
        // ignorada -- sem crash, sem mudanca de banco (continua o que o
        // 6000h deixou acima).
        bus.write(0x6000, 4);
        check(mem.PeekSlot(2, 1, 0x4000) == 0x10, "ASCII16: selecao de SRAM (V=04h) e' ignorada -- banco inalterado");

        // Registrador de controle da metade ALTA (7000h, bit12=1) --
        // protocolo real do ASCII16: 6000h e 7000h controlam metades
        // DIFERENTES, nao a mesma. V=1 -> banco 16KB #1 = bancos 8KB{2,3}.
        bus.write(0x7000, 1);
        check(mem.PeekSlot(2, 1, 0x8000) == 0x12, "ASCII16: 7000h controla a metade ALTA (8000h-BFFFh), nao a baixa");
        check(mem.PeekSlot(2, 1, 0x4000) == 0x10,
              "ASCII16: escrita em 7000h nao afeta a metade baixa (continua banco 0 de antes)");
    }

    // --- 21. Vista ativa da CPU precisa refletir a troca de banco NA HORA -
    //      (via SlotMemoryBus::write, como o Z80 faria de verdade), sem
    //      nenhum slot-switch adicional depois -- a sutileza central desta
    //      fase (ver doc/memory-map-spec.md, secao 6). --------------------
    {
        const std::vector<uint8_t> rom = MakeBankedRom(4);
        memmap::MemorySystem mem;
        std::string error;
        check(mem.LoadRom(1, 0, rom.data(), rom.size(), &error, MEMMAP_MAPPER_GEN8),
              "vista-ativa: LoadRom (GEN8) aceita (" + error + ")");

        memmap::SlotMemoryBus bus(mem);
        // Poe a combinacao 1:0 na pagina 1 (4000h-7FFFh) da vista ativa da
        // CPU: bits 2-3 do registrador de slot primario = 1 (demais
        // paginas continuam em 0). ssl_reg[1] comeca zerado, entao
        // secundario tambem fica 0 sem precisar escrever em FFFFh.
        bus.out(0xA8, 0x04);

        check(bus.read(0x4000) == 0x10, "vista-ativa: 1:0 visivel na pagina 1, banco inicial 0 (0x10)");

        // Escrita de troca de banco atraves do BARRAMENTO (como o Z80
        // faria) -- NAO por PeekSlot/PokeSlot, que sao so o backdoor do
        // depurador.
        bus.write(0x4000, 2);

        check(bus.read(0x4000) == 0x12,
              "vista-ativa: IBus::read reflete o novo banco IMEDIATAMENTE, sem troca de slot adicional");
        check(mem.PeekSlot(1, 0, 0x4000) == 0x12, "vista-ativa: PeekSlot concorda com o que a vista ativa mostra");
    }

    // --- 22. ROM plana (mapper=None, Fase 2) continua inalterada: escrita -
    //      num endereco tipico de controle de mapper NAO troca banco
    //      nenhum (nao ha' mapper pra trocar) -- so descarta, como antes. --
    {
        std::vector<uint8_t> rom(0x8000, 0x7A); // 32KB, plana, sem bank-switch
        memmap::MemorySystem mem;
        std::string error;
        check(mem.LoadRom(3, 3, rom.data(), rom.size(), &error), "ROM plana: LoadRom (mapper=None) aceita ainda");

        memmap::SlotMemoryBus bus(mem);
        SelectEverywhere(bus, 3, 3); // precisa estar na vista ativa, senao a escrita nem chega nessa combinacao
        bus.write(0x4000, 2); // pareceria um MAP_GEN8, mas essa combinacao nao tem mapper
        bus.write(0x7000, 3); // pareceria um MAP_KONAMI5
        check(mem.PeekSlot(3, 3, 0x4000) == 0x7A, "ROM plana: escrita em 4000h nao troca nada (mapper=None)");
        check(mem.PeekSlot(3, 3, 0x6000) == 0x7A, "ROM plana: escrita em 7000h nao troca nada (mapper=None)");
    }

    // --- 23. Comando 'loadrom' com mapper explicito + rejeicao de mapper --
    //      desconhecido. ---------------------------------------------------
    {
        const std::vector<uint8_t> rom = MakeBankedRom(4);
        const std::string tmp_path = "memmap_test_megarom.bin";
        {
            std::ofstream out(tmp_path, std::ios::binary);
            out.write(reinterpret_cast<const char *>(rom.data()), static_cast<std::streamsize>(rom.size()));
        }

        memmap::MemorySystem mem;
        memmap::SlotMemoryBus bus(mem);
        z80::debug::Z80DebugSession session(bus, &mem);

        const std::string ok = session.ProcessCommand({"loadrom", "0", "2", tmp_path, "gen8"});
        check(ok.find("mapper Gen8") != std::string::npos, "loadrom com mapper: resposta menciona 'mapper Gen8'");

        const std::string slots = session.ProcessCommand({"slots"});
        check(slots.find("0:2 -> ROM") != std::string::npos && slots.find("Gen8") != std::string::npos,
              "'slots' apos loadrom com mapper mostra 'Gen8' na descricao");

        const std::string bad = session.ProcessCommand({"loadrom", "1", "2", tmp_path, "bogus"});
        check(bad.find("mapper desconhecido") != std::string::npos,
              "loadrom com nome de mapper invalido reporta erro claro, nao trava");

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

    // === Estado inicial de MegaROM + deteccao de mapper (v1.10) =============
    // Uma MegaROM recem-carregada mostra os bancos 0,1,2,3 em 4000h/6000h/8000h/
    // A000h (SetMegaROM(J,0,1,2,3) do fMSX) -- varios jogos chamam rotinas em
    // 6000h-7FFFh antes de trocar qualquer banco.
    {
        auto banked = [](int banks) {
            std::vector<uint8_t> rom(static_cast<size_t>(banks) * 0x2000, 0);
            for (int b = 0; b < banks; ++b) rom[static_cast<size_t>(b) * 0x2000] = static_cast<uint8_t>(0x10 | b);
            return rom;
        };
        for (MemMapMapperType mapper : {MEMMAP_MAPPER_GEN8, MEMMAP_MAPPER_KONAMI4, MEMMAP_MAPPER_KONAMI5, MEMMAP_MAPPER_ASCII8,
                                        MEMMAP_MAPPER_GEN16, MEMMAP_MAPPER_ASCII16}) {
            const std::vector<uint8_t> rom = banked(8);
            memmap::MemorySystem mem;
            std::string error;
            mem.LoadRom(1, 0, rom.data(), rom.size(), &error, mapper);
            check(mem.PeekSlot(1, 0, 0x4000) == 0x10 && mem.PeekSlot(1, 0, 0x6000) == 0x11 && mem.PeekSlot(1, 0, 0x8000) == 0x12 &&
                      mem.PeekSlot(1, 0, 0xA000) == 0x13,
                  "MegaROM recem-carregada (mapper " + std::to_string(static_cast<int>(mapper)) + "): bancos iniciais 0,1,2,3 em 4000h/6000h/8000h/A000h");
        }
        const std::vector<uint8_t> small = banked(2); // 16KB: so' os bancos 0 e 1 existem
        memmap::MemorySystem mem;
        std::string error;
        mem.LoadRom(1, 0, small.data(), small.size(), &error, MEMMAP_MAPPER_KONAMI4);
        check(mem.PeekSlot(1, 0, 0x8000) == 0x10 && mem.PeekSlot(1, 0, 0xA000) == 0x11, "MegaROM de 2 bancos: os iniciais 2,3 sao mascarados para 0,1");
    }
    {
        // GuessMapper: conta LD (nnnn),A nos enderecos de registrador de cada mapper.
        auto with_writes = [](std::initializer_list<unsigned> addrs, int repeat) {
            std::vector<uint8_t> rom(0x20000, 0);
            size_t pos = 0x100;
            for (int r = 0; r < repeat; ++r)
                for (unsigned a : addrs) {
                    rom[pos++] = 0x32;
                    rom[pos++] = static_cast<uint8_t>(a & 0xFF);
                    rom[pos++] = static_cast<uint8_t>(a >> 8);
                }
            return rom;
        };
        auto guess = [&](std::initializer_list<unsigned> addrs) {
            const std::vector<uint8_t> rom = with_writes(addrs, 6);
            return memmap::GuessMapper(rom.data(), rom.size());
        };
        check(guess({0x5000, 0x7000, 0x9000, 0xB000}) == MEMMAP_MAPPER_KONAMI5, "GuessMapper: 5000h/7000h/9000h/B000h -> Konami5");
        check(guess({0x6000, 0x8000, 0xA000}) == MEMMAP_MAPPER_KONAMI4, "GuessMapper: 6000h/8000h/A000h -> Konami4");
        check(guess({0x6000, 0x6800, 0x7000, 0x7800}) == MEMMAP_MAPPER_ASCII8, "GuessMapper: 6000h/6800h/7000h/7800h -> ASCII8");
        check(guess({0x6000, 0x7000, 0x77FF}) == MEMMAP_MAPPER_ASCII16, "GuessMapper: 6000h/7000h/77FFh -> ASCII16");
        const std::vector<uint8_t> blank(0x20000, 0);
        check(memmap::GuessMapper(blank.data(), blank.size()) == MEMMAP_MAPPER_GEN8, "GuessMapper: sem pistas -> Gen8 (o padrao)");
    }

    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
