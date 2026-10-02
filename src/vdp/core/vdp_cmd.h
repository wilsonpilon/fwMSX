// Adaptado de fMSX (resource/fMSX/fMSX/V9938.{h,c}), Copyright (C) Marat
// Fayzullin 1994-2021; reescrito por Alex Wulms (ver o cabecalho original:
// execucao "em paralelo" com a CPU e temporizacao correta). O fwMSX evolui a
// partir do fMSX com o aval do autor original para adaptar/estudar seu
// codigo (ver README.md) -- isso nao e' uma relicenciacao: este arquivo
// continua sob os termos originais dele (nao-comercial, aviso ao autor em
// caso de mudanca), nao o BSD-3-Clause do restante do fwMSX. Ver
// LICENSE-THIRD-PARTY.md.
//
// Motor de comandos do V9938 (MSX2): POINT, PSET, SRCH, LINE, LMMV, LMMM,
// LMCM, LMMC, HMMV, HMMM, YMMM, HMMC -- operacoes graficas em VRAM
// (SCREEN 5-8) executadas pelo proprio VDP, com a CPU so' disparando o
// comando pelo registrador 46 e (nos comandos LMMC/HMMC/LMCM) trocando dados
// pelo registrador 44 / status 7 com o handshake do bit TR (S#2 bit 7).
//
// Temporizacao: como no fMSX, cada comando consome "ciclos" de um orcamento
// (ops_cnt) reposto a cada scanline por vdp_cmd_loop() -- o comando termina
// em alguns quadros se for grande, nao instantaneamente. S#2 bit 0 (CE) fica
// 1 enquanto executa. Ver doc/vdp-spec.md (Fase 5).
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct VdpState;

/* Estado interno do comando em execucao (MMC do V9938.c). */
typedef struct VdpCmd {
    int sx, sy;
    int dx, dy;
    int tx, ty;
    int nx, ny;
    int mx;
    int asx, adx, anx;
    uint8_t cl;
    uint8_t lo;
    uint8_t cm;
} VdpCmd;

/* Dispara o comando `op` (valor escrito no R#46). Devolve 1 se foi aceito. So'
 * funciona nos modos SCREEN 5-8; nos outros devolve 0. */
int vdp_cmd_draw(struct VdpState *v, uint8_t op);

/* CPU -> VDP: dado escrito no R#44 (LMMC/HMMC). */
void vdp_cmd_write(struct VdpState *v, uint8_t value);

/* VDP -> CPU: leitura do S#7 (LMCM); devolve o proximo pixel. */
uint8_t vdp_cmd_read(struct VdpState *v);

/* Avanca o comando ativo (chamar uma vez por scanline, como LoopVDP()). */
void vdp_cmd_loop(struct VdpState *v);

#ifdef __cplusplus
}
#endif
