// fwMSX -- sessao de depuracao do nucleo Z80 (Fase 4). Codigo ORIGINAL
// do fwMSX (BSD-3-Clause), nao adaptado do fMSX -- ver
// doc/z80-core-spec.md, secao 6 (Fase 4).
//
// Nucleo testavel dos comandos de depuracao: tokens de entrada, texto de
// saida, sem nenhuma dependencia de replxx/stdin -- ver z80_debug_shell.h
// para o REPL interativo que usa esta classe. Essa separacao e' o que
// permite testar os comandos via CTest (tests/z80/debug_session_test.cpp)
// sem precisar de um TTY de verdade.
#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "../cpp/z80_cpu.h"
#include "flat_memory_bus.h"

namespace z80::debug {

class Z80DebugSession {
public:
    Z80DebugSession();

    // Executa um comando (primeiro token = nome do comando) e devolve o
    // texto de resposta (sem newline final). Nunca lanca excecao por
    // entrada invalida -- erros de uso viram texto de erro na resposta.
    std::string ProcessCommand(const std::vector<std::string> &tokens);

    Z80Cpu &cpu() { return cpu_; }
    FlatMemoryBus &bus() { return bus_; }

private:
    std::string CmdReset();
    std::string CmdRegs() const;
    std::string CmdStep(const std::vector<std::string> &tokens);
    std::string CmdRun(const std::vector<std::string> &tokens);
    std::string CmdBreak(const std::vector<std::string> &tokens);
    std::string CmdClearBreak(const std::vector<std::string> &tokens);
    std::string CmdListBreaks() const;
    std::string CmdMem(const std::vector<std::string> &tokens) const;
    std::string CmdPeek(const std::vector<std::string> &tokens) const;
    std::string CmdPoke(const std::vector<std::string> &tokens);
    std::string CmdLoad(const std::vector<std::string> &tokens);
    std::string CmdFill(const std::vector<std::string> &tokens);
    std::string CmdHelp() const;

    FlatMemoryBus bus_;
    Z80Cpu cpu_;
    std::set<uint16_t> breakpoints_;
};

} // namespace z80::debug
