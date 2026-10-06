// fwMSX -- motor do chip FM OPLL (YM2413), usado pelo MSX-MUSIC (portas 7Ch/7Dh)
// e pelo cartucho FM-PAC. Codigo ORIGINAL do fwMSX (BSD-3-Clause), exceto os
// timbres prontos (ym2413_patches.h, dados do fMSX). Ver doc/fm-spec.md.
//
// Modelo: 9 canais melodicos, cada um com 2 operadores (modulador -> portadora),
// envelope ADSR, feedback, vibrato e tremolo. NAO e' emulacao ciclo-a-ciclo do
// chip: o timing e o nivel sao aproximados (ver doc/fm-spec.md, secao 2).
#ifndef FWMSX_FM_YM2413_STATE_H
#define FWMSX_FM_YM2413_STATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YM2413_CHANNELS 9
#define YM2413_Z80_CLOCK 3579545   /* clock do Z80 do MSX (Hz): base do tempo do chip */
#define YM2413_NATIVE_RATE 49716   /* clock do OPLL / 72: taxa interna do chip */
#define YM2413_FULL_SCALE 3000     /* amplitude maxima de um canal (de 9 somados) */

typedef enum {
    YM2413_OP_OFF = 0,
    YM2413_OP_ATTACK,
    YM2413_OP_DECAY,
    YM2413_OP_SUSTAIN,
    YM2413_OP_RELEASE
} Ym2413OpState;

/* Configuracao de um operador, calculada a partir do timbre e dos registradores
   (PrepareChannel). Tudo em forma pronta para o laco de amostras. */
typedef struct {
    double mult;     /* fator de frequencia (MULT) */
    double gain;     /* atenuacao fixa: TL (modulador) ou volume (portadora), mais KSL */
    double ka;       /* coeficiente de ataque (1/s); 0 = sem ataque */
    double dr_dbps;  /* queda de decaimento (dB/s) */
    double rr_dbps;  /* queda de liberacao (dB/s) */
    double sl;       /* nivel de sustentacao (ganho linear) */
    int egt;         /* 1 = sustenta ate' o key-off (EG type / bit SUS de 20h-28h) */
    int am;          /* tremolo */
    int vib;         /* vibrato */
    int wf;          /* 1 = meia onda positiva (forma de onda) */
} Ym2413OpCfg;

typedef struct {
    double phase;    /* posicao no ciclo, 0..1 */
    double env;      /* envelope (ganho linear 0..1) */
    int state;       /* Ym2413OpState */
} Ym2413Op;

typedef struct {
    Ym2413Op op[2];       /* [0] modulador, [1] portadora */
    Ym2413OpCfg cfg[2];
    double fb1, fb2;      /* ultimas saidas do modulador (feedback) */
    double fb_amt;        /* profundidade do feedback, em ciclos */
    double base_hz;       /* frequencia fundamental: fnum * 49716 / 2^(19-block) */
    uint16_t fnum;        /* 9 bits */
    uint8_t block;        /* 3 bits (oitava) */
    uint8_t key;          /* KEY-ON (bit 4 de 20h-28h), so' no modo melodico */
    uint8_t opkey[2];     /* KEY de cada operador (na bateria, cada um tem o seu) */
    uint8_t sus;          /* SUS (bit 5 de 20h-28h) */
    uint8_t inst;         /* 0 = timbre do usuario; 1-15 = timbre pronto */
    uint8_t vol;          /* volume (30h-38h, bits 3-0): atenuacao 3 dB por passo */
} Ym2413Ch;

typedef struct {
    uint8_t reg[64];      /* espelho dos registradores (00h-3Fh) */
    uint8_t latch;        /* registrador selecionado pela porta 7Ch */
    uint8_t drum_keys;    /* teclas da bateria ligadas (reg 0Eh, bits 4-0) */
    Ym2413Ch ch[YM2413_CHANNELS];
    double am_phase;      /* LFO do tremolo (ciclos) */
    double vib_phase;     /* LFO do vibrato (ciclos) */
    double frac;          /* ciclos do Z80 ainda nao convertidos em amostras */
} Ym2413State;

/* Reset do chip: todos os registradores em 0, sem nota tocando. */
void ym2413_reset(Ym2413State *s);

/* Escreve `value` no registrador `reg` (0-3Fh) e atualiza o canal afetado. */
void ym2413_write_reg(Ym2413State *s, uint8_t reg, uint8_t value);

/* Avanca `z80_cycles` ciclos do Z80 e gera as amostras correspondentes a
   `out_rate` Hz. Se `out` for NULL, so' o estado avanca. Escreve no maximo
   `max` amostras de 16 bits em `out` e devolve quantas foram geradas. */
int ym2413_advance(Ym2413State *s, int z80_cycles, int out_rate, int16_t *out, int max);

/* Timbre pronto `inst` (1-15): 8 bytes, ou NULL se fora da faixa. */
const uint8_t *ym2413_builtin_patch(int inst);

#ifdef __cplusplus
}
#endif

#endif /* FWMSX_FM_YM2413_STATE_H */
