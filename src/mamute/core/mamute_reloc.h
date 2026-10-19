/* fwMSX -- Mamute: tamanho de instrucao Z80 e onde ficam os operandos de endereco (C). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
 * Ver doc/mamute-spec.md. E' a base do XRT (relocar um programa) e do XFD (achar uma instrucao). */
#ifndef FWMSX_MAMUTE_RELOC_H
#define FWMSX_MAMUTE_RELOC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Decodifica a instrucao que comeca em b[0] (b tem `avail` bytes validos; o que faltar conta como 0) e devolve o tamanho
 * (1 a 4). *abs_off recebe a posicao, dentro da instrucao, do operando de 16 bits que e' um ENDERECO ABSOLUTO (JP, CALL, LD dd,nn,
 * LD (nn),HL...) ou -1; *rel_off recebe a posicao do deslocamento de um salto relativo (JR, DJNZ) ou -1. */
int mamute_insn_info(const uint8_t *b, int avail, int *abs_off, int *rel_off);

#ifdef __cplusplus
}
#endif

#endif
