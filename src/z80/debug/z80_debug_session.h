// fwMSX -- sessao de depuracao do nucleo Z80 (Fase 4; comandos de slot
// da Fase 1 do mapa de memoria, ver doc/memory-map-spec.md). Codigo
// ORIGINAL do fwMSX (BSD-3-Clause), nao adaptado do fMSX.
//
// Nucleo testavel dos comandos de depuracao: tokens de entrada, texto de
// saida, sem nenhuma dependencia de replxx/stdin -- ver z80_debug_shell.h
// para o REPL interativo que usa esta classe. Essa separacao e' o que
// permite testar os comandos via CTest (tests/z80/debug_session_test.cpp)
// sem precisar de um TTY de verdade.
//
// Refatoracao (mapa de memoria, Fase 1): a sessao deixou de possuir sua
// propria FlatMemoryBus internamente -- agora recebe qualquer z80::IBus
// por referencia (o chamador decide se e' uma FlatMemoryBus simples, como
// ate agora, ou uma memmap::SlotMemoryBus de verdade). Um
// memmap::MemorySystem* opcional habilita os comandos slot-aware
// (slots/pages/slotmem/slotpeek/slotpoke) -- sem ele, esses comandos
// respondem pedindo `--z80dbg --slots` em vez de travar.
#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "../cpp/z80_bus.h"
#include "../cpp/z80_cpu.h"

namespace memmap {
class MemorySystem;
} // namespace memmap

namespace z80::debug {

class Z80DebugSession {
public:
    explicit Z80DebugSession(z80::IBus &bus, memmap::MemorySystem *memory_system = nullptr);

    // Executa um comando (primeiro token = nome do comando) e devolve o
    // texto de resposta (sem newline final). Nunca lanca excecao por
    // entrada invalida -- erros de uso viram texto de erro na resposta.
    std::string ProcessCommand(const std::vector<std::string> &tokens);

    Z80Cpu &cpu() { return cpu_; }
    z80::IBus &bus() { return bus_; }

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
    std::string CmdDisasm(const std::vector<std::string> &tokens) const;
    std::string CmdSlots() const;
    std::string CmdPages() const;
    std::string CmdSlotMem(const std::vector<std::string> &tokens) const;
    std::string CmdSlotPeek(const std::vector<std::string> &tokens) const;
    std::string CmdSlotPoke(const std::vector<std::string> &tokens);
    std::string CmdHelp() const;

    z80::IBus &bus_;
    Z80Cpu cpu_;
    std::set<uint16_t> breakpoints_;
    memmap::MemorySystem *memory_system_;
};

} // namespace z80::debug
