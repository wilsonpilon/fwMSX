/* fwMSX -- montador (fwasm): codificador de instrucoes do Z80 (C). Codigo ORIGINAL do fwMSX (BSD-3-Clause),
 * escrito a partir do conjunto de instrucoes do Z80 (publico, bem conhecido) e do comportamento observado
 * do asMSX 1.2.0 -- nao do fonte dele. Ver doc/asm-spec.md.
 *
 * O codificador NAO conhece rotulos nem expressoes: o chamador (C++) classifica cada operando (registrador,
 * indireto, imediato, condicao...) e ja' avalia seu valor. O codificador devolve os bytes e descreve os
 * "campos" (imediatos, deslocamentos, saltos relativos) que o chamador preenche e confere (avisos de corte
 * de 8/16 bits, alcance do salto relativo).
 */
#ifndef FWASM_Z80_ENC_H
#define FWASM_Z80_ENC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Tipo de operando. */
typedef enum ZKind {
    ZK_NONE = 0,
    ZK_R8,      /* a b c d e h l: reg = codigo Z80 (b=0 c=1 d=2 e=3 h=4 l=5 a=7) */
    ZK_IDXH,    /* ixh ixl iyh iyl: reg = 4 (h) ou 5 (l); pfx = 0xDD ou 0xFD */
    ZK_I,       /* registrador I */
    ZK_R,       /* registrador R */
    ZK_F,       /* "registrador" F (in f,[c]) */
    ZK_RR,      /* bc de hl sp: reg = 0..3 */
    ZK_AF,
    ZK_AFP,     /* af' */
    ZK_IX,
    ZK_IY,
    ZK_IHL,     /* [hl] */
    ZK_IBC,     /* [bc] */
    ZK_IDE,     /* [de] */
    ZK_ISP,     /* [sp] */
    ZK_IC,      /* [c] */
    ZK_IIX,     /* [ix] ou [ix+d]: value = d */
    ZK_IIY,     /* [iy] ou [iy+d] */
    ZK_IABS,    /* [nn] */
    ZK_IMM,     /* nn (numero, rotulo, expressao) */
    ZK_COND     /* nz z nc c po pe p m: reg = 0..7 */
} ZKind;

typedef struct ZOp {
    int kind;      /* ZKind */
    int reg;       /* codigo do registrador/condicao */
    int pfx;       /* 0xDD / 0xFD para ZK_IDXH */
    int64_t value; /* valor do imediato / deslocamento / numero do bit */
} ZOp;

/* Campo a ser preenchido pelo chamador. */
typedef enum ZFieldKind {
    ZF_IMM8 = 1,  /* byte imediato (corte de 8 bits com aviso) */
    ZF_IMM16,     /* palavra imediata (corte de 16 bits com aviso) */
    ZF_DISP,      /* deslocamento [ix+d] (sem aviso) */
    ZF_REL,       /* salto relativo (alvo absoluto; erro "unreachable address" se longe) */
    ZF_CHK8       /* nao grava byte: so' confere a faixa de 8 bits (aviso de corte) -- out [c],0 */
} ZFieldKind;

typedef struct ZField {
    int pos;   /* indice do byte dentro de `bytes` */
    int kind;  /* ZFieldKind */
    int op;    /* indice do operando de onde vem o valor */
} ZField;

/* Erros do codificador (0 = ok). */
typedef enum ZErr {
    ZE_OK = 0,
    ZE_SYNTAX = 1,   /* combinacao de operandos que nao existe: "syntax error" */
    ZE_DEST_A = 2,   /* "destiny register should be A" */
    ZE_SRC_A = 3,    /* "source register should be A" */
    ZE_IM = 4,       /* "wrong interruption mode" */
    ZE_RST = 5,      /* "wrong restart address" */
    ZE_REGCOMB = 6,  /* "wrong register combination" */
    ZE_ZERO = 7,     /* "value should be 0" (out [c],n) */
    ZE_NOCOND = 8,   /* "missing condition" (ret r) */
    ZE_BADCOND = 9   /* "wrong condition" (jr po,x ...) */
} ZErr;

typedef struct ZEnc {
    uint8_t bytes[6];
    int len;
    int nfields;
    ZField field[2];
    int bit_warn; /* 1 se o numero do bit (BIT/RES/SET) foi cortado para 3 bits: o chamador avisa "3-bit overflow" */
} ZEnc;

/* Id do mnemonico (minusculas), ou -1 se nao e' mnemonico do Z80. */
int z80_mnemonic(const char *name);

/* Codifica `mnem` com `n` operandos. Devolve ZE_OK ou um ZErr; em erro `out` e' indefinido. */
int z80_encode(int mnem, const ZOp *ops, int n, ZEnc *out);

#ifdef __cplusplus
}
#endif

#endif
