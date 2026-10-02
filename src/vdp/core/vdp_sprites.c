// Adaptado de fMSX (Common.h -- Sprites(); MSX.c -- CheckSprites()) --
// ver vdp_sprites.h para a nota de atribuicao e simplificacoes.
#include "vdp_sprites.h"

#include <string.h>

#define MAXSPRITE1 4 /* sprites por linha em SCREEN 1-3 (MSX.h) */

static uint8_t VramAt(const VdpState *v, uint32_t offset) {
    return v->vram[offset & v->vram_mask];
}

#define MAXSPRITE2 8 /* sprites por linha em SCREEN 4-8 (MSX.h) */

/* Sprites existem em SCREEN 1-8 (nao no texto); R#8 bit 1 os desliga. */
static int SpritesActive(const VdpState *v) {
    return v->scr_mode >= 1 && v->scr_mode <= 8 && !(v->regs[8] & 0x02);
}

/* SCREEN 4-8 usam sprites de modo 2 (coloridos) */
static int ColorMode(const VdpState *v) { return v->scr_mode >= 4; }

typedef struct SpriteScan {
    int count;       /* sprites marcados para desenhar, em ordem de indice */
    uint8_t idx[MAXSPRITE1];
    int fifth;       /* 5o sprite encontrado nesta linha */
    int last;        /* "L" do original: numero do ultimo sprite checado */
} SpriteScan;

// Primeira metade de Sprites(): percorre a tabela de atributos, marca os
// sprites que cruzam a linha (ate' 4) e detecta o 5o. `y` ja' e' o Y
// efetivo (com VScroll aplicado a mais, ver nota no .h).
static void Scan(const VdpState *v, uint8_t y, SpriteScan *s) {
    static const uint8_t heights[4] = {8, 16, 16, 32};
    const int oh = heights[v->regs[1] & 0x03];
    const int ih = heights[v->regs[1] & 0x02];
    int c = MAXSPRITE1 + 1;
    int l;

    s->count = 0;
    s->fifth = 0;
    for (l = 0; l < 32; ++l) {
        int k = VramAt(v, v->spr_tab + (uint32_t)l * 4u);
        if (k == 208) break;
        if (k > 256 - ih) k -= 256;
        if (y > k && y <= k + oh) {
            if (!--c) {
                s->fifth = 1;
                break; /* sem MSX_ALLSPRITE: 5o sprite nao e' desenhado */
            }
            s->idx[s->count++] = (uint8_t)l;
        }
    }
    s->last = l < 32 ? l : 31;
}

/* Varredura de modo 2 (primeira metade de ColorSprites()): marca ate' 8
 * sprites que cruzam a linha `y`, detecta o 9o e o "ultimo checado". O fim da
 * lista e' o Y=216 (208 no modo 1) e o VScroll vale uma vez so'. */
typedef struct ColorScan {
    int count;
    uint8_t idx[MAXSPRITE2];
    int ninth;
    int last;
} ColorScan;

static void ScanColor(const VdpState *v, int y, ColorScan *s) {
    static const uint8_t heights[4] = {8, 16, 16, 32};
    const int oh = heights[v->regs[1] & 0x03];
    const int ih = heights[v->regs[1] & 0x02];
    int c = MAXSPRITE2 + 1;
    int l;

    s->count = 0;
    s->ninth = 0;
    for (l = 0; l < 32; ++l) {
        int k = VramAt(v, v->spr_tab + (uint32_t)l * 4u);
        if (k == 216) break;
        k = (uint8_t)(k - v->regs[23]);
        if (k > 256 - ih) k -= 256;
        if (y > k && y <= k + oh) {
            if (!--c) {
                s->ninth = 1;
                break; /* sem MSX_ALLSPRITE: o 9o sprite nao e' desenhado */
            }
            s->idx[s->count++] = (uint8_t)l;
        }
    }
    s->last = l < 32 ? l : 31;
}

void vdp_sprites_color_line(const VdpState *v, int y, uint8_t *zbuf) {
    static const uint8_t heights[4] = {8, 16, 16, 32};
    const int oh = heights[v->regs[1] & 0x03];
    const int ih = heights[v->regs[1] & 0x02];
    ColorScan s;
    int n;
    uint8_t or_them = 0;

    memset(zbuf, 0, 320);
    if (!SpritesActive(v)) return;
    ScanColor(v, y, &s);

    /* Do ultimo para o primeiro: o de menor indice fica por cima. */
    for (n = s.count - 1; n >= 0; --n) {
        const uint32_t at = v->spr_tab + (uint32_t)s.idx[n] * 4u;
        int k = (uint8_t)(VramAt(v, at) - v->regs[23]);
        int j, row;
        uint8_t c, bits;
        uint8_t *p;
        uint32_t pt;

        if (k > 256 - ih) k -= 256;
        row = y - k - 1;
        row = oh > ih ? (row >> 1) : row;

        /* cor desta linha do sprite: tabela de 16 bytes antes da de atributos */
        c = VramAt(v, v->spr_tab - 0x200u + (uint32_t)s.idx[n] * 16u + (uint32_t)row);
        or_them |= (uint8_t)(c & 0x40);

        if (c & 0x0F) {
            pt = v->spr_gen + ((uint32_t)(ih > 8 ? (VramAt(v, at + 2) & 0xFC) : VramAt(v, at + 2)) << 3) + (uint32_t)row;
            p = zbuf + VramAt(v, at + 1) + ((c & 0x80) ? 0 : 32);
            c &= 0x0F;
            bits = VramAt(v, pt);

            if (or_them & 0x20) {
                /* CC do sprite anterior: OR das cores */
                if (oh > ih) {
                    for (j = 0; j < 8; ++j) if (bits & (0x80 >> j)) { p[j * 2] |= c; p[j * 2 + 1] |= c; }
                    if (ih > 8) {
                        bits = VramAt(v, pt + 16);
                        for (j = 0; j < 8; ++j) if (bits & (0x80 >> j)) { p[16 + j * 2] |= c; p[17 + j * 2] |= c; }
                    }
                } else {
                    for (j = 0; j < 8; ++j) if (bits & (0x80 >> j)) p[j] |= c;
                    if (ih > 8) {
                        bits = VramAt(v, pt + 16);
                        for (j = 0; j < 8; ++j) if (bits & (0x80 >> j)) p[8 + j] |= c;
                    }
                }
            } else {
                if (oh > ih) {
                    for (j = 0; j < 8; ++j) if (bits & (0x80 >> j)) p[j * 2] = p[j * 2 + 1] = c;
                    if (ih > 8) {
                        bits = VramAt(v, pt + 16);
                        for (j = 0; j < 8; ++j) if (bits & (0x80 >> j)) p[16 + j * 2] = p[17 + j * 2] = c;
                    }
                } else {
                    for (j = 0; j < 8; ++j) if (bits & (0x80 >> j)) p[j] = c;
                    if (ih > 8) {
                        bits = VramAt(v, pt + 16);
                        for (j = 0; j < 8; ++j) if (bits & (0x80 >> j)) p[8 + j] = c;
                    }
                }
            }
        }
        or_them >>= 1;
    }
}

void vdp_sprites_update_status(VdpState *v, int y) {
    SpriteScan s;
    if (!SpritesActive(v)) return;
    if (ColorMode(v)) {
        ColorScan cs;
        ScanColor(v, y, &cs);
        v->status[0] &= (uint8_t)~0x5F;
        if (cs.ninth) v->status[0] |= 0x40;
        v->status[0] |= (uint8_t)cs.last;
        return;
    }
    /* y_efetivo = y + 2*VScroll, ver nota no .h */
    Scan(v, (uint8_t)(y + 2 * v->regs[23]), &s);
    v->status[0] &= (uint8_t)~0x5F;
    if (s.fifth) v->status[0] |= 0x40;
    v->status[0] |= (uint8_t)s.last;
}

void vdp_sprites_draw_line(const VdpState *v, int line, VdpRgb888 *row) {
    static const uint8_t heights[4] = {8, 16, 16, 32};
    const int oh = heights[v->regs[1] & 0x03];
    const int ih = heights[v->regs[1] & 0x02];
    const uint8_t y = (uint8_t)(line + 2 * v->regs[23]);
    SpriteScan s;
    int n;

    if (!SpritesActive(v)) return;
    Scan(v, y, &s);

    /* Do ultimo para o primeiro: sprite de menor indice desenhado por
       ultimo = maior prioridade (fica por cima). */
    for (n = s.count - 1; n >= 0; --n) {
        const uint32_t at = v->spr_tab + (uint32_t)s.idx[n] * 4u;
        const uint8_t attr = VramAt(v, at + 3);
        const int color = attr & 0x0F;
        const int x0 = (attr & 0x80) ? VramAt(v, at + 1) - 32 : VramAt(v, at + 1); /* early clock */
        int k, row_in_pat, px;
        uint32_t pat;
        unsigned bits;

        if (!color || x0 >= 256 || x0 <= -oh) continue; /* cor 0 = transparente */

        k = VramAt(v, at);
        if (k > 256 - ih) k -= 256;
        row_in_pat = y - k - 1;
        if (oh > ih) row_in_pat >>= 1;

        pat = v->spr_gen + ((uint32_t)(ih > 8 ? (VramAt(v, at + 2) & 0xFC) : VramAt(v, at + 2)) << 3) +
              (uint32_t)row_in_pat;
        bits = (unsigned)VramAt(v, pat) << 8;
        if (ih > 8) bits |= VramAt(v, pat + 16);
        else bits &= 0xFF00;

        for (px = 0; px < oh; ++px) {
            const int x = x0 + px;
            const int src = oh > ih ? px >> 1 : px;
            if (x < 0 || x >= 256) continue;
            if (bits & (0x8000u >> src)) {
                VdpRgb888 *dst = row + x;
                dst->r = v->palette_r[color];
                dst->g = v->palette_g[color];
                dst->b = v->palette_b[color];
            }
        }
    }
}

// CheckSprites() do fMSX, portado quase literalmente (aritmetica uint8_t
// de diferenca de posicao preservada de proposito: DV/DH sao 'byte' no
// original, o "< 8 || > 248" so' funciona com a volta de 8 bits).
// Ignora a ampliacao (magnify), como o original.
int vdp_sprites_check_collision(const VdpState *v) {
    const int big = (v->regs[1] & 0x02) != 0;
    const int size = big ? 16 : 8;
    const uint8_t limit_end = 255 - size;                       /* LD */
    const uint8_t limit_start = (v->regs[9] & 0x80) ? 211 : 191; /* LS */
    uint32_t valid = 0;
    uint32_t i, j;
    int a, b;

    if (!SpritesActive(v)) return 0;

    for (i = 0; i < 32; ++i) {
        const uint8_t y = VramAt(v, v->spr_tab + i * 4u);
        if (y == (ColorMode(v) ? 216 : 208)) break;
        if (y < limit_start || y > limit_end) valid |= 1u << i;
    }

    for (a = 0; a < 32; ++a) {
        if (!(valid & (1u << a))) continue;
        for (b = a + 1; b < 32; ++b) {
            uint32_t sa, sb, ps, pd;
            uint8_t dv, dh;
            if (!(valid & (1u << b))) continue;
            sa = v->spr_tab + (uint32_t)a * 4u;
            sb = v->spr_tab + (uint32_t)b * 4u;

            dv = (uint8_t)(VramAt(v, sa) - VramAt(v, sb));
            if (!(dv < size || dv > 256 - size)) continue;
            dh = (uint8_t)(VramAt(v, sa + 1) - VramAt(v, sb + 1));
            if (!(dh < size || dh > 256 - size)) continue;

            if (big) {
                ps = v->spr_gen + ((uint32_t)(VramAt(v, sa + 2) & 0xFC) << 3);
                pd = v->spr_gen + ((uint32_t)(VramAt(v, sb + 2) & 0xFC) << 3);
            } else {
                ps = v->spr_gen + ((uint32_t)VramAt(v, sa + 2) << 3);
                pd = v->spr_gen + ((uint32_t)VramAt(v, sb + 2) << 3);
            }
            if (dv < size) pd += dv;
            else {
                dv = (uint8_t)(256 - dv);
                ps += dv;
            }
            if (dh > 256 - size) {
                uint32_t t = ps;
                dh = (uint8_t)(256 - dh);
                ps = pd;
                pd = t;
            }
            while (dv < size) {
                const unsigned ls = big ? ((unsigned)VramAt(v, ps) << 8) + VramAt(v, ps + 16) : VramAt(v, ps);
                const unsigned ld = big ? ((unsigned)VramAt(v, pd) << 8) + VramAt(v, pd + 16) : VramAt(v, pd);
                if (ld & (ls >> dh)) return 1;
                ++dv;
                ++ps;
                ++pd;
            }
        }
    }
    return 0;
}
