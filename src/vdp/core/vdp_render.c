// Adaptado de fMSX (resource/fMSX/fMSX/Common.h) -- ver vdp_render.h
// para a nota de atribuicao completa e o escopo/simplificacoes desta
// Fase 2.
#include "vdp_render.h"
#include "vdp_sprites.h"

#define VRAM_MASK ((uint32_t)(VDP_VRAM_SIZE - 1))

// `chr_tab`/`col_tab`/`chr_gen` (ver vdp_state.h) sao deslocamentos
// (uint32_t) que podem legitimamente exceder VDP_VRAM_SIZE quando bits
// de pagina extra do MSX2 estao setados nos registradores -- diferente
// do fMSX (que usa ponteiros reais para dentro de uma VRAM
// potencialmente maior), aqui SEMPRE mascaramos o INDICE final com
// VRAM_MASK antes de acessar vram[], nunca o deslocamento em si (que
// continua fiel a formula exata do fMSX). Isso e' uma generalizacao
// segura da aritmetica de ponteiro do original: software bem-comportado
// (registradores dentro da faixa util para uma VRAM de 16KB) produz
// exatamente os mesmos resultados; so' software mal-comportado (fora do
// escopo de qualquer teste real) veria o efeito de "dar a volta" na
// VRAM em vez de ler memoria fora dos limites -- mais seguro que o
// original nesse caso extremo, nunca menos correto no caso normal.
static uint8_t VramAt(const VdpState *v, uint32_t offset) {
    return v->vram[offset & VRAM_MASK];
}

static VdpRgb888 PaletteRgb(const VdpState *v, uint8_t index) {
    const uint8_t j = index & 0x0F;
    VdpRgb888 out;
    out.r = v->palette_r[j];
    out.g = v->palette_g[j];
    out.b = v->palette_b[j];
    return out;
}

// RefreshLine0() do fMSX -- SCREEN 0, TEXT 40x24, monocromatico (uma
// unica cor de frente/fundo pra tela inteira, via regs[7]). SEM as duas
// faixas de preenchimento de borda do original (9px + 7px = 16px, que
// preenchiam o resto dos 256px do "slot" de video -- aqui a saida e'
// so' os 240px de conteudo real, ver vdp_render.h).
static void RenderLine0(const VdpState *v, int y, VdpRgb888 *out_row) {
    const VdpRgb888 bc = PaletteRgb(v, (uint8_t)(v->regs[7] & 0x0F));
    const VdpRgb888 fc = PaletteRgb(v, (uint8_t)(v->regs[7] >> 4));

    /* G=(...)+((Y+VScroll)&0x07): VScroll (regs[23]) so' afeta QUAL
       linha do glifo de 8x1 e' mostrada, nunca a linha de caracteres
       (T usa Y>>3 puro) -- fidelidade ao original, nao um erro. */
    const uint32_t g_base = v->chr_gen + (uint32_t)((y + v->regs[23]) & 0x07);
    const uint32_t t_base = v->chr_tab + 40u * (uint32_t)(y >> 3);

    for (int x = 0; x < 40; ++x) {
        const uint8_t code = VramAt(v, t_base + (uint32_t)x);
        const uint8_t bits = VramAt(v, g_base + ((uint32_t)code << 3));
        VdpRgb888 *p = out_row + x * 6;
        p[0] = (bits & 0x80) ? fc : bc;
        p[1] = (bits & 0x40) ? fc : bc;
        p[2] = (bits & 0x20) ? fc : bc;
        p[3] = (bits & 0x10) ? fc : bc;
        p[4] = (bits & 0x08) ? fc : bc;
        p[5] = (bits & 0x04) ? fc : bc;
    }
}

// RefreshLine1() do fMSX -- SCREEN 1, TEXT 32x24 com cor. Quirk real de
// hardware preservado: a cor (FC/BC) e' compartilhada por um GRUPO DE 8
// CODIGOS DE CARACTERE consecutivos (`ColTab[code>>3]`), nao por
// posicao na tela nem por caractere individual -- e' o "Graphics 1" da
// TMS9918, resolucao de cor mais grosseira que resolucao de caractere.
static void RenderLine1(const VdpState *v, int y, VdpRgb888 *out_row) {
    const int yy = (y + v->regs[23]) & 0xFF;
    const uint32_t g_base = v->chr_gen + (uint32_t)(yy & 0x07);
    const uint32_t t_base = v->chr_tab + ((uint32_t)(yy & 0xF8) << 2);

    for (int x = 0; x < 32; ++x) {
        const uint8_t code = VramAt(v, t_base + (uint32_t)x);
        const uint8_t color_byte = VramAt(v, v->col_tab + (uint32_t)(code >> 3));
        const VdpRgb888 fc = PaletteRgb(v, (uint8_t)(color_byte >> 4));
        const VdpRgb888 bc = PaletteRgb(v, (uint8_t)(color_byte & 0x0F));
        const uint8_t bits = VramAt(v, g_base + ((uint32_t)code << 3));
        VdpRgb888 *p = out_row + x * 8;
        p[0] = (bits & 0x80) ? fc : bc;
        p[1] = (bits & 0x40) ? fc : bc;
        p[2] = (bits & 0x20) ? fc : bc;
        p[3] = (bits & 0x10) ? fc : bc;
        p[4] = (bits & 0x08) ? fc : bc;
        p[5] = (bits & 0x04) ? fc : bc;
        p[6] = (bits & 0x02) ? fc : bc;
        p[7] = (bits & 0x01) ? fc : bc;
    }
}

// RefreshLine2() do fMSX -- SCREEN 2, 256x192 bitmap ("Graphics 2").
// Quirk real de hardware preservado: ColTab/ChrGen sao MASCARADOS
// (col_tab_mask/chr_gen_mask, ja calculados por RecomputeTables() em
// vdp_state.c) porque o modo so' tem 1/3 da tabela de cor/padrao que a
// aritmetica ingenua sugeriria -- os bits altos de Y (Y&0xC0) participam
// do indice ANTES da mascara, exatamente como no original.
static void RenderLine2(const VdpState *v, int y, VdpRgb888 *out_row) {
    const int yy = (y + v->regs[23]) & 0xFF;
    const uint32_t t_base = v->chr_tab + ((uint32_t)(yy & 0xF8) << 2);
    const uint32_t i_val = ((uint32_t)(yy & 0xC0) << 5) + (uint32_t)(yy & 0x07);

    for (int x = 0; x < 32; ++x) {
        const uint8_t code = VramAt(v, t_base + (uint32_t)x);
        const uint32_t j_val = (uint32_t)code << 3;
        const uint8_t color_byte = VramAt(v, v->col_tab + ((i_val + j_val) & v->col_tab_mask));
        const VdpRgb888 fc = PaletteRgb(v, (uint8_t)(color_byte >> 4));
        const VdpRgb888 bc = PaletteRgb(v, (uint8_t)(color_byte & 0x0F));
        const uint8_t bits = VramAt(v, v->chr_gen + ((i_val + j_val) & v->chr_gen_mask));
        VdpRgb888 *p = out_row + x * 8;
        p[0] = (bits & 0x80) ? fc : bc;
        p[1] = (bits & 0x40) ? fc : bc;
        p[2] = (bits & 0x20) ? fc : bc;
        p[3] = (bits & 0x10) ? fc : bc;
        p[4] = (bits & 0x08) ? fc : bc;
        p[5] = (bits & 0x04) ? fc : bc;
        p[6] = (bits & 0x02) ? fc : bc;
        p[7] = (bits & 0x01) ? fc : bc;
    }
}

int vdp_render_width(const VdpState *v) {
    return v->scr_mode == 0 ? VDP_RENDER_WIDTH_TEXT40 : VDP_RENDER_WIDTH_STD;
}

void vdp_render_line(const VdpState *v, int y, VdpRgb888 *out_row) {
    switch (v->scr_mode) {
        case 0:
            RenderLine0(v, y, out_row);
            return;
        case 1:
            RenderLine1(v, y, out_row);
            vdp_sprites_draw_line(v, y, out_row);
            return;
        case 2:
            RenderLine2(v, y, out_row);
            vdp_sprites_draw_line(v, y, out_row);
            return;
        default: {
            /* Modo ainda nao suportado (Fase 3+: sprites/MSX2/etc.) --
               preenche com a cor de fundo em vez de deixar memoria
               nao-inicializada ou tentar decodificar pixels errados. */
            const VdpRgb888 bc = PaletteRgb(v, (uint8_t)(v->regs[7] & 0x0F));
            for (int x = 0; x < VDP_RENDER_WIDTH_STD; ++x) out_row[x] = bc;
            return;
        }
    }
}

void vdp_render_frame(const VdpState *v, VdpRgb888 *out_pixels) {
    const int width = vdp_render_width(v);
    for (int y = 0; y < VDP_RENDER_HEIGHT; ++y) {
        vdp_render_line(v, y, out_pixels + (size_t)y * (size_t)width);
    }
}
