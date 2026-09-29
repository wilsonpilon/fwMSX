// Adaptado de fMSX (resource/fMSX/Z80/Z80.h), Copyright (C) Marat
// Fayzullin 1994-2021. O fwMSX evolui a partir do fMSX com o aval do
// autor original para adaptar/estudar seu codigo (ver README.md) -- isso
// nao e uma relicenciacao: este arquivo continua sob os termos originais
// dele (nao-comercial, aviso ao autor em caso de mudanca), nao o
// BSD-3-Clause do restante do fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// Estado da CPU Z80 (registradores, flags, contadores de ciclo). Ver
// doc/z80-core-spec.md, secao 3.2/3.6, para o raciocinio de design.
//
#pragma once

#include <stdint.h>
#include <assert.h> /* static_assert macro em C11 (keyword nativo em C++) */

#ifdef __cplusplus
extern "C" {
#endif

// fMSX suporta LSB_FIRST/MSB_FIRST porque tinha que rodar em CPUs de
// endianismo variado (inclusive o proprio Z80 real, big-endian em termos
// de registrador H/L). O fwMSX so roda em x86-64 (Windows/Linux), sempre
// little-endian -- por isso essa uniao fica fixa em "lo primeiro", sem a
// alternativa MSB_FIRST do original.
//
// Como no fMSX, essa uniao depende de type punning (ler um membro depois
// de escrever outro) -- tecnicamente comportamento nao especificado em C
// ISO estrito, mas uma extensao que GCC, Clang e MSVC suportam e
// documentam explicitamente, e da qual praticamente todo emulador de CPU
// (incluindo o proprio fMSX) depende na pratica.
typedef union {
    struct {
        uint8_t lo, hi;
    } b;
    uint16_t w;
} Z80Pair;

static_assert(sizeof(Z80Pair) == 2, "Z80Pair precisa ter exatamente 2 bytes");

// Bits do registrador F (flags).
#define Z80_S_FLAG 0x80 /* 1: resultado negativo */
#define Z80_Z_FLAG 0x40 /* 1: resultado zero */
#define Z80_H_FLAG 0x10 /* 1: meio-carry/meio-borrow */
#define Z80_P_FLAG 0x04 /* 1: resultado par (paridade) */
#define Z80_V_FLAG 0x04 /* 1: houve overflow (mesmo bit de P_FLAG) */
#define Z80_N_FLAG 0x02 /* 1: ultima operacao foi subtracao */
#define Z80_C_FLAG 0x01 /* 1: houve carry/borrow */

// Bits do campo "iff" (flip-flops de interrupcao + estado auxiliar).
#define Z80_IFF_1 0x01    /* flip-flop IFF1 */
#define Z80_IFF_IM1 0x02  /* 1: modo de interrupcao 1 */
#define Z80_IFF_IM2 0x04  /* 1: modo de interrupcao 2 */
#define Z80_IFF_2 0x08    /* flip-flop IFF2 */
#define Z80_IFF_EI 0x20   /* 1: EI pendente (atraso de uma instrucao) */
#define Z80_IFF_HALT 0x80 /* 1: CPU em HALT */

// Vetores que z80_interrupt() aceita.
#define Z80_INT_RST00 0x00C7
#define Z80_INT_RST08 0x00CF
#define Z80_INT_RST10 0x00D7
#define Z80_INT_RST18 0x00DF
#define Z80_INT_RST20 0x00E7
#define Z80_INT_RST28 0x00EF
#define Z80_INT_RST30 0x00F7
#define Z80_INT_RST38 0x00FF
#define Z80_INT_IRQ Z80_INT_RST38 /* opcode de IRQ default e' FFh (RST38) */
#define Z80_INT_NMI 0xFFFD
#define Z80_INT_NONE 0xFFFF

// Nota: fMSX tambem tem Trap/Trace/DebugZ80 (rastreamento passo-a-passo
// via #ifdef DEBUG) e um LoopZ80() de interrupcao periodica atrelado ao
// modelo RunZ80(). Nenhum dos dois entra nesta Fase 1 -- ver
// doc/z80-core-spec.md, secao 3.2/3.3 e secao 6 (Fase 1): o modelo de
// execucao adotado e' z80_run(cycles), controlado pelo host, sem a
// auto-varredura de interrupcao do RunZ80(); o rastreador de debug fica
// para a Fase 4, quando a CLI/TUI de debug do fwMSX existir de verdade.
typedef struct Z80State {
    Z80Pair pc, sp;
    Z80Pair af, bc, de, hl;
    Z80Pair ix, iy;
    Z80Pair af_alt, bc_alt, de_alt, hl_alt; /* registradores sombra */

    uint8_t i, r; /* registradores I (interrupt page) e R (refresh) */
    uint8_t iff;  /* flip-flops IFF1/IFF2, modo de interrupcao, EI-pendente, HALT */

    int iperiod; /* reservado para uso futuro (Fase 4, interrupcoes periodicas) */
    int icount;  /* ciclos restantes na chamada atual de z80_run() */
    int ibackup; /* uso interno do core -- nao mexer fora dele */

    uint16_t irequest;  /* vetor de IRQ pendente, ou Z80_INT_NONE */
    uint8_t iautoreset; /* 1: z80_interrupt() reseta irequest automaticamente */
    uint8_t trapbadops; /* 1: avisa (stderr) sobre opcodes nao reconhecidos */

    void *user_data; /* dado livre do host (contexto, id da maquina, etc.) */
} Z80State;

#ifdef __cplusplus
}
#endif
