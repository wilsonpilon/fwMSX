// Adaptado de fMSX (resource/fMSX/Z80/CodesED.h), Copyright (C) Marat
// Fayzullin 1994-2021. O fwMSX evolui a partir do fMSX com o aval do
// autor original para adaptar/estudar seu codigo (ver README.md) -- isso
// nao e uma relicenciacao: este arquivo continua sob os termos originais
// dele (nao-comercial, aviso ao autor em caso de mudanca), nao o
// BSD-3-Clause do restante do fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// Corpo dos opcodes prefixados por ED -- incluso dentro do switch(I) de
// z80_exec_ed() em z80_core.c. Ver opcodes_base.h para a nota completa
// sobre as variaveis locais esperadas (`state`, `bus`, `I`, `J`).

// Patch especial para emular chamadas de BIOS (ED FE) -- ver z80_bus.h.
case Z80_DB_FE: if (bus->patch) bus->patch(bus->ctx, state); break;

case Z80_ADC_HL_BC: Z80_M_ADCW(bc); break;
case Z80_ADC_HL_DE: Z80_M_ADCW(de); break;
case Z80_ADC_HL_HL: Z80_M_ADCW(hl); break;
case Z80_ADC_HL_SP: Z80_M_ADCW(sp); break;

case Z80_SBC_HL_BC: Z80_M_SBCW(bc); break;
case Z80_SBC_HL_DE: Z80_M_SBCW(de); break;
case Z80_SBC_HL_HL: Z80_M_SBCW(hl); break;
case Z80_SBC_HL_SP: Z80_M_SBCW(sp); break;

case Z80_LD_xWORDe_HL:
    J.b.lo = Z80_RD(state->pc.w++);
    J.b.hi = Z80_RD(state->pc.w++);
    Z80_WR(J.w++, state->hl.b.lo);
    Z80_WR(J.w, state->hl.b.hi);
    break;
case Z80_LD_xWORDe_DE:
    J.b.lo = Z80_RD(state->pc.w++);
    J.b.hi = Z80_RD(state->pc.w++);
    Z80_WR(J.w++, state->de.b.lo);
    Z80_WR(J.w, state->de.b.hi);
    break;
case Z80_LD_xWORDe_BC:
    J.b.lo = Z80_RD(state->pc.w++);
    J.b.hi = Z80_RD(state->pc.w++);
    Z80_WR(J.w++, state->bc.b.lo);
    Z80_WR(J.w, state->bc.b.hi);
    break;
case Z80_LD_xWORDe_SP:
    J.b.lo = Z80_RD(state->pc.w++);
    J.b.hi = Z80_RD(state->pc.w++);
    Z80_WR(J.w++, state->sp.b.lo);
    Z80_WR(J.w, state->sp.b.hi);
    break;

case Z80_LD_HL_xWORDe:
    J.b.lo = Z80_RD(state->pc.w++);
    J.b.hi = Z80_RD(state->pc.w++);
    state->hl.b.lo = Z80_RD(J.w++);
    state->hl.b.hi = Z80_RD(J.w);
    break;
case Z80_LD_DE_xWORDe:
    J.b.lo = Z80_RD(state->pc.w++);
    J.b.hi = Z80_RD(state->pc.w++);
    state->de.b.lo = Z80_RD(J.w++);
    state->de.b.hi = Z80_RD(J.w);
    break;
case Z80_LD_BC_xWORDe:
    J.b.lo = Z80_RD(state->pc.w++);
    J.b.hi = Z80_RD(state->pc.w++);
    state->bc.b.lo = Z80_RD(J.w++);
    state->bc.b.hi = Z80_RD(J.w);
    break;
case Z80_LD_SP_xWORDe:
    J.b.lo = Z80_RD(state->pc.w++);
    J.b.hi = Z80_RD(state->pc.w++);
    state->sp.b.lo = Z80_RD(J.w++);
    state->sp.b.hi = Z80_RD(J.w);
    break;

case Z80_RRD:
    I = Z80_RD(state->hl.w);
    J.b.lo = (uint8_t)((I >> 4) | (state->af.b.hi << 4));
    Z80_WR(state->hl.w, J.b.lo);
    state->af.b.hi = (uint8_t)((I & 0x0F) | (state->af.b.hi & 0xF0));
    state->af.b.lo = (uint8_t)(g_z80_pzs_table[state->af.b.hi] | (state->af.b.lo & Z80_C_FLAG));
    break;
case Z80_RLD:
    I = Z80_RD(state->hl.w);
    J.b.lo = (uint8_t)((I << 4) | (state->af.b.hi & 0x0F));
    Z80_WR(state->hl.w, J.b.lo);
    state->af.b.hi = (uint8_t)((I >> 4) | (state->af.b.hi & 0xF0));
    state->af.b.lo = (uint8_t)(g_z80_pzs_table[state->af.b.hi] | (state->af.b.lo & Z80_C_FLAG));
    break;

case Z80_LD_A_I:
    state->af.b.hi = state->i;
    state->af.b.lo = (uint8_t)((state->af.b.lo & Z80_C_FLAG) | (state->iff & Z80_IFF_2 ? Z80_P_FLAG : 0) | g_z80_zs_table[state->af.b.hi]);
    break;

case Z80_LD_A_R:
    state->af.b.hi = state->r;
    state->af.b.lo = (uint8_t)((state->af.b.lo & Z80_C_FLAG) | (state->iff & Z80_IFF_2 ? Z80_P_FLAG : 0) | g_z80_zs_table[state->af.b.hi]);
    break;

case Z80_LD_I_A: state->i = state->af.b.hi; break;
case Z80_LD_R_A: state->r = state->af.b.hi; break;

case Z80_IM_0: state->iff &= (uint8_t)~(Z80_IFF_IM1 | Z80_IFF_IM2); break;
case Z80_IM_1: state->iff = (uint8_t)((state->iff & (uint8_t)~Z80_IFF_IM2) | Z80_IFF_IM1); break;
case Z80_IM_2: state->iff = (uint8_t)((state->iff & (uint8_t)~Z80_IFF_IM1) | Z80_IFF_IM2); break;

case Z80_RETI:
case Z80_RETN:
    if (state->iff & Z80_IFF_2) state->iff |= Z80_IFF_1; else state->iff &= (uint8_t)~Z80_IFF_1;
    Z80_M_RET;
    break;

case Z80_NEG: I = state->af.b.hi; state->af.b.hi = 0; Z80_M_SUB(I); break;

case Z80_IN_B_xC: Z80_M_IN(state->bc.b.hi); break;
case Z80_IN_C_xC: Z80_M_IN(state->bc.b.lo); break;
case Z80_IN_D_xC: Z80_M_IN(state->de.b.hi); break;
case Z80_IN_E_xC: Z80_M_IN(state->de.b.lo); break;
case Z80_IN_H_xC: Z80_M_IN(state->hl.b.hi); break;
case Z80_IN_L_xC: Z80_M_IN(state->hl.b.lo); break;
case Z80_IN_A_xC: Z80_M_IN(state->af.b.hi); break;
case Z80_IN_F_xC: Z80_M_IN(J.b.lo); break;

case Z80_OUT_xC_B: Z80_OUT(state->bc.w, state->bc.b.hi); break;
case Z80_OUT_xC_C: Z80_OUT(state->bc.w, state->bc.b.lo); break;
case Z80_OUT_xC_D: Z80_OUT(state->bc.w, state->de.b.hi); break;
case Z80_OUT_xC_E: Z80_OUT(state->bc.w, state->de.b.lo); break;
case Z80_OUT_xC_H: Z80_OUT(state->bc.w, state->hl.b.hi); break;
case Z80_OUT_xC_L: Z80_OUT(state->bc.w, state->hl.b.lo); break;
case Z80_OUT_xC_A: Z80_OUT(state->bc.w, state->af.b.hi); break;
case Z80_OUT_xC_F: Z80_OUT(state->bc.w, 0); break;

case Z80_INI:
    Z80_WR(state->hl.w++, Z80_IN(state->bc.w));
    --state->bc.b.hi;
    state->af.b.lo = (uint8_t)(Z80_N_FLAG | (state->bc.b.hi ? 0 : Z80_Z_FLAG));
    break;

case Z80_INIR:
    Z80_WR(state->hl.w++, Z80_IN(state->bc.w));
    if (--state->bc.b.hi) { state->af.b.lo = Z80_N_FLAG; state->icount -= 21; state->pc.w -= 2; }
    else                  { state->af.b.lo = Z80_Z_FLAG | Z80_N_FLAG; state->icount -= 16; }
    break;

case Z80_IND:
    Z80_WR(state->hl.w--, Z80_IN(state->bc.w));
    --state->bc.b.hi;
    state->af.b.lo = (uint8_t)(Z80_N_FLAG | (state->bc.b.hi ? 0 : Z80_Z_FLAG));
    break;

case Z80_INDR:
    Z80_WR(state->hl.w--, Z80_IN(state->bc.w));
    if (!--state->bc.b.hi) { state->af.b.lo = Z80_N_FLAG; state->icount -= 21; state->pc.w -= 2; }
    else                   { state->af.b.lo = Z80_Z_FLAG | Z80_N_FLAG; state->icount -= 16; }
    break;

case Z80_OUTI:
    --state->bc.b.hi;
    I = Z80_RD(state->hl.w++);
    Z80_OUT(state->bc.w, I);
    state->af.b.lo = (uint8_t)(Z80_N_FLAG | (state->bc.b.hi ? 0 : Z80_Z_FLAG) | ((state->hl.b.lo + I > 255) ? (Z80_C_FLAG | Z80_H_FLAG) : 0));
    break;

case Z80_OTIR:
    --state->bc.b.hi;
    I = Z80_RD(state->hl.w++);
    Z80_OUT(state->bc.w, I);
    if (state->bc.b.hi) {
        state->af.b.lo = (uint8_t)(Z80_N_FLAG | ((state->hl.b.lo + I > 255) ? (Z80_C_FLAG | Z80_H_FLAG) : 0));
        state->icount -= 21;
        state->pc.w -= 2;
    } else {
        state->af.b.lo = (uint8_t)(Z80_Z_FLAG | Z80_N_FLAG | ((state->hl.b.lo + I > 255) ? (Z80_C_FLAG | Z80_H_FLAG) : 0));
        state->icount -= 16;
    }
    break;

case Z80_OUTD:
    --state->bc.b.hi;
    I = Z80_RD(state->hl.w--);
    Z80_OUT(state->bc.w, I);
    state->af.b.lo = (uint8_t)(Z80_N_FLAG | (state->bc.b.hi ? 0 : Z80_Z_FLAG) | ((state->hl.b.lo + I > 255) ? (Z80_C_FLAG | Z80_H_FLAG) : 0));
    break;

case Z80_OTDR:
    --state->bc.b.hi;
    I = Z80_RD(state->hl.w--);
    Z80_OUT(state->bc.w, I);
    if (state->bc.b.hi) {
        state->af.b.lo = (uint8_t)(Z80_N_FLAG | ((state->hl.b.lo + I > 255) ? (Z80_C_FLAG | Z80_H_FLAG) : 0));
        state->icount -= 21;
        state->pc.w -= 2;
    } else {
        state->af.b.lo = (uint8_t)(Z80_Z_FLAG | Z80_N_FLAG | ((state->hl.b.lo + I > 255) ? (Z80_C_FLAG | Z80_H_FLAG) : 0));
        state->icount -= 16;
    }
    break;

case Z80_LDI:
    Z80_WR(state->de.w++, Z80_RD(state->hl.w++));
    --state->bc.w;
    state->af.b.lo = (uint8_t)((state->af.b.lo & (uint8_t)~(Z80_N_FLAG | Z80_H_FLAG | Z80_P_FLAG)) | (state->bc.w ? Z80_P_FLAG : 0));
    break;

case Z80_LDIR:
    Z80_WR(state->de.w++, Z80_RD(state->hl.w++));
    if (--state->bc.w) {
        state->af.b.lo = (uint8_t)((state->af.b.lo & (uint8_t)~(Z80_H_FLAG | Z80_P_FLAG)) | Z80_N_FLAG);
        state->icount -= 21;
        state->pc.w -= 2;
    } else {
        state->af.b.lo &= (uint8_t)~(Z80_N_FLAG | Z80_H_FLAG | Z80_P_FLAG);
        state->icount -= 16;
    }
    break;

case Z80_LDD:
    Z80_WR(state->de.w--, Z80_RD(state->hl.w--));
    --state->bc.w;
    state->af.b.lo = (uint8_t)((state->af.b.lo & (uint8_t)~(Z80_N_FLAG | Z80_H_FLAG | Z80_P_FLAG)) | (state->bc.w ? Z80_P_FLAG : 0));
    break;

case Z80_LDDR:
    Z80_WR(state->de.w--, Z80_RD(state->hl.w--));
    state->af.b.lo &= (uint8_t)~(Z80_N_FLAG | Z80_H_FLAG | Z80_P_FLAG);
    if (--state->bc.w) {
        state->af.b.lo = (uint8_t)((state->af.b.lo & (uint8_t)~(Z80_H_FLAG | Z80_P_FLAG)) | Z80_N_FLAG);
        state->icount -= 21;
        state->pc.w -= 2;
    } else {
        state->af.b.lo &= (uint8_t)~(Z80_N_FLAG | Z80_H_FLAG | Z80_P_FLAG);
        state->icount -= 16;
    }
    break;

case Z80_CPI:
    I = Z80_RD(state->hl.w++);
    J.b.lo = (uint8_t)(state->af.b.hi - I);
    --state->bc.w;
    state->af.b.lo = (uint8_t)(
        Z80_N_FLAG | (state->af.b.lo & Z80_C_FLAG) | g_z80_zs_table[J.b.lo] |
        ((state->af.b.hi ^ I ^ J.b.lo) & Z80_H_FLAG) | (state->bc.w ? Z80_P_FLAG : 0));
    break;

case Z80_CPIR:
    I = Z80_RD(state->hl.w++);
    J.b.lo = (uint8_t)(state->af.b.hi - I);
    if (--state->bc.w && J.b.lo) { state->icount -= 21; state->pc.w -= 2; } else state->icount -= 16;
    state->af.b.lo = (uint8_t)(
        Z80_N_FLAG | (state->af.b.lo & Z80_C_FLAG) | g_z80_zs_table[J.b.lo] |
        ((state->af.b.hi ^ I ^ J.b.lo) & Z80_H_FLAG) | (state->bc.w ? Z80_P_FLAG : 0));
    break;

case Z80_CPD:
    I = Z80_RD(state->hl.w--);
    J.b.lo = (uint8_t)(state->af.b.hi - I);
    --state->bc.w;
    state->af.b.lo = (uint8_t)(
        Z80_N_FLAG | (state->af.b.lo & Z80_C_FLAG) | g_z80_zs_table[J.b.lo] |
        ((state->af.b.hi ^ I ^ J.b.lo) & Z80_H_FLAG) | (state->bc.w ? Z80_P_FLAG : 0));
    break;

case Z80_CPDR:
    I = Z80_RD(state->hl.w--);
    J.b.lo = (uint8_t)(state->af.b.hi - I);
    if (--state->bc.w && J.b.lo) { state->icount -= 21; state->pc.w -= 2; } else state->icount -= 16;
    state->af.b.lo = (uint8_t)(
        Z80_N_FLAG | (state->af.b.lo & Z80_C_FLAG) | g_z80_zs_table[J.b.lo] |
        ((state->af.b.hi ^ I ^ J.b.lo) & Z80_H_FLAG) | (state->bc.w ? Z80_P_FLAG : 0));
    break;
