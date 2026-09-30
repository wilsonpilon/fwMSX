// Adaptado de fMSX (resource/fMSX/Z80/Z80.c), Copyright (C) Marat
// Fayzullin 1994-2021. O fwMSX evolui a partir do fMSX com o aval do
// autor original para adaptar/estudar seu codigo (ver README.md) -- isso
// nao e uma relicenciacao: este arquivo continua sob os termos originais
// dele (nao-comercial, aviso ao autor em caso de mudanca), nao o
// BSD-3-Clause do restante do fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// Motor de despacho do Z80: mesma tecnica do original (switch(opcode)
// com os corpos de cada tabela inclusos via #include, dispatchers
// separados por prefixo). Ver doc/z80-core-spec.md, secao 2 e 3.2, para
// o raciocinio completo, e secao 6 (Fase 1 -- Notas de implementacao)
// para os desvios registrados nesta adaptacao.
//
// Modelo de execucao: z80_run() adapta o ExecZ80() do fMSX (execucao
// controlada pelo host, por numero de ciclos), NAO o RunZ80()/LoopZ80()
// (auto-loop de interrupcao periodica) -- ver doc/z80-core-spec.md,
// secao 3.2.

#include "z80_core.h"

#include <stdio.h>

#include "z80_opcodes.h"
#include "z80_tables.h"
#include "../asm/block_ops.h"

// --- Macros auxiliares --------------------------------------------
// Adaptadas dos macros M_*/S/R/FLAGS/INCR de Z80.c. Esperam variaveis
// locais chamadas exatamente `state` (Z80State*), `bus` (const Z80Bus*),
// `I` (uint8_t) e `J` (Z80Pair) em escopo -- mesmo estilo implicito do
// original (R/I/J).

#define Z80_FSET(fl) (state->af.b.lo |= (uint8_t)(fl))
#define Z80_FCLR(fl) (state->af.b.lo &= (uint8_t)~(fl))
#define Z80_INCR(n) (state->r = (uint8_t)(((state->r + (n)) & 0x7F) | (state->r & 0x80)))

#define Z80_RD(addr) (bus->read(bus->ctx, (addr)))
#define Z80_WR(addr, val) (bus->write(bus->ctx, (addr), (val)))
#define Z80_IN(port) (bus->in(bus->ctx, (port)))
#define Z80_OUT(port, val) (bus->out(bus->ctx, (port), (val)))
#define Z80_JUMP(pc) do { if (bus->jump) bus->jump(bus->ctx, (uint16_t)(pc)); } while (0)

#define Z80_M_RLC(reg) do { \
        state->af.b.lo = (uint8_t)((reg) >> 7); \
        (reg) = (uint8_t)(((reg) << 1) | state->af.b.lo); \
        state->af.b.lo |= g_z80_pzs_table[(uint8_t)(reg)]; \
    } while (0)

#define Z80_M_RRC(reg) do { \
        state->af.b.lo = (uint8_t)((reg) & 0x01); \
        (reg) = (uint8_t)(((reg) >> 1) | (state->af.b.lo << 7)); \
        state->af.b.lo |= g_z80_pzs_table[(uint8_t)(reg)]; \
    } while (0)

#define Z80_M_RL(reg) do { \
        if ((reg) & 0x80) { \
            (reg) = (uint8_t)(((reg) << 1) | (state->af.b.lo & Z80_C_FLAG)); \
            state->af.b.lo = (uint8_t)(g_z80_pzs_table[(uint8_t)(reg)] | Z80_C_FLAG); \
        } else { \
            (reg) = (uint8_t)(((reg) << 1) | (state->af.b.lo & Z80_C_FLAG)); \
            state->af.b.lo = g_z80_pzs_table[(uint8_t)(reg)]; \
        } \
    } while (0)

#define Z80_M_RR(reg) do { \
        if ((reg) & 0x01) { \
            (reg) = (uint8_t)(((reg) >> 1) | (state->af.b.lo << 7)); \
            state->af.b.lo = (uint8_t)(g_z80_pzs_table[(uint8_t)(reg)] | Z80_C_FLAG); \
        } else { \
            (reg) = (uint8_t)(((reg) >> 1) | (state->af.b.lo << 7)); \
            state->af.b.lo = g_z80_pzs_table[(uint8_t)(reg)]; \
        } \
    } while (0)

#define Z80_M_SLA(reg) do { \
        state->af.b.lo = (uint8_t)((reg) >> 7); \
        (reg) = (uint8_t)((reg) << 1); \
        state->af.b.lo |= g_z80_pzs_table[(uint8_t)(reg)]; \
    } while (0)

#define Z80_M_SRA(reg) do { \
        state->af.b.lo = (uint8_t)((reg) & Z80_C_FLAG); \
        (reg) = (uint8_t)(((reg) >> 1) | ((reg) & 0x80)); \
        state->af.b.lo |= g_z80_pzs_table[(uint8_t)(reg)]; \
    } while (0)

#define Z80_M_SLL(reg) do { \
        state->af.b.lo = (uint8_t)((reg) >> 7); \
        (reg) = (uint8_t)(((reg) << 1) | 0x01); \
        state->af.b.lo |= g_z80_pzs_table[(uint8_t)(reg)]; \
    } while (0)

#define Z80_M_SRL(reg) do { \
        state->af.b.lo = (uint8_t)((reg) & 0x01); \
        (reg) = (uint8_t)((reg) >> 1); \
        state->af.b.lo |= g_z80_pzs_table[(uint8_t)(reg)]; \
    } while (0)

#define Z80_M_BIT(bit, reg) \
    (state->af.b.lo = (uint8_t)((state->af.b.lo & Z80_C_FLAG) | Z80_H_FLAG | g_z80_pzs_table[(reg) & (1 << (bit))]))

#define Z80_M_SET(bit, reg) ((reg) = (uint8_t)((reg) | (1 << (bit))))
#define Z80_M_RES(bit, reg) ((reg) = (uint8_t)((reg) & (uint8_t)~(1 << (bit))))

#define Z80_M_POP(reg) do { \
        state->reg.b.lo = Z80_RD(state->sp.w++); \
        state->reg.b.hi = Z80_RD(state->sp.w++); \
    } while (0)

#define Z80_M_PUSH(reg) do { \
        Z80_WR(--state->sp.w, state->reg.b.hi); \
        Z80_WR(--state->sp.w, state->reg.b.lo); \
    } while (0)

#define Z80_M_CALL do { \
        J.b.lo = Z80_RD(state->pc.w++); \
        J.b.hi = Z80_RD(state->pc.w++); \
        Z80_WR(--state->sp.w, state->pc.b.hi); \
        Z80_WR(--state->sp.w, state->pc.b.lo); \
        state->pc.w = J.w; \
        Z80_JUMP(J.w); \
    } while (0)

#define Z80_M_JP do { \
        J.b.lo = Z80_RD(state->pc.w++); \
        J.b.hi = Z80_RD(state->pc.w); \
        state->pc.w = J.w; \
        Z80_JUMP(J.w); \
    } while (0)

#define Z80_M_JR do { \
        state->pc.w = (uint16_t)(state->pc.w + (int8_t)Z80_RD(state->pc.w) + 1); \
        Z80_JUMP(state->pc.w); \
    } while (0)

#define Z80_M_RET do { \
        state->pc.b.lo = Z80_RD(state->sp.w++); \
        state->pc.b.hi = Z80_RD(state->sp.w++); \
        Z80_JUMP(state->pc.w); \
    } while (0)

#define Z80_M_RST(addr) do { \
        Z80_WR(--state->sp.w, state->pc.b.hi); \
        Z80_WR(--state->sp.w, state->pc.b.lo); \
        state->pc.w = (addr); \
        Z80_JUMP(addr); \
    } while (0)

#define Z80_M_LDWORD(reg) do { \
        state->reg.b.lo = Z80_RD(state->pc.w++); \
        state->reg.b.hi = Z80_RD(state->pc.w++); \
    } while (0)

#define Z80_M_ADD(reg) do { \
        J.w = (uint16_t)(state->af.b.hi + (reg)); \
        state->af.b.lo = (uint8_t)( \
            ((~(state->af.b.hi ^ (reg)) & ((reg) ^ J.b.lo) & 0x80) ? Z80_V_FLAG : 0) | \
            J.b.hi | g_z80_zs_table[J.b.lo] | \
            ((state->af.b.hi ^ (reg) ^ J.b.lo) & Z80_H_FLAG)); \
        state->af.b.hi = J.b.lo; \
    } while (0)

#define Z80_M_SUB(reg) do { \
        J.w = (uint16_t)(state->af.b.hi - (reg)); \
        state->af.b.lo = (uint8_t)( \
            (((state->af.b.hi ^ (reg)) & (state->af.b.hi ^ J.b.lo) & 0x80) ? Z80_V_FLAG : 0) | \
            Z80_N_FLAG | (uint8_t)(-(int)J.b.hi) | g_z80_zs_table[J.b.lo] | \
            ((state->af.b.hi ^ (reg) ^ J.b.lo) & Z80_H_FLAG)); \
        state->af.b.hi = J.b.lo; \
    } while (0)

#define Z80_M_ADC(reg) do { \
        J.w = (uint16_t)(state->af.b.hi + (reg) + (state->af.b.lo & Z80_C_FLAG)); \
        state->af.b.lo = (uint8_t)( \
            ((~(state->af.b.hi ^ (reg)) & ((reg) ^ J.b.lo) & 0x80) ? Z80_V_FLAG : 0) | \
            J.b.hi | g_z80_zs_table[J.b.lo] | \
            ((state->af.b.hi ^ (reg) ^ J.b.lo) & Z80_H_FLAG)); \
        state->af.b.hi = J.b.lo; \
    } while (0)

#define Z80_M_SBC(reg) do { \
        J.w = (uint16_t)(state->af.b.hi - (reg) - (state->af.b.lo & Z80_C_FLAG)); \
        state->af.b.lo = (uint8_t)( \
            (((state->af.b.hi ^ (reg)) & (state->af.b.hi ^ J.b.lo) & 0x80) ? Z80_V_FLAG : 0) | \
            Z80_N_FLAG | (uint8_t)(-(int)J.b.hi) | g_z80_zs_table[J.b.lo] | \
            ((state->af.b.hi ^ (reg) ^ J.b.lo) & Z80_H_FLAG)); \
        state->af.b.hi = J.b.lo; \
    } while (0)

#define Z80_M_CP(reg) do { \
        J.w = (uint16_t)(state->af.b.hi - (reg)); \
        state->af.b.lo = (uint8_t)( \
            (((state->af.b.hi ^ (reg)) & (state->af.b.hi ^ J.b.lo) & 0x80) ? Z80_V_FLAG : 0) | \
            Z80_N_FLAG | (uint8_t)(-(int)J.b.hi) | g_z80_zs_table[J.b.lo] | \
            ((state->af.b.hi ^ (reg) ^ J.b.lo) & Z80_H_FLAG)); \
    } while (0)

#define Z80_M_AND(reg) do { \
        state->af.b.hi &= (uint8_t)(reg); \
        state->af.b.lo = (uint8_t)(Z80_H_FLAG | g_z80_pzs_table[state->af.b.hi]); \
    } while (0)

#define Z80_M_OR(reg) do { \
        state->af.b.hi |= (uint8_t)(reg); \
        state->af.b.lo = g_z80_pzs_table[state->af.b.hi]; \
    } while (0)

#define Z80_M_XOR(reg) do { \
        state->af.b.hi ^= (uint8_t)(reg); \
        state->af.b.lo = g_z80_pzs_table[state->af.b.hi]; \
    } while (0)

#define Z80_M_IN(reg) do { \
        (reg) = Z80_IN(state->bc.w); \
        state->af.b.lo = (uint8_t)(g_z80_pzs_table[(uint8_t)(reg)] | (state->af.b.lo & Z80_C_FLAG)); \
    } while (0)

#define Z80_M_INC(reg) do { \
        (reg)++; \
        state->af.b.lo = (uint8_t)( \
            (state->af.b.lo & Z80_C_FLAG) | g_z80_zs_table[(uint8_t)(reg)] | \
            ((reg) == 0x80 ? Z80_V_FLAG : 0) | ((reg) & 0x0F ? 0 : Z80_H_FLAG)); \
    } while (0)

#define Z80_M_DEC(reg) do { \
        (reg)--; \
        state->af.b.lo = (uint8_t)( \
            Z80_N_FLAG | (state->af.b.lo & Z80_C_FLAG) | g_z80_zs_table[(uint8_t)(reg)] | \
            ((reg) == 0x7F ? Z80_V_FLAG : 0) | (((reg) & 0x0F) == 0x0F ? Z80_H_FLAG : 0)); \
    } while (0)

#define Z80_M_ADDW(reg1, reg2) do { \
        J.w = (uint16_t)((state->reg1.w + state->reg2.w) & 0xFFFF); \
        state->af.b.lo = (uint8_t)( \
            (state->af.b.lo & (uint8_t)~(Z80_H_FLAG | Z80_N_FLAG | Z80_C_FLAG)) | \
            (((state->reg1.w ^ state->reg2.w ^ J.w) & 0x1000) ? Z80_H_FLAG : 0) | \
            ((((int32_t)state->reg1.w + (int32_t)state->reg2.w) & 0x10000) ? Z80_C_FLAG : 0)); \
        state->reg1.w = J.w; \
    } while (0)

#define Z80_M_ADCW(reg) do { \
        I = (uint8_t)(state->af.b.lo & Z80_C_FLAG); \
        J.w = (uint16_t)((state->hl.w + state->reg.w + I) & 0xFFFF); \
        state->af.b.lo = (uint8_t)( \
            ((((int32_t)state->hl.w + (int32_t)state->reg.w + (int32_t)I) & 0x10000) ? Z80_C_FLAG : 0) | \
            ((~(state->hl.w ^ state->reg.w) & (state->reg.w ^ J.w) & 0x8000) ? Z80_V_FLAG : 0) | \
            (((state->hl.w ^ state->reg.w ^ J.w) & 0x1000) ? Z80_H_FLAG : 0) | \
            (J.w ? 0 : Z80_Z_FLAG) | (J.b.hi & Z80_S_FLAG)); \
        state->hl.w = J.w; \
    } while (0)

#define Z80_M_SBCW(reg) do { \
        I = (uint8_t)(state->af.b.lo & Z80_C_FLAG); \
        J.w = (uint16_t)((state->hl.w - state->reg.w - I) & 0xFFFF); \
        state->af.b.lo = (uint8_t)( \
            Z80_N_FLAG | \
            ((((int32_t)state->hl.w - (int32_t)state->reg.w - (int32_t)I) & 0x10000) ? Z80_C_FLAG : 0) | \
            (((state->hl.w ^ state->reg.w) & (state->hl.w ^ J.w) & 0x8000) ? Z80_V_FLAG : 0) | \
            (((state->hl.w ^ state->reg.w ^ J.w) & 0x1000) ? Z80_H_FLAG : 0) | \
            (J.w ? 0 : Z80_Z_FLAG) | (J.b.hi & Z80_S_FLAG)); \
        state->hl.w = J.w; \
    } while (0)

// --- Sub-dispatchers (um por prefixo, mesma tecnica do fMSX) --------

static void z80_exec_cb(Z80State *state, const Z80Bus *bus) {
    uint8_t I;

    I = Z80_RD(state->pc.w++);
    state->icount -= g_z80_cycles_cb[I];
    Z80_INCR(1);

    switch (I) {
#include "opcodes_cb.h"
    default:
        if (state->trapbadops)
            fprintf(stderr, "[Z80 %p] Unrecognized instruction: CB %02X at PC=%04X\n",
                    state->user_data, Z80_RD((uint16_t)(state->pc.w - 1)), (unsigned)(state->pc.w - 2));
    }
}

static void z80_exec_ddcb(Z80State *state, const Z80Bus *bus) {
    Z80Pair J;
    uint8_t I;

#define Z80_XX ix
    J.w = (uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++));
    I = Z80_RD(state->pc.w++);
    state->icount -= g_z80_cycles_xxcb[I];

    switch (I) {
#include "opcodes_xcb.h"
    default:
        if (state->trapbadops)
            fprintf(stderr, "[Z80 %p] Unrecognized instruction: DD CB %02X %02X at PC=%04X\n",
                    state->user_data, Z80_RD((uint16_t)(state->pc.w - 2)), Z80_RD((uint16_t)(state->pc.w - 1)),
                    (unsigned)(state->pc.w - 4));
    }
#undef Z80_XX
}

static void z80_exec_fdcb(Z80State *state, const Z80Bus *bus) {
    Z80Pair J;
    uint8_t I;

#define Z80_XX iy
    J.w = (uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++));
    I = Z80_RD(state->pc.w++);
    state->icount -= g_z80_cycles_xxcb[I];

    switch (I) {
#include "opcodes_xcb.h"
    default:
        if (state->trapbadops)
            fprintf(stderr, "[Z80 %p] Unrecognized instruction: FD CB %02X %02X at PC=%04X\n",
                    state->user_data, Z80_RD((uint16_t)(state->pc.w - 2)), Z80_RD((uint16_t)(state->pc.w - 1)),
                    (unsigned)(state->pc.w - 4));
    }
#undef Z80_XX
}

static void z80_exec_ed(Z80State *state, const Z80Bus *bus) {
    uint8_t I;
    Z80Pair J;

    I = Z80_RD(state->pc.w++);
    state->icount -= g_z80_cycles_ed[I];
    Z80_INCR(1);

    switch (I) {
#include "opcodes_ed.h"
    case Z80_PFX_ED:
        state->pc.w--;
        break;
    default:
        if (state->trapbadops)
            fprintf(stderr, "[Z80 %p] Unrecognized instruction: ED %02X at PC=%04X\n",
                    state->user_data, Z80_RD((uint16_t)(state->pc.w - 1)), (unsigned)(state->pc.w - 2));
    }
}

static void z80_exec_dd(Z80State *state, const Z80Bus *bus) {
    uint8_t I;
    Z80Pair J;

#define Z80_XX ix
    I = Z80_RD(state->pc.w++);
    state->icount -= g_z80_cycles_xx[I];
    Z80_INCR(1);

    switch (I) {
#include "opcodes_xx.h"
    case Z80_PFX_FD:
    case Z80_PFX_DD:
        state->pc.w--;
        break;
    case Z80_PFX_CB:
        z80_exec_ddcb(state, bus);
        break;
    default:
        if (state->trapbadops)
            fprintf(stderr, "[Z80 %p] Unrecognized instruction: DD %02X at PC=%04X\n",
                    state->user_data, Z80_RD((uint16_t)(state->pc.w - 1)), (unsigned)(state->pc.w - 2));
    }
#undef Z80_XX
}

static void z80_exec_fd(Z80State *state, const Z80Bus *bus) {
    uint8_t I;
    Z80Pair J;

#define Z80_XX iy
    I = Z80_RD(state->pc.w++);
    state->icount -= g_z80_cycles_xx[I];
    Z80_INCR(1);

    switch (I) {
#include "opcodes_xx.h"
    case Z80_PFX_FD:
    case Z80_PFX_DD:
        state->pc.w--;
        break;
    case Z80_PFX_CB:
        z80_exec_fdcb(state, bus);
        break;
    default:
        if (state->trapbadops)
            fprintf(stderr, "[Z80 %p] Unrecognized instruction: FD %02X at PC=%04X\n",
                    state->user_data, Z80_RD((uint16_t)(state->pc.w - 1)), (unsigned)(state->pc.w - 2));
    }
#undef Z80_XX
}

// --- API publica -----------------------------------------------------

void z80_reset(Z80State *state, const Z80Bus *bus) {
    z80_tables_init();

    state->pc.w = 0x0000;
    state->sp.w = 0xF000;
    state->af.w = 0x0000;
    state->bc.w = 0x0000;
    state->de.w = 0x0000;
    state->hl.w = 0x0000;
    state->af_alt.w = 0x0000;
    state->bc_alt.w = 0x0000;
    state->de_alt.w = 0x0000;
    state->hl_alt.w = 0x0000;
    state->ix.w = 0x0000;
    state->iy.w = 0x0000;
    state->i = 0x00;
    state->r = 0x00;
    state->iff = 0x00;
    state->icount = state->iperiod;
    state->irequest = Z80_INT_NONE;
    state->ibackup = 0;

    Z80_JUMP(state->pc.w);
}

int z80_run(Z80State *state, const Z80Bus *bus, int cycles) {
    uint8_t I;
    Z80Pair J;

    for (state->icount = cycles;;) {
        while (state->icount > 0) {
            I = Z80_RD(state->pc.w++);
            state->icount -= g_z80_cycles[I];
            Z80_INCR(1);

            switch (I) {
#include "opcodes_base.h"
            case Z80_PFX_CB: z80_exec_cb(state, bus); break;
            case Z80_PFX_ED: z80_exec_ed(state, bus); break;
            case Z80_PFX_FD: z80_exec_fd(state, bus); break;
            case Z80_PFX_DD: z80_exec_dd(state, bus); break;
            }
        }

        // Sai, a nao ser que tenhamos chegado aqui logo depois de um EI.
        if (!(state->iff & Z80_IFF_EI)) return state->icount;

        state->iff = (uint8_t)((state->iff & (uint8_t)~Z80_IFF_EI) | Z80_IFF_1);
        state->icount += state->ibackup - 1;
        if (state->irequest != Z80_INT_NONE) z80_interrupt(state, bus, state->irequest);
    }
}

void z80_interrupt(Z80State *state, const Z80Bus *bus, uint16_t vector) {
    if (state->iff & Z80_IFF_HALT) {
        state->pc.w++;
        state->iff &= (uint8_t)~Z80_IFF_HALT;
    }

    if ((state->iff & Z80_IFF_1) || vector == Z80_INT_NMI) {
        Z80_WR(--state->sp.w, state->pc.b.hi);
        Z80_WR(--state->sp.w, state->pc.b.lo);

        if (state->iautoreset && vector == state->irequest) state->irequest = Z80_INT_NONE;

        if (vector == Z80_INT_NMI) {
            state->iff &= (uint8_t)~(Z80_IFF_1 | Z80_IFF_EI);
            state->pc.w = 0x0066;
            Z80_JUMP(0x0066);
            return;
        }

        state->iff &= (uint8_t)~(Z80_IFF_1 | Z80_IFF_2 | Z80_IFF_EI);

        if (state->iff & Z80_IFF_IM2) {
            uint16_t vec = (uint16_t)((vector & 0xFF) | ((uint16_t)state->i << 8));
            state->pc.b.lo = Z80_RD(vec++);
            state->pc.b.hi = Z80_RD(vec);
            Z80_JUMP(state->pc.w);
            return;
        }

        if (state->iff & Z80_IFF_IM1) {
            state->pc.w = 0x0038;
            Z80_JUMP(0x0038);
            return;
        }

        switch (vector) {
        case Z80_INT_RST00: state->pc.w = 0x0000; Z80_JUMP(0x0000); break;
        case Z80_INT_RST08: state->pc.w = 0x0008; Z80_JUMP(0x0008); break;
        case Z80_INT_RST10: state->pc.w = 0x0010; Z80_JUMP(0x0010); break;
        case Z80_INT_RST18: state->pc.w = 0x0018; Z80_JUMP(0x0018); break;
        case Z80_INT_RST20: state->pc.w = 0x0020; Z80_JUMP(0x0020); break;
        case Z80_INT_RST28: state->pc.w = 0x0028; Z80_JUMP(0x0028); break;
        case Z80_INT_RST30: state->pc.w = 0x0030; Z80_JUMP(0x0030); break;
        case Z80_INT_RST38: state->pc.w = 0x0038; Z80_JUMP(0x0038); break;
        }
    }
}
