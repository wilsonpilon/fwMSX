// Adaptado de fMSX (resource/fMSX/Z80/CodesXX.h), Copyright (C) Marat
// Fayzullin 1994-2021. O fwMSX evolui a partir do fMSX com o aval do
// autor original para adaptar/estudar seu codigo (ver README.md) -- isso
// nao e uma relicenciacao: este arquivo continua sob os termos originais
// dele (nao-comercial, aviso ao autor em caso de mudanca), nao o
// BSD-3-Clause do restante do fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// Corpo dos opcodes prefixados por DD/FD (IX/IY) -- incluso dentro do
// switch(I) de z80_exec_dd()/z80_exec_fd() em z80_core.c, que definem a
// macro Z80_XX como `ix` ou `iy` antes do #include (mesma tecnica do
// CodesXX.h original com sua macro XX). Ver opcodes_base.h para a nota
// completa sobre `state`/`bus`/`I`/`J`.
//
// ATENCAO -- fidelidade a um quirk real do hardware Z80, preservado de
// proposito: em "LD H,(IX+d)"/"LD L,(IX+d)" e "LD (IX+d),H"/"LD (IX+d),L"
// o operando de registrador e' sempre o H/L REAL (state->hl), nunca a
// metade alta/baixa de IX/IY -- exatamente como no hardware e no fMSX.
// Nao "corrija" isso para state->Z80_XX.b.hi/lo, seria uma regressao de
// fidelidade.

case Z80_JR_NZ: if (state->af.b.lo & Z80_Z_FLAG) state->pc.w++; else { state->icount -= 5; Z80_M_JR; } break;
case Z80_JR_NC: if (state->af.b.lo & Z80_C_FLAG) state->pc.w++; else { state->icount -= 5; Z80_M_JR; } break;
case Z80_JR_Z:  if (state->af.b.lo & Z80_Z_FLAG) { state->icount -= 5; Z80_M_JR; } else state->pc.w++; break;
case Z80_JR_C:  if (state->af.b.lo & Z80_C_FLAG) { state->icount -= 5; Z80_M_JR; } else state->pc.w++; break;

case Z80_JP_NZ: if (state->af.b.lo & Z80_Z_FLAG) state->pc.w += 2; else { Z80_M_JP; } break;
case Z80_JP_NC: if (state->af.b.lo & Z80_C_FLAG) state->pc.w += 2; else { Z80_M_JP; } break;
case Z80_JP_PO: if (state->af.b.lo & Z80_P_FLAG) state->pc.w += 2; else { Z80_M_JP; } break;
case Z80_JP_P:  if (state->af.b.lo & Z80_S_FLAG) state->pc.w += 2; else { Z80_M_JP; } break;
case Z80_JP_Z:  if (state->af.b.lo & Z80_Z_FLAG) { Z80_M_JP; } else state->pc.w += 2; break;
case Z80_JP_C:  if (state->af.b.lo & Z80_C_FLAG) { Z80_M_JP; } else state->pc.w += 2; break;
case Z80_JP_PE: if (state->af.b.lo & Z80_P_FLAG) { Z80_M_JP; } else state->pc.w += 2; break;
case Z80_JP_M:  if (state->af.b.lo & Z80_S_FLAG) { Z80_M_JP; } else state->pc.w += 2; break;

case Z80_RET_NZ: if (!(state->af.b.lo & Z80_Z_FLAG)) { state->icount -= 6; Z80_M_RET; } break;
case Z80_RET_NC: if (!(state->af.b.lo & Z80_C_FLAG)) { state->icount -= 6; Z80_M_RET; } break;
case Z80_RET_PO: if (!(state->af.b.lo & Z80_P_FLAG)) { state->icount -= 6; Z80_M_RET; } break;
case Z80_RET_P:  if (!(state->af.b.lo & Z80_S_FLAG)) { state->icount -= 6; Z80_M_RET; } break;
case Z80_RET_Z:  if (state->af.b.lo & Z80_Z_FLAG)    { state->icount -= 6; Z80_M_RET; } break;
case Z80_RET_C:  if (state->af.b.lo & Z80_C_FLAG)    { state->icount -= 6; Z80_M_RET; } break;
case Z80_RET_PE: if (state->af.b.lo & Z80_P_FLAG)    { state->icount -= 6; Z80_M_RET; } break;
case Z80_RET_M:  if (state->af.b.lo & Z80_S_FLAG)    { state->icount -= 6; Z80_M_RET; } break;

case Z80_CALL_NZ: if (state->af.b.lo & Z80_Z_FLAG) state->pc.w += 2; else { state->icount -= 7; Z80_M_CALL; } break;
case Z80_CALL_NC: if (state->af.b.lo & Z80_C_FLAG) state->pc.w += 2; else { state->icount -= 7; Z80_M_CALL; } break;
case Z80_CALL_PO: if (state->af.b.lo & Z80_P_FLAG) state->pc.w += 2; else { state->icount -= 7; Z80_M_CALL; } break;
case Z80_CALL_P:  if (state->af.b.lo & Z80_S_FLAG) state->pc.w += 2; else { state->icount -= 7; Z80_M_CALL; } break;
case Z80_CALL_Z:  if (state->af.b.lo & Z80_Z_FLAG) { state->icount -= 7; Z80_M_CALL; } else state->pc.w += 2; break;
case Z80_CALL_C:  if (state->af.b.lo & Z80_C_FLAG) { state->icount -= 7; Z80_M_CALL; } else state->pc.w += 2; break;
case Z80_CALL_PE: if (state->af.b.lo & Z80_P_FLAG) { state->icount -= 7; Z80_M_CALL; } else state->pc.w += 2; break;
case Z80_CALL_M:  if (state->af.b.lo & Z80_S_FLAG) { state->icount -= 7; Z80_M_CALL; } else state->pc.w += 2; break;

case Z80_ADD_B:    Z80_M_ADD(state->bc.b.hi); break;
case Z80_ADD_C:    Z80_M_ADD(state->bc.b.lo); break;
case Z80_ADD_D:    Z80_M_ADD(state->de.b.hi); break;
case Z80_ADD_E:    Z80_M_ADD(state->de.b.lo); break;
case Z80_ADD_H:    Z80_M_ADD(state->Z80_XX.b.hi); break;
case Z80_ADD_L:    Z80_M_ADD(state->Z80_XX.b.lo); break;
case Z80_ADD_A:    Z80_M_ADD(state->af.b.hi); break;
case Z80_ADD_xHL:  I = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++)));
                   Z80_M_ADD(I); break;
case Z80_ADD_BYTE: I = Z80_RD(state->pc.w++); Z80_M_ADD(I); break;

case Z80_SUB_B:    Z80_M_SUB(state->bc.b.hi); break;
case Z80_SUB_C:    Z80_M_SUB(state->bc.b.lo); break;
case Z80_SUB_D:    Z80_M_SUB(state->de.b.hi); break;
case Z80_SUB_E:    Z80_M_SUB(state->de.b.lo); break;
case Z80_SUB_H:    Z80_M_SUB(state->Z80_XX.b.hi); break;
case Z80_SUB_L:    Z80_M_SUB(state->Z80_XX.b.lo); break;
case Z80_SUB_A:    state->af.b.hi = 0; state->af.b.lo = Z80_N_FLAG | Z80_Z_FLAG; break;
case Z80_SUB_xHL:  I = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++)));
                   Z80_M_SUB(I); break;
case Z80_SUB_BYTE: I = Z80_RD(state->pc.w++); Z80_M_SUB(I); break;

case Z80_AND_B:    Z80_M_AND(state->bc.b.hi); break;
case Z80_AND_C:    Z80_M_AND(state->bc.b.lo); break;
case Z80_AND_D:    Z80_M_AND(state->de.b.hi); break;
case Z80_AND_E:    Z80_M_AND(state->de.b.lo); break;
case Z80_AND_H:    Z80_M_AND(state->Z80_XX.b.hi); break;
case Z80_AND_L:    Z80_M_AND(state->Z80_XX.b.lo); break;
case Z80_AND_A:    Z80_M_AND(state->af.b.hi); break;
case Z80_AND_xHL:  I = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++)));
                   Z80_M_AND(I); break;
case Z80_AND_BYTE: I = Z80_RD(state->pc.w++); Z80_M_AND(I); break;

case Z80_OR_B:     Z80_M_OR(state->bc.b.hi); break;
case Z80_OR_C:     Z80_M_OR(state->bc.b.lo); break;
case Z80_OR_D:     Z80_M_OR(state->de.b.hi); break;
case Z80_OR_E:     Z80_M_OR(state->de.b.lo); break;
case Z80_OR_H:     Z80_M_OR(state->Z80_XX.b.hi); break;
case Z80_OR_L:     Z80_M_OR(state->Z80_XX.b.lo); break;
case Z80_OR_A:     Z80_M_OR(state->af.b.hi); break;
case Z80_OR_xHL:   I = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++)));
                   Z80_M_OR(I); break;
case Z80_OR_BYTE:  I = Z80_RD(state->pc.w++); Z80_M_OR(I); break;

case Z80_ADC_B:    Z80_M_ADC(state->bc.b.hi); break;
case Z80_ADC_C:    Z80_M_ADC(state->bc.b.lo); break;
case Z80_ADC_D:    Z80_M_ADC(state->de.b.hi); break;
case Z80_ADC_E:    Z80_M_ADC(state->de.b.lo); break;
case Z80_ADC_H:    Z80_M_ADC(state->Z80_XX.b.hi); break;
case Z80_ADC_L:    Z80_M_ADC(state->Z80_XX.b.lo); break;
case Z80_ADC_A:    Z80_M_ADC(state->af.b.hi); break;
case Z80_ADC_xHL:  I = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++)));
                   Z80_M_ADC(I); break;
case Z80_ADC_BYTE: I = Z80_RD(state->pc.w++); Z80_M_ADC(I); break;

case Z80_SBC_B:    Z80_M_SBC(state->bc.b.hi); break;
case Z80_SBC_C:    Z80_M_SBC(state->bc.b.lo); break;
case Z80_SBC_D:    Z80_M_SBC(state->de.b.hi); break;
case Z80_SBC_E:    Z80_M_SBC(state->de.b.lo); break;
case Z80_SBC_H:    Z80_M_SBC(state->Z80_XX.b.hi); break;
case Z80_SBC_L:    Z80_M_SBC(state->Z80_XX.b.lo); break;
case Z80_SBC_A:    Z80_M_SBC(state->af.b.hi); break;
case Z80_SBC_xHL:  I = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++)));
                   Z80_M_SBC(I); break;
case Z80_SBC_BYTE: I = Z80_RD(state->pc.w++); Z80_M_SBC(I); break;

case Z80_XOR_B:    Z80_M_XOR(state->bc.b.hi); break;
case Z80_XOR_C:    Z80_M_XOR(state->bc.b.lo); break;
case Z80_XOR_D:    Z80_M_XOR(state->de.b.hi); break;
case Z80_XOR_E:    Z80_M_XOR(state->de.b.lo); break;
case Z80_XOR_H:    Z80_M_XOR(state->Z80_XX.b.hi); break;
case Z80_XOR_L:    Z80_M_XOR(state->Z80_XX.b.lo); break;
case Z80_XOR_A:    state->af.b.hi = 0; state->af.b.lo = Z80_P_FLAG | Z80_Z_FLAG; break;
case Z80_XOR_xHL:  I = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++)));
                   Z80_M_XOR(I); break;
case Z80_XOR_BYTE: I = Z80_RD(state->pc.w++); Z80_M_XOR(I); break;

case Z80_CP_B:     Z80_M_CP(state->bc.b.hi); break;
case Z80_CP_C:     Z80_M_CP(state->bc.b.lo); break;
case Z80_CP_D:     Z80_M_CP(state->de.b.hi); break;
case Z80_CP_E:     Z80_M_CP(state->de.b.lo); break;
case Z80_CP_H:     Z80_M_CP(state->Z80_XX.b.hi); break;
case Z80_CP_L:     Z80_M_CP(state->Z80_XX.b.lo); break;
case Z80_CP_A:     state->af.b.lo = Z80_N_FLAG | Z80_Z_FLAG; break;
case Z80_CP_xHL:   I = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++)));
                   Z80_M_CP(I); break;
case Z80_CP_BYTE:  I = Z80_RD(state->pc.w++); Z80_M_CP(I); break;

case Z80_LD_BC_WORD: Z80_M_LDWORD(bc); break;
case Z80_LD_DE_WORD: Z80_M_LDWORD(de); break;
case Z80_LD_HL_WORD: Z80_M_LDWORD(Z80_XX); break;
case Z80_LD_SP_WORD: Z80_M_LDWORD(sp); break;

case Z80_LD_PC_HL: state->pc.w = state->Z80_XX.w; Z80_JUMP(state->pc.w); break;
case Z80_LD_SP_HL: state->sp.w = state->Z80_XX.w; break;
case Z80_LD_A_xBC: state->af.b.hi = Z80_RD(state->bc.w); break;
case Z80_LD_A_xDE: state->af.b.hi = Z80_RD(state->de.w); break;

case Z80_ADD_HL_BC: Z80_M_ADDW(Z80_XX, bc); break;
case Z80_ADD_HL_DE: Z80_M_ADDW(Z80_XX, de); break;
case Z80_ADD_HL_HL: Z80_M_ADDW(Z80_XX, Z80_XX); break;
case Z80_ADD_HL_SP: Z80_M_ADDW(Z80_XX, sp); break;

case Z80_DEC_BC: state->bc.w--; break;
case Z80_DEC_DE: state->de.w--; break;
case Z80_DEC_HL: state->Z80_XX.w--; break;
case Z80_DEC_SP: state->sp.w--; break;

case Z80_INC_BC: state->bc.w++; break;
case Z80_INC_DE: state->de.w++; break;
case Z80_INC_HL: state->Z80_XX.w++; break;
case Z80_INC_SP: state->sp.w++; break;

case Z80_DEC_B: Z80_M_DEC(state->bc.b.hi); break;
case Z80_DEC_C: Z80_M_DEC(state->bc.b.lo); break;
case Z80_DEC_D: Z80_M_DEC(state->de.b.hi); break;
case Z80_DEC_E: Z80_M_DEC(state->de.b.lo); break;
case Z80_DEC_H: Z80_M_DEC(state->Z80_XX.b.hi); break;
case Z80_DEC_L: Z80_M_DEC(state->Z80_XX.b.lo); break;
case Z80_DEC_A: Z80_M_DEC(state->af.b.hi); break;
// Fiel ao original: le o byte de deslocamento duas vezes (uma sem
// avancar PC, para o endereco de leitura; outra avancando PC, para o
// endereco de escrita) -- o valor e' o mesmo (memoria nao muda entre as
// duas leituras), mas PC so avanca na segunda. Nao "otimizar" cacheando
// o deslocamento, para manter o mesmo numero/ordem de acessos ao bus.
case Z80_DEC_xHL:
    I = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w)));
    Z80_M_DEC(I);
    Z80_WR((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++)), I);
    break;

case Z80_INC_B: Z80_M_INC(state->bc.b.hi); break;
case Z80_INC_C: Z80_M_INC(state->bc.b.lo); break;
case Z80_INC_D: Z80_M_INC(state->de.b.hi); break;
case Z80_INC_E: Z80_M_INC(state->de.b.lo); break;
case Z80_INC_H: Z80_M_INC(state->Z80_XX.b.hi); break;
case Z80_INC_L: Z80_M_INC(state->Z80_XX.b.lo); break;
case Z80_INC_A: Z80_M_INC(state->af.b.hi); break;
// Mesmo padrao de acesso duplo ao deslocamento do DEC_xHL acima.
case Z80_INC_xHL:
    I = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w)));
    Z80_M_INC(I);
    Z80_WR((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++)), I);
    break;

case Z80_RLCA:
    I = (uint8_t)(state->af.b.hi & 0x80 ? Z80_C_FLAG : 0);
    state->af.b.hi = (uint8_t)((state->af.b.hi << 1) | I);
    state->af.b.lo = (uint8_t)((state->af.b.lo & (uint8_t)~(Z80_C_FLAG | Z80_N_FLAG | Z80_H_FLAG)) | I);
    break;
case Z80_RLA:
    I = (uint8_t)(state->af.b.hi & 0x80 ? Z80_C_FLAG : 0);
    state->af.b.hi = (uint8_t)((state->af.b.hi << 1) | (state->af.b.lo & Z80_C_FLAG));
    state->af.b.lo = (uint8_t)((state->af.b.lo & (uint8_t)~(Z80_C_FLAG | Z80_N_FLAG | Z80_H_FLAG)) | I);
    break;
case Z80_RRCA:
    I = state->af.b.hi & 0x01;
    state->af.b.hi = (uint8_t)((state->af.b.hi >> 1) | (I ? 0x80 : 0));
    state->af.b.lo = (uint8_t)((state->af.b.lo & (uint8_t)~(Z80_C_FLAG | Z80_N_FLAG | Z80_H_FLAG)) | I);
    break;
case Z80_RRA:
    I = state->af.b.hi & 0x01;
    state->af.b.hi = (uint8_t)((state->af.b.hi >> 1) | (state->af.b.lo & Z80_C_FLAG ? 0x80 : 0));
    state->af.b.lo = (uint8_t)((state->af.b.lo & (uint8_t)~(Z80_C_FLAG | Z80_N_FLAG | Z80_H_FLAG)) | I);
    break;

case Z80_RST00: Z80_M_RST(0x0000); break;
case Z80_RST08: Z80_M_RST(0x0008); break;
case Z80_RST10: Z80_M_RST(0x0010); break;
case Z80_RST18: Z80_M_RST(0x0018); break;
case Z80_RST20: Z80_M_RST(0x0020); break;
case Z80_RST28: Z80_M_RST(0x0028); break;
case Z80_RST30: Z80_M_RST(0x0030); break;
case Z80_RST38: Z80_M_RST(0x0038); break;

case Z80_PUSH_BC: Z80_M_PUSH(bc); break;
case Z80_PUSH_DE: Z80_M_PUSH(de); break;
case Z80_PUSH_HL: Z80_M_PUSH(Z80_XX); break;
case Z80_PUSH_AF: Z80_M_PUSH(af); break;

case Z80_POP_BC: Z80_M_POP(bc); break;
case Z80_POP_DE: Z80_M_POP(de); break;
case Z80_POP_HL: Z80_M_POP(Z80_XX); break;
case Z80_POP_AF: Z80_M_POP(af); break;

case Z80_DJNZ: if (--state->bc.b.hi) { state->icount -= 5; Z80_M_JR; } else state->pc.w++; break;
case Z80_JP:   Z80_M_JP; break;
case Z80_JR:   Z80_M_JR; break;
case Z80_CALL: Z80_M_CALL; break;
case Z80_RET:  Z80_M_RET; break;
case Z80_SCF:  Z80_FSET(Z80_C_FLAG); Z80_FCLR(Z80_N_FLAG | Z80_H_FLAG); break;
case Z80_CPL:  state->af.b.hi = (uint8_t)~state->af.b.hi; Z80_FSET(Z80_N_FLAG | Z80_H_FLAG); break;
case Z80_NOP:  break;
case Z80_OUTA: I = Z80_RD(state->pc.w++); Z80_OUT((uint16_t)(I | (state->af.w & 0xFF00)), state->af.b.hi); break;
case Z80_INA:  I = Z80_RD(state->pc.w++); state->af.b.hi = Z80_IN((uint16_t)(I | (state->af.w & 0xFF00))); break;

case Z80_HALT:
    state->pc.w--;
    state->iff |= Z80_IFF_HALT;
    state->ibackup = 0;
    state->icount = 0;
    break;

case Z80_DI:
    if (state->iff & Z80_IFF_EI) state->icount += state->ibackup - 1;
    state->iff &= (uint8_t)~(Z80_IFF_1 | Z80_IFF_2 | Z80_IFF_EI);
    break;

case Z80_EI:
    if (!(state->iff & (Z80_IFF_1 | Z80_IFF_EI))) {
        state->iff |= (Z80_IFF_2 | Z80_IFF_EI);
        state->ibackup = state->icount;
        state->icount = 1;
    }
    break;

case Z80_CCF:
    state->af.b.lo ^= Z80_C_FLAG;
    Z80_FCLR(Z80_N_FLAG | Z80_H_FLAG);
    state->af.b.lo |= (state->af.b.lo & Z80_C_FLAG) ? 0 : Z80_H_FLAG;
    break;

case Z80_EXX:
    J.w = state->bc.w; state->bc.w = state->bc_alt.w; state->bc_alt.w = J.w;
    J.w = state->de.w; state->de.w = state->de_alt.w; state->de_alt.w = J.w;
    J.w = state->hl.w; state->hl.w = state->hl_alt.w; state->hl_alt.w = J.w;
    break;

case Z80_EX_DE_HL: J.w = state->de.w; state->de.w = state->hl.w; state->hl.w = J.w; break;
case Z80_EX_AF_AF: J.w = state->af.w; state->af.w = state->af_alt.w; state->af_alt.w = J.w; break;

case Z80_LD_B_B: state->bc.b.hi = state->bc.b.hi; break;
case Z80_LD_C_B: state->bc.b.lo = state->bc.b.hi; break;
case Z80_LD_D_B: state->de.b.hi = state->bc.b.hi; break;
case Z80_LD_E_B: state->de.b.lo = state->bc.b.hi; break;
case Z80_LD_H_B: state->Z80_XX.b.hi = state->bc.b.hi; break;
case Z80_LD_L_B: state->Z80_XX.b.lo = state->bc.b.hi; break;
case Z80_LD_A_B: state->af.b.hi = state->bc.b.hi; break;
case Z80_LD_xHL_B: J.w = (uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++));
                   Z80_WR(J.w, state->bc.b.hi); break;

case Z80_LD_B_C: state->bc.b.hi = state->bc.b.lo; break;
case Z80_LD_C_C: state->bc.b.lo = state->bc.b.lo; break;
case Z80_LD_D_C: state->de.b.hi = state->bc.b.lo; break;
case Z80_LD_E_C: state->de.b.lo = state->bc.b.lo; break;
case Z80_LD_H_C: state->Z80_XX.b.hi = state->bc.b.lo; break;
case Z80_LD_L_C: state->Z80_XX.b.lo = state->bc.b.lo; break;
case Z80_LD_A_C: state->af.b.hi = state->bc.b.lo; break;
case Z80_LD_xHL_C: J.w = (uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++));
                   Z80_WR(J.w, state->bc.b.lo); break;

case Z80_LD_B_D: state->bc.b.hi = state->de.b.hi; break;
case Z80_LD_C_D: state->bc.b.lo = state->de.b.hi; break;
case Z80_LD_D_D: state->de.b.hi = state->de.b.hi; break;
case Z80_LD_E_D: state->de.b.lo = state->de.b.hi; break;
case Z80_LD_H_D: state->Z80_XX.b.hi = state->de.b.hi; break;
case Z80_LD_L_D: state->Z80_XX.b.lo = state->de.b.hi; break;
case Z80_LD_A_D: state->af.b.hi = state->de.b.hi; break;
case Z80_LD_xHL_D: J.w = (uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++));
                   Z80_WR(J.w, state->de.b.hi); break;

case Z80_LD_B_E: state->bc.b.hi = state->de.b.lo; break;
case Z80_LD_C_E: state->bc.b.lo = state->de.b.lo; break;
case Z80_LD_D_E: state->de.b.hi = state->de.b.lo; break;
case Z80_LD_E_E: state->de.b.lo = state->de.b.lo; break;
case Z80_LD_H_E: state->Z80_XX.b.hi = state->de.b.lo; break;
case Z80_LD_L_E: state->Z80_XX.b.lo = state->de.b.lo; break;
case Z80_LD_A_E: state->af.b.hi = state->de.b.lo; break;
case Z80_LD_xHL_E: J.w = (uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++));
                   Z80_WR(J.w, state->de.b.lo); break;

// Quirk real preservado: acessa H/L de verdade (state->hl), nao Z80_XX
// -- ver o aviso no topo do arquivo.
case Z80_LD_B_H: state->bc.b.hi = state->Z80_XX.b.hi; break;
case Z80_LD_C_H: state->bc.b.lo = state->Z80_XX.b.hi; break;
case Z80_LD_D_H: state->de.b.hi = state->Z80_XX.b.hi; break;
case Z80_LD_E_H: state->de.b.lo = state->Z80_XX.b.hi; break;
case Z80_LD_H_H: state->Z80_XX.b.hi = state->Z80_XX.b.hi; break;
case Z80_LD_L_H: state->Z80_XX.b.lo = state->Z80_XX.b.hi; break;
case Z80_LD_A_H: state->af.b.hi = state->Z80_XX.b.hi; break;
case Z80_LD_xHL_H: J.w = (uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++));
                   Z80_WR(J.w, state->hl.b.hi); break;

case Z80_LD_B_L: state->bc.b.hi = state->Z80_XX.b.lo; break;
case Z80_LD_C_L: state->bc.b.lo = state->Z80_XX.b.lo; break;
case Z80_LD_D_L: state->de.b.hi = state->Z80_XX.b.lo; break;
case Z80_LD_E_L: state->de.b.lo = state->Z80_XX.b.lo; break;
case Z80_LD_H_L: state->Z80_XX.b.hi = state->Z80_XX.b.lo; break;
case Z80_LD_L_L: state->Z80_XX.b.lo = state->Z80_XX.b.lo; break;
case Z80_LD_A_L: state->af.b.hi = state->Z80_XX.b.lo; break;
case Z80_LD_xHL_L: J.w = (uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++));
                   Z80_WR(J.w, state->hl.b.lo); break;

case Z80_LD_B_A: state->bc.b.hi = state->af.b.hi; break;
case Z80_LD_C_A: state->bc.b.lo = state->af.b.hi; break;
case Z80_LD_D_A: state->de.b.hi = state->af.b.hi; break;
case Z80_LD_E_A: state->de.b.lo = state->af.b.hi; break;
case Z80_LD_H_A: state->Z80_XX.b.hi = state->af.b.hi; break;
case Z80_LD_L_A: state->Z80_XX.b.lo = state->af.b.hi; break;
case Z80_LD_A_A: state->af.b.hi = state->af.b.hi; break;
case Z80_LD_xHL_A: J.w = (uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++));
                   Z80_WR(J.w, state->af.b.hi); break;

case Z80_LD_xBC_A: Z80_WR(state->bc.w, state->af.b.hi); break;
case Z80_LD_xDE_A: Z80_WR(state->de.w, state->af.b.hi); break;

// Quirk real preservado: LD r,(IX+d) grava no H/L REAL (state->hl) para
// H/L/A -- ver o aviso no topo do arquivo.
case Z80_LD_B_xHL: state->bc.b.hi = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++))); break;
case Z80_LD_C_xHL: state->bc.b.lo = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++))); break;
case Z80_LD_D_xHL: state->de.b.hi = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++))); break;
case Z80_LD_E_xHL: state->de.b.lo = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++))); break;
case Z80_LD_H_xHL: state->hl.b.hi = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++))); break;
case Z80_LD_L_xHL: state->hl.b.lo = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++))); break;
case Z80_LD_A_xHL: state->af.b.hi = Z80_RD((uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++))); break;

case Z80_LD_B_BYTE:   state->bc.b.hi = Z80_RD(state->pc.w++); break;
case Z80_LD_C_BYTE:   state->bc.b.lo = Z80_RD(state->pc.w++); break;
case Z80_LD_D_BYTE:   state->de.b.hi = Z80_RD(state->pc.w++); break;
case Z80_LD_E_BYTE:   state->de.b.lo = Z80_RD(state->pc.w++); break;
case Z80_LD_H_BYTE:   state->Z80_XX.b.hi = Z80_RD(state->pc.w++); break;
case Z80_LD_L_BYTE:   state->Z80_XX.b.lo = Z80_RD(state->pc.w++); break;
case Z80_LD_A_BYTE:   state->af.b.hi = Z80_RD(state->pc.w++); break;
case Z80_LD_xHL_BYTE: J.w = (uint16_t)(state->Z80_XX.w + (int8_t)Z80_RD(state->pc.w++));
                      Z80_WR(J.w, Z80_RD(state->pc.w++)); break;

case Z80_LD_xWORD_HL:
    J.b.lo = Z80_RD(state->pc.w++);
    J.b.hi = Z80_RD(state->pc.w++);
    Z80_WR(J.w++, state->Z80_XX.b.lo);
    Z80_WR(J.w, state->Z80_XX.b.hi);
    break;

case Z80_LD_HL_xWORD:
    J.b.lo = Z80_RD(state->pc.w++);
    J.b.hi = Z80_RD(state->pc.w++);
    state->Z80_XX.b.lo = Z80_RD(J.w++);
    state->Z80_XX.b.hi = Z80_RD(J.w);
    break;

case Z80_LD_A_xWORD:
    J.b.lo = Z80_RD(state->pc.w++);
    J.b.hi = Z80_RD(state->pc.w++);
    state->af.b.hi = Z80_RD(J.w);
    break;

case Z80_LD_xWORD_A:
    J.b.lo = Z80_RD(state->pc.w++);
    J.b.hi = Z80_RD(state->pc.w++);
    Z80_WR(J.w, state->af.b.hi);
    break;

case Z80_EX_HL_xSP:
    J.b.lo = Z80_RD(state->sp.w); Z80_WR(state->sp.w++, state->Z80_XX.b.lo);
    J.b.hi = Z80_RD(state->sp.w); Z80_WR(state->sp.w--, state->Z80_XX.b.hi);
    state->Z80_XX.w = J.w;
    break;

case Z80_DAA:
    J.w = state->af.b.hi;
    if (state->af.b.lo & Z80_C_FLAG) J.w |= 256;
    if (state->af.b.lo & Z80_H_FLAG) J.w |= 512;
    if (state->af.b.lo & Z80_N_FLAG) J.w |= 1024;
    state->af.w = g_z80_daa_table[J.w];
    break;
