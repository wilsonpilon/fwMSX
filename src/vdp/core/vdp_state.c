// Adaptado de fMSX (resource/fMSX/fMSX/MSX.c, resource/fMSX/fMSX/MSX.h)
// -- ver vdp_state.h para a nota de atribuicao completa e o escopo exato
// desta Fase 1 (VDP "digital", sem renderizacao).
#include "vdp_state.h"

#include <string.h>

// Constantes de tempo do fMSX (resource/fMSX/fMSX/MSX.h), derivadas do
// clock do VDP (6x o clock da CPU) -- valores factuais de hardware,
// citados aqui por transparencia (nao "estilo de codigo" do fMSX):
//   CPU_CLOCK=3579545 Hz; HPERIOD=1368 (ciclos de VDP por linha);
//   HREFRESH_240=960, HREFRESH_256=1024 (ciclos de VDP de refresh ativo,
//   conforme a largura do modo de tela); CPU_H240=HREFRESH_240/6=160;
//   CPU_H256=HREFRESH_256/6=170 (truncado); CPU_HPERIOD=HPERIOD/6=228;
//   VPERIOD_NTSC=HPERIOD*262, VPERIOD_PAL=HPERIOD*313;
//   CPU_V262=VPERIOD_NTSC/6=59736; CPU_V313=VPERIOD_PAL/6=71364.
#define VDP_CPU_H240 160
#define VDP_CPU_H256 170
#define VDP_CPU_HPERIOD 228
#define VDP_CPU_V262 59736
#define VDP_CPU_V313 71364

// MSK[] do fMSX (resource/fMSX/fMSX/MSX.c) -- mascaras de registrador por
// modo de tela, usadas para computar os deslocamentos de tabela
// (ChrTab/ColTab/ChrGen/SprTab). Dado factual de hardware/tabela de
// modos do V9938, copiado verbatim (nao "estilo de codigo").
static const struct {
    uint8_t r2, r3, r4, r5, m2, m3, m4, m5;
} kScreenMask[VDP_MAXSCREEN + 2] = {
    {0x7F, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00}, /* SCR 0:  TEXT 40x24  */
    {0x7F, 0xFF, 0x3F, 0xFF, 0x00, 0x00, 0x00, 0x00}, /* SCR 1:  TEXT 32x24  */
    {0x7F, 0x80, 0x3C, 0xFF, 0x00, 0x7F, 0x03, 0x00}, /* SCR 2:  BLK 256x192 */
    {0x7F, 0x00, 0x3F, 0xFF, 0x00, 0x00, 0x00, 0x00}, /* SCR 3:  64x48x16    */
    {0x7F, 0x80, 0x3C, 0xFC, 0x00, 0x7F, 0x03, 0x03}, /* SCR 4:  BLK 256x192 */
    {0x60, 0x00, 0x00, 0xFC, 0x1F, 0x00, 0x00, 0x03}, /* SCR 5:  256x192x16  */
    {0x60, 0x00, 0x00, 0xFC, 0x1F, 0x00, 0x00, 0x03}, /* SCR 6:  512x192x4   */
    {0x20, 0x00, 0x00, 0xFC, 0x1F, 0x00, 0x00, 0x03}, /* SCR 7:  512x192x16  */
    {0x20, 0x00, 0x00, 0xFC, 0x1F, 0x00, 0x00, 0x03}, /* SCR 8:  256x192x256 */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* SCR 9:  NONE        */
    {0x20, 0x00, 0x00, 0xFC, 0x1F, 0x00, 0x00, 0x03}, /* SCR 10: YAE 256x192 */
    {0x20, 0x00, 0x00, 0xFC, 0x1F, 0x00, 0x00, 0x03}, /* SCR 11: YAE 256x192 */
    {0x20, 0x00, 0x00, 0xFC, 0x1F, 0x00, 0x00, 0x03}, /* SCR 12: YJK 256x192 */
    {0x7C, 0xF8, 0x3F, 0x00, 0x03, 0x07, 0x00, 0x00}  /* SCR 0:  TEXT 80x24  */
};

// SetIRQ() do fMSX -- IRQ com bit 7 setado (0x80) e' uma mascara AND
// (limpa bits), sem o bit 7 e' uma mascara OR (liga bits). O truncamento
// para uint8_t (parametro, nao int) e' o que faz `~VDP_INT_IE0` virar
// 0xFE (bit7 setado, limpa so' o bit 0) -- mesmo truque do fMSX (`byte
// IRQ` truncando um `~int`).
static void SetVdpIrq(VdpState *v, uint8_t irq) {
    if (irq & 0x80) v->irq_pending &= irq;
    else v->irq_pending |= irq;
}

// VDPOut()/SetScreen() do fMSX: recomputa ChrTab/ColTab/ChrGen/SprTab/
// SprGen (e as mascaras correspondentes) a partir do modo de tela atual
// e dos registradores 2/3/4/5/6/10/11 -- SEMPRE a formula exata do
// original, mesmo que o resultado exceda VDP_VRAM_SIZE (ver a nota em
// vdp_state.h sobre deslocamento vs. ponteiro).
static void RecomputeTables(VdpState *v) {
    const int j = v->scr_mode;
    const int i = (j > 6 && j != VDP_MAXSCREEN + 1) ? 11 : 10;

    v->chr_tab = ((uint32_t)(v->regs[2] & kScreenMask[j].r2)) << i;
    v->chr_gen = ((uint32_t)(v->regs[4] & kScreenMask[j].r4)) << 11;
    v->col_tab = (((uint32_t)(v->regs[3] & kScreenMask[j].r3)) << 6) + (((uint32_t)v->regs[10]) << 14);
    v->spr_tab = (((uint32_t)(v->regs[5] & kScreenMask[j].r5)) << 7) + (((uint32_t)v->regs[11]) << 15);
    v->spr_gen = ((uint32_t)v->regs[6]) << 11;

    v->chr_tab_mask = (((uint32_t)(uint8_t)(v->regs[2] | (uint8_t)~kScreenMask[j].m2)) << i) | (uint32_t)((1 << i) - 1);
    v->chr_gen_mask = (((uint32_t)(uint8_t)(v->regs[4] | (uint8_t)~kScreenMask[j].m4)) << 11) | 0x007FFu;
    v->col_tab_mask = (((uint32_t)(uint8_t)(v->regs[3] | (uint8_t)~kScreenMask[j].m3)) << 6) | 0x1C03Fu;
    v->spr_tab_mask = (((uint32_t)(uint8_t)(v->regs[5] | (uint8_t)~kScreenMask[j].m5)) << 7) | 0x1807Fu;
}

int vdp_set_screen(VdpState *v) {
    int j;
    switch (((v->regs[0] & 0x0E) >> 1) | (v->regs[1] & 0x18)) {
        case 0x10: j = 0; break;
        case 0x00: j = 1; break;
        case 0x01: j = 2; break;
        case 0x08: j = 3; break;
        case 0x02: j = 4; break;
        case 0x03: j = 5; break;
        case 0x04: j = 6; break;
        case 0x05: j = 7; break;
        case 0x07: j = 8; break;
        case 0x12: j = VDP_MAXSCREEN + 1; break;
        default: j = v->scr_mode; break;
    }
    v->scr_mode = (uint8_t)j;
    RecomputeTables(v);
    return j;
}

void vdp_write_register(VdpState *v, int reg, uint8_t value) {
    switch (reg) {
        case 0:
            /* Reseta interrupcao de HBlank se estava ligada e o bit de
               habilitacao (0x10) foi desligado nesta escrita. */
            if ((v->status[1] & 0x01) && !(value & 0x10)) {
                v->status[1] &= 0xFE;
                SetVdpIrq(v, (uint8_t)~VDP_INT_IE1);
            }
            v->regs[0] = value;
            vdp_set_screen(v);
            break;
        case 1:
            /* Liga/desliga IRQ de VBlank conforme o bit 0x20, so' se o
               flag de VBlank (status[0] bit 0x80) ja' estiver ativo. */
            if (v->status[0] & 0x80) SetVdpIrq(v, (value & 0x20) ? VDP_INT_IE0 : (uint8_t)~VDP_INT_IE0);
            v->regs[1] = value;
            vdp_set_screen(v);
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 10:
        case 11:
            v->regs[reg] = value;
            RecomputeTables(v);
            break;
        case 14:
            v->regs[14] = (uint8_t)(value & (VDP_VRAM_PAGES - 1));
            break;
        case 15:
            v->regs[15] = (uint8_t)(value & 0x0F);
            break;
        case 16:
            v->regs[16] = (uint8_t)(value & 0x0F);
            v->pkey = 1;
            break;
        case 17:
            v->regs[17] = (uint8_t)(value & 0xBF);
            break;
        case 25:
            v->regs[25] = value;
            vdp_set_screen(v);
            break;
        case 44: /* VDPWrite(V) -- motor de comando V9938, fora de escopo (Fase 5) */
        case 46: /* VDPDraw(V) -- idem */
            v->regs[reg] = value;
            break;
        default:
            if (reg >= 0 && reg < 64) v->regs[reg] = value;
            break;
    }
}

void vdp_reset(VdpState *v) {
    memset(v->regs, 0, sizeof(v->regs));
    memset(v->status, 0, sizeof(v->status));
    memset(v->vram, 0, sizeof(v->vram));
    v->vaddr = 0;
    v->vdata = 0;
    v->vkey = 1;
    v->alatch = 0;
    v->pkey = 1;
    v->platch = 0;
    memset(v->palette_r, 0, sizeof(v->palette_r));
    memset(v->palette_g, 0, sizeof(v->palette_g));
    memset(v->palette_b, 0, sizeof(v->palette_b));
    v->scr_mode = 0;
    v->scanline = 0;
    v->drawing = 0;
    v->irq_pending = 0;
    RecomputeTables(v);
}

// InZ80()/WrZ80() do fMSX, casos 98h/99h/9Ah/9Bh -- ver vdp_state.h para
// a nota de escopo (S#7/LMCM omitido, motor de comando fora de escopo).
static void AdvanceVramAddress(VdpState *v) {
    v->vaddr = (uint16_t)((v->vaddr + 1) & 0x3FFF);
    if (v->vaddr == 0 && v->scr_mode > 3) {
        v->regs[14] = (uint8_t)((v->regs[14] + 1) & (VDP_VRAM_PAGES - 1));
    }
}

uint8_t vdp_in(VdpState *v, uint16_t port) {
    switch (port & 0xFF) {
        case 0x98: {
            const uint8_t result = v->vdata;
            v->vkey = 1;
            v->vdata = v->vram[v->vaddr];
            AdvanceVramAddress(v);
            return result;
        }
        case 0x99: {
            const uint8_t result = v->status[v->regs[15]];
            // NOTA (fMSX): VKey NAO e' resetado aqui -- um comentario no
            // fMSX original diz que isso "quebra Sir Lancelot no
            // ColecoVision", removido de proposito. Preservado como esta.
            switch (v->regs[15]) {
                case 0:
                    v->status[0] &= 0x5F;
                    SetVdpIrq(v, (uint8_t)~VDP_INT_IE0);
                    break;
                case 1:
                    v->status[1] &= 0xFE;
                    SetVdpIrq(v, (uint8_t)~VDP_INT_IE1);
                    break;
                default:
                    break; /* S#7 (LMCM) fora de escopo -- motor de comando, Fase 5 */
            }
            return result;
        }
        default:
            return 0xFF; /* NORAM do fMSX */
    }
}

void vdp_out(VdpState *v, uint16_t port, uint8_t value) {
    switch (port & 0xFF) {
        case 0x98:
            v->vkey = 1;
            v->vdata = value;
            v->vram[v->vaddr] = value;
            AdvanceVramAddress(v);
            return;

        case 0x99:
            if (v->vkey) {
                v->alatch = value;
                v->vkey = 0;
                return;
            }
            v->vkey = 1;
            switch (value & 0xC0) {
                case 0x80:
                    vdp_write_register(v, value & 0x3F, v->alatch);
                    break;
                case 0x00:
                case 0x40:
                    v->vaddr = (uint16_t)((((uint16_t)value << 8) + v->alatch) & 0x3FFF);
                    if (!(value & 0x40)) {
                        v->vdata = v->vram[v->vaddr];
                        AdvanceVramAddress(v);
                    }
                    break;
                default:
                    break;
            }
            return;

        case 0x9A:
            if (v->pkey) {
                v->platch = value;
                v->pkey = 0;
                return;
            }
            {
                const int j = v->regs[16] & 0x0F;
                v->palette_r[j] = (uint8_t)((v->platch & 0x70) * 255 / 112);
                v->palette_g[j] = (uint8_t)((value & 0x07) * 255 / 7);
                v->palette_b[j] = (uint8_t)((v->platch & 0x07) * 255 / 7);
                v->pkey = 1;
                v->regs[16] = (uint8_t)((j + 1) & 0x0F);
            }
            return;

        case 0x9B: {
            const int j = v->regs[17] & 0x3F;
            if (j != 17) vdp_write_register(v, j, value);
            if (!(v->regs[17] & 0x80)) v->regs[17] = (uint8_t)((j + 1) & 0x3F);
            return;
        }

        default:
            return;
    }
}

// LoopZ80() do fMSX -- SO a fatia de VBlank(IE0)/HBlank e coincidencia
// de linha (IE1). Ver vdp_state.h para a lista completa do que fica de
// fora (LoopVDP/RefreshLine/som/sprites/teclado/joystick/mouse/cheats).
VdpStepResult vdp_step_scanline(VdpState *v) {
    VdpStepResult result;
    const int pal = (v->regs[9] & 0x02) != 0;      /* PALVideo */
    const int lines212 = (v->regs[9] & 0x80) != 0; /* ScanLines212 */

    /* Alterna o bit de HRefresh (status[2] bit 0x20). */
    v->status[2] ^= 0x20;

    if (!(v->status[2] & 0x20)) {
        /* HRefresh em andamento -- "primeira metade" da linha. */
        result.next_period_cycles = (!v->scr_mode || v->scr_mode == VDP_MAXSCREEN + 1) ? VDP_CPU_H240 : VDP_CPU_H256;

        v->scanline = (v->scanline < (pal ? 312 : 261)) ? v->scanline + 1 : 0;

        if (v->scanline == 0) {
            v->drawing = 1;
            v->status[2] &= 0xBF; /* reseta bit de VRefresh */
        }

        {
            const int coincidence_limit = pal ? 256 : (lines212 ? 245 : 235);

            if (v->scanline == coincidence_limit) {
                v->status[1] &= 0xFE;
                SetVdpIrq(v, (uint8_t)~VDP_INT_IE1);
            }

            if (v->scanline < coincidence_limit) {
                int j = (((v->scanline + v->regs[23]) & 0xFF) - v->regs[19]) & 0xFF;
                if (j == 2) {
                    v->status[1] |= 0x01;
                    if (v->regs[0] & 0x10) SetVdpIrq(v, VDP_INT_IE1);
                } else if (!(v->regs[0] & 0x10)) {
                    v->status[1] &= 0xFE;
                }
            }
        }

        result.irq_pending = v->irq_pending != 0;
        return result;
    }

    /* HBlank -- "segunda metade" da linha. */
    result.next_period_cycles = (!v->scr_mode || v->scr_mode == VDP_MAXSCREEN + 1) ? VDP_CPU_H240 : VDP_CPU_H256;
    result.next_period_cycles = VDP_CPU_HPERIOD - result.next_period_cycles;

    {
        const int total_lines = pal ? 313 : 262;
        if (v->scanline >= total_lines - 1) {
            const int j = total_lines * VDP_CPU_HPERIOD;
            const int vperiod = pal ? VDP_CPU_V313 : VDP_CPU_V262;
            if (vperiod > j) result.next_period_cycles += vperiod - j;
        }
    }

    if (v->scanline == (lines212 ? 212 : 192)) v->drawing = 0;

    {
        const int vblank_start =
            pal ? (lines212 ? 212 + 42 : 192 + 52) : (lines212 ? 212 + 18 : 192 + 28);
        if (!v->drawing && v->scanline == vblank_start) {
            v->status[0] |= 0x80;
            v->status[2] |= 0x40;
            if (v->regs[1] & 0x20) SetVdpIrq(v, VDP_INT_IE0);
        }
    }

    /* LoopVDP()/RefreshLine[]/som/sprites/teclado/joystick/mouse/cheats:
       fora de escopo desta fase, ver vdp_state.h. */

    result.irq_pending = v->irq_pending != 0;
    return result;
}
