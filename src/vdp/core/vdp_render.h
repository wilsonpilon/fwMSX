// Adaptado de fMSX (resource/fMSX/fMSX/Common.h -- RefreshLine0/1/2),
// Copyright (C) Marat Fayzullin 1994-2021. O fwMSX evolui a partir do
// fMSX com o aval do autor original para adaptar/estudar seu codigo
// (ver README.md) -- isso nao e uma relicenciacao: este arquivo
// continua sob os termos originais dele (nao-comercial, aviso ao autor
// em caso de mudanca), nao o BSD-3-Clause do restante do fwMSX. Ver
// LICENSE-THIRD-PARTY.md.
//
// Decodificacao de pixel de verdade para os tres modos MSX1 mais
// simples: SCREEN 0 (TEXT 40x24), SCREEN 1 (TEXT 32x24 com cor por
// grupo de 8 caracteres, "Graphics 1"), SCREEN 2 (256x192 bitmap,
// "Graphics 2"). Ver doc/vdp-spec.md, secao 6 (Fase 2), para o
// detalhamento completo do escopo e das simplificacoes.
//
// Simplificacoes deliberadas em relacao ao fMSX (RefreshLine0/1/2 em
// Common.h): SEM borda/overscan (RefreshBorder() inteiro fica de fora
// -- so' a area ativa e' desenhada, por isso a largura de SCREEN 0 aqui
// e' 240px, nao 256px como as outras duas, ja' que os 16px de
// preenchimento de borda proprios do SCREEN 0 tambem ficam de fora);
// SEM sprites (Sprites(), Fase 3); SEM tratamento de ScreenON=0 (tela
// desligada) alem de mostrar a cor de fundo solida; SEM FontBuf/
// MSX_FIXEDFONT (recurso de conveniencia do fMSX para substituir a
// fonte por uma do host, nao existe no hardware real).
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
#define VDP_RENDER_WIDTH_STD 256    /* SCREEN 1/2 (e fallback): 32 colunas * 8px/char */
#define VDP_RENDER_HEIGHT 192

// Largura em pixels da linha de varredura para o modo de tela ATUAL de
// `v` (240 para SCREEN 0, 256 para SCREEN 1/2 e para qualquer modo
// ainda nao suportado -- ver vdp_render_line()).
int vdp_render_width(const VdpState *v);

// Renderiza a linha de varredura `y` (0..191) em `out_row` (alocado
// pelo chamador, pelo menos vdp_render_width(v) entradas). Modos fora
// de {0,1,2}: preenche `out_row` inteiro com a cor de fundo (regs[7] &
// 0x0F) em vez de decodificar pixels de verdade -- fallback
// deliberado, documentado, nao um crash nem memoria nao-inicializada.
void vdp_render_line(const VdpState *v, int y, VdpRgb888 *out_row);

// Renderiza as 192 linhas do modo de tela atual em `out_pixels`
// (alocado pelo chamador, vdp_render_width(v) * VDP_RENDER_HEIGHT
// entradas, linha-a-linha).
void vdp_render_frame(const VdpState *v, VdpRgb888 *out_pixels);

#ifdef __cplusplus
}
#endif
