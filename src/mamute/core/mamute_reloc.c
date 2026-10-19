/* fwMSX -- Mamute: tamanho de instrucao Z80 e operandos de endereco (C). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
 *
 * O XRT precisa saber, instrucao por instrucao, ONDE esta' o endereco absoluto que vai ser ajustado quando um programa muda de lugar.
 * O disassembler do fwMSX devolve texto; aqui e' so' a estrutura dos opcodes, em tabelas curtas. */
#include "mamute_reloc.h"

/* opcodes base de 2 bytes (operando de 8 bits) */
static int base_has_imm8(int op) {
    switch (op) {
    case 0x06: case 0x0E: case 0x16: case 0x1E: case 0x26: case 0x2E: case 0x36: case 0x3E: /* LD r,n */
    case 0xC6: case 0xCE: case 0xD6: case 0xDE: case 0xE6: case 0xEE: case 0xF6: case 0xFE: /* ADD..CP n */
    case 0xD3: case 0xDB:                                                                   /* OUT (n),A / IN A,(n) */
        return 1;
    default:
        return 0;
    }
}

static int base_is_rel(int op) { return op == 0x10 || op == 0x18 || op == 0x20 || op == 0x28 || op == 0x30 || op == 0x38; }

/* opcodes base de 3 bytes (operando de 16 bits) -- todos carregam um endereco ou um numero de 16 bits */
static int base_has_imm16(int op) {
    switch (op) {
    case 0x01: case 0x11: case 0x21: case 0x31:               /* LD dd,nn */
    case 0x22: case 0x2A: case 0x32: case 0x3A:               /* LD (nn),HL / HL,(nn) / (nn),A / A,(nn) */
    case 0xC2: case 0xC3: case 0xC4: case 0xCA: case 0xCC: case 0xCD:
    case 0xD2: case 0xD4: case 0xDA: case 0xDC:
    case 0xE2: case 0xE4: case 0xEA: case 0xEC:
    case 0xF2: case 0xF4: case 0xFA: case 0xFC:               /* JP, CALL (com e sem condicao) */
        return 1;
    default:
        return 0;
    }
}

/* com DD/FD, o operando (HL) vira (IX+d) e ganha um byte de deslocamento */
static int uses_hl_indirect(int op) {
    if (op == 0x34 || op == 0x35 || op == 0x36) return 1;                  /* INC/DEC (HL), LD (HL),n */
    if (op >= 0x40 && op <= 0x7F && op != 0x76) return (op & 7) == 6 || ((op >> 3) & 7) == 6; /* LD r,(HL) / LD (HL),r */
    if (op >= 0x80 && op <= 0xBF) return (op & 7) == 6;                    /* ADD..CP (HL) */
    return 0;
}

int mamute_insn_info(const uint8_t *b, int avail, int *abs_off, int *rel_off) {
    int op, op2, len;
    uint8_t buf[4];
    int i;
    for (i = 0; i < 4; ++i) buf[i] = i < avail ? b[i] : 0;
    *abs_off = -1;
    *rel_off = -1;
    op = buf[0];

    if (op == 0xCB) return 2;
    if (op == 0xED) {
        op2 = buf[1];
        if (op2 == 0x43 || op2 == 0x4B || op2 == 0x53 || op2 == 0x5B || op2 == 0x63 || op2 == 0x6B || op2 == 0x73 || op2 == 0x7B) {
            *abs_off = 2; /* LD (nn),dd / LD dd,(nn) */
            return 4;
        }
        return 2;
    }
    if (op == 0xDD || op == 0xFD) {
        op2 = buf[1];
        if (op2 == 0xCB) return 4;                               /* DD CB d op */
        if (op2 == 0xDD || op2 == 0xFD || op2 == 0xED) return 1; /* prefixo sem efeito: o proximo reinicia a decodificacao */
        if (op2 == 0x21 || op2 == 0x22 || op2 == 0x2A) {         /* LD IX,nn / LD (nn),IX / LD IX,(nn) */
            *abs_off = 2;
            return 4;
        }
        len = 2;
        if (base_has_imm8(op2) && op2 != 0xD3 && op2 != 0xDB) len += 1; /* LD IXH,n e LD (IX+d),n (este tem os dois bytes) */
        if (uses_hl_indirect(op2)) len += 1;
        return len > 4 ? 4 : len;
    }
    if (base_has_imm16(op)) {
        *abs_off = 1;
        return 3;
    }
    if (base_is_rel(op)) {
        *rel_off = 1;
        return 2;
    }
    if (base_has_imm8(op)) return 2;
    return 1;
}
