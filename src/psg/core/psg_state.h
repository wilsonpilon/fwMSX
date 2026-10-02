// Adaptado de fMSX (resource/fMSX/EMULib/AY8910.{h,c} -- mascaras de
// registrador e protocolo de portas -- e dos casos A0h-A2h de InZ80()/
// OutZ80() em resource/fMSX/fMSX/MSX.c), Copyright (C) Marat Fayzullin
// 1996-2021. O fwMSX evolui a partir do fMSX com o aval do autor original
// para adaptar/estudar seu codigo (ver README.md) -- isso nao e' uma
// relicenciacao: este arquivo continua sob os termos originais dele
// (nao-comercial, aviso ao autor em caso de mudanca), nao o BSD-3-Clause
// do restante do fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// PSG AY-3-8910 do MSX (portas A0h-A2h): 3 canais de tom + gerador de
// ruido + gerador de envelope. DIFERENTE do fMSX (que repassa freq/volume
// para um sintetizador de alto nivel, Sound()), este motor gera AMOSTRAS
// PCM de verdade, avancadas por ciclos de Z80 -- assim o som e' testavel
// sem placa de audio (ver doc/psg-spec.md).
//   A0h -- latch do numero do registrador (4 bits)
//   A1h -- escrita no registrador latched
//   A2h -- leitura do registrador latched (R14 = joystick, R15 = so' 4
//          bits altos)
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PSG_REGS 16
#define PSG_CHANNELS 3

/* Clock do Z80 no MSX (NTSC). O PSG recebe metade (1.789772 MHz). Um "tick"
 * de gerador dura 8 ciclos de PSG = 16 ciclos de Z80: o tom alterna a cada
 * `periodo` ticks (onda completa = 16*periodo ciclos de PSG, como diz o
 * datasheet: f = fclock/(16*periodo)). */
#define PSG_Z80_CLOCK 3579545
#define PSG_CYCLES_PER_TICK 16

/* Bits do joystick MSX (JST_* do fMSX), 1 = pressionado: o estado "do mundo
 * externo" que o host informa com psg_set_joystick(). */
#define PSG_JOY_UP 0x01
#define PSG_JOY_DOWN 0x02
#define PSG_JOY_LEFT 0x04
#define PSG_JOY_RIGHT 0x08
#define PSG_JOY_FIRE_A 0x10
#define PSG_JOY_FIRE_B 0x20

typedef struct PsgState {
    uint8_t r[PSG_REGS]; /* Registradores, ja' mascarados como Write8910() do fMSX */
    uint8_t latch;       /* Registrador selecionado por A0h */

    uint16_t tone_cnt[PSG_CHANNELS];
    uint8_t tone_out[PSG_CHANNELS];

    uint8_t noise_cnt;
    uint8_t noise_half; /* o LFSR so' desloca a cada 2 periodos de ruido */
    uint32_t lfsr;      /* registrador de deslocamento de 17 bits */

    uint32_t env_cnt;
    uint8_t env_step;   /* 0-15 dentro do ciclo atual */
    uint8_t env_attack; /* 1 = rampa subindo */
    uint8_t env_hold;   /* 1 = envelope parado no valor final */

    /* Joystick das portas A (0) e B (1), bits PSG_JOY_*, 1 = pressionado. Nao e'
     * registrador do chip: e' o mundo externo, como as teclas do PPI -- psg_reset()
     * o zera, mas PsgDevice::Reset() (reset de maquina) o preserva. */
    uint8_t joy[2];

    /* Reamostragem: ciclos de Z80 ainda nao convertidos em ticks, fase do
     * reamostrador e soma do filtro de caixa. */
    uint32_t cycle_acc;
    uint32_t phase;
    int64_t box_sum;
    int32_t box_count;
} PsgState;

// Estado de reset do AY8910 (RegInit do fMSX: R7=FDh, R14=FFh) e geradores
// zerados.
void psg_reset(PsgState *p);

// Informa o estado do joystick da porta `port` (0 = A, 1 = B): mascara de
// PSG_JOY_*, 1 = pressionado. Lido pelo software em R14 (ver
// psg_read_data()).
void psg_set_joystick(PsgState *p, int port, uint8_t bits);

// Latch/escrita/leitura de registrador pelas portas A0h/A1h/A2h.
void psg_write_latch(PsgState *p, uint8_t value);
void psg_write_data(PsgState *p, uint8_t value);
uint8_t psg_read_data(const PsgState *p);

// Escrita direta em um registrador (0-15), com as mascaras do hardware.
void psg_write_reg(PsgState *p, int reg, uint8_t value);

// Frequencia de tom (Hz) de um canal 0-2 -- Z80_CLOCK/2/(16*periodo);
// 0.0 se o periodo for 0 ou o canal estiver com tom desligado no mixer.
double psg_tone_hz(const PsgState *p, int channel);

// Amplitude (0-15) efetiva de um canal -- ja' resolve o bit de envelope.
int psg_channel_level(const PsgState *p, int channel);

// Avanca `z80_cycles` ciclos de Z80 e escreve ate' `max_samples` amostras
// PCM mono de 16 bits em `out` (taxa `sample_rate` Hz). `out` pode ser
// NULL: o estado avanca e as amostras sao descartadas. Devolve quantas
// amostras foram produzidas.
int psg_advance(PsgState *p, int z80_cycles, int sample_rate, int16_t *out, int max_samples);

#ifdef __cplusplus
}
#endif
