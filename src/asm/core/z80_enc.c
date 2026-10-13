/* fwMSX -- codificador de instrucoes do Z80. Ver z80_enc.h. */
#include "z80_enc.h"

#include <string.h>

enum {
    M_ADC, M_ADD, M_AND, M_BIT, M_CALL, M_CCF, M_CP, M_CPD, M_CPDR, M_CPI, M_CPIR, M_CPL, M_DAA, M_DEC, M_DI, M_DJNZ, M_EI,
    M_EX, M_EXX, M_HALT, M_IM, M_IN, M_INC, M_IND, M_INDR, M_INI, M_INIR, M_JP, M_JR, M_LD, M_LDD, M_LDDR, M_LDI, M_LDIR,
    M_NEG, M_NOP, M_OR, M_OTDR, M_OTIR, M_OUT, M_OUTD, M_OUTI, M_POP, M_PUSH, M_RES, M_RET, M_RETI, M_RETN, M_RL, M_RLA,
    M_RLC, M_RLCA, M_RLD, M_RR, M_RRA, M_RRC, M_RRCA, M_RRD, M_RST, M_SBC, M_SCF, M_SET, M_SLA, M_SLL, M_SRA, M_SRL, M_SUB,
    M_XOR, M_COUNT
};

static const char *const kNames[M_COUNT] = {
    "adc", "add", "and", "bit", "call", "ccf", "cp", "cpd", "cpdr", "cpi", "cpir", "cpl", "daa", "dec", "di", "djnz", "ei",
    "ex", "exx", "halt", "im", "in", "inc", "ind", "indr", "ini", "inir", "jp", "jr", "ld", "ldd", "lddr", "ldi", "ldir",
    "neg", "nop", "or", "otdr", "otir", "out", "outd", "outi", "pop", "push", "res", "ret", "reti", "retn", "rl", "rla",
    "rlc", "rlca", "rld", "rr", "rra", "rrc", "rrca", "rrd", "rst", "sbc", "scf", "set", "sla", "sll", "sra", "srl", "sub",
    "xor"};

int z80_mnemonic(const char *name) {
    int i;
    for (i = 0; i < M_COUNT; ++i) {
        if (strcmp(kNames[i], name) == 0) return i;
    }
    return -1;
}

/* ---------------------------------------------------------------- utilitarios */

static void put(ZEnc *e, int b) { e->bytes[e->len++] = (uint8_t)b; }

static void field(ZEnc *e, int kind, int op) {
    e->field[e->nfields].pos = e->len;
    e->field[e->nfields].kind = kind;
    e->field[e->nfields].op = op;
    ++e->nfields;
    if (kind == ZF_CHK8) return; /* so' confere a faixa: nao ocupa byte */
    put(e, 0);
    if (kind == ZF_IMM16) put(e, 0);
}

static int is_r8(const ZOp *o) { return o->kind == ZK_R8; }
static int is_idx_ind(const ZOp *o) { return o->kind == ZK_IIX || o->kind == ZK_IIY; }
static int idx_pfx(const ZOp *o) { return (o->kind == ZK_IIX || o->kind == ZK_IX) ? 0xDD : 0xFD; }
static int is_a(const ZOp *o) { return o->kind == ZK_R8 && o->reg == 7; }
static int is_imm(const ZOp *o) { return o->kind == ZK_IMM; }
static int is_rr(const ZOp *o, int code) { return o->kind == ZK_RR && o->reg == code; }

/* r de 3 bits para um operando "registrador comum": A B C D E H L ou [HL]. -1 se nao for. */
static int reg3(const ZOp *o) {
    if (o->kind == ZK_R8) return o->reg;
    if (o->kind == ZK_IHL) return 6;
    return -1;
}

/* Operando de 8 bits geral das operacoes aritmeticas (r, [hl], [ix+d], ixh.., n): emite bytes de `base`
 * (base = 0x80 add, 0x88 adc, 0x90 sub, 0x98 sbc, 0xA0 and, 0xA8 xor, 0xB0 or, 0xB8 cp) e devolve ZE_OK/ZE_SYNTAX. */
static int alu8(ZEnc *e, int base, const ZOp *src, int src_index) {
    int r = reg3(src);
    if (r >= 0) {
        put(e, base | r);
        return ZE_OK;
    }
    if (src->kind == ZK_IDXH) {
        put(e, src->pfx);
        put(e, base | src->reg);
        return ZE_OK;
    }
    if (is_idx_ind(src)) {
        put(e, idx_pfx(src));
        put(e, base | 6);
        field(e, ZF_DISP, src_index);
        return ZE_OK;
    }
    if (is_imm(src)) {
        put(e, base | 0x46); /* C6 CE D6 DE E6 EE F6 FE */
        field(e, ZF_IMM8, src_index);
        return ZE_OK;
    }
    return ZE_SYNTAX;
}

static int alu_index(int mnem) {
    switch (mnem) {
    case M_ADD: return 0x80;
    case M_ADC: return 0x88;
    case M_SUB: return 0x90;
    case M_SBC: return 0x98;
    case M_AND: return 0xA0;
    case M_XOR: return 0xA8;
    case M_OR: return 0xB0;
    case M_CP: return 0xB8;
    }
    return 0;
}

/* ---------------------------------------------------------------- LD */

static int enc_ld(ZEnc *e, const ZOp *a, const ZOp *b) {
    int ra = reg3(a), rb = reg3(b);

    /* ld r,r' / ld r,n / ld r,[ix+d] / ld r,ixh ... (destino r8 ou [hl]) */
    if (ra >= 0) {
        if (rb >= 0) {
            if (ra == 6 && rb == 6) return ZE_SYNTAX; /* seria HALT */
            put(e, 0x40 | (ra << 3) | rb);
            return ZE_OK;
        }
        if (is_imm(b)) {
            put(e, 0x06 | (ra << 3));
            field(e, ZF_IMM8, 1);
            return ZE_OK;
        }
        if (a->kind == ZK_R8 && is_idx_ind(b)) {
            put(e, idx_pfx(b));
            put(e, 0x46 | (ra << 3));
            field(e, ZF_DISP, 1);
            return ZE_OK;
        }
        if (a->kind == ZK_R8 && b->kind == ZK_IDXH && ra != 4 && ra != 5) {
            put(e, b->pfx);
            put(e, 0x40 | (ra << 3) | b->reg);
            return ZE_OK;
        }
        if (is_a(a)) {
            switch (b->kind) {
            case ZK_IBC: put(e, 0x0A); return ZE_OK;
            case ZK_IDE: put(e, 0x1A); return ZE_OK;
            case ZK_IABS: put(e, 0x3A); field(e, ZF_IMM16, 1); return ZE_OK;
            case ZK_I: put(e, 0xED); put(e, 0x57); return ZE_OK;
            case ZK_R: put(e, 0xED); put(e, 0x5F); return ZE_OK;
            }
        }
        /* B..L com origem que so' existe para o acumulador: o asMSX avisa que o destino deve ser A */
        if (a->kind == ZK_R8 && (b->kind == ZK_IBC || b->kind == ZK_IDE || b->kind == ZK_IABS || b->kind == ZK_I || b->kind == ZK_R))
            return ZE_DEST_A;
        if (a->kind == ZK_R8 && (a->reg == 4 || a->reg == 5) && b->kind == ZK_IDXH) return ZE_REGCOMB;
        return ZE_SYNTAX;
    }

    /* ld ixh,x */
    if (a->kind == ZK_IDXH) {
        if (is_r8(b) && b->reg != 4 && b->reg != 5) {
            put(e, a->pfx);
            put(e, 0x40 | (a->reg << 3) | b->reg);
            return ZE_OK;
        }
        if (b->kind == ZK_IDXH && b->pfx == a->pfx) {
            put(e, a->pfx);
            put(e, 0x40 | (a->reg << 3) | b->reg);
            return ZE_OK;
        }
        if (is_imm(b)) {
            put(e, a->pfx);
            put(e, 0x06 | (a->reg << 3));
            field(e, ZF_IMM8, 1);
            return ZE_OK;
        }
        if (is_r8(b) && (b->reg == 4 || b->reg == 5)) return ZE_REGCOMB;
        return ZE_SYNTAX;
    }

    /* ld [ix+d],r / ld [ix+d],n */
    if (is_idx_ind(a)) {
        if (is_r8(b)) {
            put(e, idx_pfx(a));
            put(e, 0x70 | b->reg);
            field(e, ZF_DISP, 0);
            return ZE_OK;
        }
        if (is_imm(b)) {
            put(e, idx_pfx(a));
            put(e, 0x36);
            field(e, ZF_DISP, 0);
            field(e, ZF_IMM8, 1);
            return ZE_OK;
        }
        return ZE_SYNTAX;
    }

    switch (a->kind) {
    case ZK_IBC:
        if (is_a(b)) { put(e, 0x02); return ZE_OK; }
        if (is_r8(b)) return ZE_SRC_A;
        return ZE_SYNTAX;
    case ZK_IDE:
        if (is_a(b)) { put(e, 0x12); return ZE_OK; }
        if (is_r8(b)) return ZE_SRC_A;
        return ZE_SYNTAX;
    case ZK_IABS:
        if (is_a(b)) { put(e, 0x32); field(e, ZF_IMM16, 0); return ZE_OK; }
        if (is_r8(b)) return ZE_SRC_A;
        if (b->kind == ZK_RR) {
            if (b->reg == 2) { put(e, 0x22); field(e, ZF_IMM16, 0); return ZE_OK; }
            put(e, 0xED);
            put(e, 0x43 | (b->reg << 4));
            field(e, ZF_IMM16, 0);
            return ZE_OK;
        }
        if (b->kind == ZK_IX || b->kind == ZK_IY) {
            put(e, idx_pfx(b));
            put(e, 0x22);
            field(e, ZF_IMM16, 0);
            return ZE_OK;
        }
        return ZE_SYNTAX;
    case ZK_I:
        if (is_a(b)) { put(e, 0xED); put(e, 0x47); return ZE_OK; }
        if (is_r8(b) || b->kind == ZK_IHL) return ZE_SRC_A;
        return ZE_SYNTAX;
    case ZK_R:
        if (is_a(b)) { put(e, 0xED); put(e, 0x4F); return ZE_OK; }
        if (is_r8(b) || b->kind == ZK_IHL) return ZE_SRC_A;
        return ZE_SYNTAX;
    case ZK_RR:
        if (is_imm(b)) {
            put(e, 0x01 | (a->reg << 4));
            field(e, ZF_IMM16, 1);
            return ZE_OK;
        }
        if (b->kind == ZK_IABS) {
            if (a->reg == 2) { put(e, 0x2A); field(e, ZF_IMM16, 1); return ZE_OK; }
            put(e, 0xED);
            put(e, 0x4B | (a->reg << 4));
            field(e, ZF_IMM16, 1);
            return ZE_OK;
        }
        if (a->reg == 3) { /* ld sp,hl|ix|iy */
            if (is_rr(b, 2)) { put(e, 0xF9); return ZE_OK; }
            if (b->kind == ZK_IX || b->kind == ZK_IY) { put(e, idx_pfx(b)); put(e, 0xF9); return ZE_OK; }
            if (b->kind == ZK_RR) return ZE_REGCOMB;
        }
        return ZE_SYNTAX;
    case ZK_IX:
    case ZK_IY:
        if (is_imm(b)) { put(e, idx_pfx(a)); put(e, 0x21); field(e, ZF_IMM16, 1); return ZE_OK; }
        if (b->kind == ZK_IABS) { put(e, idx_pfx(a)); put(e, 0x2A); field(e, ZF_IMM16, 1); return ZE_OK; }
        return ZE_SYNTAX;
    }
    return ZE_SYNTAX;
}

/* ---------------------------------------------------------------- rotacoes, BIT/RES/SET */

static int rot_base(int mnem) {
    switch (mnem) {
    case M_RLC: return 0x00;
    case M_RRC: return 0x08;
    case M_RL: return 0x10;
    case M_RR: return 0x18;
    case M_SLA: return 0x20;
    case M_SRA: return 0x28;
    case M_SLL: return 0x30;
    case M_SRL: return 0x38;
    }
    return 0;
}

/* Operando de CB: r, [hl] ou [ix+d] (+ opcionalmente a copia para um registrador). `base` ja' inclui o bit. */
static int cb_op(ZEnc *e, int base, const ZOp *tgt, int tgt_index, const ZOp *copy) {
    int r = reg3(tgt);
    if (r >= 0) {
        if (copy) return ZE_SYNTAX;
        put(e, 0xCB);
        put(e, base | r);
        return ZE_OK;
    }
    if (is_idx_ind(tgt)) {
        int low = 6;
        if (copy) {
            if (!is_r8(copy)) return ZE_SYNTAX;
            low = copy->reg;
        }
        put(e, idx_pfx(tgt));
        put(e, 0xCB);
        field(e, ZF_DISP, tgt_index);
        put(e, base | low);
        return ZE_OK;
    }
    return ZE_SYNTAX;
}

/* ---------------------------------------------------------------- operando de salto relativo/absoluto */

static int cond_code(const ZOp *o) { return o->kind == ZK_COND ? o->reg : -1; }

int z80_encode(int mnem, const ZOp *ops, int n, ZEnc *out) {
    const ZOp *a = n > 0 ? &ops[0] : NULL;
    const ZOp *b = n > 1 ? &ops[1] : NULL;
    const ZOp *c = n > 2 ? &ops[2] : NULL;
    ZEnc *e = out;
    int r, v, cc;

    memset(e, 0, sizeof(*e));

    switch (mnem) {
    /* ---- sem operandos ---- */
    case M_NOP: case M_HALT: case M_DI: case M_EI: case M_EXX: case M_DAA: case M_CPL: case M_CCF: case M_SCF:
    case M_RLCA: case M_RRCA: case M_RLA: case M_RRA:
    case M_LDI: case M_LDIR: case M_LDD: case M_LDDR: case M_CPI: case M_CPIR: case M_CPD: case M_CPDR:
    case M_INI: case M_INIR: case M_IND: case M_INDR: case M_OUTI: case M_OTIR: case M_OUTD: case M_OTDR:
    case M_NEG: case M_RLD: case M_RRD: case M_RETI: case M_RETN:
        if (n != 0) return ZE_SYNTAX;
        switch (mnem) {
        case M_NOP: put(e, 0x00); break;
        case M_HALT: put(e, 0x76); break;
        case M_DI: put(e, 0xF3); break;
        case M_EI: put(e, 0xFB); break;
        case M_EXX: put(e, 0xD9); break;
        case M_DAA: put(e, 0x27); break;
        case M_CPL: put(e, 0x2F); break;
        case M_CCF: put(e, 0x3F); break;
        case M_SCF: put(e, 0x37); break;
        case M_RLCA: put(e, 0x07); break;
        case M_RRCA: put(e, 0x0F); break;
        case M_RLA: put(e, 0x17); break;
        case M_RRA: put(e, 0x1F); break;
        case M_LDI: put(e, 0xED); put(e, 0xA0); break;
        case M_LDIR: put(e, 0xED); put(e, 0xB0); break;
        case M_LDD: put(e, 0xED); put(e, 0xA8); break;
        case M_LDDR: put(e, 0xED); put(e, 0xB8); break;
        case M_CPI: put(e, 0xED); put(e, 0xA1); break;
        case M_CPIR: put(e, 0xED); put(e, 0xB1); break;
        case M_CPD: put(e, 0xED); put(e, 0xA9); break;
        case M_CPDR: put(e, 0xED); put(e, 0xB9); break;
        case M_INI: put(e, 0xED); put(e, 0xA2); break;
        case M_INIR: put(e, 0xED); put(e, 0xB2); break;
        case M_IND: put(e, 0xED); put(e, 0xAA); break;
        case M_INDR: put(e, 0xED); put(e, 0xBA); break;
        case M_OUTI: put(e, 0xED); put(e, 0xA3); break;
        case M_OTIR: put(e, 0xED); put(e, 0xB3); break;
        case M_OUTD: put(e, 0xED); put(e, 0xAB); break;
        case M_OTDR: put(e, 0xED); put(e, 0xBB); break;
        case M_NEG: put(e, 0xED); put(e, 0x44); break;
        case M_RLD: put(e, 0xED); put(e, 0x6F); break;
        case M_RRD: put(e, 0xED); put(e, 0x67); break;
        case M_RETI: put(e, 0xED); put(e, 0x4D); break;
        case M_RETN: put(e, 0xED); put(e, 0x45); break;
        }
        return ZE_OK;

    /* ---- LD ---- */
    case M_LD:
        if (n != 2) return ZE_SYNTAX;
        return enc_ld(e, a, b);

    /* ---- aritmeticas de 8 bits ---- */
    case M_ADD: case M_ADC: case M_SBC:
        if (n != 2) return ZE_SYNTAX;
        if (is_a(a)) return alu8(e, alu_index(mnem), b, 1);
        if (a->kind == ZK_RR && a->reg != 2 && b->kind == ZK_RR) return ZE_REGCOMB;
        if (mnem == M_ADD && (a->kind == ZK_IX || a->kind == ZK_IY) && is_rr(b, 2)) return ZE_REGCOMB;
        if (is_rr(a, 2)) { /* hl,rr */
            if (b->kind != ZK_RR) return ZE_SYNTAX;
            if (mnem == M_ADD) put(e, 0x09 | (b->reg << 4));
            else {
                put(e, 0xED);
                put(e, (mnem == M_ADC ? 0x4A : 0x42) | (b->reg << 4));
            }
            return ZE_OK;
        }
        if (mnem == M_ADD && (a->kind == ZK_IX || a->kind == ZK_IY)) {
            int pp;
            if (b->kind == ZK_RR && b->reg != 2) pp = b->reg;
            else if (b->kind == a->kind) pp = 2;
            else return ZE_SYNTAX;
            put(e, idx_pfx(a));
            put(e, 0x09 | (pp << 4));
            return ZE_OK;
        }
        if (is_r8(a)) {
            /* destino que nao e' A: o asMSX avisa que o destino deve ser A quando a origem seria valida */
            if (alu8(e, 0x80, b, 1) == ZE_OK) return ZE_DEST_A;
        }
        return ZE_SYNTAX;
    case M_SUB: case M_AND: case M_XOR: case M_OR: case M_CP:
        if (n != 1) return ZE_SYNTAX;
        return alu8(e, alu_index(mnem), a, 0);

    /* ---- INC / DEC ---- */
    case M_INC: case M_DEC: {
        int dec = mnem == M_DEC;
        if (n != 1) return ZE_SYNTAX;
        r = reg3(a);
        if (r >= 0) { put(e, (dec ? 0x05 : 0x04) | (r << 3)); return ZE_OK; }
        if (a->kind == ZK_IDXH) { put(e, a->pfx); put(e, (dec ? 0x05 : 0x04) | (a->reg << 3)); return ZE_OK; }
        if (is_idx_ind(a)) { put(e, idx_pfx(a)); put(e, dec ? 0x35 : 0x34); field(e, ZF_DISP, 0); return ZE_OK; }
        if (a->kind == ZK_RR) { put(e, (dec ? 0x0B : 0x03) | (a->reg << 4)); return ZE_OK; }
        if (a->kind == ZK_IX || a->kind == ZK_IY) { put(e, idx_pfx(a)); put(e, dec ? 0x2B : 0x23); return ZE_OK; }
        return ZE_SYNTAX;
    }

    /* ---- PUSH / POP ---- */
    case M_PUSH: case M_POP: {
        int base = mnem == M_PUSH ? 0xC5 : 0xC1;
        if (n != 1) return ZE_SYNTAX;
        if (a->kind == ZK_RR && a->reg != 3) { put(e, base | (a->reg << 4)); return ZE_OK; }
        if (a->kind == ZK_RR) return ZE_REGCOMB;
        if (a->kind == ZK_AF) { put(e, base | 0x30); return ZE_OK; }
        if (a->kind == ZK_IX || a->kind == ZK_IY) { put(e, idx_pfx(a)); put(e, base | 0x20); return ZE_OK; }
        return ZE_SYNTAX;
    }

    /* ---- EX ---- */
    case M_EX:
        if (n != 2) return ZE_SYNTAX;
        if (a->kind == ZK_AF && b->kind == ZK_AFP) { put(e, 0x08); return ZE_OK; }
        if ((is_rr(a, 1) && is_rr(b, 2)) || (is_rr(a, 2) && is_rr(b, 1))) { put(e, 0xEB); return ZE_OK; }
        if (a->kind == ZK_ISP) {
            if (is_rr(b, 2)) { put(e, 0xE3); return ZE_OK; }
            if (b->kind == ZK_IX || b->kind == ZK_IY) { put(e, idx_pfx(b)); put(e, 0xE3); return ZE_OK; }
            if (b->kind == ZK_RR) return ZE_REGCOMB;
        }
        if (a->kind == ZK_RR && b->kind == ZK_RR) return ZE_REGCOMB;
        return ZE_SYNTAX;

    /* ---- saltos ---- */
    case M_JP:
        if (n == 1) {
            if (a->kind == ZK_IHL || is_rr(a, 2)) { put(e, 0xE9); return ZE_OK; }
            if (a->kind == ZK_IIX || a->kind == ZK_IIY || a->kind == ZK_IX || a->kind == ZK_IY) { put(e, idx_pfx(a)); put(e, 0xE9); return ZE_OK; }
            if (is_imm(a)) { put(e, 0xC3); field(e, ZF_IMM16, 0); return ZE_OK; }
            if (a->kind == ZK_RR && a->reg != 2) return ZE_REGCOMB;
            return ZE_SYNTAX;
        }
        if (n == 2) {
            cc = cond_code(a);
            if (cc >= 0 && is_imm(b)) { put(e, 0xC2 | (cc << 3)); field(e, ZF_IMM16, 1); return ZE_OK; }
        }
        return ZE_SYNTAX;
    case M_CALL:
        if (n == 1 && is_imm(a)) { put(e, 0xCD); field(e, ZF_IMM16, 0); return ZE_OK; }
        if (n == 2) {
            cc = cond_code(a);
            if (cc >= 0 && is_imm(b)) { put(e, 0xC4 | (cc << 3)); field(e, ZF_IMM16, 1); return ZE_OK; }
        }
        return ZE_SYNTAX;
    case M_JR:
        if (n == 1 && is_imm(a)) { put(e, 0x18); field(e, ZF_REL, 0); return ZE_OK; }
        if (n == 2) {
            cc = cond_code(a);
            if (cc >= 0 && cc <= 3 && is_imm(b)) { put(e, 0x20 | (cc << 3)); field(e, ZF_REL, 1); return ZE_OK; }
            if (cc > 3 && is_imm(b)) return ZE_BADCOND;
        }
        return ZE_SYNTAX;
    case M_DJNZ:
        if (n == 1 && is_imm(a)) { put(e, 0x10); field(e, ZF_REL, 0); return ZE_OK; }
        return ZE_SYNTAX;
    case M_RET:
        if (n == 0) { put(e, 0xC9); return ZE_OK; }
        if (n == 1) {
            cc = cond_code(a);
            if (cc >= 0) { put(e, 0xC0 | (cc << 3)); return ZE_OK; }
            if (is_r8(a)) return ZE_NOCOND;
        }
        return ZE_SYNTAX;
    case M_RST:
        if (n != 1 || !is_imm(a)) return ZE_SYNTAX;
        v = (int)(a->value & 0xFF);
        if ((v & 0xC7) != 0) return ZE_RST;
        put(e, 0xC7 | v);
        field(e, ZF_CHK8, 0);
        return ZE_OK;
    case M_IM:
        /* operandos a mais: o modo invalido e' reportado primeiro; modo valido com sobra e' erro de sintaxe */
        if (n > 1 && is_imm(a)) return (a->value & 0xFF) > 2 ? ZE_IM : ZE_SYNTAX;
        if (n != 1 || !is_imm(a)) return ZE_SYNTAX;
        v = (int)(a->value & 0xFF);
        if (v > 2) return ZE_IM;
        put(e, 0xED);
        put(e, v == 0 ? 0x46 : (v == 1 ? 0x56 : 0x5E));
        field(e, ZF_CHK8, 0);
        return ZE_OK;

    /* ---- entrada e saida ---- */
    case M_IN:
        if (n == 1) { /* in [c] / in [n] / in n */
            if (a->kind == ZK_IC) { put(e, 0xED); put(e, 0x70); return ZE_OK; }
            if (a->kind == ZK_IABS || is_imm(a)) { put(e, 0xDB); field(e, ZF_IMM8, 0); return ZE_OK; }
            return ZE_SYNTAX;
        }
        if (n == 2) {
            if (b->kind == ZK_IC) {
                if (is_r8(a)) { put(e, 0xED); put(e, 0x40 | (a->reg << 3)); return ZE_OK; }
                if (a->kind == ZK_F) { put(e, 0xED); put(e, 0x70); return ZE_OK; }
                return ZE_SYNTAX;
            }
            if (is_a(a) && (b->kind == ZK_IABS || is_imm(b))) { put(e, 0xDB); field(e, ZF_IMM8, 1); return ZE_OK; }
            if (is_r8(a) && (b->kind == ZK_IABS || is_imm(b))) return ZE_DEST_A;
        }
        return ZE_SYNTAX;
    case M_OUT:
        if (n == 1 && a->kind == ZK_IC) { put(e, 0xED); put(e, 0x71); return ZE_OK; }
        if (n == 1 && (a->kind == ZK_IABS || is_imm(a))) { put(e, 0xD3); field(e, ZF_IMM8, 0); return ZE_OK; }
        if (n != 2) return ZE_SYNTAX;
        if (a->kind == ZK_IC) {
            if (is_r8(b)) { put(e, 0xED); put(e, 0x41 | (b->reg << 3)); return ZE_OK; }
            if (is_imm(b)) {
                if ((b->value & 0xFF) != 0) return ZE_ZERO;
                put(e, 0xED);
                put(e, 0x71);
                field(e, ZF_CHK8, 1);
                return ZE_OK;
            }
            return ZE_SYNTAX;
        }
        if (a->kind == ZK_IABS || is_imm(a)) {
            if (is_a(b)) { put(e, 0xD3); field(e, ZF_IMM8, 0); return ZE_OK; }
            if (is_r8(b)) return ZE_SRC_A;
        }
        return ZE_SYNTAX;

    /* ---- CB: rotacoes e deslocamentos ---- */
    case M_RLC: case M_RRC: case M_RL: case M_RR: case M_SLA: case M_SRA: case M_SLL: case M_SRL:
        if (n == 1) return cb_op(e, rot_base(mnem), a, 0, NULL);
        if (n == 2) return cb_op(e, rot_base(mnem), a, 0, b);
        return ZE_SYNTAX;

    /* ---- BIT / RES / SET ---- */
    case M_BIT: case M_RES: case M_SET: {
        int base = mnem == M_BIT ? 0x40 : (mnem == M_RES ? 0x80 : 0xC0);
        if (n < 2 || n > 3 || !is_imm(a)) return ZE_SYNTAX;
        if (n == 3 && mnem == M_BIT) return ZE_SYNTAX;
        if (a->value < 0 || a->value > 7) e->bit_warn = 1;
        v = (int)(a->value & 7);
        r = cb_op(e, base | (v << 3), b, 1, n == 3 ? c : NULL);
        if (r != ZE_OK) e->bit_warn = 0;
        return r;
    }
    }
    return ZE_SYNTAX;
}
