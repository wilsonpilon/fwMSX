// Aceleracao em Assembly para o bloco de transferencia LDIR/LDDR -- design
// proprio (BSD-3-Clause), nao adaptado do fMSX. Ver
// src/z80/asm/block_ops.asm para a implementacao (NASM, dual-ABI Win64/
// SysV AMD64) e doc/z80-core-spec.md, secao 3.4, para o raciocinio.
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Copia `len` bytes entre `dst` e `src`. `reverse == 0` copia em ordem
// crescente de endereco (LDIR); `reverse != 0` copia em ordem decrescente
// (LDDR) -- ATENCAO: no caso reverse!=0, `dst`/`src` devem apontar para o
// ULTIMO byte de cada regiao (isto e', `base + len - 1`), nao o primeiro.
// Ver o comentario detalhado no topo de block_ops.asm antes de chamar
// este caso.
void z80_fast_block_move(uint8_t *dst, const uint8_t *src, uint16_t len, int reverse);

// Procura `value` em `len` bytes a partir de `p` (CPIR: para cima; `reverse != 0`
// (CPDR): para baixo, com `p` apontando para o ULTIMO byte da regiao). Devolve
// quantos bytes foram examinados, de 1 a `len` (0 se len == 0): o ultimo
// examinado e' o casado, ou o ultimo da regiao se nao houve casamento. Ver
// o comentario em block_ops.asm.
int z80_fast_block_search(const uint8_t *p, uint16_t len, uint8_t value, int reverse);

#ifdef __cplusplus
}
#endif
