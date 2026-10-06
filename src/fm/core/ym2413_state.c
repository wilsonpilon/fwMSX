// fwMSX -- motor do chip FM OPLL (YM2413). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause), exceto os timbres prontos (ym2413_patches.h). Ver
// ym2413_state.h e doc/fm-spec.md.
//
// Caminho do sinal de um canal: modulador (fase + feedback, forma de onda,
// envelope, atenuacao) -> modula a fase da portadora -> portadora (envelope,
// atenuacao) -> FULL_SCALE. As tabelas de seno e de atenuacao em dB saem do
// Fortran (src/fm/fortran/ym2413_tables.f90); a soma dos canais e' feita no
// kernel de Assembly (src/fm/asm/accumulate.asm).
#include "ym2413_state.h"

#include <math.h>
#include <string.h>

#include "ym2413_patches.h"

/* Fortran (ym2413_tables.f90): seno de 1024 pontos e ganho de 0 a -256 dB em
   passos de 0.25 dB. */
void ym2413_build_tables(double *sine, double *db_gain);
/* Assembly dual-ABI (accumulate.asm): acc[i] += src[i] para i < n. */
void ym2413_accumulate(int32_t *acc, const int32_t *src, int n);

#define TABLE_SIZE 1024
#define BLOCK 256 /* amostras processadas por bloco */

/* Desvio de fase (em ciclos) com o modulador em ganho maximo. */
#define MOD_INDEX 0.5
/* LFOs: tremolo de 4.8 dB a 3.7 Hz; vibrato de +-7 centesimos a 6.4 Hz. */
#define AM_DEPTH_DB 4.8
#define AM_HZ 3.7
#define VIB_CENTS 7.0
#define VIB_HZ 6.4
/* Atenuacao do KSL, em dB por oitava acima de ~C4 (KSL = 0..3). */
static const double kKslDbPerOct[4] = {0.0, 1.5, 3.0, 6.0};
/* MULT (0..15): 0 vale meia oitava, os demais o numero inteiro. */
static const double kMult[16] = {0.5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 12, 12, 15, 15};
/* ln(10)/20: converte dB/s em fator exponencial por segundo. */
#define LN10_20 0.11512925464970229

static double g_sine[TABLE_SIZE];
static double g_db[TABLE_SIZE];
static int g_tables_ready = 0;

static void EnsureTables(void) {
    if (!g_tables_ready) {
        ym2413_build_tables(g_sine, g_db);
        g_tables_ready = 1;
    }
}

/* Ganho linear de uma atenuacao em dB (positiva = mais baixo). */
static double Db2Gain(double att_db) {
    int idx = (int)(att_db * 4.0 + 0.5);
    if (idx < 0) idx = 0;
    if (idx >= TABLE_SIZE) idx = TABLE_SIZE - 1;
    return g_db[idx];
}

/* Parte fracionaria em [0, 1), tambem para valores negativos. */
static double Frac(double x) {
    const double f = x - (double)(long long)x;
    return f < 0.0 ? f + 1.0 : f;
}

/* Forma de onda: seno, ou so' a meia onda positiva (half = 1). */
static double Wave(double x, int half) {
    const int idx = (int)(Frac(x) * TABLE_SIZE) & (TABLE_SIZE - 1);
    const double v = g_sine[idx];
    return (half && v < 0.0) ? 0.0 : v;
}

const uint8_t *ym2413_builtin_patch(int inst) {
    return (inst >= 1 && inst <= 15) ? kYm2413Patches[inst] : NULL;
}

/* Timbre efetivo do canal: o do usuario (registradores 00h-07h) ou um pronto. */
static int RhythmOn(const Ym2413State *s) { return (s->reg[0x0E] >> 5) & 1; }

static const uint8_t *PatchFor(const Ym2413State *s, int inst) {
    return inst == 0 ? s->reg : kYm2413Patches[inst];
}

/* Recalcula a configuracao dos dois operadores de um canal a partir do timbre
   e dos registradores. Nao mexe em fase nem em envelope. */
static void PrepareChannel(Ym2413State *s, int c) {
    Ym2413Ch *ch = &s->ch[c];
    const int drum = RhythmOn(s) && c >= 6; /* canais 7-9 viram bateria */
    const uint8_t *p = PatchFor(s, drum ? 16 + (c - 6) : ch->inst);
    const int mod_vol = (drum && c >= 7) ? (s->reg[0x30 + c] >> 4) : 0; /* HH e TOM */

    ch->base_hz = (double)ch->fnum * YM2413_NATIVE_RATE / (double)(1 << (19 - ch->block));
    const int fb = p[3] & 7;
    ch->fb_amt = fb ? (double)(1 << fb) / 256.0 : 0.0;

    for (int o = 0; o < 2; ++o) { /* o = 0: modulador, 1: portadora */
        Ym2413OpCfg *cfg = &ch->cfg[o];
        const uint8_t e = p[o]; /* AM VIB EGT KSR MULT */
        const uint8_t ar_dr = p[4 + o];
        const uint8_t sl_rr = p[6 + o];

        const int ksr = (e >> 4) & 1;
        const int ks = ksr ? ch->block * 2 + ((ch->fnum >> 8) & 1) : (ch->block >> 1);
        const int ksl = o == 0 ? (p[2] >> 6) : (p[3] >> 6);
        double octave = ch->block + ch->fnum / 512.0 - 4.0;
        if (octave < 0.0) octave = 0.0;
        double att = kKslDbPerOct[ksl] * octave;
        if (o == 0) att += (p[2] & 63) * 0.75 + mod_vol * 3.0; /* TL do modulador, 0,75 dB por passo */
        else att += ch->vol * 3.0;             /* volume da portadora, 3 dB por passo */
        cfg->gain = Db2Gain(att);

        cfg->am = (e >> 7) & 1;
        cfg->vib = (e >> 6) & 1;
        cfg->egt = ((e >> 5) & 1) || ch->sus;
        cfg->mult = kMult[e & 15];
        cfg->wf = o == 0 ? ((p[3] >> 3) & 1) : ((p[3] >> 4) & 1);

        /* Taxas: R = 4*valor + ks. Ataque: coeficiente exponencial. Decaimento e
           liberacao: queda em dB/s, dobrando a cada 8 unidades de R. */
        const int ar = ar_dr >> 4, dr = ar_dr & 15;
        const int sl = sl_rr >> 4, rr = sl_rr & 15;
        cfg->ka = ar ? 0.6 * pow(2.0, (4 * ar + ks) / 8.0) : 0.0;
        cfg->dr_dbps = dr ? 4.0 * pow(2.0, (4 * dr + ks - 4) / 8.0) : 0.0;
        cfg->rr_dbps = rr ? 4.0 * pow(2.0, (4 * rr + ks - 4) / 8.0) : 0.0;
        cfg->sl = Db2Gain(sl * 3.0); /* SL: 3 dB por passo */
    }
}

/* Tecla de um operador: a nota comeca (ataque e fase zerada) ou entra em liberacao. */
static void SetOpKey(Ym2413Ch *ch, int o, int on) {
    Ym2413Op *op = &ch->op[o];
    if (on && !ch->opkey[o]) {
        op->state = YM2413_OP_ATTACK;
        op->phase = 0.0; /* a nota recomeca a fase */
        if (o == 0) ch->fb1 = ch->fb2 = 0.0;
    } else if (!on && ch->opkey[o]) {
        if (op->state != YM2413_OP_OFF) op->state = YM2413_OP_RELEASE;
    }
    ch->opkey[o] = (uint8_t)(on ? 1 : 0);
}

/* KEY-ON do canal (modo melodico): os dois operadores juntos. */
static void SetKey(Ym2413Ch *ch, int on) {
    for (int o = 0; o < 2; ++o) SetOpKey(ch, o, on);
    ch->key = (uint8_t)(on ? 1 : 0);
}

/* Bateria: BD (bit 4) no canal 7 (os dois operadores); HH (bit 0) e SD (bit 3)
   nos operadores do canal 8; TOM (bit 2) e TC (bit 1) nos do canal 9. */
static void DrumOp(Ym2413State *s, uint8_t mask, int on) {
    switch (mask) {
    case 0x10: SetOpKey(&s->ch[6], 0, on); SetOpKey(&s->ch[6], 1, on); break;
    case 0x01: SetOpKey(&s->ch[7], 0, on); break;
    case 0x08: SetOpKey(&s->ch[7], 1, on); break;
    case 0x04: SetOpKey(&s->ch[8], 0, on); break;
    case 0x02: SetOpKey(&s->ch[8], 1, on); break;
    default: break;
    }
}

/* Reg 0Eh: entrar ou sair do modo ritmo solta as notas dos canais 7-9 e, com o
   modo ligado, cada tecla de bateria dispara na borda de subida e solta na de descida. */
static void UpdateDrums(Ym2413State *s, uint8_t previous, uint8_t value) {
    const int was_on = (previous >> 5) & 1;
    const int on = (value >> 5) & 1;
    if (on && !was_on) {
        for (int c = 6; c < 9; ++c) SetKey(&s->ch[c], 0);
    }
    if (!on && was_on) {
        for (int c = 6; c < 9; ++c) {
            SetOpKey(&s->ch[c], 0, 0);
            SetOpKey(&s->ch[c], 1, 0);
        }
    }
    const uint8_t keys = on ? (uint8_t)(value & 0x1F) : 0;
    for (int bit = 0; bit < 5; ++bit) {
        const uint8_t mask = (uint8_t)(1 << bit);
        const int pressed = (keys & mask) != 0;
        if (pressed != ((s->drum_keys & mask) != 0)) DrumOp(s, mask, pressed);
    }
    s->drum_keys = keys;
}

/* Um passo do envelope de um operador. O ataque sobe exponencialmente ate' 1;
   o decaimento desce ate' SL e, com egt = 0, segue em liberacao (RR) ate' zero. */
static void StepEnv(Ym2413Op *op, const Ym2413OpCfg *cfg, double dt) {
    switch (op->state) {
    case YM2413_OP_ATTACK:
        if (cfg->ka > 0.0) {
            op->env += (1.0 - op->env) * (1.0 - exp(-cfg->ka * dt));
            if (op->env >= 0.999) {
                op->env = 1.0;
                op->state = YM2413_OP_DECAY;
            }
        }
        break;
    case YM2413_OP_DECAY:
        op->env *= exp(-LN10_20 * cfg->dr_dbps * dt);
        if (op->env <= cfg->sl) {
            op->env = cfg->sl;
            op->state = cfg->egt ? YM2413_OP_SUSTAIN : YM2413_OP_RELEASE;
        }
        break;
    case YM2413_OP_SUSTAIN:
        break;
    case YM2413_OP_RELEASE:
        op->env *= exp(-LN10_20 * cfg->rr_dbps * dt);
        if (op->env < 1e-4) {
            op->env = 0.0;
            op->state = YM2413_OP_OFF;
        }
        break;
    default:
        break;
    }
}

/* Gera `n` amostras de um canal (portadora, ja' em escala FULL_SCALE). Os LFOs
   chegam por amostra: tremolo em dB e vibrato em centesimos. */
static void RenderChannel(Ym2413Ch *ch, int n, double dt, const double *am_db, const double *vib_c, int32_t *out) {
    Ym2413Op *mo = &ch->op[0];
    Ym2413Op *co = &ch->op[1];
    const Ym2413OpCfg *mc = &ch->cfg[0];
    const Ym2413OpCfg *cc = &ch->cfg[1];

    for (int i = 0; i < n; ++i) {
        if (mo->state == YM2413_OP_OFF && co->state == YM2413_OP_OFF) {
            out[i] = 0;
            continue;
        }
        StepEnv(mo, mc, dt);
        StepEnv(co, cc, dt);

        const double am = Db2Gain(am_db[i]);
        const double vmod = mc->vib ? exp2(vib_c[i] / 1200.0) : 1.0;
        const double vcar = cc->vib ? exp2(vib_c[i] / 1200.0) : 1.0;

        mo->phase = Frac(mo->phase + ch->base_hz * mc->mult * vmod * dt);
        const double fb_term = ch->fb_amt * 0.5 * (ch->fb1 + ch->fb2);
        const double mod_out = Wave(mo->phase + fb_term, mc->wf) * mo->env * mc->gain * (mc->am ? am : 1.0);
        ch->fb2 = ch->fb1;
        ch->fb1 = mod_out;

        co->phase = Frac(co->phase + ch->base_hz * cc->mult * vcar * dt);
        const double car_out = Wave(co->phase + MOD_INDEX * mod_out, cc->wf) * co->env * cc->gain * (cc->am ? am : 1.0);
        out[i] = (int32_t)(car_out * YM2413_FULL_SCALE);
    }
}

void ym2413_reset(Ym2413State *s) {
    EnsureTables();
    memset(s, 0, sizeof(*s));
    for (int c = 0; c < YM2413_CHANNELS; ++c) PrepareChannel(s, c);
}

void ym2413_write_reg(Ym2413State *s, uint8_t reg, uint8_t value) {
    EnsureTables();
    reg &= 0x3F;
    const uint8_t previous = s->reg[reg];
    s->reg[reg] = value;

    if (reg >= 0x10 && reg <= 0x18) { /* fnum, bits 7-0 */
        Ym2413Ch *ch = &s->ch[reg - 0x10];
        ch->fnum = (uint16_t)((ch->fnum & 0x100) | value);
    } else if (reg >= 0x20 && reg <= 0x28) { /* key, sus, bloco, fnum bit 8 */
        Ym2413Ch *ch = &s->ch[reg - 0x20];
        ch->fnum = (uint16_t)((ch->fnum & 0x0FF) | ((value & 1) << 8));
        ch->block = (uint8_t)((value >> 1) & 7);
        ch->sus = (uint8_t)((value >> 5) & 1);
        /* No modo ritmo, os canais 7-9 nao tem KEY-ON melodico (sao a bateria). */
        if (!RhythmOn(s) || reg < 0x26) SetKey(ch, (value >> 4) & 1);
    } else if (reg >= 0x30 && reg <= 0x38) { /* timbre e volume */
        Ym2413Ch *ch = &s->ch[reg - 0x30];
        ch->inst = (uint8_t)(value >> 4);
        ch->vol = (uint8_t)(value & 15);
    }
    if (reg == 0x0E) UpdateDrums(s, previous, value);
    /* Timbres do usuario (00h-07h) e configuracoes afetam os canais; recalcular
       todos e' barato e escrita de registrador e' rara. */
    for (int c = 0; c < YM2413_CHANNELS; ++c) PrepareChannel(s, c);
}

int ym2413_advance(Ym2413State *s, int z80_cycles, int out_rate, int16_t *out, int max) {
    EnsureTables();
    s->frac += (double)z80_cycles * out_rate / YM2413_Z80_CLOCK;
    int total = (int)s->frac;
    s->frac -= total;
    if (out && total > max) total = max;

    const double dt = 1.0 / out_rate;
    double am_db[BLOCK], vib_c[BLOCK];
    int32_t mix[BLOCK], chan[BLOCK];
    int produced = 0;

    while (produced < total) {
        const int n = (total - produced) < BLOCK ? (total - produced) : BLOCK;

        for (int i = 0; i < n; ++i) {
            am_db[i] = AM_DEPTH_DB * 0.5 * (1.0 + Wave(s->am_phase, 0));
            vib_c[i] = VIB_CENTS * Wave(s->vib_phase, 0);
            s->am_phase = Frac(s->am_phase + AM_HZ * dt);
            s->vib_phase = Frac(s->vib_phase + VIB_HZ * dt);
        }

        memset(mix, 0, sizeof(int32_t) * (size_t)n);
        for (int c = 0; c < YM2413_CHANNELS; ++c) {
            const Ym2413Ch *ch = &s->ch[c];
            if (ch->op[0].state == YM2413_OP_OFF && ch->op[1].state == YM2413_OP_OFF) continue;
            RenderChannel(&s->ch[c], n, dt, am_db, vib_c, chan);
            ym2413_accumulate(mix, chan, n);
        }

        if (out) {
            for (int i = 0; i < n; ++i) {
                int v = mix[i];
                if (v > 32767) v = 32767;
                if (v < -32768) v = -32768;
                out[produced + i] = (int16_t)v;
            }
        }
        produced += n;
    }
    return total;
}
