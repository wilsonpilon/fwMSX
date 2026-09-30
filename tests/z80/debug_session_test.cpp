// Teste da sessao de depuracao do nucleo Z80 (Fase 4) -- ver
// doc/z80-core-spec.md, secao 6. Dirige Z80DebugSession::ProcessCommand()
// diretamente com vetores de tokens (sem replxx, sem stdin) e confere o
// texto de resposta / estado resultante.
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "../../src/z80/debug/flat_memory_bus.h"
#include "../../src/z80/debug/z80_debug_session.h"
#include "../../src/z80/debug/z80_debug_shell_startup.h"
#include "../../src/z80/debug/z80_disasm.h"

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

std::string Cmd(z80::debug::Z80DebugSession &session, std::vector<std::string> tokens) {
    return session.ProcessCommand(tokens);
}

} // namespace

int main() {
    using z80::debug::Z80DebugSession;

    // --- reset / regs ---------------------------------------------------
    {
        z80::debug::FlatMemoryBus bus;
        Z80DebugSession session(bus);
        Cmd(session, {"reset"});
        const std::string regs = Cmd(session, {"regs"});
        check(Contains(regs, "PC=0000"), "reset+regs: PC=0000");
        check(Contains(regs, "SP=F000"), "reset+regs: SP=F000 (valor inicial do z80_reset)");
    }

    // --- poke/peek round-trip -------------------------------------------
    {
        z80::debug::FlatMemoryBus bus;
        Z80DebugSession session(bus);
        Cmd(session, {"poke", "0x1234", "0xAB"});
        const std::string peeked = Cmd(session, {"peek", "0x1234"});
        check(Contains(peeked, "AB"), "poke+peek: round-trip em 0x1234");
    }

    // --- fill + mem -------------------------------------------------------
    {
        z80::debug::FlatMemoryBus bus;
        Z80DebugSession session(bus);
        Cmd(session, {"fill", "0x2000", "8", "0x55"});
        const std::string dump = Cmd(session, {"mem", "0x2000", "8"});
        bool all_55 = true;
        // Conta quantas ocorrencias de "55" aparecem no dump -- esperado 8.
        size_t pos = 0, count = 0;
        while ((pos = dump.find("55", pos)) != std::string::npos) {
            ++count;
            pos += 2;
        }
        check(count == 8, "fill+mem: 8 bytes 0x55 aparecem no dump (achou " + std::to_string(count) + ")");
        (void)all_55;
    }

    // --- load: arquivo existente ------------------------------------------
    {
        z80::debug::FlatMemoryBus bus;
        Z80DebugSession session(bus);
        const auto tmp_path = std::filesystem::temp_directory_path() / "fwmsx_z80dbg_test_load.bin";
        {
            std::ofstream f(tmp_path, std::ios::binary);
            const unsigned char bytes[] = {0xDE, 0xAD, 0xBE, 0xEF};
            f.write(reinterpret_cast<const char *>(bytes), sizeof(bytes));
        }
        const std::string result = Cmd(session, {"load", tmp_path.string(), "0x3000"});
        check(Contains(result, "4 byte"), "load: reporta 4 bytes carregados");
        check(Contains(Cmd(session, {"peek", "0x3000"}), "DE"), "load: byte 0 == DE em 0x3000");
        check(Contains(Cmd(session, {"peek", "0x3003"}), "EF"), "load: byte 3 == EF em 0x3003");
        std::filesystem::remove(tmp_path);
    }

    // --- load: arquivo inexistente (nao pode travar) -----------------------
    {
        z80::debug::FlatMemoryBus bus;
        Z80DebugSession session(bus);
        const std::string result = Cmd(session, {"load", "C:/caminho/que/nao/existe/arquivo.bin", "0x0000"});
        check(!result.empty(), "load (inexistente): devolve mensagem de erro, nao trava");
        check(Contains(result, "load"), "load (inexistente): mensagem menciona o comando");
    }

    // --- step/run sobre um programa pequeno ---------------------------------
    // LD A,5 / LD B,3 / ADD A,B / HALT
    {
        z80::debug::FlatMemoryBus bus;
        Z80DebugSession session(bus);
        Cmd(session, {"reset"});
        Cmd(session, {"poke", "0x0000", "0x3E"});
        Cmd(session, {"poke", "0x0001", "0x05"});
        Cmd(session, {"poke", "0x0002", "0x06"});
        Cmd(session, {"poke", "0x0003", "0x03"});
        Cmd(session, {"poke", "0x0004", "0x80"});
        Cmd(session, {"poke", "0x0005", "0x76"});

        Cmd(session, {"step", "4"}); // LD A,5 / LD B,3 / ADD A,B / HALT
        const std::string regs = Cmd(session, {"regs"});
        check(Contains(regs, "AF=0800"), "step: A=08 (5+3) apos 4 passos (regs: " + regs.substr(0, 40) + ")");
    }

    // --- breakpoint interrompe 'run' antes do orcamento esgotar --------------
    {
        z80::debug::FlatMemoryBus bus;
        Z80DebugSession session(bus);
        Cmd(session, {"reset"});
        // JR $ (0x18 0xFE) em 0x0000 -- loop infinito de 1 instrucao (13 T-states
        // por iteracao: 12 quando tomado -- nao importa o valor exato aqui).
        Cmd(session, {"poke", "0x0000", "0x18"});
        Cmd(session, {"poke", "0x0001", "0xFE"});
        Cmd(session, {"break", "0x0000"});

        const std::string result = Cmd(session, {"run", "1000000"});
        check(Contains(result, "breakpoint"), "run: para por breakpoint, nao por esgotar o orcamento (" + result + ")");
    }

    // --- desassemblador: checagens exatas (Fase 4, disasm) ---------------
    {
        using z80::debug::Disassemble;
        auto make_reader = [](std::vector<uint8_t> bytes) {
            return [bytes](uint16_t addr) -> uint8_t { return addr < bytes.size() ? bytes[addr] : 0x00; };
        };
        auto check_exact = [&](std::vector<uint8_t> bytes, const std::string &expected_text, uint16_t expected_len,
                                const std::string &label) {
            const auto r = Disassemble(make_reader(bytes), 0);
            check(r.text == expected_text && r.length == expected_len,
                  label + ": esperado '" + expected_text + "' (" + std::to_string(expected_len) +
                      " bytes), obtido '" + r.text + "' (" + std::to_string(r.length) + " bytes)");
        };

        check_exact({0x00}, "NOP", 1, "disasm: NOP");
        check_exact({0x3E, 0x42}, "LD A,42h", 2, "disasm: LD A,42h (imediato de 8 bits, '*')");
        check_exact({0x01, 0x34, 0x12}, "LD BC,1234h", 3, "disasm: LD BC,1234h (imediato de 16 bits, '#')");
        check_exact({0x20, 0x05}, "JR NZ,+05h", 2, "disasm: JR NZ,+05h (offset relativo positivo, '@')");
        check_exact({0x20, 0xFB}, "JR NZ,-05h", 2, "disasm: JR NZ,-05h (offset relativo negativo, '@')");
        check_exact({0xCB, 0x00}, "RLC B", 2, "disasm: RLC B (tabela CB)");
        check_exact({0xCB, 0x7E}, "BIT 7,(HL)", 2, "disasm: BIT 7,(HL) (tabela CB)");
        check_exact({0xED, 0x78}, "IN A,(C)", 2, "disasm: IN A,(C) (tabela ED)");
        check_exact({0xED, 0xB0}, "LDIR", 2, "disasm: LDIR (tabela ED)");
        check_exact({0xDD, 0x09}, "ADD IX,BC", 2, "disasm: ADD IX,BC (tabela XX, prefixo DD)");
        check_exact({0xDD, 0x8F}, "ADC A", 2, "disasm: ADC A com prefixo IX (correcao do typo 'ADC,A' do fMSX)");
        check_exact({0xFD, 0x8F}, "ADC A", 2, "disasm: ADC A com prefixo IY (mesma correcao, '%' -> Y)");
        // "LD I%h,I%l" -> ambas ocorrencias de '%' precisam virar X/Y (correcao
        // do bug do fMSX que so trocava a primeira) -- o sufixo minusculo
        // "h"/"l" e' o proprio estilo original das tabelas (nao e' erro).
        check_exact({0xDD, 0x65}, "LD IXh,IXl", 2, "disasm: LD IXh,IXl (dupla substituicao de '%', prefixo DD)");
        check_exact({0xFD, 0x65}, "LD IYh,IYl", 2, "disasm: LD IYh,IYl (dupla substituicao de '%', prefixo FD)");
        check_exact({0xDD, 0xCB, 0x03, 0x06}, "RLC (IX+03h)", 4,
                     "disasm: RLC (IX+03h) (tabela XCB, deslocamento positivo)");
        check_exact({0xFD, 0xCB, 0xFE, 0x66}, "BIT 4,(IY-02h)", 4,
                     "disasm: BIT 4,(IY-02h) (tabela XCB, deslocamento negativo)");
        check_exact({0xDD, 0x34, 0xFF}, "INC (IX-01h)", 3,
                     "disasm: INC (IX-01h) ('^' com deslocamento negativo -- correcao #3, sinal explicito)");
    }

    // --- desassemblador: varredura de completude (sem crash em nenhum opcode) ---
    {
        using z80::debug::Disassemble;
        auto reader_over = [](std::vector<uint8_t> buf) {
            return [buf](uint16_t addr) -> uint8_t { return addr < buf.size() ? buf[addr] : 0x00; };
        };

        int leading_ok = 0;
        for (int op = 0; op < 256; ++op) {
            std::vector<uint8_t> buf(8, 0x00);
            buf[0] = static_cast<uint8_t>(op);
            const auto r = Disassemble(reader_over(buf), 0);
            if (!r.text.empty() && r.length >= 1 && r.length <= 4) ++leading_ok;
        }
        check(leading_ok == 256,
              "disasm: varredura dos 256 valores possiveis do primeiro byte -- sem crash, texto nao-vazio, "
              "tamanho 1..4 (achou " +
                  std::to_string(leading_ok) + "/256)");

        int cb_ok = 0;
        for (int op = 0; op < 256; ++op) {
            std::vector<uint8_t> buf = {0xCB, static_cast<uint8_t>(op), 0, 0, 0, 0, 0, 0};
            const auto r = Disassemble(reader_over(buf), 0);
            if (!r.text.empty() && r.length == 2) ++cb_ok;
        }
        check(cb_ok == 256, "disasm: varredura completa da tabela CB -- 256/256 sem crash, tamanho sempre 2 (achou " +
                                 std::to_string(cb_ok) + "/256)");

        int ed_ok = 0;
        for (int op = 0; op < 256; ++op) {
            std::vector<uint8_t> buf = {0xED, static_cast<uint8_t>(op), 0, 0, 0, 0, 0, 0};
            const auto r = Disassemble(reader_over(buf), 0);
            if (!r.text.empty() && r.length >= 2 && r.length <= 4) ++ed_ok;
        }
        check(ed_ok == 256, "disasm: varredura completa da tabela ED -- 256/256 sem crash, tamanho 2..4 (achou " +
                                 std::to_string(ed_ok) + "/256)");

        int xx_ok = 0;
        for (uint8_t prefix : {static_cast<uint8_t>(0xDD), static_cast<uint8_t>(0xFD)}) {
            for (int op = 0; op < 256; ++op) {
                if (op == 0xCB) continue;  // esse caso delega pra tabela XCB, testado a parte abaixo
                std::vector<uint8_t> buf = {prefix, static_cast<uint8_t>(op), 0, 0, 0, 0, 0, 0};
                const auto r = Disassemble(reader_over(buf), 0);
                if (!r.text.empty() && r.length >= 2 && r.length <= 4) ++xx_ok;
            }
        }
        check(xx_ok == 255 * 2,
              "disasm: varredura completa da tabela XX sob DD e FD -- 255/255 * 2 sem crash, tamanho 2..4 "
              "(opcode 0xCB delega pra XCB, testado a parte -- achou " +
                  std::to_string(xx_ok) + "/510)");

        int xcb_ok = 0;
        for (uint8_t prefix : {static_cast<uint8_t>(0xDD), static_cast<uint8_t>(0xFD)}) {
            for (int op = 0; op < 256; ++op) {
                std::vector<uint8_t> buf = {prefix, 0xCB, 0x00, static_cast<uint8_t>(op), 0, 0, 0, 0};
                const auto r = Disassemble(reader_over(buf), 0);
                if (!r.text.empty() && r.length == 4) ++xcb_ok;
            }
        }
        check(xcb_ok == 256 * 2,
              "disasm: varredura completa da tabela XCB sob DD CB e FD CB -- 256/256 * 2 sem crash, tamanho "
              "sempre 4 (achou " +
                  std::to_string(xcb_ok) + "/512)");
    }

    // --- comando 'disasm' da sessao (integracao, nao so a funcao pura) -----
    {
        z80::debug::FlatMemoryBus bus;
        Z80DebugSession session(bus);
        Cmd(session, {"reset"});
        Cmd(session, {"poke", "0x0000", "0x3E"});
        Cmd(session, {"poke", "0x0001", "0x05"});
        Cmd(session, {"poke", "0x0002", "0x76"});
        const std::string listing = Cmd(session, {"disasm", "0x0000", "2"});
        check(Contains(listing, "LD A,05h"), "disasm (sessao): mostra LD A,05h na primeira linha");
        check(Contains(listing, "HALT"), "disasm (sessao): mostra HALT na segunda linha (endereco avancado certo)");
    }

    // --- Fase 4 do mapa de memoria: BuildZ80DebugShellStartup() -------------
    // (logica de inicializacao de "fwmsx --z80dbg [--slots [<rom>]]",
    // separada do loop replxx -- ver z80_debug_shell.h/.cpp e
    // doc/memory-map-spec.md, secao 6, Fase 4.)
    using z80::debug::BuildZ80DebugShellStartup;

    // --- "--slots" sem caminho: RAM vazia em 0:0 (regressao da Fase 1) -----
    {
        const auto startup = BuildZ80DebugShellStartup({"--slots"});
        check(startup.use_slots, "startup --slots (sem rom): use_slots == true");
        check(!startup.boot_rom_requested, "startup --slots (sem rom): boot_rom_requested == false");
        check(!startup.boot_rom_loaded, "startup --slots (sem rom): boot_rom_loaded == false");
        check(startup.memory_system != nullptr, "startup --slots (sem rom): MemorySystem existe");
        const memmap::SlotDescriptor desc = startup.memory_system->Describe(0, 0);
        check(desc.kind == MEMMAP_KIND_RAM, "startup --slots (sem rom): 0:0 e' RAM");
        check(desc.size == 0x10000, "startup --slots (sem rom): 0:0 tem 64KB");
    }

    // --- "--slots <rom valida>": ROM carregada em 0:0, vista ativa ---------
    {
        const auto tmp_path = std::filesystem::temp_directory_path() / "fwmsx_z80dbg_test_bootrom.bin";
        {
            std::ofstream f(tmp_path, std::ios::binary);
            std::vector<unsigned char> data(0x4000, 0x7A); // 16KB, byte 7Ah
            f.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
        }
        const auto startup = BuildZ80DebugShellStartup({"--slots", tmp_path.string()});
        check(startup.use_slots, "startup --slots <rom>: use_slots == true");
        check(startup.boot_rom_requested, "startup --slots <rom>: boot_rom_requested == true");
        check(startup.boot_rom_loaded, "startup --slots <rom>: boot_rom_loaded == true");
        check(startup.boot_rom_error.empty(), "startup --slots <rom>: sem mensagem de erro");
        const memmap::SlotDescriptor desc = startup.memory_system->Describe(0, 0);
        check(desc.kind == MEMMAP_KIND_ROM, "startup --slots <rom>: 0:0 e' ROM");
        check(desc.size == 0x4000, "startup --slots <rom>: tamanho bate (16KB)");
        check(startup.memory_system->PeekSlot(0, 0, 0x0000) == 0x7A,
              "startup --slots <rom>: PeekSlot(0,0,0) le o conteudo carregado");
        // A ROM tambem precisa ser a vista ativa da CPU -- via IBus::read,
        // nao so PeekSlot (que enxerga por fora, independente do que esta
        // visivel -- aqui queremos confirmar que ALEM disso esta' visivel).
        check(startup.Bus().read(0x0000) == 0x7A,
              "startup --slots <rom>: IBus::read(0) tambem le a ROM (e' a vista ativa)");
        std::filesystem::remove(tmp_path);
    }

    // --- "--slots <rom inexistente>": nao trava, cai pra RAM vazia --------
    {
        const auto startup = BuildZ80DebugShellStartup({"--slots", "C:/caminho/que/nao/existe/boot.rom"});
        check(startup.use_slots, "startup --slots <rom inexistente>: use_slots == true (sessao ainda inicia)");
        check(startup.boot_rom_requested, "startup --slots <rom inexistente>: boot_rom_requested == true");
        check(!startup.boot_rom_loaded, "startup --slots <rom inexistente>: boot_rom_loaded == false");
        check(!startup.boot_rom_error.empty(), "startup --slots <rom inexistente>: boot_rom_error preenchido");
        const memmap::SlotDescriptor desc = startup.memory_system->Describe(0, 0);
        check(desc.kind == MEMMAP_KIND_RAM, "startup --slots <rom inexistente>: cai para RAM em 0:0 (fallback limpo)");
        check(desc.size == 0x10000, "startup --slots <rom inexistente>: RAM de fallback tem 64KB");
    }

    // --- "--z80dbg" simples (sem --slots): comportamento da Fase 4 original,
    // inalterado -- FlatMemoryBus, sem MemorySystem. ------------------------
    {
        const auto startup = BuildZ80DebugShellStartup({});
        check(!startup.use_slots, "startup sem argumentos: use_slots == false");
        check(startup.memory_system == nullptr, "startup sem argumentos: sem MemorySystem (FlatMemoryBus puro)");
        Z80DebugSession session(startup.Bus());
        Cmd(session, {"poke", "0x1234", "0x99"});
        check(Contains(Cmd(session, {"peek", "0x1234"}), "99"),
              "startup sem argumentos: sessao funciona normalmente sobre FlatMemoryBus (regressao)");
    }

    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
