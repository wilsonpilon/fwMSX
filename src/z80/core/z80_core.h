// API publica do motor Z80 do fwMSX -- design proprio (BSD-3-Clause).
// A implementacao (z80_core.c) adapta a logica de execucao/decodificacao
// de resource/fMSX/Z80/Z80.c -- ver z80_opcodes.h para a nota de
// atribuicao completa daquela parte. Ver doc/z80-core-spec.md, secao 3.2
// e 6 (Fase 1), para o raciocinio de design desta API.
#pragma once

#include <stdint.h>

#include "../common/z80_state.h"
#include "z80_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

// Zera os registradores para o estado inicial de reset do Z80 (PC=0,
// SP=0xF000 -- mesmos defaults do fMSX) e chama bus->jump(0), se houver.
void z80_reset(Z80State *state, const Z80Bus *bus);

// Executa opcodes ate consumir `cycles` ciclos (podendo passar um pouco,
// como no ExecZ80 original) e devolve o saldo de ciclos restante
// (possivelmente negativo). Controlado pelo host -- nao ha auto-loop de
// interrupcao periodica nesta Fase 1 (ver doc/z80-core-spec.md, secao
// 3.2/3.3): o host decide quando chamar z80_interrupt() entre chamadas.
int z80_run(Z80State *state, const Z80Bus *bus, int cycles);

// Gera uma interrupcao do vetor dado (Z80_INT_NMI, ou um dos
// Z80_INT_RSTxx para IM0, ou qualquer vetor para IM1/IM2 conforme o modo
// de interrupcao corrente). Tira a CPU de HALT se necessario.
void z80_interrupt(Z80State *state, const Z80Bus *bus, uint16_t vector);

#ifdef __cplusplus
}
#endif
