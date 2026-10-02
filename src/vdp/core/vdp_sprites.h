// Adaptado de fMSX (resource/fMSX/fMSX/Common.h -- Sprites();
// resource/fMSX/fMSX/MSX.c -- CheckSprites() e o trecho de status de
// sprite de LoopZ80()), Copyright (C) Marat Fayzullin 1994-2021. O fwMSX
// evolui a partir do fMSX com o aval do autor original para adaptar/
// estudar seu codigo (ver README.md) -- isso nao e uma relicenciacao:
// este arquivo continua sob os termos originais dele (nao-comercial,
// aviso ao autor em caso de mudanca), nao o BSD-3-Clause do restante do
// fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// Sprites de "modo 1" do TMS9918 (SCREEN 1/2/3): ate' 32 sprites de 8x8
// ou 16x16 (opcionalmente ampliados 2x), 4 por linha de varredura, flag
// de "quinto sprite" e flag de colisao. Ver doc/vdp-spec.md, secao 6
// (Fase 3).
//
// Sprites de "modo 2" (SCREEN 4-8, ColorSprites() do fMSX, V9938): cor por
// linha de sprite, ate' 8 por linha, bits CC (OR de cores)/IC/EC (early
// clock) -- ver vdp_sprites_color_line(). Status do "9o sprite" e colisao
// tambem valem nesses modos.
//
// Simplificacoes deliberadas em relacao ao fMSX:
//   - SEM a opcao MSX_ALLSPRITE (desenhar alem do 4o sprite por linha):
//     comportamento de hardware real, como o fMSX com a opcao desligada.
//   - Tecnica de mascara de bits do original (K&=...) substituida por
//     recorte pixel a pixel -- mesmo resultado, mais legivel; verificado
//     por testes contra os casos de borda (X negativo/early-clock, borda
//     direita, Y negativo, ampliacao).
//   - O mesmo fMSX aplica VScroll (R#23, recurso de MSX2) duas vezes no
//     eixo Y dos sprites em SCREEN 1 (uma em RefreshLine1(), outra em
//     Sprites()) -- preservado como esta: com R#23=0 (todo software
//     MSX1) nao faz diferenca.
//   - ScreenON=0 nao esconde sprites (consistente com vdp_render.h, que
//     tambem nao trata tela desligada alem da cor de fundo).
#pragma once

#include <stdint.h>

#include "vdp_render.h"
#include "vdp_state.h"

#ifdef __cplusplus
extern "C" {
#endif

// Desenha por cima de `row` (ja contendo o fundo da linha `y`, 256px) os
// sprites visiveis nessa linha. No-op fora de SCREEN 1/2/3 ou com
// sprites desligados (R#8 bit 1). Nao altera `v` (so' le).
void vdp_sprites_draw_line(const VdpState *v, int y, VdpRgb888 *row);

// Sprites COLORIDOS de modo 2 (SCREEN 4-8, ColorSprites() do fMSX): cada linha
// de cada sprite tem sua propria cor (e bits CC/IC/EC) numa tabela de 16 bytes
// por sprite logo ANTES da tabela de atributos (spr_tab - 200h). Ate' 8 por
// linha. Preenche `zbuf` (320 bytes = 32 + 256 + 32 de margem para sprites
// parcialmente fora da tela) com o indice de cor de cada pixel (0 = sem
// sprite); o pixel x da tela esta' em zbuf[32 + x]. Nao altera `v`.
void vdp_sprites_color_line(const VdpState *v, int y, uint8_t *zbuf);

// Efeito colateral de status de Sprites() do fMSX: recalcula os bits do
// "quinto sprite" (S#0 bit 6 = flag, bits 4-0 = numero do ultimo sprite
// checado) para a linha `y`. Chamado por vdp_step_scanline() para cada
// linha visivel. Preserva os bits 7 (VBlank) e 5 (colisao).
void vdp_sprites_update_status(VdpState *v, int y);

// CheckSprites() do fMSX: true se algum par de sprites validos tem
// pixels acesos sobrepostos. Chamado por vdp_step_scanline() na linha
// 192, setando S#0 bit 5.
int vdp_sprites_check_collision(const VdpState *v);

#ifdef __cplusplus
}
#endif
