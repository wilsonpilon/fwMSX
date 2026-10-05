// Adaptado de fMSX (resource/fMSX/EMULib/SCC.{h,c}), Copyright (C) Marat
// Fayzullin 1996-2021. O fwMSX evolui a partir do fMSX com o aval do autor
// original para adaptar/estudar seu codigo (ver README.md) -- isso nao e uma
// relicenciacao: este arquivo continua sob os termos originais dele
// (nao-comercial, aviso ao autor em caso de mudanca), nao o BSD-3-Clause do
// restante do fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// Chip de som SCC (Konami, cartuchos com 5 canais de onda de 32 amostras).
// DIFERENTE do fMSX (que repassa freq/volume para um sintetizador, Sound()),
// este motor gera AMOSTRAS PCM de verdade, como o PSG -- ver doc/scc-spec.md.
//
// As regras de registrador (espelhamento B0h-BFh, mascara do mixer, volume de
// 4 bits, ondas compartilhadas dos canais 3 e 4 no modo generico) seguem
// WriteSCC()/WriteSCCP()/ReadSCC() do fMSX linha a linha.
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCC_CHANNELS 5
#define SCC_WAVE_SIZE 32

/* Clock do Z80 no MSX (NTSC). O SCC conta no mesmo clock: a onda avanca uma
 * amostra a cada `periodo` ciclos (frequencia = clock/(32*periodo), como o
 * fMSX calcula com SCC_BASE = clock/32). */
#define SCC_Z80_CLOCK 3579545

typedef struct SccState {
    uint8_t r[256];                       /* registradores, como SCC.R do fMSX */
    uint32_t wave_phase[SCC_CHANNELS];    /* indice da onda em Q16 (bits 16-20 = 0..31) */
    uint64_t cycle_phase;                 /* resto do reamostrador (ciclos -> amostras) */
} SccState;

void scc_reset(SccState *s);

/* Leitura de registrador: `plus` = 0 modo generico (SCC), 1 modo SCC+ (enderecos com
 * A13 setado, B8xxh). Registradores sem leitura devolvem 0FFh, como no fMSX. */
uint8_t scc_read(const SccState *s, uint8_t reg, int plus);

/* Escrita de registrador: mesmas regras de WriteSCC()/WriteSCCP() do fMSX. */
void scc_write(SccState *s, uint8_t reg, uint8_t value, int plus);

/* Soma a contribuicao de um canal em `n` amostras de `acc` e avanca a fase da onda.
 * Implementacao em Assembly dual-ABI (src/scc/asm/render_channel.asm).
 * wave: 32 amostras com sinal; phase: fase Q16 do canal (atualizada);
 * step: incremento de fase por amostra em Q16; level: nivel de volume (Fortran). */
void scc_render_channel(int32_t *acc, int n, const int8_t *wave, uint32_t *phase, uint32_t step, int32_t level);

/* Referencia em C de scc_render_channel(), so para o teste diferencial do Assembly. */
void scc_render_channel_ref(int32_t *acc, int n, const int8_t *wave, uint32_t *phase, uint32_t step, int32_t level);

/* Avanca `z80_cycles` ciclos de Z80 e escreve ate `max_samples` amostras PCM mono
 * de 16 bits em `out` (taxa `sample_rate` Hz). `out` pode ser NULL: o estado
 * avanca e as amostras sao descartadas. Devolve quantas amostras foram escritas. */
int scc_advance(SccState *s, int z80_cycles, int sample_rate, int16_t *out, int max_samples);

#ifdef __cplusplus
}
#endif
