// Adaptado de fMSX (resource/fMSX/fMSX/Common.h e Wide.h --
// RefreshLine0..8 e RefreshLineTx80), Copyright (C) Marat Fayzullin
// 1994-2021. O fwMSX evolui a partir do fMSX com o aval do autor original
// para adaptar/estudar seu codigo (ver README.md) -- isso nao e uma
// relicenciacao: este arquivo continua sob os termos originais dele
// (nao-comercial, aviso ao autor em caso de mudanca), nao o
// BSD-3-Clause do restante do fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// Decodificacao de pixel de TODOS os modos de tela do MSX1/MSX2:
//   SCREEN 0 (TEXT 40x24 e TEXT80), 1 (Graphics 1), 2 (Graphics 2),
//   3 (Multicolor), 4 (Graphics 3), 5 (256x212x16), 6 (512x212x4),
//   7 (512x212x16), 8 (256x212x256). Ver doc/vdp-spec.md.
//
// Diferencas deliberadas em relacao ao fMSX:
//   - SEM borda/overscan: so' a area ativa e' desenhada (a borda e' a cor
//     de vdp_render_border_color(), que quem monta a imagem final usa para
//     preencher). Por isso a largura de SCREEN 0 e' 240px e a de TEXT80 480px.
//   - SCREEN 6/7 e TEXT80 saem com 512 pixels de verdade (como Wide.h do
//     fMSX), nao reduzidos a 256 como a compilacao NARROW.
//   - V9958 (MSX2+): SCREEN 10-12 (YJK/YAE, so' com R#25 bit 3/4 em scr 7/8),
//     scroll horizontal de 9 bits (R#26/R#27, HScroll512 em R#25 bit 0) em
//     SCREEN 5-8 e em YJK/YAE, e mascara da esquerda (R#25 bit 1). Ver
//     doc/msx2p-spec.md. Tudo so' no modelo VDP_MODEL_MSX2P.
//   - SEM FontBuf/MSX_FIXEDFONT (conveniencia do fMSX para trocar a fonte).
//   - A imagem e' montada por quadro (nao por scanline durante a execucao):
//     efeitos de rastreio no meio do quadro (paleta/scroll por linha) nao
//     aparecem.
#pragma once

#include <stdint.h>

#include "vdp_state.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VdpRgb888 {
    uint8_t r, g, b;
} VdpRgb888;

#define VDP_RENDER_WIDTH_TEXT40 240 /* SCREEN 0: 40 colunas * 6px/char */
#define VDP_RENDER_WIDTH_STD 256    /* SCREEN 1-5, 8 (e fallback) */
#define VDP_RENDER_WIDTH_TEXT80 480 /* TEXT80: 80 colunas * 6px/char */
#define VDP_RENDER_WIDTH_WIDE 512   /* SCREEN 6 e 7 */
#define VDP_RENDER_HEIGHT 192       /* altura minima; MSX2 pode ter 212 (ver vdp_render_height) */
#define VDP_RENDER_MAX_WIDTH 512

// Largura em pixels da linha de varredura para o modo de tela ATUAL de
// `v`: 240 (SCREEN 0 de 40 colunas), 480 (TEXT80), 512 (SCREEN 6/7) ou 256.
int vdp_render_width(const VdpState *v);

// Altura da imagem: 212 se o MSX2 ligou R#9 bit 7, senao 192.
int vdp_render_height(const VdpState *v);

// Cor da borda/fundo do modo atual (usada para preencher o que a area ativa
// nao cobre, e para a tela desligada).
VdpRgb888 vdp_render_border_color(const VdpState *v);

// Renderiza a linha de varredura `y` (0..191 ou 0..211) em `out_row`
// (alocado pelo chamador, pelo menos VDP_RENDER_MAX_WIDTH entradas).
void vdp_render_line(const VdpState *v, int y, VdpRgb888 *out_row);

// Renderiza as vdp_render_height(v) linhas do modo atual em `out_pixels`
// (alocado pelo chamador, vdp_render_width(v) * vdp_render_height(v) entradas).
void vdp_render_frame(const VdpState *v, VdpRgb888 *out_pixels);

#ifdef __cplusplus
}
#endif
