// Adaptado de fMSX (resource/fMSX/EMULib/SCC.c), Copyright (C) Marat Fayzullin
// 1996-2021 -- ver o cabecalho de scc_state.h e LICENSE-THIRD-PARTY.md.
#include "scc_state.h"

#include <string.h>

#define SCC_SAMPLE_CHUNK 256
#define SCC_REG_FREQ_LO 0xA0
#define SCC_REG_VOLUME 0xAA
#define SCC_REG_MIXER 0xAF

extern void scc_build_volume_table(int32_t levels[16]);

static int32_t g_levels[16];
static int g_levels_ready = 0;

static void ensure_tables(void) {
    if (!g_levels_ready) {
        scc_build_volume_table(g_levels);
        g_levels_ready = 1;
    }
}

void scc_reset(SccState *s) {
    memset(s, 0, sizeof(*s));
}

uint8_t scc_read(const SccState *s, uint8_t reg, int plus) {
    const uint8_t limit = plus ? 0xA0 : 0x80;
    return reg < limit ? s->r[reg] : 0xFF;
}

/* Modo SCC+ (WriteSCCP do fMSX): ondas em 00h-9Fh, registradores em A0h-BFh (com
 * espelho em B0h-BFh), e o resto da faixa fica so' guardado. */
static void write_plus(SccState *s, uint8_t reg, uint8_t value) {
    if (value == s->r[reg]) return;

    if ((reg & 0xE0) == 0xA0) {
        const uint8_t base = reg & 0xEF;
        s->r[base] = s->r[base + 0x10] = value;
        return;
    }

    s->r[reg] = value;
}

void scc_write(SccState *s, uint8_t reg, uint8_t value, int plus) {
    if (plus) {
        write_plus(s, reg, value);
        return;
    }
    /* Generico: nao tem a ultima faixa de registradores (E0h+) e tem uma onda a
     * menos que o SCC+. O generico desloca 80h+ para os registradores (+20h). */
    if (reg >= 0xE0) return;
    if (reg >= 0x80) {
        write_plus(s, (uint8_t)(reg + 0x20), value);
        return;
    }
    /* Ondas 3 e 4 compartilham a mesma memoria no modo generico. */
    if (reg >= 0x60) {
        write_plus(s, reg, value);
        write_plus(s, (uint8_t)(reg + 0x20), value);
        return;
    }
    write_plus(s, reg, value);
}

void scc_render_channel_ref(int32_t *acc, int n, const int8_t *wave, uint32_t *phase, uint32_t step, int32_t level) {
    uint32_t p = *phase;
    for (int i = 0; i < n; ++i) {
        const int32_t sample = wave[(p >> 16) & (SCC_WAVE_SIZE - 1)];
        acc[i] += (sample * level) >> 7;
        p += step;
    }
    *phase = p;
}

/* Soma a contribuicao de um canal ao acumulador. Canal silenciado pelo mixer ou
 * com periodo 0 nao avanca a fase (o fMSX tambem zera a frequencia nesses casos). */
static void render_channel(SccState *s, int ch, int32_t *acc, int n, int sample_rate) {
    if (!(s->r[SCC_REG_MIXER] & (1 << ch))) return;

    const unsigned period = s->r[SCC_REG_FREQ_LO + 2 * ch] | ((s->r[SCC_REG_FREQ_LO + 2 * ch + 1] & 0x0F) << 8);
    if (period == 0) return;

    const int32_t level = g_levels[s->r[SCC_REG_VOLUME + ch] & 0x0F];
    /* Incremento Q16 por amostra: (clock/periodo) indices por segundo / taxa. */
    const uint32_t step = (uint32_t)(((uint64_t)SCC_Z80_CLOCK << 16) / ((uint64_t)period * (uint64_t)sample_rate));
    scc_render_channel(acc, n, (const int8_t *)&s->r[ch * SCC_WAVE_SIZE], &s->wave_phase[ch], step, level);
}

int scc_advance(SccState *s, int z80_cycles, int sample_rate, int16_t *out, int max_samples) {
    if (z80_cycles <= 0 || sample_rate <= 0) return 0;
    ensure_tables();

    const uint64_t total = s->cycle_phase + (uint64_t)z80_cycles * (uint64_t)sample_rate;
    uint64_t count = total / SCC_Z80_CLOCK;
    s->cycle_phase = total % SCC_Z80_CLOCK;

    int produced = 0;
    int32_t acc[SCC_SAMPLE_CHUNK];
    while (count > 0) {
        const int n = count > SCC_SAMPLE_CHUNK ? SCC_SAMPLE_CHUNK : (int)count;
        memset(acc, 0, sizeof(int32_t) * (size_t)n);
        for (int ch = 0; ch < SCC_CHANNELS; ++ch) render_channel(s, ch, acc, n, sample_rate);
        if (out) {
            for (int i = 0; i < n && produced < max_samples; ++i) out[produced++] = (int16_t)acc[i];
        }
        count -= (uint64_t)n;
    }
    return produced;
}
