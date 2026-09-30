// fwMSX -- inicializacao de "fwmsx --z80dbg [--slots [<rom>]]" (Fase 4
// do mapa de memoria, ver doc/memory-map-spec.md). Codigo ORIGINAL do
// fwMSX (BSD-3-Clause), nao adaptado do fMSX.
//
// Separado de z80_debug_shell.{h,cpp} DE PROPOSITO: este arquivo nao usa
// replxx nem stdin, entao pode ser compilado nos alvos de teste
// (z80dbgtest/memmaptest, que nao linkam replxx) sem arrastar essa
// dependencia -- so o REPL interativo em si (RunZ80DebugShell(), em
// z80_debug_shell.cpp) precisa de replxx. Ver CMakeLists.txt.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../memmap/cpp/memory_system.h"
#include "../../memmap/cpp/slot_memory_bus.h"
#include "../cpp/z80_bus.h"
#include "flat_memory_bus.h"

namespace z80::debug {

// Resultado de interpretar "fwmsx --z80dbg [--slots [<rom>]]" e montar os
// objetos de barramento/memoria correspondentes. RunZ80DebugShell() usa
// isto internamente antes de entrar no loop interativo; testes
// automatizados usam diretamente (ver tests/z80/debug_session_test.cpp).
//
// Regra de sintaxe (deliberadamente simples, ver doc/memory-map-spec.md,
// secao 6, Fase 4): SO o token IMEDIATAMENTE seguinte a "--slots" (se
// houver) e' tratado como caminho de ROM de boot -- "fwmsx --z80dbg
// --slots caminho.rom". Nao ha suporte a outras ordens de argumento nem
// a mais flags nesta fase.
struct Z80DebugShellStartup {
    bool use_slots = false;
    // Um caminho de ROM foi passado logo apos "--slots" (independente de
    // ter carregado com sucesso ou nao).
    bool boot_rom_requested = false;
    // So true quando boot_rom_requested E' o carregamento deu certo.
    bool boot_rom_loaded = false;
    std::string boot_rom_path;
    // Nao-vazio quando boot_rom_requested && !boot_rom_loaded -- motivo
    // do carregamento ter falhado (arquivo nao encontrado, tamanho
    // invalido para LoadRom(), etc.). Nesse caso a sessao ainda inicia,
    // com RAM vazia em 0:0 (mesmo estado de "--slots" sem caminho) --
    // ver a nota "warn, don't crash" em memory-map-spec.md.
    std::string boot_rom_error;

    // Sempre construido (mesmo quando use_slots==false, so' fica sem uso
    // nesse caso) -- mais simples do que um ponteiro opcional, e o custo
    // de 64KB e' irrelevante.
    std::unique_ptr<FlatMemoryBus> flat_bus;
    // So construidos quando use_slots==true.
    std::unique_ptr<memmap::MemorySystem> memory_system;
    std::unique_ptr<memmap::SlotMemoryBus> slot_bus;

    // O IBus que a sessao deve usar, de acordo com use_slots.
    z80::IBus &Bus() const {
        return use_slots ? static_cast<z80::IBus &>(*slot_bus) : static_cast<z80::IBus &>(*flat_bus);
    }
};

// Interpreta os argumentos encaminhados de "fwmsx --z80dbg ..." e monta
// os objetos de barramento/memoria de acordo -- ver Z80DebugShellStartup
// acima para a regra de sintaxe e o comportamento de erro. Nunca lanca;
// uma falha ao carregar a ROM de boot cai no estado "--slots" vazio
// (RAM em 0:0), registrada em boot_rom_error.
Z80DebugShellStartup BuildZ80DebugShellStartup(const std::vector<std::string> &args);

} // namespace z80::debug
