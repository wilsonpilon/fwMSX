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

namespace vdp {
class VdpDevice;
} // namespace vdp

namespace z80::debug {

class Z80DebugSession {
public:
    // `vdp_device` (Fase 0.5/1 do VDP, ver doc/vdp-spec.md): quando
    // presente, `run`/`step` tambem avancam a maquina de estados do VDP
    // e entregam interrupcoes de verdade ao Z80Cpu (ver DriveVdp() na
    // implementacao) -- sem ele, os comandos vdp* respondem pedindo
    // `--z80dbg --slots --vdp` em vez de travar, mesmo padrao ja usado
    // para `memory_system` (mapa de memoria).
    explicit Z80DebugSession(z80::IBus &bus, memmap::MemorySystem *memory_system = nullptr,
                              vdp::VdpDevice *vdp_device = nullptr);

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
    std::string CmdLoadRom(const std::vector<std::string> &tokens);
    std::string CmdVdpRegs() const;
    std::string CmdVdpMem(const std::vector<std::string> &tokens) const;
    std::string CmdVdpPeek(const std::vector<std::string> &tokens) const;
    std::string CmdVdpPoke(const std::vector<std::string> &tokens);
    std::string CmdVdpStep(const std::vector<std::string> &tokens);
    std::string CmdHelp() const;

    // Avanca a maquina de estados do VDP o quanto for necessario para
    // "consumir" `cycles_consumed` ciclos de Z80 que acabaram de rodar,
    // entregando cpu_.interrupt(Z80_INT_IRQ) sempre que um passo reportar
    // irq_pending -- ver doc/vdp-spec.md, secao 3.3, para o raciocinio
    // completo (isto e' o que substitui o papel de LoopZ80()/IRequest do
    // fMSX no nosso modelo "host decide quando" de execucao). Sem
    // vdp_device_, e' um no-op.
    void DriveVdp(int cycles_consumed);

    z80::IBus &bus_;
    Z80Cpu cpu_;
    std::set<uint16_t> breakpoints_;
    memmap::MemorySystem *memory_system_;
    vdp::VdpDevice *vdp_device_;
    // Ciclos restantes ate' a proxima vez que o VDP precisa ser avancado
    // -- ver DriveVdp(). Comeca em 0 (avanca o VDP uma vez antes de
    // qualquer instrucao rodar, pegando o primeiro next_period_cycles de
    // verdade).
    int vdp_pending_cycles_ = 0;
};

} // namespace z80::debug
