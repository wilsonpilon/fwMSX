// fwMSX -- tipos/constantes compartilhados do VDP (V9938/TMS9918).
// Codigo ORIGINAL do fwMSX (BSD-3-Clause) -- os NUMEROS aqui (tamanho de
// VRAM, modo maximo de tela, bits de interrupcao) sao fatos de hardware
// do MSX/V9938, nao "expressao" do fMSX, mas os nomes/organizacao sao
// design proprio. Ver doc/vdp-spec.md, secao 3.
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Fase 1 (ver doc/vdp-spec.md, secao 6): UMA pagina de 16KB de VRAM --
// simplificacao deliberada. O V9938 real suporta ate 8 paginas de 16KB
// (128KB) via o registrador 14; paginacao de VRAM multi-pagina fica para
// quando a Fase 2 (renderizacao) precisar de verdade de mais de 16KB
// (modos SCREEN 5-8 do MSX2 tipicamente usam 64-128KB). Com 1 pagina,
// VDP_VRAM_PAGES=1 faz `regs[14] &= (VDP_VRAM_PAGES-1)` sempre dar 0 --
// o rollover de pagina em vdp_in()/vdp_out() vira um no-op observavel,
// nao removido do codigo (mantido fiel ao fMSX, so' inofensivo agora).
#define VDP_VRAM_SIZE 0x4000
#define VDP_VRAM_PAGES 1

// MAXSCREEN do fMSX (resource/fMSX/fMSX/MSX.h) -- maior modo de tela
// numerado (SCREEN 12); MAXSCREEN+1 e' o modo especial TEXT80 (SCREEN 0
// de 80 colunas do MSX2+). Fato de hardware/nomenclatura do fMSX, nao
// dado inventado aqui.
#define VDP_MAXSCREEN 12

// Bits de interrupcao pendente -- mesmos valores de INT_IE0/INT_IE1 em
// resource/fMSX/fMSX/MSX.h (INT_IE2 do fMSX nao tem uso conhecido no
// VDP em si, omitido). Note que isso e' um bitmask LOCAL ao VDP (o
// "IRQPending" do fMSX), sem nenhuma relacao com as constantes
// Z80_INT_* do nucleo Z80 (que sao vetores de interrupcao do Z80, nao
// bits de fonte de interrupcao do VDP) -- os dois "INT_" so' coincidem
// de nome com o fMSX, nao de significado.
#define VDP_INT_IE0 0x01 /* VBlank */
#define VDP_INT_IE1 0x02 /* HBlank / coincidencia de linha */

#ifdef __cplusplus
}
#endif
