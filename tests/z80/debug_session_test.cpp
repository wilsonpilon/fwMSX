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
        Z80DebugSession session;
        Cmd(session, {"reset"});
        const std::string regs = Cmd(session, {"regs"});
        check(Contains(regs, "PC=0000"), "reset+regs: PC=0000");
        check(Contains(regs, "SP=F000"), "reset+regs: SP=F000 (valor inicial do z80_reset)");
    }

    // --- poke/peek round-trip -------------------------------------------
    {
        Z80DebugSession session;
        Cmd(session, {"poke", "0x1234", "0xAB"});
        const std::string peeked = Cmd(session, {"peek", "0x1234"});
        check(Contains(peeked, "AB"), "poke+peek: round-trip em 0x1234");
    }

    // --- fill + mem -------------------------------------------------------
    {
        Z80DebugSession session;
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
        Z80DebugSession session;
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
        Z80DebugSession session;
        const std::string result = Cmd(session, {"load", "C:/caminho/que/nao/existe/arquivo.bin", "0x0000"});
        check(!result.empty(), "load (inexistente): devolve mensagem de erro, nao trava");
        check(Contains(result, "load"), "load (inexistente): mensagem menciona o comando");
    }

    // --- step/run sobre um programa pequeno ---------------------------------
    // LD A,5 / LD B,3 / ADD A,B / HALT
    {
        Z80DebugSession session;
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
        Z80DebugSession session;
        Cmd(session, {"reset"});
        // JR $ (0x18 0xFE) em 0x0000 -- loop infinito de 1 instrucao (13 T-states
        // por iteracao: 12 quando tomado -- nao importa o valor exato aqui).
        Cmd(session, {"poke", "0x0000", "0x18"});
        Cmd(session, {"poke", "0x0001", "0xFE"});
        Cmd(session, {"break", "0x0000"});

        const std::string result = Cmd(session, {"run", "1000000"});
        check(Contains(result, "breakpoint"), "run: para por breakpoint, nao por esgotar o orcamento (" + result + ")");
    }

    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
