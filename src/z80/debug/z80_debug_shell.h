// fwMSX -- REPL interativo de depuracao do nucleo Z80 (Fase 4; opcao
// --slots da Fase 1 do mapa de memoria, ver doc/memory-map-spec.md).
// Codigo ORIGINAL do fwMSX (BSD-3-Clause), nao adaptado do fMSX.
#pragma once

#include <string>
#include <vector>

namespace z80::debug {

// Ponto de entrada de "fwmsx --z80dbg [--slots]" -- ver src/cpp/main.cpp.
// Sem "--slots": RAM plana de 64KB (FlatMemoryBus), comportamento
// identico ao da Fase 4 original. Com "--slots": mapa de memoria real
// (memmap::SlotMemoryBus) com RAM alocada em 0:0 (visivel desde o
// reset) -- habilita os comandos slots/pages/slotmem/slotpeek/slotpoke.
int RunZ80DebugShell(const std::vector<std::string> &args = {});

} // namespace z80::debug
