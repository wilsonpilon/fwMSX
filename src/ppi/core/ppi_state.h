// Adaptado de fMSX (resource/fMSX/EMULib/I8255.{h,c} e os casos A8h-ABh de
// InZ80()/WrZ80() em resource/fMSX/fMSX/MSX.c), Copyright (C) Marat
// Fayzullin 1994-2021. O fwMSX evolui a partir do fMSX com o aval do
// autor original para adaptar/estudar seu codigo (ver README.md) -- isso
// nao e uma relicenciacao: este arquivo continua sob os termos originais
// dele (nao-comercial, aviso ao autor em caso de mudanca), nao o
// BSD-3-Clause do restante do fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// PPI i8255 do MSX (portas A8h-ABh) + matriz de teclado. No MSX:
//   A8h (porta A, saida)   -- registrador de slot primario
//   A9h (porta B, entrada) -- linha da matriz de teclado selecionada
//   AAh (porta C, saida)   -- bits 0-3: linha do teclado a ler; bit 4:
//                             rele do motor do cassete; bit 5: saida de
//                             cassete; bit 6: LED de CAPS; bit 7: click
//   ABh (controle)         -- modo (bit 7=1) ou set/reset de um bit de C
// Ver doc/ppi-spec.md.
//
// Escopo desta fase: so' o chip e a matriz; este motor em C NAO sabe o que
// o slot primario faz (isso e' do MemorySystem, ligado pela camada C++ --
// ppi::PpiDevice), nem produz som de click/rele (PPIOut() do fMSX chama
// Drum(), som ainda nao existe).
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PPI_KEY_ROWS 16       /* KeyState[16] do fMSX; so' as linhas 0-10 tem teclas */
#define PPI_KEY_MATRIX_ROWS 11

typedef struct PpiState {
    uint8_t r[4];    /* Registradores (A, B, C, controle) -- I8255.R */
    uint8_t rout[3]; /* Valores nos pinos de saida das portas A/B/C -- I8255.Rout */
    uint8_t rin[3];  /* Valores nos pinos de entrada -- I8255.Rin */
    uint8_t key_state[PPI_KEY_ROWS]; /* KeyState[]: bit em 0 = tecla pressionada */
} PpiState;

// Zera registradores/portas, poe tudo em modo "entrada" (controle 0x9B,
// como Reset8255() do fMSX) e SOLTA todas as teclas -- ver ppi_reset_chip()
// para resetar so' o chip, mantendo as teclas.
void ppi_reset(PpiState *p);

// So' o chip (Reset8255()): o que um reset de maquina faz. A matriz de
// teclado e' o mundo externo e nao e' tocada.
void ppi_reset_chip(PpiState *p);

// Registrador `reg` (0-3 = portas A8h-ABh; so' os 2 bits baixos importam).
// Escrita: Write8255() do fMSX (incluindo set/reset de bit do controle).
void ppi_write(PpiState *p, int reg, uint8_t value);
// Leitura: antes de Read8255(), carrega a porta B com a linha do teclado
// escolhida por AAh (PPI.Rin[1]=KeyState[PPI.Rout[2]&0x0F] no fMSX).
uint8_t ppi_read(PpiState *p, int reg);

// --- Teclado ---------------------------------------------------------

#define PPI_KEY_NONE (-1)

// Numero de teclas nomeadas e nome de cada uma ("a".."z", "0".."9",
// "shift", "enter", "space", "pad5"...). NULL se `id` for invalido.
int ppi_key_count(void);
const char *ppi_key_name(int id);
// id da tecla pelo nome (sem diferenciar maiusculas), ou PPI_KEY_NONE.
int ppi_key_lookup(const char *name);
// Posicao da tecla na matriz (linha 0-10, mascara de 1 bit); 0 se invalido.
int ppi_key_position(int id, int *row, uint8_t *mask);

// Pressiona/solta uma tecla (bit em 0 = pressionada, logica invertida da
// matriz real). Devolve 0 se `id` for invalido.
int ppi_key_set(PpiState *p, int id, int pressed);
void ppi_key_release_all(PpiState *p);

// Quantas teclas estao pressionadas nas linhas 0-10 -- implementada em
// Assembly (src/ppi/asm/key_count.asm, dual-ABI Win64/SysV).
int ppi_pressed_count(const uint8_t *key_state);

#ifdef __cplusplus
}
#endif
