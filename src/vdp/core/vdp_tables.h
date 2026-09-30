// fwMSX -- tabela de conversao de paleta do VDP (RGB 3+3+3 bits ->
// RGB888), gerada em Fortran (src/vdp/fortran/palette_table.f90).
// Codigo ORIGINAL do fwMSX (BSD-3-Clause) -- mesma tecnica ja usada em
// src/z80/core/z80_tables.h (tabelas de flag) e
// src/memmap/cpp/memory_system.cpp (CRC32): tabela global, calculada
// uma unica vez (idempotente), nunca por instancia de VdpState. Ver
// doc/vdp-spec.md, secao 3.4/6 (Fase 2).
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 512 entradas, indexadas por (r3*64 + g3*8 + b3), r3/g3/b3 em 0..7 --
// mesma convencao de indexacao de src/vdp/fortran/palette_table.f90.
extern uint8_t g_vdp_palette_table_r[512];
extern uint8_t g_vdp_palette_table_g[512];
extern uint8_t g_vdp_palette_table_b[512];

// Preenche as tres tabelas acima (via Fortran) na primeira chamada;
// chamadas seguintes sao no-op. Chamado de vdp_reset().
void vdp_tables_init(void);

#ifdef __cplusplus
}
#endif
