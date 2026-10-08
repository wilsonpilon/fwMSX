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
#define VDP_VRAM_SIZE 0x20000
#define VDP_VRAM_PAGES 8

// MSX1 (TMS9918): 16KB de VRAM, 1 pagina; MSX2 (V9938): 128KB, 8 paginas de
// 16KB selecionadas pelo registrador 14. O VdpState sempre aloca os 128KB e o
// modelo escolhe quanto usa (vram_pages/vram_mask) -- ver vdp_set_model().
#define VDP_MODEL_MSX1 0
#define VDP_MODEL_MSX2 1
/* V9958 (MSX2+): V9938 mais modos YJK/YAE (SCREEN 10-12), R#25-R#27 e o bit 2 de
   S#1. Tudo que e' "V9938" vale para ele tambem (ver VDP_MODEL_IS_V9938). */
#define VDP_MODEL_MSX2P 2
#define VDP_MODEL_IS_V9938(m) ((m) >= VDP_MODEL_MSX2)

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

// Linhas de varredura por quadro: 0..261 (NTSC) ou 0..311 (PAL) -- ver
// ScanLine em vdp_step_scanline(). Usado para dimensionar o snapshot por
// linha (VdpScanlineSnapshot, ver vdp_state.h) que guarda o estado dos
// registradores/paleta/cache de tabela EM CADA linha, para o renderizador
// poder reproduzir efeitos de rastreio (paleta/scroll trocados no meio do
// quadro por uma interrupcao IE1) -- ver doc/vdp-spec.md, secao 2.
#define VDP_MAX_SCANLINES 313

#ifdef __cplusplus
}
#endif
