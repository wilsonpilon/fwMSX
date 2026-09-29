// Adaptado de fMSX (resource/fMSX/Z80/Tables.h), Copyright (C) Marat
// Fayzullin 1994-2021. O fwMSX evolui a partir do fMSX com o aval do
// autor original para adaptar/estudar seu codigo (ver README.md) -- isso
// nao e uma relicenciacao: este arquivo continua sob os termos originais
// dele (nao-comercial, aviso ao autor em caso de mudanca), nao o
// BSD-3-Clause do restante do fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// Tabelas de temporizacao (ciclos por opcode) e de flags pre-computadas
// (Sign/Zero, Parity/Zero/Sign, correcao DAA) usadas pelo dispatcher em
// z80_core.c. Ver doc/z80-core-spec.md, secao 2 e 3.2 -- essas tabelas
// sao a tecnica central de desempenho do nucleo inteiro (flags via
// lookup, em vez de recalcular paridade/sinal a cada instrucao).
//
// Nota: nesta Fase 1, g_z80_zs_table/g_z80_pzs_table sao arrays C
// estaticos, como no fMSX. A Fase 2 (ver doc/z80-core-spec.md, secao 6)
// move a GERACAO desses dois arrays para Fortran -- ver o design na
// secao 3.5 do documento.
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t g_z80_cycles[256];
extern const uint8_t g_z80_cycles_cb[256];
extern const uint8_t g_z80_cycles_ed[256];
extern const uint8_t g_z80_cycles_xx[256];
extern const uint8_t g_z80_cycles_xxcb[256];

extern const uint8_t g_z80_zs_table[256];
extern const uint8_t g_z80_pzs_table[256];
extern const uint16_t g_z80_daa_table[2048];

#ifdef __cplusplus
}
#endif
