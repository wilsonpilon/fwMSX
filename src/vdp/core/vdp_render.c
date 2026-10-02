// Adaptado de fMSX (Common.h, Wide.h) -- ver vdp_render.h para a nota de
// atribuicao completa e as diferencas deliberadas.
#include "vdp_render.h"

#include <string.h>

#include "vdp_sprites.h"

// `chr_tab`/`col_tab`/`chr_gen` (ver vdp_state.h) sao deslocamentos (uint32_t)
// que podem exceder a VRAM em uso quando bits de pagina extra estao setados
// nos registradores -- diferente do fMSX (ponteiros reais para dentro de uma
// VRAM maior), aqui SEMPRE mascaramos o INDICE final com vram_mask antes de
// acessar vram[], nunca o deslocamento em si (que continua fiel a formula
// exata do fMSX). Software bem-comportado produz exatamente os mesmos
// resultados; so' o mal-comportado "da' a volta" na VRAM em vez de ler fora
// dos limites.
static uint8_t VramAt(const VdpState *v, uint32_t offset) {
    return v->vram[offset & v->vram_mask];
}

/* --- paleta ---------------------------------------------------------------- */

/* Paleta efetiva de 16 cores: XPal[] do fMSX. A cor 0 e' "transparente": mostra
 * a cor de fundo (R#7), a menos que o fundo seja 0 ou R#8 bit 5 (TP) force a
 * cor 0 a ser solida (SolidColor0). */
typedef struct PalSet {
    VdpRgb888 c[16];
} PalSet;

static void BuildPal(const VdpState *v, PalSet *p) {
    const int bg = v->regs[7] & 0x0F;
    int i;
    for (i = 0; i < 16; ++i) {
        p->c[i].r = v->palette_r[i];
        p->c[i].g = v->palette_g[i];
        p->c[i].b = v->palette_b[i];
    }
    if (bg && !(v->regs[8] & 0x20)) p->c[0] = p->c[bg];
}

/* Paleta fixa de SCREEN 8 (GRB 3-3-2): BPal[] do Unix.c do fMSX. */
static VdpRgb888 g_bpal[256];
static int g_bpal_ready = 0;

static void EnsureBpal(void) {
    int j;
    if (g_bpal_ready) return;
    for (j = 0; j < 256; ++j) {
        g_bpal[j].r = (uint8_t)(((j >> 2) & 0x07) * 255 / 7);
        g_bpal[j].g = (uint8_t)(((j >> 5) & 0x07) * 255 / 7);
        g_bpal[j].b = (uint8_t)((j & 0x03) * 255 / 3);
    }
    g_bpal_ready = 1;
}

static int ScreenOn(const VdpState *v) { return (v->regs[1] & 0x40) != 0; }

static void Fill(VdpRgb888 *row, int n, VdpRgb888 c) {
    int i;
    for (i = 0; i < n; ++i) row[i] = c;
}

/* --- modos de texto ---------------------------------------------------------- */

/* SCREEN 0, TEXT 40x24, monocromatico (cor de frente/fundo da tela inteira, via
 * R#7). VScroll so' afeta QUAL linha do glifo e' mostrada, nunca a linha de
 * caracteres (T usa y>>3 puro) -- fidelidade ao original. */
static void RenderLine0(const VdpState *v, const PalSet *pal, int y, VdpRgb888 *out) {
    const VdpRgb888 bc = pal->c[v->regs[7] & 0x0F];
    const VdpRgb888 fc = pal->c[v->regs[7] >> 4];
    const uint32_t g_base = v->chr_gen + (uint32_t)((y + v->regs[23]) & 0x07);
    const uint32_t t_base = v->chr_tab + 40u * (uint32_t)(y >> 3);
    int x;

    for (x = 0; x < 40; ++x) {
        const uint8_t code = VramAt(v, t_base + (uint32_t)x);
        const uint8_t bits = VramAt(v, g_base + ((uint32_t)code << 3));
        VdpRgb888 *p = out + x * 6;
        p[0] = (bits & 0x80) ? fc : bc;
        p[1] = (bits & 0x40) ? fc : bc;
        p[2] = (bits & 0x20) ? fc : bc;
        p[3] = (bits & 0x10) ? fc : bc;
        p[4] = (bits & 0x08) ? fc : bc;
        p[5] = (bits & 0x04) ? fc : bc;
    }
}

/* TEXT80 (SCREEN 0 com WIDTH 80): 80x24, 6px por caractere = 480px. A tabela
 * de cor (1 bit por caractere) escolhe entre as cores normais (R#7) e as de
 * piscar (R#12 / XFG/XBG). */
static void RenderLineTx80(const VdpState *v, const PalSet *pal, int y, VdpRgb888 *out) {
    const uint32_t g_base = v->chr_gen + (uint32_t)((y + v->regs[23]) & 0x07);
    const uint32_t t_base = v->chr_tab + ((80u * (uint32_t)(y >> 3)) & v->chr_tab_mask);
    uint32_t c_base = v->col_tab + ((10u * (uint32_t)(y >> 3)) & v->col_tab_mask);
    const VdpRgb888 fg = pal->c[v->regs[7] >> 4];
    const VdpRgb888 bg = pal->c[v->regs[7] & 0x0F];
    const VdpRgb888 xfg = pal->c[v->x_fg];
    const VdpRgb888 xbg = pal->c[v->x_bg];
    uint8_t m = 0;
    int x;

    for (x = 0; x < 80; ++x) {
        VdpRgb888 fc, bc;
        uint8_t bits;
        VdpRgb888 *p = out + x * 6;
        if (!(x & 0x07)) m = VramAt(v, c_base++);
        if (m & 0x80) { fc = xfg; bc = xbg; }
        else { fc = fg; bc = bg; }
        m = (uint8_t)(m << 1);
        bits = VramAt(v, g_base + ((uint32_t)VramAt(v, t_base + (uint32_t)x) << 3));
        p[0] = (bits & 0x80) ? fc : bc;
        p[1] = (bits & 0x40) ? fc : bc;
        p[2] = (bits & 0x20) ? fc : bc;
        p[3] = (bits & 0x10) ? fc : bc;
        p[4] = (bits & 0x08) ? fc : bc;
        p[5] = (bits & 0x04) ? fc : bc;
    }
}

/* --- modos de caractere/bloco de 256 pixels ------------------------------------ */

/* SCREEN 1: TEXT 32x24 com cor. Quirk real de hardware: a cor (FC/BC) e'
 * compartilhada por um GRUPO DE 8 CODIGOS DE CARACTERE (ColTab[code>>3]). */
static void RenderLine1(const VdpState *v, const PalSet *pal, int y, VdpRgb888 *out) {
    const int yy = (y + v->regs[23]) & 0xFF;
    const uint32_t g_base = v->chr_gen + (uint32_t)(yy & 0x07);
    const uint32_t t_base = v->chr_tab + ((uint32_t)(yy & 0xF8) << 2);
    int x;

    for (x = 0; x < 32; ++x) {
        const uint8_t code = VramAt(v, t_base + (uint32_t)x);
        const uint8_t color_byte = VramAt(v, v->col_tab + (uint32_t)(code >> 3));
        const VdpRgb888 fc = pal->c[color_byte >> 4];
        const VdpRgb888 bc = pal->c[color_byte & 0x0F];
        const uint8_t bits = VramAt(v, g_base + ((uint32_t)code << 3));
        VdpRgb888 *p = out + x * 8;
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

/* SCREEN 2 (e a base de SCREEN 4): 256x192 bitmap. ColTab/ChrGen sao
 * MASCARADOS (col_tab_mask/chr_gen_mask) porque o modo so' tem 1/3 da tabela
 * que a aritmetica ingenua sugeriria; os bits altos de Y participam do indice
 * ANTES da mascara, como no original. `zbuf` (NULL em SCREEN 2) traz os
 * sprites coloridos de modo 2 (SCREEN 4). */
static void RenderLineBlock(const VdpState *v, const PalSet *pal, int y, const uint8_t *zbuf, VdpRgb888 *out) {
    const int yy = (y + v->regs[23]) & 0xFF;
    const uint32_t t_base = v->chr_tab + ((uint32_t)(yy & 0xF8) << 2);
    const uint32_t i_val = ((uint32_t)(yy & 0xC0) << 5) + (uint32_t)(yy & 0x07);
    int x;

    for (x = 0; x < 32; ++x) {
        const uint8_t code = VramAt(v, t_base + (uint32_t)x);
        const uint32_t j_val = (uint32_t)code << 3;
        const uint8_t color_byte = VramAt(v, v->col_tab + ((i_val + j_val) & v->col_tab_mask));
        const VdpRgb888 fc = pal->c[color_byte >> 4];
        const VdpRgb888 bc = pal->c[color_byte & 0x0F];
        const uint8_t bits = VramAt(v, v->chr_gen + ((i_val + j_val) & v->chr_gen_mask));
        VdpRgb888 *p = out + x * 8;
        int b;
        for (b = 0; b < 8; ++b) {
            const uint8_t c = zbuf ? zbuf[x * 8 + b] : 0;
            p[b] = c ? pal->c[c] : ((bits & (0x80 >> b)) ? fc : bc);
        }
    }
}

/* SCREEN 3: multicolor, 64x48 blocos de 4x4 pixels (2 cores por caractere). */
static void RenderLine3(const VdpState *v, const PalSet *pal, int y, VdpRgb888 *out) {
    const int yy = (y + v->regs[23]) & 0xFF;
    const uint32_t t_base = v->chr_tab + ((uint32_t)(yy & 0xF8) << 2);
    const uint32_t g_base = v->chr_gen + (uint32_t)((yy & 0x1C) >> 2);
    int x;

    for (x = 0; x < 32; ++x) {
        const uint8_t k = VramAt(v, g_base + ((uint32_t)VramAt(v, t_base + (uint32_t)x) << 3));
        const VdpRgb888 left = pal->c[k >> 4];
        const VdpRgb888 right = pal->c[k & 0x0F];
        VdpRgb888 *p = out + x * 8;
        p[0] = p[1] = p[2] = p[3] = left;
        p[4] = p[5] = p[6] = p[7] = right;
    }
}

/* --- modos bitmap do MSX2 ------------------------------------------------------- */

/* Endereco da linha de bitmap: ((y+VScroll) << shift) & ChrTabM & mascara de
 * pagina, somado a chr_tab (que ja' tem a pagina de R#2). */
static uint32_t BitmapRow(const VdpState *v, int y, int shift, uint32_t page_mask) {
    return v->chr_tab + ((((uint32_t)(y + v->regs[23])) << shift) & v->chr_tab_mask & page_mask);
}

static void RenderLine5(const VdpState *v, const PalSet *pal, int y, const uint8_t *z, VdpRgb888 *out) {
    const uint32_t t = BitmapRow(v, y, 7, 0x7FFF);
    int x;
    for (x = 0; x < 128; ++x) {
        const uint8_t b = VramAt(v, t + (uint32_t)x);
        const int i0 = z[x * 2], i1 = z[x * 2 + 1];
        out[x * 2] = pal->c[i0 ? i0 : (b >> 4)];
        out[x * 2 + 1] = pal->c[i1 ? i1 : (b & 0x0F)];
    }
}

static void RenderLine6(const VdpState *v, const PalSet *pal, int y, const uint8_t *z, VdpRgb888 *out) {
    const uint32_t t = BitmapRow(v, y, 7, 0x7FFF);
    int x;
    /* 512 pixels de 2 bits; os sprites tem 256 de largura (cada um cobre 2 pixels) */
    for (x = 0; x < 128; ++x) {
        const uint8_t b = VramAt(v, t + (uint32_t)x);
        const int c0 = z[x * 2], c1 = z[x * 2 + 1];
        out[x * 4] = pal->c[c0 ? c0 : (b >> 6)];
        out[x * 4 + 1] = pal->c[c0 ? c0 : ((b >> 4) & 0x03)];
        out[x * 4 + 2] = pal->c[c1 ? c1 : ((b >> 2) & 0x03)];
        out[x * 4 + 3] = pal->c[c1 ? c1 : (b & 0x03)];
    }
}

static void RenderLine7(const VdpState *v, const PalSet *pal, int y, const uint8_t *z, VdpRgb888 *out) {
    const uint32_t t = BitmapRow(v, y, 8, 0xFFFF);
    int x;
    for (x = 0; x < 256; ++x) {
        const uint8_t b = VramAt(v, t + (uint32_t)x);
        const int c = z[x];
        out[x * 2] = pal->c[c ? c : (b >> 4)];
        out[x * 2 + 1] = pal->c[c ? c : (b & 0x0F)];
    }
}

static void RenderLine8(const VdpState *v, int y, const uint8_t *z, VdpRgb888 *out) {
    /* Cor do sprite -> byte GRB do SCREEN 8 (SprToScr[] do fMSX) */
    static const uint8_t spr_to_scr[16] = {0x00, 0x02, 0x10, 0x12, 0x80, 0x82, 0x90, 0x92,
                                           0x49, 0x4B, 0x59, 0x5B, 0xC9, 0xCB, 0xD9, 0xDB};
    const uint32_t t = BitmapRow(v, y, 8, 0xFFFF);
    int x;
    EnsureBpal();
    for (x = 0; x < 256; ++x) {
        const int c = z[x];
        out[x] = g_bpal[c ? spr_to_scr[c] : VramAt(v, t + (uint32_t)x)];
    }
}

/* --- interface -------------------------------------------------------------------- */

int vdp_render_width(const VdpState *v) {
    switch (v->scr_mode) {
    case 0: return VDP_RENDER_WIDTH_TEXT40;
    case 6:
    case 7: return VDP_RENDER_WIDTH_WIDE;
    case VDP_MAXSCREEN + 1: return VDP_RENDER_WIDTH_TEXT80;
    default: return VDP_RENDER_WIDTH_STD;
    }
}

int vdp_render_height(const VdpState *v) {
    return (v->model == VDP_MODEL_MSX2 && (v->regs[9] & 0x80)) ? 212 : VDP_RENDER_HEIGHT;
}

VdpRgb888 vdp_render_border_color(const VdpState *v) {
    PalSet pal;
    BuildPal(v, &pal);
    switch (v->scr_mode) {
    case 8:
        EnsureBpal();
        return g_bpal[v->regs[7]];
    case 6: return pal.c[v->regs[7] & 0x03];
    default: return pal.c[v->regs[7] & 0x0F];
    }
}

void vdp_render_line(const VdpState *v, int y, VdpRgb888 *out_row) {
    PalSet pal;
    uint8_t zbuf[320];
    const int width = vdp_render_width(v);

    BuildPal(v, &pal);

    /* Tela desligada (R#1 bit 6): so' a cor de fundo */
    if (!ScreenOn(v)) {
        Fill(out_row, width, vdp_render_border_color(v));
        return;
    }

    switch (v->scr_mode) {
    case 0:
        RenderLine0(v, &pal, y, out_row);
        return;
    case VDP_MAXSCREEN + 1:
        RenderLineTx80(v, &pal, y, out_row);
        return;
    case 1:
        RenderLine1(v, &pal, y, out_row);
        vdp_sprites_draw_line(v, y, out_row);
        return;
    case 2:
        RenderLineBlock(v, &pal, y, NULL, out_row);
        vdp_sprites_draw_line(v, y, out_row);
        return;
    case 3:
        RenderLine3(v, &pal, y, out_row);
        vdp_sprites_draw_line(v, y, out_row);
        return;
    case 4:
        vdp_sprites_color_line(v, y, zbuf);
        RenderLineBlock(v, &pal, y, zbuf + 32, out_row);
        return;
    case 5:
        vdp_sprites_color_line(v, y, zbuf);
        RenderLine5(v, &pal, y, zbuf + 32, out_row);
        return;
    case 6:
        vdp_sprites_color_line(v, y, zbuf);
        RenderLine6(v, &pal, y, zbuf + 32, out_row);
        return;
    case 7:
        vdp_sprites_color_line(v, y, zbuf);
        RenderLine7(v, &pal, y, zbuf + 32, out_row);
        return;
    case 8:
        vdp_sprites_color_line(v, y, zbuf);
        RenderLine8(v, y, zbuf + 32, out_row);
        return;
    default:
        /* Modo nao suportado (SCREEN 9-12: V9958): cor de fundo */
        Fill(out_row, width, vdp_render_border_color(v));
        return;
    }
}

void vdp_render_frame(const VdpState *v, VdpRgb888 *out_pixels) {
    const int width = vdp_render_width(v);
    const int height = vdp_render_height(v);
    int y;
    for (y = 0; y < height; ++y) vdp_render_line(v, y, out_pixels + (size_t)y * (size_t)width);
}
