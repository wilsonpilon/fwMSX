// fwMSX -- REPL interativo de depuracao do nucleo Z80 (Fase 4). Estilo
// de shell (replxx, tokenizacao com aspas, historico em arquivo) espelha
// src/msxdisk/shell/shell.cpp de proposito, pra manter os dois REPLs do
// projeto consistentes -- ver doc/z80-core-spec.md, secao 6 (Fase 4).
//
// A montagem de "--slots [<rom>]" (Fase 1/4 do mapa de memoria, ver
// doc/memory-map-spec.md) mora em z80_debug_shell_startup.{h,cpp},
// separada deste arquivo de proposito: aquele nao usa replxx, entao pode
// ser testado (tests/z80/debug_session_test.cpp) sem essa dependencia.
#include "z80_debug_shell.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <replxx.hxx>

#include "z80_debug_session.h"
#include "z80_debug_shell_startup.h"

namespace z80::debug {

namespace {

// Mesma logica de src/msxdisk/shell/shell.cpp::Tokenize -- duplicada
// aqui (nao vale a pena criar uma lib compartilhada so por causa de 20
// linhas identicas entre dois REPLs independentes).
std::vector<std::string> Tokenize(const std::string &line) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;
    char quote_char = '"';

    for (char c : line) {
        if (in_quotes) {
            if (c == quote_char) {
                in_quotes = false;
            } else {
                current += c;
            }
            continue;
        }
        if (c == '"' || c == '\'') {
            in_quotes = true;
            quote_char = c;
        } else if (std::isspace(static_cast<unsigned char>(c))) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

std::string HistoryFilePath() {
    const char *home =
#if defined(_WIN32)
        std::getenv("USERPROFILE");
#else
        std::getenv("HOME");
#endif
    if (home == nullptr || *home == '\0') return ".fwmsx_z80dbg_history";
    return std::string(home) + "/.fwmsx_z80dbg_history";
}

} // namespace

int RunZ80DebugShell(const std::vector<std::string> &args) {
    Z80DebugShellStartup startup = BuildZ80DebugShellStartup(args);

    replxx::Replxx rx;
    const std::string history_path = HistoryFilePath();
    rx.history_load(history_path);

    Z80DebugSession session(startup.Bus(), startup.memory_system.get());

    std::cout << "fwMSX - depurador do nucleo Z80 ("
               << (startup.use_slots ? "mapa de memoria real (slots/subslots)" : "RAM plana de teste")
               << ", sem maquina MSX ainda)." << std::endl;
    if (startup.boot_rom_loaded) {
        std::cout << "ROM de boot carregada em 0:0: " << startup.boot_rom_path << std::endl;
    } else if (startup.boot_rom_requested) {
        std::cout << "Aviso: nao foi possivel carregar a ROM de boot '" << startup.boot_rom_path << "' ("
                   << startup.boot_rom_error << ") -- iniciando com RAM vazia em 0:0." << std::endl;
    }
    std::cout << "Digite 'help' para a lista de comandos, 'exit' ou Ctrl-D para sair." << std::endl;

    while (true) {
        const char *raw_line = rx.input("z80dbg> ");
        if (raw_line == nullptr) {
            std::cout << std::endl;
            break;
        }

        const std::string line(raw_line);
        if (line.empty()) continue;
        rx.history_add(line);

        const auto tokens = Tokenize(line);
        if (tokens.empty()) continue;
        if (tokens[0] == "exit" || tokens[0] == "quit") break;

        const std::string response = session.ProcessCommand(tokens);
        if (!response.empty()) std::cout << response << std::endl;
    }

    rx.history_save(history_path);
    std::cout << "Ate mais!" << std::endl;
    return 0;
}

} // namespace z80::debug
