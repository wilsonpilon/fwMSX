// Adaptado de fMSX (resource/fMSX/fMSX/MSX.c, resource/fMSX/fMSX/MSX.h),
// Copyright (C) Marat Fayzullin 1994-2021. O fwMSX evolui a partir do
// fMSX com o aval do autor original para adaptar/estudar seu codigo (ver
// README.md) -- isso nao e uma relicenciacao: este arquivo continua sob
// os termos originais dele (nao-comercial, aviso ao autor em caso de
// mudanca), nao o BSD-3-Clause do restante do fwMSX. Ver
// LICENSE-THIRD-PARTY.md.
//
// Motor "digital" do VDP (registradores, protocolo de porta 98h-9Bh,
// maquina de estados de scanline/interrupcao) -- adapta a parte de
// MSX.c relativa a isso: WrZ80/InZ80 (casos 98h-9Bh), VDPOut(),
// SetScreen(), SetIRQ(), e a maquina de estados de LoopZ80() (SO a parte
// de VBlank/HBlank/coincidencia de linha -- ver a lista de exclusoes
// abaixo). NAO adapta nada de V9938.c (motor de comando MSX2 -- Fase 5,
// ver doc/vdp-spec.md, secao 6) nem as ~12 funcoes RefreshLineN()
// (renderizacao de pixel -- Fase 2).
//
// Escopo desta Fase 1 (ver doc/vdp-spec.md, secao 4/6 para o raciocinio
// completo) -- "VDP digital", SEM desenhar nenhum pixel:
//   - Registradores (64) + status (16) + protocolo das 4 portas
//     (98h dados, 99h endereco/registrador, 9Ah paleta, 9Bh registrador
//     indireto), fielmente portado incluindo os dois latches de 2
//     escritas (99h/9Ah) e o comportamento de reconhecimento de
//     interrupcao ao ler o status (99h) -- ver vdp_in().
//   - Cache de ponteiro (deslocamento, nao ponteiro cru -- ver nota em
//     VdpState abaixo) por tabela (ChrTab/ColTab/ChrGen/SprTab/SprGen),
//     recomputado em vdp_write_register()/vdp_set_screen(), mesmo
//     principio ja' usado no nucleo Z80 e no mapa de memoria.
//   - Maquina de estados de scanline/interrupcao (vdp_step_scanline()),
//     adaptada de LoopZ80() -- SO a parte de VBlank(IE0)/HBlank e
//     coincidencia de linha (IE1). Deliberadamente NAO adaptado dessa
//     mesma funcao (fora do escopo desta fase, ver doc/vdp-spec.md):
//     LoopVDP() (motor de comando V9938), RefreshLine[]() (renderizacao),
//     piscar de texto (BFlag/BCount/UCount, so' usado por rendering),
//     som (Loop8910/Sync8910/etc.), teclado/joystick/mouse, cheats.
//
// Diferencas deliberadas em relacao ao fMSX:
//   - UMA pagina de VRAM (16KB) fixa nesta fase -- ver VDP_VRAM_PAGES em
//     vdp_types.h. Os deslocamentos de tabela (chr_tab/col_tab/etc.) sao
//     calculados com a formula EXATA do fMSX, sem mascarar para caber em
//     16KB -- podem exceder VDP_VRAM_SIZE quando os bits MSX2 de pagina
//     extra (regs[10]/regs[11]) estiverem setados; isso e' esperado e
//     inofensivo nesta fase (nada ainda desreferencia esses
//     deslocamentos), e sera' resolvido quando VRAM multi-pagina for
//     implementada (Fase 2+).
//   - `irq_pending` resume o IRQPending (bitmask VDP_INT_IE0|VDP_INT_IE1)
//     do fMSX; nao existe SetIRQ() combinando fontes nao-VDP (o fMSX usa
//     o mesmo IRQPending para VDP e outras fontes que o fwMSX ainda nao
//     tem) -- aqui e' so' e exclusivamente o VDP.
//   - S#7 (leitura de porta 99h com regs[15]==7, usada pelo motor de
//     comando V9938 para LMCM/VRAM->CPU) NAO tem efeito colateral
//     proprio aqui -- fora de escopo (motor de comando, Fase 5).
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "../common/vdp_types.h"
#include "vdp_cmd.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VdpState {
    uint8_t regs[64];   /* VDP[] do fMSX -- R#0..R#46 usados, resto reservado */
    uint8_t status[16]; /* VDPStatus[] do fMSX -- S#0..S#9ish usados */

    uint8_t vram[VDP_VRAM_SIZE];

    /* Modelo do VDP (VDP_MODEL_*): MSX1 = 16KB e sem comandos/modos 4-8; MSX2 =
       128KB (8 paginas via R#14), modos SCREEN 4-8, sprites de modo 2 e motor
       de comandos. vram_mask = tamanho da VRAM em uso - 1 (3FFFh/1FFFFh). */
    uint8_t model;
    uint8_t vram_pages;
    uint32_t vram_mask;

    /* Motor de comandos do V9938 (ver vdp_cmd.h): estado do comando ativo, o
       orcamento de "ciclos" desta scanline e qual engine esta rodando. */
    VdpCmd cmd;
    int ops_cnt;
    uint8_t engine;

    /* Piscar de TEXT80 (BFlag/BCount do LoopZ80 do fMSX): cores de frente/fundo
       "extras" (XFGColor/XBGColor) usadas pelos caracteres com o bit de piscar. */
    uint8_t blink_flag;
    uint8_t blink_count;
    uint8_t x_fg, x_bg;

    /* Protocolo de porta -- ver InZ80/WrZ80 98h-9Bh em MSX.c. */
    uint16_t vaddr; /* VAddr: endereco de VRAM atual (14 bits, 0..3FFFh) */
    uint8_t vdata;  /* VDPData: buffer de leitura de um passo atras */
    uint8_t vkey;   /* VKey: sequenciador do latch de 2 escritas da porta 99h */
    uint8_t alatch; /* ALatch: primeiro byte da sequencia de 99h */
    uint8_t pkey;   /* PKey: sequenciador do latch de 2 escritas da porta 9Ah */
    uint8_t platch; /* PLatch: primeiro byte da sequencia de 9Ah */

    /* Paleta (V9938+, 16 entradas) -- RGB888 ja' convertido (formula
       exata de WrZ80 caso 9Ah: R=(PLatch&0x70)*255/112 etc.). Nao
       consumido por nada ainda nesta fase (sem renderizacao) -- ver
       tambem src/vdp/fortran/palette_table.f90 para a tabela completa
       de conversao (todas as 512 combinacoes possiveis, nao so' as 16
       efetivamente escritas ate agora). */
    uint8_t palette_r[16];
    uint8_t palette_g[16];
    uint8_t palette_b[16];

    /* Modo de tela + cache de tabela (ver SetScreen()/VDPOut() no
       fMSX). Deslocamentos (uint32_t), NAO ponteiros -- vram[] e' um
       array de tamanho fixo dentro deste struct, e a formula do fMSX
       pode legitimamente exceder VDP_VRAM_SIZE quando bits de pagina
       extra do MSX2 estao setados (ver nota de topo do arquivo) --
       um deslocamento largo o bastante evita qualquer risco de
       ponteiro-fora-dos-limites antes que isso seja resolvido (Fase
       2+). Mascaras (chr_tab_mask etc.) tambem guardadas, mesma
       formula do fMSX, para paridade -- ainda sem uso nesta fase. */
    uint8_t scr_mode; /* ScrMode: 0..8, 9=nenhum, 10..12=YAE/YJK, 13=TEXT80 */
    uint32_t chr_tab, col_tab, chr_gen, spr_tab, spr_gen;
    uint32_t chr_tab_mask, col_tab_mask, chr_gen_mask, spr_tab_mask;

    /* Maquina de estados de scanline/interrupcao -- ver LoopZ80() no
       fMSX (so' a fatia de VBlank/HBlank/coincidencia de linha). */
    int scanline; /* ScanLine: 0..261 (NTSC) ou 0..311 (PAL) */
    int drawing;  /* Drawing: fase ativa de desenho (controla a janela de VBlank) */
    uint8_t irq_pending; /* IRQPending do fMSX, restrito as fontes do VDP */
} VdpState;

typedef struct VdpStepResult {
    /* Proximo valor de R->IPeriod (T-states ate a proxima chamada
       necessaria de vdp_step_scanline()) -- equivalente ao que LoopZ80
       escreve em R->IPeriod a cada chamada seguindo o modelo original de
       "roda ate a proxima borda de HRefresh/HBlank". O host (VdpDevice/
       Z80DebugSession) decide como usar isso para escalonar as chamadas
       -- ver doc/vdp-spec.md, secao 3.3. */
    int next_period_cycles;
    /* true quando ha' uma interrupcao de fonte VDP pendente agora
       (equivalente a IRQPending!=0 do fMSX) -- o host decide QUANDO
       efetivamente entregar isso ao Z80 (cpu.interrupt()), este motor
       em C so' informa o estado. */
    int irq_pending;
} VdpStepResult;

/* Escolhe o modelo (VDP_MODEL_MSX1/MSX2): ajusta o tamanho da VRAM em uso e
   zera o VDP (vdp_reset()). Chamar antes de rodar; o modelo sobrevive a
   vdp_reset() (um reset de maquina nao troca o VDP). */
void vdp_set_model(VdpState *v, int model);

/* Reset de maquina: zera o VDP mantendo o modelo ja' escolhido. */
void vdp_reset_keep_model(VdpState *v);

/* Zera registradores/status/VRAM/paleta, poe o modo de tela em 0 e
   recomputa o cache de tabela -- equivalente ao trecho de VDP em
   ResetMSX() do fMSX. Sempre como MSX1 (ver vdp_set_model()). */
void vdp_reset(VdpState *v);

/* Le/escreve nas portas 98h-9Bh -- protocolo completo (latches de 2
   escritas de 99h/9Ah, auto-incremento/rollover de pagina de 98h,
   acesso indireto a registrador de 9Bh, e o reconhecimento de
   interrupcao ao ler o status via 99h). `port` e' mascarado internamente
   com &0xFF, entao o chamador pode passar o numero de porta cru do Z80.
   Portas fora de 98h-9Bh devolvem 0xFF (leitura) / sao ignoradas
   (escrita) -- este motor so' conhece essas 4 portas, o resto e'
   trabalho de outro dispositivo (ver z80::CompositeBus). */
uint8_t vdp_in(VdpState *v, uint16_t port);
void vdp_out(VdpState *v, uint16_t port, uint8_t value);

/* Escreve diretamente num registrador de controle (0-63) -- equivalente
   a VDPOut() do fMSX. Chamado internamente por vdp_out() (portas 99h/
   9Bh), exposto tambem para o depurador (`vdpregs`/testes). Recomputa o
   cache de tabela quando o registrador afeta isso. */
void vdp_write_register(VdpState *v, int reg, uint8_t value);

/* Recalcula scr_mode a partir de regs[0]/regs[1] e o cache de tabela --
   equivalente a SetScreen() do fMSX. Chamado internamente por
   vdp_write_register() quando regs[0]/regs[1]/regs[25] mudam. */
int vdp_set_screen(VdpState *v);

/* Avanca a maquina de estados um "meio-scanline" (a mesma granularidade
   de LoopZ80() do fMSX, que alterna HRefresh duas vezes por linha) --
   ver a nota de escopo no topo do arquivo para o que NAO esta incluido
   (renderizacao, som, sprites, teclado/joystick/mouse). */
VdpStepResult vdp_step_scanline(VdpState *v);

#ifdef __cplusplus
}
#endif
