// fwMSX -- REPL interativo de depuracao do nucleo Z80 (Fase 4). Estilo
// de shell (replxx, tokenizacao com aspas, historico em arquivo) espelha
// src/msxdisk/shell/shell.cpp de proposito, pra manter os dois REPLs do
// projeto consistentes -- ver doc/z80-core-spec.md, secao 6 (Fase 4).
#include "z80_debug_shell.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <replxx.hxx>

#include "../../memmap/cpp/memory_system.h"
#include "../../memmap/cpp/slot_memory_bus.h"
#include "flat_memory_bus.h"
#include "z80_debug_session.h"

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
    bool use_slots = false;
    for (const std::string &arg : args) {
        if (arg == "--slots") use_slots = true;
    }

    replxx::Replxx rx;
    const std::string history_path = HistoryFilePath();
    rx.history_load(history_path);

    // Sem --slots: RAM plana de 64KB, exatamente como na Fase 4 original
    // (FlatMemoryBus). Com --slots: mapa de memoria real (Fase 1, ver
    // doc/memory-map-spec.md) -- RAM alocada na combinacao 0:0, que ja'
    // e' a que fica visivel por padrao logo apos memmap_init() (todo
    // psl[pagina]/ssl[pagina] comeca em 0), entao o usuario pode
    // poke/run direto sem precisar trocar de slot primeiro. Decisao
    // documentada em doc/memory-map-spec.md, Fase 1 (notas de
    // implementacao): 64KB inteiros (o maximo possivel numa combinacao)
    // em vez de um tamanho menor, pra nao impor um limite arbitrario de
    // RAM de teste.
    z80::debug::FlatMemoryBus flat_bus;
    std::unique_ptr<memmap::MemorySystem> memory_system;
    std::unique_ptr<memmap::SlotMemoryBus> slot_bus;
    if (use_slots) {
        memory_system = std::make_unique<memmap::MemorySystem>();
        memory_system->AllocateRam(0, 0, 0x10000);
        slot_bus = std::make_unique<memmap::SlotMemoryBus>(*memory_system);
    }

    z80::IBus &bus = use_slots ? static_cast<z80::IBus &>(*slot_bus) : static_cast<z80::IBus &>(flat_bus);
    Z80DebugSession session(bus, memory_system.get());

    std::cout << "fwMSX - depurador do nucleo Z80 ("
               << (use_slots ? "mapa de memoria real (slots/subslots)" : "RAM plana de teste") << ", sem maquina MSX ainda)."
               << std::endl;
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
