// fwMSX -- REPL interativo de depuracao do nucleo Z80 (Fase 4; opcoes
// --slots [rom] da Fase 1/4 do mapa de memoria, ver
// doc/memory-map-spec.md). Codigo ORIGINAL do fwMSX (BSD-3-Clause), nao
// adaptado do fMSX.
#pragma once

#include <string>
#include <vector>

namespace z80::debug {

// Ponto de entrada de "fwmsx --z80dbg [--slots [<rom>]]" -- ver
// src/cpp/main.cpp. Sem "--slots": RAM plana de 64KB (FlatMemoryBus),
// comportamento identico ao da Fase 4 original -- ver
// doc/memory-map-spec.md, secao 4/6 (Fase 4): essa decisao foi mantida
// de proposito, o modo simples/default nao ganha uma dependencia de
// achar/carregar uma ROM so para iniciar. Com "--slots": mapa de memoria
// real (memmap::SlotMemoryBus); com "--slots <rom>", essa ROM e'
// carregada em 0:0 como ROM plana antes de entrar no REPL, em vez de
// exigir um "loadrom" manual toda vez -- ver z80_debug_shell_startup.h
// (BuildZ80DebugShellStartup()) para a logica de montagem em si,
// separada daqui por nao depender de replxx.
int RunZ80DebugShell(const std::vector<std::string> &args = {});

} // namespace z80::debug
