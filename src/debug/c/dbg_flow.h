// fwMSX -- depurador: classificacao do fluxo de uma instrucao Z80 (C). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Usado por "next" (passo sobre): decide se a instrucao em PC e' uma chamada/laco que
// deve ser executada por inteiro. Ver doc/debug-spec.md.
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DBG_FLOW_PLAIN = 0,  // instrucao comum: "next" e' igual a "stepi"
    DBG_FLOW_CALL = 1,   // CALL nn, CALL cc,nn e RST n: executa a subrotina inteira
    DBG_FLOW_BLOCK = 2,  // LDIR/LDDR/CPIR/CPDR/INIR/INDR/OTIR/OTDR: executa o bloco inteiro
    DBG_FLOW_HALT = 3    // HALT: roda ate' a proxima interrupcao
};

// `bytes` aponta para os 2 primeiros bytes da instrucao (o segundo so' importa depois do prefixo ED).
int dbg_flow_kind(const uint8_t *bytes);

#ifdef __cplusplus
}
#endif
