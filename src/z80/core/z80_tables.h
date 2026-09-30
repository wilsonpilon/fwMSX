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
// Nota (Fase 2, ver doc/z80-core-spec.md secao 3.5/6): g_z80_zs_table e
// g_z80_pzs_table NAO sao mais literais -- sao preenchidas em tempo de
// execucao por z80_build_flag_tables() (src/z80/fortran/flag_tables.f90)
// via z80_tables_init(), chamada de dentro de z80_reset(). Por isso nao
// sao mais `const`. g_z80_daa_table continua literal em C (ver a nota em
// z80_tables.c sobre por que essa tabela ficou de fora do escopo do
// Fortran).
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

extern uint8_t g_z80_zs_table[256];
extern uint8_t g_z80_pzs_table[256];
extern const uint16_t g_z80_daa_table[2048];

// Preenche g_z80_zs_table/g_z80_pzs_table (via Fortran -- ver
// src/z80/fortran/flag_tables.f90). Idempotente: chamadas depois da
// primeira nao fazem nada. Chamada automaticamente por z80_reset(); nao
// e' preciso chamar na mao, mas e' seguro fazer isso mais de uma vez.
void z80_tables_init(void);

#ifdef __cplusplus
}
#endif
