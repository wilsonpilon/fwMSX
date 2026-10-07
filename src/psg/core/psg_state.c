// Adaptado de fMSX -- ver psg_state.h para o aviso de licenca completo.
#include "psg_state.h"

#include <string.h>

// Implementada em Fortran (src/psg/fortran/volume_table.f90).
extern void psg_build_volume_table(int16_t levels[16]);

static int16_t g_levels[16];
static int g_levels_ready = 0;

static void ensure_tables(void) {
    if (!g_levels_ready) {
        psg_build_volume_table(g_levels);
        g_levels_ready = 1;
    }
}

void psg_reset(PsgState *p) {
    /* RegInit[] de Reset8910() do fMSX: R7=FDh (so' o canal A com tom, I/O em
     * saida), R14=FFh (porta de joystick sem nada pressionado). */
    static const uint8_t reg_init[PSG_REGS] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD,
                                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00};
    ensure_tables();
    memset(p, 0, sizeof(*p));
    memcpy(p->r, reg_init, sizeof(p->r));
    p->lfsr = 1;
}

void psg_set_joystick(PsgState *p, int port, uint8_t bits) {
    if (port == 0 || port == 1) p->joy[port] = bits & 0x3F;
}

void psg_set_cassette_in(PsgState *p, int level) { p->cassette_in = level ? 1 : 0; }

void psg_write_reg(PsgState *p, int reg, uint8_t v) {
    switch (reg) {
    case 1: case 3: case 5: p->r[reg] = v & 0x0F; break;
    case 0: case 2: case 4: p->r[reg] = v; break;
    case 6: p->r[6] = v & 0x1F; break;
    case 7: p->r[7] = v; break;
    case 8: case 9: case 10: p->r[reg] = v & 0x1F; break;
    case 11: case 12: p->r[reg] = v; break;
    case 13:
        /* Escrever em R13 sempre reinicia o envelope (Write8910() do fMSX). */
        p->r[13] = v & 0x0F;
        p->env_cnt = 0;
        p->env_step = 0;
        p->env_hold = 0;
        p->env_attack = (p->r[13] & 0x04) ? 1 : 0;
        break;
    case 14: case 15: p->r[reg] = v; break;
    default: break;
    }
}

void psg_write_latch(PsgState *p, uint8_t value) { p->latch = value & 0x0F; }

void psg_write_data(PsgState *p, uint8_t value) { psg_write_reg(p, p->latch, value); }

uint8_t psg_read_data(const PsgState *p) {
    /* InZ80() 0xA2 do fMSX. R14 e' a porta de joystick: o bit 6 de R15
     * escolhe A (0) ou B (1); R14 devolve os 6 bits do joystick em logica
     * invertida (0 = pressionado) com o bit 6 sempre 1 -- 7Fh sem nada
     * pressionado. Se o software desliga as linhas desse joystick (bit 4/5 de
     * R15) le tudo solto, como no fMSX. R15 devolve so' os 4 bits altos; os
     * demais registradores sao lidos como estao. */
    if (p->latch == 14) {
        const int port = (p->r[15] >> 6) & 1;
        const uint8_t cas = (uint8_t)(p->cassette_in ? 0x80 : 0x00);
        if (p->r[15] & (0x10 << port)) return (uint8_t)(0x7F | cas);
        return (uint8_t)(((~p->joy[port] & 0x3F) | 0x40) | cas);
    }
    if (p->latch == 15) return p->r[15] & 0xF0;
    return p->r[p->latch];
}

static int tone_period(const PsgState *p, int ch) {
    const int v = ((p->r[ch * 2 + 1] & 0x0F) << 8) | p->r[ch * 2];
    return v ? v : 1;
}

static int env_period(const PsgState *p) {
    const int v = (p->r[12] << 8) | p->r[11];
    return v ? v : 1;
}

static int env_level(const PsgState *p) { return p->env_attack ? p->env_step : 15 - p->env_step; }

double psg_tone_hz(const PsgState *p, int channel) {
    if (channel < 0 || channel >= PSG_CHANNELS) return 0.0;
    if (p->r[7] & (1 << channel)) return 0.0;
    const int period = ((p->r[channel * 2 + 1] & 0x0F) << 8) | p->r[channel * 2];
    if (!period) return 0.0;
    return (PSG_Z80_CLOCK / 2.0) / (16.0 * period);
}

int psg_channel_level(const PsgState *p, int channel) {
    if (channel < 0 || channel >= PSG_CHANNELS) return 0;
    const uint8_t a = p->r[8 + channel];
    return (a & 0x10) ? env_level(p) : (a & 0x0F);
}

/* Fim de um ciclo de 16 passos do envelope -- as 16 formas de R13. */
static void env_cycle_end(PsgState *p) {
    const uint8_t shape = p->r[13];

    p->env_step = 0;
    if (!(shape & 0x08)) {
        /* Formas 0-7: um unico ciclo e depois silencio (nivel 0). */
        p->env_hold = 1;
        p->env_attack = 0;
        p->env_step = 15;
    } else if (shape & 0x01) {
        /* Hold: para no valor final -- 15 se estiver em attack, 0 se nao;
         * Alt inverte o sentido antes de parar. Nos dois casos o nivel final
         * e' o do passo 15. */
        p->env_hold = 1;
        if (shape & 0x02) p->env_attack ^= 1;
        p->env_step = 15;
    } else if (shape & 0x02) {
        p->env_attack ^= 1; /* Alt sem hold: triangulo */
    }
}

/* Um tick de gerador (8 ciclos de PSG). */
static void psg_tick(PsgState *p) {
    int ch;

    for (ch = 0; ch < PSG_CHANNELS; ++ch) {
        if (++p->tone_cnt[ch] >= tone_period(p, ch)) {
            p->tone_cnt[ch] = 0;
            p->tone_out[ch] ^= 1;
        }
    }

    {
        int np = p->r[6] & 0x1F;
        if (!np) np = 1;
        if (++p->noise_cnt >= np) {
            p->noise_cnt = 0;
            p->noise_half ^= 1;
            if (p->noise_half) { /* desloca a cada 2*periodo ticks = 16*periodo ciclos de PSG */
                /* LFSR de 17 bits, realimentacao bit0 XOR bit3 (AY-3-8910). */
                const uint32_t fb = (p->lfsr ^ (p->lfsr >> 3)) & 1u;
                p->lfsr = (p->lfsr >> 1) | (fb << 16);
            }
        }
    }

    /* Envelope: um passo a cada 16*periodo ciclos de PSG (= 2*periodo ticks). */
    if (!p->env_hold && ++p->env_cnt >= 2u * (uint32_t)env_period(p)) {
        p->env_cnt = 0;
        if (++p->env_step == 16) env_cycle_end(p);
    }
}

static int mix_level(const PsgState *p) {
    const uint8_t mixer = p->r[7];
    const int noise = (int)(p->lfsr & 1u);
    int sum = 0, ch;

    for (ch = 0; ch < PSG_CHANNELS; ++ch) {
        const int tone_on = (mixer & (1 << ch)) ? 1 : p->tone_out[ch];
        const int noise_on = (mixer & (8 << ch)) ? 1 : noise;
        if (tone_on && noise_on) sum += g_levels[psg_channel_level(p, ch)];
    }
    return sum;
}

int psg_advance(PsgState *p, int z80_cycles, int sample_rate, int16_t *out, int max_samples) {
    int produced = 0;

    if (z80_cycles <= 0 || sample_rate <= 0) return 0;
    ensure_tables();

    p->cycle_acc += (uint32_t)z80_cycles;
    while (p->cycle_acc >= PSG_CYCLES_PER_TICK) {
        p->cycle_acc -= PSG_CYCLES_PER_TICK;
        psg_tick(p);

        /* Filtro de caixa: media dos ticks dentro de cada periodo de
         * amostra de saida (tick = 16 ciclos de Z80). */
        p->box_sum += mix_level(p);
        p->box_count += 1;
        p->phase += (uint32_t)sample_rate * PSG_CYCLES_PER_TICK;
        if (p->phase >= PSG_Z80_CLOCK) {
            p->phase -= PSG_Z80_CLOCK;
            if (out && produced < max_samples) {
                out[produced] = (int16_t)(p->box_sum / p->box_count);
                ++produced;
            }
            p->box_sum = 0;
            p->box_count = 0;
        }
    }
    return produced;
}
