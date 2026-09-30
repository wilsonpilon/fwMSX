// fwMSX -- ver vdp_tables.h. Codigo ORIGINAL do fwMSX (BSD-3-Clause);
// os DADOS resultantes replicam a formula de conversao de paleta do
// fMSX (resource/fMSX/fMSX/MSX.c, caso 9Ah de WrZ80), mas o mecanismo
// de tabela global pre-computada e' design proprio -- ver
// src/vdp/fortran/palette_table.f90 para o raciocinio completo.
#include "vdp_tables.h"

uint8_t g_vdp_palette_table_r[512];
uint8_t g_vdp_palette_table_g[512];
uint8_t g_vdp_palette_table_b[512];

// Implementada em src/vdp/fortran/palette_table.f90. Nome exposto via
// bind(c, name=...), sem name mangling a considerar aqui.
extern void vdp_build_palette_table(uint8_t r_table[512], uint8_t g_table[512], uint8_t b_table[512]);

static int g_vdp_tables_ready = 0;

void vdp_tables_init(void) {
    if (g_vdp_tables_ready) return;
    vdp_build_palette_table(g_vdp_palette_table_r, g_vdp_palette_table_g, g_vdp_palette_table_b);
    g_vdp_tables_ready = 1;
}
