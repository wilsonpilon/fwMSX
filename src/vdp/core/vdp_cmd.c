// Adaptado de fMSX (V9938.c) -- ver vdp_cmd.h para a nota de atribuicao.
#include "vdp_cmd.h"

#include "vdp_state.h"

/* Enderecos de pixel em VRAM por modo (SCREEN 5-8). A VRAM aqui e' linear: a
 * intercalacao fisica dos modos 6-8 e' transparente para a CPU e para os
 * comandos (o mesmo modelo do fMSX). Todos os enderecos cabem em 128KB. */
#define VRMP5(V, X, Y) ((V)->vram + ((((Y) & 1023) << 7) + (((X) & 255) >> 1)))
#define VRMP6(V, X, Y) ((V)->vram + ((((Y) & 1023) << 7) + (((X) & 511) >> 2)))
#define VRMP7(V, X, Y) ((V)->vram + ((((Y) & 511) << 8) + (((X) & 511) >> 1)))
#define VRMP8(V, X, Y) ((V)->vram + ((((Y) & 511) << 8) + ((X) & 255)))

enum {
    CM_ABRT = 0x0,
    CM_POINT = 0x4,
    CM_PSET = 0x5,
    CM_SRCH = 0x6,
    CM_LINE = 0x7,
    CM_LMMV = 0x8,
    CM_LMMM = 0x9,
    CM_LMCM = 0xA,
    CM_LMMC = 0xB,
    CM_HMMV = 0xC,
    CM_HMMM = 0xD,
    CM_YMMM = 0xE,
    CM_HMMC = 0xF
};

enum { ENG_NONE = 0, ENG_SRCH, ENG_LINE, ENG_LMMV, ENG_LMMM, ENG_LMCM, ENG_LMMC, ENG_HMMV, ENG_HMMM, ENG_YMMM, ENG_HMMC };

static const uint8_t kMask[4] = {0x0F, 0x03, 0x0F, 0xFF};
static const int kPPB[4] = {2, 4, 2, 1};
static const int kPPL[4] = {256, 512, 512, 256};

/* Tempo (em unidades do orcamento) por passo de cada comando, indexado por
 * [tela ligada, sprites desligados, PAL] -- tabelas medidas do V9938 (fMSX). */
static const int kSrchTiming[8] = {818, 1025, 818, 830, 696, 854, 696, 684};
static const int kLineTiming[8] = {1063, 1259, 1063, 1161, 904, 1026, 904, 953};
static const int kHmmvTiming[8] = {439, 549, 439, 531, 366, 439, 366, 427};
static const int kLmmvTiming[8] = {873, 1135, 873, 1056, 732, 909, 732, 854};
static const int kYmmmTiming[8] = {586, 952, 586, 610, 488, 720, 488, 500};
static const int kHmmmTiming[8] = {818, 1111, 818, 854, 684, 879, 684, 708};
static const int kLmmmTiming[8] = {1160, 1599, 1160, 1172, 964, 1257, 964, 977};

static int TimingValue(const VdpState *v, const int *table) {
    return table[((v->regs[1] >> 6) & 1) | (v->regs[8] & 2) | ((v->regs[9] << 1) & 4)];
}

/* --- leitura/escrita de pixel ------------------------------------------- */

static uint8_t Point5(const VdpState *v, int x, int y) { return (uint8_t)((*VRMP5(v, x, y) >> (((~x) & 1) << 2)) & 15); }
static uint8_t Point6(const VdpState *v, int x, int y) { return (uint8_t)((*VRMP6(v, x, y) >> (((~x) & 3) << 1)) & 3); }
static uint8_t Point7(const VdpState *v, int x, int y) { return (uint8_t)((*VRMP7(v, x, y) >> (((~x) & 1) << 2)) & 15); }
static uint8_t Point8(const VdpState *v, int x, int y) { return *VRMP8(v, x, y); }

static uint8_t Point(const VdpState *v, int sm, int x, int y) {
    switch (sm) {
    case 0: return Point5(v, x, y);
    case 1: return Point6(v, x, y);
    case 2: return Point7(v, x, y);
    case 3: return Point8(v, x, y);
    }
    return 0;
}

/* Aplica a operacao logica `op` (LO do comando) ao byte *p com a mascara m. */
static void PsetLow(uint8_t *p, uint8_t cl, uint8_t m, uint8_t op) {
    switch (op) {
    case 0: *p = (uint8_t)((*p & m) | cl); break;
    case 1: *p = (uint8_t)(*p & (cl | m)); break;
    case 2: *p |= cl; break;
    case 3: *p ^= cl; break;
    case 4: *p = (uint8_t)((*p & m) | ~(cl | m)); break;
    case 8: if (cl) *p = (uint8_t)((*p & m) | cl); break;
    case 9: if (cl) *p = (uint8_t)(*p & (cl | m)); break;
    case 10: if (cl) *p |= cl; break;
    case 11: if (cl) *p ^= cl; break;
    case 12: if (cl) *p = (uint8_t)((*p & m) | ~(cl | m)); break;
    default: break;
    }
}

static void Pset5(VdpState *v, int x, int y, uint8_t cl, uint8_t op) {
    const uint8_t sh = (uint8_t)(((~x) & 1) << 2);
    PsetLow(VRMP5(v, x, y), (uint8_t)(cl << sh), (uint8_t)~(15 << sh), op);
}
static void Pset6(VdpState *v, int x, int y, uint8_t cl, uint8_t op) {
    const uint8_t sh = (uint8_t)(((~x) & 3) << 1);
    PsetLow(VRMP6(v, x, y), (uint8_t)(cl << sh), (uint8_t)~(3 << sh), op);
}
static void Pset7(VdpState *v, int x, int y, uint8_t cl, uint8_t op) {
    const uint8_t sh = (uint8_t)(((~x) & 1) << 2);
    PsetLow(VRMP7(v, x, y), (uint8_t)(cl << sh), (uint8_t)~(15 << sh), op);
}
static void Pset8(VdpState *v, int x, int y, uint8_t cl, uint8_t op) { PsetLow(VRMP8(v, x, y), cl, 0, op); }

static void Pset(VdpState *v, int sm, int x, int y, uint8_t cl, uint8_t op) {
    switch (sm) {
    case 0: Pset5(v, x, y, cl, op); break;
    case 1: Pset6(v, x, y, cl, op); break;
    case 2: Pset7(v, x, y, cl, op); break;
    case 3: Pset8(v, x, y, cl, op); break;
    }
}

/* --- estruturas de laco reaproveitadas pelos comandos ------------------- */
/* (as mesmas macros do V9938.c: `cnt` e' o orcamento restante, `delta` o custo
 * de um passo; o laco para quando o orcamento acaba ou o comando termina) */
#define PRE_LOOP while ((cnt -= delta) > 0) {

/* Laco sobre DX, DY */
#define POST_X_Y(MX)                                                  \
    if (!--anx || ((adx += tx) & (MX))) {                             \
        if (!(--ny & 1023) || (dy += ty) == -1) break;                \
        else { adx = dx; anx = nx; }                                  \
    }                                                                 \
    }

/* Laco sobre DX, SY, DY */
#define POST_XYY(MX)                                                  \
    if ((adx += tx) & (MX)) {                                         \
        if (!(--ny & 1023) || (sy += ty) == -1 || (dy += ty) == -1) break; \
        else adx = dx;                                                \
    }                                                                 \
    }

/* Laco sobre SX, DX, SY, DY */
#define POST_XXYY(MX)                                                 \
    if (!--anx || ((asx += tx) & (MX)) || ((adx += tx) & (MX))) {     \
        if (!(--ny & 1023) || (sy += ty) == -1 || (dy += ty) == -1) break; \
        else { asx = sx; adx = dx; anx = nx; }                        \
    }                                                                 \
    }

/* --- engines ------------------------------------------------------------- */

static void SrchEngine(VdpState *v) {
    int sx = v->cmd.sx, sy = v->cmd.sy, tx = v->cmd.tx, anx = v->cmd.anx;
    const uint8_t cl = v->cmd.cl;
    int cnt = v->ops_cnt;
    const int delta = TimingValue(v, kSrchTiming);

#define SRCH_LOOP(POINT, MX)                                              \
    PRE_LOOP                                                              \
    if (((POINT) == cl) ^ anx) {                                          \
        v->status[2] |= 0x10; /* borda detectada */                       \
        break;                                                            \
    }                                                                     \
    if ((sx += tx) & (MX)) {                                              \
        v->status[2] &= 0xEF; /* borda nao detectada */                   \
        break;                                                            \
    }                                                                     \
    }

    switch (v->scr_mode) {
    case 5: SRCH_LOOP(Point5(v, sx, sy), 256) break;
    case 6: SRCH_LOOP(Point6(v, sx, sy), 512) break;
    case 7: SRCH_LOOP(Point7(v, sx, sy), 512) break;
    case 8: SRCH_LOOP(Point8(v, sx, sy), 256) break;
    }
#undef SRCH_LOOP

    if ((v->ops_cnt = cnt) > 0) {
        v->status[2] &= 0xFE;
        v->engine = ENG_NONE;
        v->status[8] = (uint8_t)(sx & 0xFF);
        v->status[9] = (uint8_t)((sx >> 8) | 0xFE);
    } else {
        v->cmd.sx = sx;
    }
}

static void LineEngine(VdpState *v) {
    int dx = v->cmd.dx, dy = v->cmd.dy, tx = v->cmd.tx, ty = v->cmd.ty, nx = v->cmd.nx, ny = v->cmd.ny;
    int asx = v->cmd.asx, adx = v->cmd.adx;
    const uint8_t cl = v->cmd.cl, lo = v->cmd.lo;
    int cnt = v->ops_cnt;
    const int delta = TimingValue(v, kLineTiming);

#define POST_LINEXMAJ(MX)                                                 \
    dx += tx;                                                             \
    if ((asx -= ny) < 0) { asx += nx; dy += ty; }                         \
    asx &= 1023;                                                          \
    if (adx++ == nx || (dx & (MX))) break;                                \
    }
#define POST_LINEYMAJ(MX)                                                 \
    dy += ty;                                                             \
    if ((asx -= ny) < 0) { asx += nx; dx += tx; }                         \
    asx &= 1023;                                                          \
    if (adx++ == nx || (dx & (MX))) break;                                \
    }

    if ((v->regs[45] & 0x01) == 0) {
        switch (v->scr_mode) { /* o eixo X e' o principal */
        case 5: PRE_LOOP Pset5(v, dx, dy, cl, lo); POST_LINEXMAJ(256) break;
        case 6: PRE_LOOP Pset6(v, dx, dy, cl, lo); POST_LINEXMAJ(512) break;
        case 7: PRE_LOOP Pset7(v, dx, dy, cl, lo); POST_LINEXMAJ(512) break;
        case 8: PRE_LOOP Pset8(v, dx, dy, cl, lo); POST_LINEXMAJ(256) break;
        }
    } else {
        switch (v->scr_mode) { /* o eixo Y e' o principal */
        case 5: PRE_LOOP Pset5(v, dx, dy, cl, lo); POST_LINEYMAJ(256) break;
        case 6: PRE_LOOP Pset6(v, dx, dy, cl, lo); POST_LINEYMAJ(512) break;
        case 7: PRE_LOOP Pset7(v, dx, dy, cl, lo); POST_LINEYMAJ(512) break;
        case 8: PRE_LOOP Pset8(v, dx, dy, cl, lo); POST_LINEYMAJ(256) break;
        }
    }
#undef POST_LINEXMAJ
#undef POST_LINEYMAJ

    if ((v->ops_cnt = cnt) > 0) {
        v->status[2] &= 0xFE;
        v->engine = ENG_NONE;
        v->regs[38] = (uint8_t)(dy & 0xFF);
        v->regs[39] = (uint8_t)((dy >> 8) & 0x03);
    } else {
        v->cmd.dx = dx;
        v->cmd.dy = dy;
        v->cmd.asx = asx;
        v->cmd.adx = adx;
    }
}

/* Fim de um comando de bloco: grava de volta NY/SY/DY nos registradores. */
static void FinishBlock(VdpState *v, int ny, int sy, int dy, int ty, int has_src, int src_wrapped) {
    if (!ny) {
        if (has_src) sy += ty;
        dy += ty;
    } else if (has_src && src_wrapped) {
        dy += ty;
    }
    v->regs[42] = (uint8_t)(ny & 0xFF);
    v->regs[43] = (uint8_t)((ny >> 8) & 0x03);
    if (has_src) {
        v->regs[34] = (uint8_t)(sy & 0xFF);
        v->regs[35] = (uint8_t)((sy >> 8) & 0x03);
    }
    v->regs[38] = (uint8_t)(dy & 0xFF);
    v->regs[39] = (uint8_t)((dy >> 8) & 0x03);
}

static void LmmvEngine(VdpState *v) {
    int dy = v->cmd.dy, tx = v->cmd.tx, ty = v->cmd.ty, nx = v->cmd.nx, ny = v->cmd.ny, adx = v->cmd.adx, anx = v->cmd.anx;
    const int dx = v->cmd.dx;
    const uint8_t cl = v->cmd.cl, lo = v->cmd.lo;
    int cnt = v->ops_cnt;
    const int delta = TimingValue(v, kLmmvTiming);

    switch (v->scr_mode) {
    case 5: PRE_LOOP Pset5(v, adx, dy, cl, lo); POST_X_Y(256) break;
    case 6: PRE_LOOP Pset6(v, adx, dy, cl, lo); POST_X_Y(512) break;
    case 7: PRE_LOOP Pset7(v, adx, dy, cl, lo); POST_X_Y(512) break;
    case 8: PRE_LOOP Pset8(v, adx, dy, cl, lo); POST_X_Y(256) break;
    }

    if ((v->ops_cnt = cnt) > 0) {
        v->status[2] &= 0xFE;
        v->engine = ENG_NONE;
        FinishBlock(v, ny, 0, dy, ty, 0, 0);
    } else {
        v->cmd.dy = dy;
        v->cmd.ny = ny;
        v->cmd.anx = anx;
        v->cmd.adx = adx;
    }
}

static void LmmmEngine(VdpState *v) {
    int sx = v->cmd.sx, sy = v->cmd.sy, dx = v->cmd.dx, dy = v->cmd.dy, tx = v->cmd.tx, ty = v->cmd.ty, nx = v->cmd.nx, ny = v->cmd.ny;
    int asx = v->cmd.asx, adx = v->cmd.adx, anx = v->cmd.anx;
    const uint8_t lo = v->cmd.lo;
    int cnt = v->ops_cnt;
    const int delta = TimingValue(v, kLmmmTiming);

    switch (v->scr_mode) {
    case 5: PRE_LOOP Pset5(v, adx, dy, Point5(v, asx, sy), lo); POST_XXYY(256) break;
    case 6: PRE_LOOP Pset6(v, adx, dy, Point6(v, asx, sy), lo); POST_XXYY(512) break;
    case 7: PRE_LOOP Pset7(v, adx, dy, Point7(v, asx, sy), lo); POST_XXYY(512) break;
    case 8: PRE_LOOP Pset8(v, adx, dy, Point8(v, asx, sy), lo); POST_XXYY(256) break;
    }

    if ((v->ops_cnt = cnt) > 0) {
        v->status[2] &= 0xFE;
        v->engine = ENG_NONE;
        FinishBlock(v, ny, sy, dy, ty, 1, sy == -1);
    } else {
        v->cmd.sy = sy;
        v->cmd.dy = dy;
        v->cmd.ny = ny;
        v->cmd.anx = anx;
        v->cmd.asx = asx;
        v->cmd.adx = adx;
    }
}

static void LmcmEngine(VdpState *v) {
    if ((v->status[2] & 0x80) != 0x80) {
        v->status[7] = v->regs[44] = Point(v, v->scr_mode - 5, v->cmd.asx, v->cmd.sy);
        v->ops_cnt -= TimingValue(v, kLmmvTiming);
        v->status[2] |= 0x80;

        if (!--v->cmd.anx || ((v->cmd.asx += v->cmd.tx) & v->cmd.mx)) {
            if (!(--v->cmd.ny & 1023) || (v->cmd.sy += v->cmd.ty) == -1) {
                v->status[2] &= 0xFE;
                v->engine = ENG_NONE;
                if (!v->cmd.ny) v->cmd.dy += v->cmd.ty;
                v->regs[42] = (uint8_t)(v->cmd.ny & 0xFF);
                v->regs[43] = (uint8_t)((v->cmd.ny >> 8) & 0x03);
                v->regs[34] = (uint8_t)(v->cmd.sy & 0xFF);
                v->regs[35] = (uint8_t)((v->cmd.sy >> 8) & 0x03);
            } else {
                v->cmd.asx = v->cmd.sx;
                v->cmd.anx = v->cmd.nx;
            }
        }
    }
}

static void LmmcEngine(VdpState *v) {
    if ((v->status[2] & 0x80) != 0x80) {
        const int sm = v->scr_mode - 5;
        v->status[7] = v->regs[44] &= kMask[sm];
        Pset(v, sm, v->cmd.adx, v->cmd.dy, v->regs[44], v->cmd.lo);
        v->ops_cnt -= TimingValue(v, kLmmvTiming);
        v->status[2] |= 0x80;

        if (!--v->cmd.anx || ((v->cmd.adx += v->cmd.tx) & v->cmd.mx)) {
            if (!(--v->cmd.ny & 1023) || (v->cmd.dy += v->cmd.ty) == -1) {
                v->status[2] &= 0xFE;
                v->engine = ENG_NONE;
                if (!v->cmd.ny) v->cmd.dy += v->cmd.ty;
                v->regs[42] = (uint8_t)(v->cmd.ny & 0xFF);
                v->regs[43] = (uint8_t)((v->cmd.ny >> 8) & 0x03);
                v->regs[38] = (uint8_t)(v->cmd.dy & 0xFF);
                v->regs[39] = (uint8_t)((v->cmd.dy >> 8) & 0x03);
            } else {
                v->cmd.adx = v->cmd.dx;
                v->cmd.anx = v->cmd.nx;
            }
        }
    }
}

static void HmmvEngine(VdpState *v) {
    int dy = v->cmd.dy, tx = v->cmd.tx, ty = v->cmd.ty, nx = v->cmd.nx, ny = v->cmd.ny, adx = v->cmd.adx, anx = v->cmd.anx;
    const int dx = v->cmd.dx;
    const uint8_t cl = v->cmd.cl;
    int cnt = v->ops_cnt;
    const int delta = TimingValue(v, kHmmvTiming);

    switch (v->scr_mode) {
    case 5: PRE_LOOP *VRMP5(v, adx, dy) = cl; POST_X_Y(256) break;
    case 6: PRE_LOOP *VRMP6(v, adx, dy) = cl; POST_X_Y(512) break;
    case 7: PRE_LOOP *VRMP7(v, adx, dy) = cl; POST_X_Y(512) break;
    case 8: PRE_LOOP *VRMP8(v, adx, dy) = cl; POST_X_Y(256) break;
    }

    if ((v->ops_cnt = cnt) > 0) {
        v->status[2] &= 0xFE;
        v->engine = ENG_NONE;
        FinishBlock(v, ny, 0, dy, ty, 0, 0);
    } else {
        v->cmd.dy = dy;
        v->cmd.ny = ny;
        v->cmd.anx = anx;
        v->cmd.adx = adx;
    }
}

static void HmmmEngine(VdpState *v) {
    int sx = v->cmd.sx, sy = v->cmd.sy, dx = v->cmd.dx, dy = v->cmd.dy, tx = v->cmd.tx, ty = v->cmd.ty, nx = v->cmd.nx, ny = v->cmd.ny;
    int asx = v->cmd.asx, adx = v->cmd.adx, anx = v->cmd.anx;
    int cnt = v->ops_cnt;
    const int delta = TimingValue(v, kHmmmTiming);

    switch (v->scr_mode) {
    case 5: PRE_LOOP *VRMP5(v, adx, dy) = *VRMP5(v, asx, sy); POST_XXYY(256) break;
    case 6: PRE_LOOP *VRMP6(v, adx, dy) = *VRMP6(v, asx, sy); POST_XXYY(512) break;
    case 7: PRE_LOOP *VRMP7(v, adx, dy) = *VRMP7(v, asx, sy); POST_XXYY(512) break;
    case 8: PRE_LOOP *VRMP8(v, adx, dy) = *VRMP8(v, asx, sy); POST_XXYY(256) break;
    }

    if ((v->ops_cnt = cnt) > 0) {
        v->status[2] &= 0xFE;
        v->engine = ENG_NONE;
        FinishBlock(v, ny, sy, dy, ty, 1, sy == -1);
    } else {
        v->cmd.sy = sy;
        v->cmd.dy = dy;
        v->cmd.ny = ny;
        v->cmd.anx = anx;
        v->cmd.asx = asx;
        v->cmd.adx = adx;
    }
}

static void YmmmEngine(VdpState *v) {
    int sy = v->cmd.sy, dy = v->cmd.dy, tx = v->cmd.tx, ty = v->cmd.ty, ny = v->cmd.ny, adx = v->cmd.adx;
    const int dx = v->cmd.dx;
    int cnt = v->ops_cnt;
    const int delta = TimingValue(v, kYmmmTiming);

    switch (v->scr_mode) {
    case 5: PRE_LOOP *VRMP5(v, adx, dy) = *VRMP5(v, adx, sy); POST_XYY(256) break;
    case 6: PRE_LOOP *VRMP6(v, adx, dy) = *VRMP6(v, adx, sy); POST_XYY(512) break;
    case 7: PRE_LOOP *VRMP7(v, adx, dy) = *VRMP7(v, adx, sy); POST_XYY(512) break;
    case 8: PRE_LOOP *VRMP8(v, adx, dy) = *VRMP8(v, adx, sy); POST_XYY(256) break;
    }

    if ((v->ops_cnt = cnt) > 0) {
        v->status[2] &= 0xFE;
        v->engine = ENG_NONE;
        FinishBlock(v, ny, sy, dy, ty, 1, sy == -1);
    } else {
        v->cmd.sy = sy;
        v->cmd.dy = dy;
        v->cmd.ny = ny;
        v->cmd.adx = adx;
    }
}

static uint8_t *VrmpMode(VdpState *v, int sm, int x, int y) {
    switch (sm) {
    case 0: return VRMP5(v, x, y);
    case 1: return VRMP6(v, x, y);
    case 2: return VRMP7(v, x, y);
    case 3: return VRMP8(v, x, y);
    }
    return v->vram;
}

static void HmmcEngine(VdpState *v) {
    if ((v->status[2] & 0x80) != 0x80) {
        *VrmpMode(v, v->scr_mode - 5, v->cmd.adx, v->cmd.dy) = v->regs[44];
        v->ops_cnt -= TimingValue(v, kHmmvTiming);
        v->status[2] |= 0x80;

        if (!--v->cmd.anx || ((v->cmd.adx += v->cmd.tx) & v->cmd.mx)) {
            if (!(--v->cmd.ny & 1023) || (v->cmd.dy += v->cmd.ty) == -1) {
                v->status[2] &= 0xFE;
                v->engine = ENG_NONE;
                if (!v->cmd.ny) v->cmd.dy += v->cmd.ty;
                v->regs[42] = (uint8_t)(v->cmd.ny & 0xFF);
                v->regs[43] = (uint8_t)((v->cmd.ny >> 8) & 0x03);
                v->regs[38] = (uint8_t)(v->cmd.dy & 0xFF);
                v->regs[39] = (uint8_t)((v->cmd.dy >> 8) & 0x03);
            } else {
                v->cmd.adx = v->cmd.dx;
                v->cmd.anx = v->cmd.nx;
            }
        }
    }
}

static void RunEngine(VdpState *v) {
    switch (v->engine) {
    case ENG_SRCH: SrchEngine(v); break;
    case ENG_LINE: LineEngine(v); break;
    case ENG_LMMV: LmmvEngine(v); break;
    case ENG_LMMM: LmmmEngine(v); break;
    case ENG_LMCM: LmcmEngine(v); break;
    case ENG_LMMC: LmmcEngine(v); break;
    case ENG_HMMV: HmmvEngine(v); break;
    case ENG_HMMM: HmmmEngine(v); break;
    case ENG_YMMM: YmmmEngine(v); break;
    case ENG_HMMC: HmmcEngine(v); break;
    default: break;
    }
}

/* --- interface ------------------------------------------------------------ */

void vdp_cmd_write(VdpState *v, uint8_t value) {
    v->status[2] &= 0x7F;
    v->status[7] = v->regs[44] = value;
    if (v->engine && v->ops_cnt > 0) RunEngine(v);
}

uint8_t vdp_cmd_read(VdpState *v) {
    v->status[2] &= 0x7F;
    if (v->engine && v->ops_cnt > 0) RunEngine(v);
    return v->regs[44];
}

int vdp_cmd_draw(VdpState *v, uint8_t op) {
    int sm;

    /* So' funciona nas telas SCREEN 5-8 */
    if (v->scr_mode < 5 || v->scr_mode > 8) return 0;
    sm = v->scr_mode - 5;

    v->cmd.cm = (uint8_t)(op >> 4);
    if ((v->cmd.cm & 0x0C) != 0x0C && v->cmd.cm != 0) {
        /* operacao de ponto: so' os bits relevantes da cor */
        v->status[7] = (v->regs[44] &= kMask[sm]);
    }

    switch (op >> 4) {
    case CM_ABRT:
        v->status[2] &= 0xFE;
        v->engine = ENG_NONE;
        return 1;
    case CM_POINT:
        v->status[2] &= 0xFE;
        v->engine = ENG_NONE;
        v->status[7] = v->regs[44] = Point(v, sm, v->regs[32] + ((int)v->regs[33] << 8), v->regs[34] + ((int)v->regs[35] << 8));
        return 1;
    case CM_PSET:
        v->status[2] &= 0xFE;
        v->engine = ENG_NONE;
        Pset(v, sm, v->regs[36] + ((int)v->regs[37] << 8), v->regs[38] + ((int)v->regs[39] << 8), v->regs[44], (uint8_t)(op & 0x0F));
        return 1;
    case CM_SRCH: v->engine = ENG_SRCH; break;
    case CM_LINE: v->engine = ENG_LINE; break;
    case CM_LMMV: v->engine = ENG_LMMV; break;
    case CM_LMMM: v->engine = ENG_LMMM; break;
    case CM_LMCM: v->engine = ENG_LMCM; break;
    case CM_LMMC: v->engine = ENG_LMMC; break;
    case CM_HMMV: v->engine = ENG_HMMV; break;
    case CM_HMMM: v->engine = ENG_HMMM; break;
    case CM_YMMM: v->engine = ENG_YMMM; break;
    case CM_HMMC: v->engine = ENG_HMMC; break;
    default: return 0;
    }

    /* argumentos incondicionais */
    v->cmd.sx = (v->regs[32] + ((int)v->regs[33] << 8)) & 511;
    v->cmd.sy = (v->regs[34] + ((int)v->regs[35] << 8)) & 1023;
    v->cmd.dx = (v->regs[36] + ((int)v->regs[37] << 8)) & 511;
    v->cmd.dy = (v->regs[38] + ((int)v->regs[39] << 8)) & 1023;
    v->cmd.ny = (v->regs[42] + ((int)v->regs[43] << 8)) & 1023;
    v->cmd.ty = (v->regs[45] & 0x08) ? -1 : 1;
    v->cmd.mx = kPPL[sm];
    v->cmd.cl = v->regs[44];
    v->cmd.lo = (uint8_t)(op & 0x0F);

    /* NX depende de ser comando de byte (H***) ou de ponto (L***) */
    if ((v->cmd.cm & 0x0C) == 0x0C) {
        v->cmd.tx = (v->regs[45] & 0x04) ? -kPPB[sm] : kPPB[sm];
        v->cmd.nx = ((v->regs[40] + ((int)v->regs[41] << 8)) & 1023) / kPPB[sm];
    } else {
        v->cmd.tx = (v->regs[45] & 0x04) ? -1 : 1;
        v->cmd.nx = (v->regs[40] + ((int)v->regs[41] << 8)) & 1023;
    }

    /* variaveis X do laco: tratamento especial para LINE */
    if (v->cmd.cm == CM_LINE) {
        v->cmd.asx = (v->cmd.nx - 1) >> 1;
        v->cmd.adx = 0;
    } else {
        v->cmd.asx = v->cmd.sx;
        v->cmd.adx = v->cmd.dx;
    }

    /* NX para SRCH: procurar "==" ou "!="? */
    if (v->cmd.cm == CM_SRCH) v->cmd.anx = (v->regs[45] & 0x02) != 0;
    else v->cmd.anx = v->cmd.nx;

    /* Desvio deliberado do fMSX: o bit TR (S#2 bit 7, "pronto para transferir")
     * comeca LIMPO a cada comando. No fMSX ele fica ligado depois de um
     * LMMC/HMMC/LMCM e o proximo comando de transferencia nunca consome o 1o
     * dado (o LMCM seguinte devolveria um pixel velho). */
    v->status[2] &= 0x7F;

    /* comando em execucao */
    v->status[2] |= 0x01;

    /* comeca ja' se ainda ha' orcamento */
    if (v->engine && v->ops_cnt > 0) RunEngine(v);
    return 1;
}

void vdp_cmd_loop(VdpState *v) {
    if (v->ops_cnt <= 0) {
        v->ops_cnt += 12500;
        if (v->engine && v->ops_cnt > 0) RunEngine(v);
    } else {
        v->ops_cnt = 12500;
        if (v->engine) RunEngine(v);
    }
}
