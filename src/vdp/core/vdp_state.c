// Adaptado de fMSX (resource/fMSX/fMSX/MSX.c, resource/fMSX/fMSX/MSX.h)
// -- ver vdp_state.h para a nota de atribuicao completa e o escopo exato
// desta Fase 1 (VDP "digital", sem renderizacao).
#include "vdp_state.h"

#include <string.h>

#include "vdp_cmd.h"
#include "vdp_sprites.h"
#include "vdp_tables.h"

// PalInit[] do fMSX (resource/fMSX/fMSX/MSX.c, ~linha 687, dentro da
// rotina de reset/troca de modelo) -- a paleta padrao de 16 cores do
// TMS9918/MSX1, formato 0x00RRGGBB. Software SCREEN 0/1/2 real quase
// nunca escreve os registradores de paleta (9Ah e' recurso do V9938+):
// ele conta com essa paleta fixa existir desde o power-on. Achado nesta
// Fase 2 (renderizacao): a Fase 1 zerava palette_r/g/b[] no reset, o que
// faria QUALQUER pixel renderizado sair preto -- corrigido aqui.
static const uint32_t kPalInit[16] = {
    0x00000000, 0x00000000, 0x0020C020, 0x0060E060,
    0x002020E0, 0x004060E0, 0x00A02020, 0x0040C0E0,
    0x00E02020, 0x00E06060, 0x00C0C020, 0x00C0C080,
    0x00208020, 0x00C040A0, 0x00A0A0A0, 0x00E0E0E0,
};

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
            v->regs[reg] = value;
            RecomputeTables(v);
            break;
        case 6:
            v->regs[6] = (uint8_t)(value & 0x3F);
            RecomputeTables(v);
            break;
        case 10: /* bits 16-18 da tabela de cor (MSX2) */
            v->regs[10] = (uint8_t)(value & 0x07);
            RecomputeTables(v);
            break;
        case 11: /* bits 15-16 da tabela de atributos de sprite (MSX2) */
            v->regs[11] = (uint8_t)(value & 0x03);
            RecomputeTables(v);
            break;
        case 14: /* pagina de 16KB da VRAM (MSX1: so' a pagina 0) */
            v->regs[14] = (uint8_t)(value & (v->vram_pages - 1));
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
        case 44: /* dado da CPU para o motor de comandos (VDPWrite) */
            if (VDP_MODEL_IS_V9938(v->model)) vdp_cmd_write(v, value);
            v->regs[44] = value;
            break;
        case 46: /* dispara um comando (VDPDraw) */
            v->regs[46] = value;
            if (VDP_MODEL_IS_V9938(v->model)) vdp_cmd_draw(v, value);
            break;
        default:
            if (reg >= 0 && reg < 64) v->regs[reg] = value;
            break;
    }
}

static void ResetInternal(VdpState *v) {
    /* Aqui o modelo ja' esta' escolhido (v->model). */
    if (!VDP_MODEL_IS_V9938(v->model)) {
        v->model = VDP_MODEL_MSX1;
        v->vram_pages = 1;
        v->vram_mask = 0x3FFFu;
    } else {
        v->vram_pages = VDP_VRAM_PAGES;
        v->vram_mask = VDP_VRAM_SIZE - 1;
    }
    memset(v->regs, 0, sizeof(v->regs));
    memset(v->status, 0, sizeof(v->status));
    memset(v->vram, 0, sizeof(v->vram));
    memset(&v->cmd, 0, sizeof(v->cmd));
    v->ops_cnt = 0;
    v->engine = 0;
    v->blink_flag = 0;
    v->blink_count = 0;
    v->x_fg = 0;
    v->x_bg = 0;
    if (VDP_MODEL_IS_V9938(v->model)) {
        /* VDPSInit[] do fMSX: S#0=9Fh, S#2=6Ch (os bits fixos em 1 do V9938) */
        v->status[0] = 0x9F;
        v->status[2] = 0x6C;
    }
    /* "Set V9958 VDP version for MSX2+": bit 2 de S#1 (MSX.c do fMSX). */
    if (v->model == VDP_MODEL_MSX2P) v->status[1] |= 0x04;
    v->vaddr = 0;
    v->vdata = 0;
    v->vkey = 1;
    v->alatch = 0;
    v->pkey = 1;
    v->platch = 0;
    vdp_tables_init();
    for (int j = 0; j < 16; ++j) {
        v->palette_r[j] = (uint8_t)((kPalInit[j] >> 16) & 0xFF);
        v->palette_g[j] = (uint8_t)((kPalInit[j] >> 8) & 0xFF);
        v->palette_b[j] = (uint8_t)(kPalInit[j] & 0xFF);
    }
    v->scr_mode = 0;
    v->scanline = 0;
    v->drawing = 0;
    v->irq_pending = 0;
    RecomputeTables(v);
}

void vdp_set_model(VdpState *v, int model) {
    v->model = (model == VDP_MODEL_MSX2 || model == VDP_MODEL_MSX2P) ? model : VDP_MODEL_MSX1;
    ResetInternal(v);
}

void vdp_reset(VdpState *v) {
    /* Reset "de fabrica": sempre MSX1 (um VdpState nao inicializado nao tem
       modelo valido). Use vdp_set_model() para MSX2 e vdp_reset_keep_model()
       para um reset de maquina. */
    v->model = VDP_MODEL_MSX1;
    ResetInternal(v);
}

void vdp_reset_keep_model(VdpState *v) { ResetInternal(v); }

// InZ80()/WrZ80() do fMSX, casos 98h/99h/9Ah/9Bh -- ver vdp_state.h para
// a nota de escopo (S#7/LMCM omitido, motor de comando fora de escopo).
static uint32_t VramIndex(const VdpState *v) {
    return (((uint32_t)v->regs[14] << 14) | v->vaddr) & v->vram_mask;
}

static void AdvanceVramAddress(VdpState *v) {
    v->vaddr = (uint16_t)((v->vaddr + 1) & 0x3FFF);
    /* Ao dar a volta nos 16KB, a pagina (R#14) avanca -- so' nos modos MSX2 */
    if (v->vaddr == 0 && v->scr_mode > 3) {
        v->regs[14] = (uint8_t)((v->regs[14] + 1) & (v->vram_pages - 1));
    }
}

uint8_t vdp_in(VdpState *v, uint16_t port) {
    switch (port & 0xFF) {
        case 0x98: {
            const uint8_t result = v->vdata;
            v->vkey = 1;
            v->vdata = v->vram[VramIndex(v)];
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
                case 7: /* S#7 = proximo pixel do LMCM (motor de comandos, MSX2) */
                    if (VDP_MODEL_IS_V9938(v->model)) v->status[7] = v->regs[44] = vdp_cmd_read(v);
                    break;
                default:
                    break;
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
            v->vram[VramIndex(v)] = value;
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
                        v->vdata = v->vram[VramIndex(v)];
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
                // Formula exata do fMSX (ver o comentario de topo de
                // src/vdp/fortran/palette_table.f90 para a prova de
                // equivalencia): R vem de PLatch bits 6-4, G de value
                // bits 2-0, B de PLatch bits 2-0 -- cada um um
                // componente de 3 bits (0-7). Fase 1 calculava isso
                // inline a cada escrita; Fase 2 troca para a tabela de
                // 512 entradas pre-computada em Fortran (antes sem
                // nenhum consumidor) -- mesmos valores, calculados uma
                // unica vez em vez de a cada escrita de paleta.
                const int j = v->regs[16] & 0x0F;
                const int r3 = (v->platch >> 4) & 0x07;
                const int g3 = value & 0x07;
                const int b3 = v->platch & 0x07;
                const int idx = r3 * 64 + g3 * 8 + b3;
                v->palette_r[j] = g_vdp_palette_table_r[idx];
                v->palette_g[j] = g_vdp_palette_table_g[idx];
                v->palette_b[j] = g_vdp_palette_table_b[idx];
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

void vdp_capture_snapshot(const VdpState *v, VdpScanlineSnapshot *out) {
    memcpy(out->regs, v->regs, sizeof(out->regs));
    memcpy(out->palette_r, v->palette_r, sizeof(out->palette_r));
    memcpy(out->palette_g, v->palette_g, sizeof(out->palette_g));
    memcpy(out->palette_b, v->palette_b, sizeof(out->palette_b));
    out->scr_mode = v->scr_mode;
    out->model = v->model;
    out->x_fg = v->x_fg;
    out->x_bg = v->x_bg;
    out->chr_tab = v->chr_tab;
    out->col_tab = v->col_tab;
    out->chr_gen = v->chr_gen;
    out->spr_tab = v->spr_tab;
    out->spr_gen = v->spr_gen;
    out->chr_tab_mask = v->chr_tab_mask;
    out->col_tab_mask = v->col_tab_mask;
    out->chr_gen_mask = v->chr_gen_mask;
}

void vdp_apply_snapshot(VdpState *v, const VdpScanlineSnapshot *snap) {
    memcpy(v->regs, snap->regs, sizeof(v->regs));
    memcpy(v->palette_r, snap->palette_r, sizeof(v->palette_r));
    memcpy(v->palette_g, snap->palette_g, sizeof(v->palette_g));
    memcpy(v->palette_b, snap->palette_b, sizeof(v->palette_b));
    v->scr_mode = snap->scr_mode;
    v->model = snap->model;
    v->x_fg = snap->x_fg;
    v->x_bg = snap->x_bg;
    v->chr_tab = snap->chr_tab;
    v->col_tab = snap->col_tab;
    v->chr_gen = snap->chr_gen;
    v->spr_tab = snap->spr_tab;
    v->spr_gen = snap->spr_gen;
    v->chr_tab_mask = snap->chr_tab_mask;
    v->col_tab_mask = snap->col_tab_mask;
    v->chr_gen_mask = snap->chr_gen_mask;
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

            /* Piscar de TEXT80 (R#12 = cores do piscar, R#13 = tempos on/off) */
            if (v->blink_count) {
                --v->blink_count;
            } else {
                v->blink_flag = (uint8_t)!v->blink_flag;
                if (!v->regs[13]) {
                    v->x_fg = (uint8_t)(v->regs[7] >> 4);
                    v->x_bg = (uint8_t)(v->regs[7] & 0x0F);
                } else {
                    v->blink_count = (uint8_t)((v->blink_flag ? (v->regs[13] & 0x0F) : (v->regs[13] >> 4)) * 10);
                    if (v->blink_count) {
                        if (v->blink_flag) {
                            v->x_fg = (uint8_t)(v->regs[7] >> 4);
                            v->x_bg = (uint8_t)(v->regs[7] & 0x0F);
                        } else {
                            v->x_fg = (uint8_t)(v->regs[12] >> 4);
                            v->x_bg = (uint8_t)(v->regs[12] & 0x0F);
                        }
                    }
                }
            }
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

    /* Sprites (Fase 3): efeito colateral de status de Sprites() do fMSX
       em cada linha visivel (flag/numero do 5o sprite) ... */
    if (v->drawing && v->scanline < 192) vdp_sprites_update_status(v, v->scanline);

    /* ... e, na linha 192, limpa o 5o sprite e checa colisao (trecho de
       LoopZ80() do fMSX; so' o de status de sprite, o resto -- teclado,
       joystick, mouse, cheats -- continua fora de escopo). */
    if (v->scanline == 192) {
        v->status[0] = (uint8_t)((v->status[0] & ~0x40) | 0x1F);
        if (!(v->status[0] & 0x20) && vdp_sprites_check_collision(v)) v->status[0] |= 0x20;
    }

    /* LoopVDP(): avanca o comando do V9938 em execucao (so' MSX2) */
    if (VDP_MODEL_IS_V9938(v->model)) vdp_cmd_loop(v);

    /* Snapshot por linha para renderizacao (ver VdpScanlineSnapshot em
       vdp_state.h, e Machine::RenderFrame() no lado C++): guarda o estado
       de registradores/paleta/cache de tabela EXATAMENTE como esta' ao
       fim do processamento desta linha -- e' esse o estado que uma
       interrupcao IE1 disparada durante esta linha ja' teve chance de
       alterar (o Z80 roda intercalado T-state a T-state com o VDP, ver
       Machine::RunFrame()), entao um efeito de rastreio (paleta/scroll
       trocados na ISR) fica visivel a partir da linha certa, nao
       retroativo ao quadro inteiro. */
    if (v->scanline >= 0 && v->scanline < VDP_MAX_SCANLINES) {
        vdp_capture_snapshot(v, &v->scanline_snapshot[v->scanline]);
    }

    result.irq_pending = v->irq_pending != 0;
    return result;
}
