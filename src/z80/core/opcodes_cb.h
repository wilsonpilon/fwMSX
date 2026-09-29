// Adaptado de fMSX (resource/fMSX/Z80/CodesCB.h), Copyright (C) Marat
// Fayzullin 1994-2021. O fwMSX evolui a partir do fMSX com o aval do
// autor original para adaptar/estudar seu codigo (ver README.md) -- isso
// nao e uma relicenciacao: este arquivo continua sob os termos originais
// dele (nao-comercial, aviso ao autor em caso de mudanca), nao o
// BSD-3-Clause do restante do fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// Corpo dos opcodes prefixados por CB -- incluso dentro do switch(I) de
// z80_exec_cb() em z80_core.c. Ver opcodes_base.h para a nota completa
// sobre as variaveis locais esperadas (`state`, `bus`, `I`, `J`).

case Z80_RLC_B: Z80_M_RLC(state->bc.b.hi); break;  case Z80_RLC_C: Z80_M_RLC(state->bc.b.lo); break;
case Z80_RLC_D: Z80_M_RLC(state->de.b.hi); break;  case Z80_RLC_E: Z80_M_RLC(state->de.b.lo); break;
case Z80_RLC_H: Z80_M_RLC(state->hl.b.hi); break;  case Z80_RLC_L: Z80_M_RLC(state->hl.b.lo); break;
case Z80_RLC_xHL: I = Z80_RD(state->hl.w); Z80_M_RLC(I); Z80_WR(state->hl.w, I); break;
case Z80_RLC_A: Z80_M_RLC(state->af.b.hi); break;

case Z80_RRC_B: Z80_M_RRC(state->bc.b.hi); break;  case Z80_RRC_C: Z80_M_RRC(state->bc.b.lo); break;
case Z80_RRC_D: Z80_M_RRC(state->de.b.hi); break;  case Z80_RRC_E: Z80_M_RRC(state->de.b.lo); break;
case Z80_RRC_H: Z80_M_RRC(state->hl.b.hi); break;  case Z80_RRC_L: Z80_M_RRC(state->hl.b.lo); break;
case Z80_RRC_xHL: I = Z80_RD(state->hl.w); Z80_M_RRC(I); Z80_WR(state->hl.w, I); break;
case Z80_RRC_A: Z80_M_RRC(state->af.b.hi); break;

case Z80_RL_B: Z80_M_RL(state->bc.b.hi); break;  case Z80_RL_C: Z80_M_RL(state->bc.b.lo); break;
case Z80_RL_D: Z80_M_RL(state->de.b.hi); break;  case Z80_RL_E: Z80_M_RL(state->de.b.lo); break;
case Z80_RL_H: Z80_M_RL(state->hl.b.hi); break;  case Z80_RL_L: Z80_M_RL(state->hl.b.lo); break;
case Z80_RL_xHL: I = Z80_RD(state->hl.w); Z80_M_RL(I); Z80_WR(state->hl.w, I); break;
case Z80_RL_A: Z80_M_RL(state->af.b.hi); break;

case Z80_RR_B: Z80_M_RR(state->bc.b.hi); break;  case Z80_RR_C: Z80_M_RR(state->bc.b.lo); break;
case Z80_RR_D: Z80_M_RR(state->de.b.hi); break;  case Z80_RR_E: Z80_M_RR(state->de.b.lo); break;
case Z80_RR_H: Z80_M_RR(state->hl.b.hi); break;  case Z80_RR_L: Z80_M_RR(state->hl.b.lo); break;
case Z80_RR_xHL: I = Z80_RD(state->hl.w); Z80_M_RR(I); Z80_WR(state->hl.w, I); break;
case Z80_RR_A: Z80_M_RR(state->af.b.hi); break;

case Z80_SLA_B: Z80_M_SLA(state->bc.b.hi); break;  case Z80_SLA_C: Z80_M_SLA(state->bc.b.lo); break;
case Z80_SLA_D: Z80_M_SLA(state->de.b.hi); break;  case Z80_SLA_E: Z80_M_SLA(state->de.b.lo); break;
case Z80_SLA_H: Z80_M_SLA(state->hl.b.hi); break;  case Z80_SLA_L: Z80_M_SLA(state->hl.b.lo); break;
case Z80_SLA_xHL: I = Z80_RD(state->hl.w); Z80_M_SLA(I); Z80_WR(state->hl.w, I); break;
case Z80_SLA_A: Z80_M_SLA(state->af.b.hi); break;

case Z80_SRA_B: Z80_M_SRA(state->bc.b.hi); break;  case Z80_SRA_C: Z80_M_SRA(state->bc.b.lo); break;
case Z80_SRA_D: Z80_M_SRA(state->de.b.hi); break;  case Z80_SRA_E: Z80_M_SRA(state->de.b.lo); break;
case Z80_SRA_H: Z80_M_SRA(state->hl.b.hi); break;  case Z80_SRA_L: Z80_M_SRA(state->hl.b.lo); break;
case Z80_SRA_xHL: I = Z80_RD(state->hl.w); Z80_M_SRA(I); Z80_WR(state->hl.w, I); break;
case Z80_SRA_A: Z80_M_SRA(state->af.b.hi); break;

case Z80_SLL_B: Z80_M_SLL(state->bc.b.hi); break;  case Z80_SLL_C: Z80_M_SLL(state->bc.b.lo); break;
case Z80_SLL_D: Z80_M_SLL(state->de.b.hi); break;  case Z80_SLL_E: Z80_M_SLL(state->de.b.lo); break;
case Z80_SLL_H: Z80_M_SLL(state->hl.b.hi); break;  case Z80_SLL_L: Z80_M_SLL(state->hl.b.lo); break;
case Z80_SLL_xHL: I = Z80_RD(state->hl.w); Z80_M_SLL(I); Z80_WR(state->hl.w, I); break;
case Z80_SLL_A: Z80_M_SLL(state->af.b.hi); break;

case Z80_SRL_B: Z80_M_SRL(state->bc.b.hi); break;  case Z80_SRL_C: Z80_M_SRL(state->bc.b.lo); break;
case Z80_SRL_D: Z80_M_SRL(state->de.b.hi); break;  case Z80_SRL_E: Z80_M_SRL(state->de.b.lo); break;
case Z80_SRL_H: Z80_M_SRL(state->hl.b.hi); break;  case Z80_SRL_L: Z80_M_SRL(state->hl.b.lo); break;
case Z80_SRL_xHL: I = Z80_RD(state->hl.w); Z80_M_SRL(I); Z80_WR(state->hl.w, I); break;
case Z80_SRL_A: Z80_M_SRL(state->af.b.hi); break;

case Z80_BIT0_B: Z80_M_BIT(0, state->bc.b.hi); break;  case Z80_BIT0_C: Z80_M_BIT(0, state->bc.b.lo); break;
case Z80_BIT0_D: Z80_M_BIT(0, state->de.b.hi); break;  case Z80_BIT0_E: Z80_M_BIT(0, state->de.b.lo); break;
case Z80_BIT0_H: Z80_M_BIT(0, state->hl.b.hi); break;  case Z80_BIT0_L: Z80_M_BIT(0, state->hl.b.lo); break;
case Z80_BIT0_xHL: I = Z80_RD(state->hl.w); Z80_M_BIT(0, I); break;
case Z80_BIT0_A: Z80_M_BIT(0, state->af.b.hi); break;

case Z80_BIT1_B: Z80_M_BIT(1, state->bc.b.hi); break;  case Z80_BIT1_C: Z80_M_BIT(1, state->bc.b.lo); break;
case Z80_BIT1_D: Z80_M_BIT(1, state->de.b.hi); break;  case Z80_BIT1_E: Z80_M_BIT(1, state->de.b.lo); break;
case Z80_BIT1_H: Z80_M_BIT(1, state->hl.b.hi); break;  case Z80_BIT1_L: Z80_M_BIT(1, state->hl.b.lo); break;
case Z80_BIT1_xHL: I = Z80_RD(state->hl.w); Z80_M_BIT(1, I); break;
case Z80_BIT1_A: Z80_M_BIT(1, state->af.b.hi); break;

case Z80_BIT2_B: Z80_M_BIT(2, state->bc.b.hi); break;  case Z80_BIT2_C: Z80_M_BIT(2, state->bc.b.lo); break;
case Z80_BIT2_D: Z80_M_BIT(2, state->de.b.hi); break;  case Z80_BIT2_E: Z80_M_BIT(2, state->de.b.lo); break;
case Z80_BIT2_H: Z80_M_BIT(2, state->hl.b.hi); break;  case Z80_BIT2_L: Z80_M_BIT(2, state->hl.b.lo); break;
case Z80_BIT2_xHL: I = Z80_RD(state->hl.w); Z80_M_BIT(2, I); break;
case Z80_BIT2_A: Z80_M_BIT(2, state->af.b.hi); break;

case Z80_BIT3_B: Z80_M_BIT(3, state->bc.b.hi); break;  case Z80_BIT3_C: Z80_M_BIT(3, state->bc.b.lo); break;
case Z80_BIT3_D: Z80_M_BIT(3, state->de.b.hi); break;  case Z80_BIT3_E: Z80_M_BIT(3, state->de.b.lo); break;
case Z80_BIT3_H: Z80_M_BIT(3, state->hl.b.hi); break;  case Z80_BIT3_L: Z80_M_BIT(3, state->hl.b.lo); break;
case Z80_BIT3_xHL: I = Z80_RD(state->hl.w); Z80_M_BIT(3, I); break;
case Z80_BIT3_A: Z80_M_BIT(3, state->af.b.hi); break;

case Z80_BIT4_B: Z80_M_BIT(4, state->bc.b.hi); break;  case Z80_BIT4_C: Z80_M_BIT(4, state->bc.b.lo); break;
case Z80_BIT4_D: Z80_M_BIT(4, state->de.b.hi); break;  case Z80_BIT4_E: Z80_M_BIT(4, state->de.b.lo); break;
case Z80_BIT4_H: Z80_M_BIT(4, state->hl.b.hi); break;  case Z80_BIT4_L: Z80_M_BIT(4, state->hl.b.lo); break;
case Z80_BIT4_xHL: I = Z80_RD(state->hl.w); Z80_M_BIT(4, I); break;
case Z80_BIT4_A: Z80_M_BIT(4, state->af.b.hi); break;

case Z80_BIT5_B: Z80_M_BIT(5, state->bc.b.hi); break;  case Z80_BIT5_C: Z80_M_BIT(5, state->bc.b.lo); break;
case Z80_BIT5_D: Z80_M_BIT(5, state->de.b.hi); break;  case Z80_BIT5_E: Z80_M_BIT(5, state->de.b.lo); break;
case Z80_BIT5_H: Z80_M_BIT(5, state->hl.b.hi); break;  case Z80_BIT5_L: Z80_M_BIT(5, state->hl.b.lo); break;
case Z80_BIT5_xHL: I = Z80_RD(state->hl.w); Z80_M_BIT(5, I); break;
case Z80_BIT5_A: Z80_M_BIT(5, state->af.b.hi); break;

case Z80_BIT6_B: Z80_M_BIT(6, state->bc.b.hi); break;  case Z80_BIT6_C: Z80_M_BIT(6, state->bc.b.lo); break;
case Z80_BIT6_D: Z80_M_BIT(6, state->de.b.hi); break;  case Z80_BIT6_E: Z80_M_BIT(6, state->de.b.lo); break;
case Z80_BIT6_H: Z80_M_BIT(6, state->hl.b.hi); break;  case Z80_BIT6_L: Z80_M_BIT(6, state->hl.b.lo); break;
case Z80_BIT6_xHL: I = Z80_RD(state->hl.w); Z80_M_BIT(6, I); break;
case Z80_BIT6_A: Z80_M_BIT(6, state->af.b.hi); break;

case Z80_BIT7_B: Z80_M_BIT(7, state->bc.b.hi); break;  case Z80_BIT7_C: Z80_M_BIT(7, state->bc.b.lo); break;
case Z80_BIT7_D: Z80_M_BIT(7, state->de.b.hi); break;  case Z80_BIT7_E: Z80_M_BIT(7, state->de.b.lo); break;
case Z80_BIT7_H: Z80_M_BIT(7, state->hl.b.hi); break;  case Z80_BIT7_L: Z80_M_BIT(7, state->hl.b.lo); break;
case Z80_BIT7_xHL: I = Z80_RD(state->hl.w); Z80_M_BIT(7, I); break;
case Z80_BIT7_A: Z80_M_BIT(7, state->af.b.hi); break;

case Z80_RES0_B: Z80_M_RES(0, state->bc.b.hi); break;  case Z80_RES0_C: Z80_M_RES(0, state->bc.b.lo); break;
case Z80_RES0_D: Z80_M_RES(0, state->de.b.hi); break;  case Z80_RES0_E: Z80_M_RES(0, state->de.b.lo); break;
case Z80_RES0_H: Z80_M_RES(0, state->hl.b.hi); break;  case Z80_RES0_L: Z80_M_RES(0, state->hl.b.lo); break;
case Z80_RES0_xHL: I = Z80_RD(state->hl.w); Z80_M_RES(0, I); Z80_WR(state->hl.w, I); break;
case Z80_RES0_A: Z80_M_RES(0, state->af.b.hi); break;

case Z80_RES1_B: Z80_M_RES(1, state->bc.b.hi); break;  case Z80_RES1_C: Z80_M_RES(1, state->bc.b.lo); break;
case Z80_RES1_D: Z80_M_RES(1, state->de.b.hi); break;  case Z80_RES1_E: Z80_M_RES(1, state->de.b.lo); break;
case Z80_RES1_H: Z80_M_RES(1, state->hl.b.hi); break;  case Z80_RES1_L: Z80_M_RES(1, state->hl.b.lo); break;
case Z80_RES1_xHL: I = Z80_RD(state->hl.w); Z80_M_RES(1, I); Z80_WR(state->hl.w, I); break;
case Z80_RES1_A: Z80_M_RES(1, state->af.b.hi); break;

case Z80_RES2_B: Z80_M_RES(2, state->bc.b.hi); break;  case Z80_RES2_C: Z80_M_RES(2, state->bc.b.lo); break;
case Z80_RES2_D: Z80_M_RES(2, state->de.b.hi); break;  case Z80_RES2_E: Z80_M_RES(2, state->de.b.lo); break;
case Z80_RES2_H: Z80_M_RES(2, state->hl.b.hi); break;  case Z80_RES2_L: Z80_M_RES(2, state->hl.b.lo); break;
case Z80_RES2_xHL: I = Z80_RD(state->hl.w); Z80_M_RES(2, I); Z80_WR(state->hl.w, I); break;
case Z80_RES2_A: Z80_M_RES(2, state->af.b.hi); break;

case Z80_RES3_B: Z80_M_RES(3, state->bc.b.hi); break;  case Z80_RES3_C: Z80_M_RES(3, state->bc.b.lo); break;
case Z80_RES3_D: Z80_M_RES(3, state->de.b.hi); break;  case Z80_RES3_E: Z80_M_RES(3, state->de.b.lo); break;
case Z80_RES3_H: Z80_M_RES(3, state->hl.b.hi); break;  case Z80_RES3_L: Z80_M_RES(3, state->hl.b.lo); break;
case Z80_RES3_xHL: I = Z80_RD(state->hl.w); Z80_M_RES(3, I); Z80_WR(state->hl.w, I); break;
case Z80_RES3_A: Z80_M_RES(3, state->af.b.hi); break;

case Z80_RES4_B: Z80_M_RES(4, state->bc.b.hi); break;  case Z80_RES4_C: Z80_M_RES(4, state->bc.b.lo); break;
case Z80_RES4_D: Z80_M_RES(4, state->de.b.hi); break;  case Z80_RES4_E: Z80_M_RES(4, state->de.b.lo); break;
case Z80_RES4_H: Z80_M_RES(4, state->hl.b.hi); break;  case Z80_RES4_L: Z80_M_RES(4, state->hl.b.lo); break;
case Z80_RES4_xHL: I = Z80_RD(state->hl.w); Z80_M_RES(4, I); Z80_WR(state->hl.w, I); break;
case Z80_RES4_A: Z80_M_RES(4, state->af.b.hi); break;

case Z80_RES5_B: Z80_M_RES(5, state->bc.b.hi); break;  case Z80_RES5_C: Z80_M_RES(5, state->bc.b.lo); break;
case Z80_RES5_D: Z80_M_RES(5, state->de.b.hi); break;  case Z80_RES5_E: Z80_M_RES(5, state->de.b.lo); break;
case Z80_RES5_H: Z80_M_RES(5, state->hl.b.hi); break;  case Z80_RES5_L: Z80_M_RES(5, state->hl.b.lo); break;
case Z80_RES5_xHL: I = Z80_RD(state->hl.w); Z80_M_RES(5, I); Z80_WR(state->hl.w, I); break;
case Z80_RES5_A: Z80_M_RES(5, state->af.b.hi); break;

case Z80_RES6_B: Z80_M_RES(6, state->bc.b.hi); break;  case Z80_RES6_C: Z80_M_RES(6, state->bc.b.lo); break;
case Z80_RES6_D: Z80_M_RES(6, state->de.b.hi); break;  case Z80_RES6_E: Z80_M_RES(6, state->de.b.lo); break;
case Z80_RES6_H: Z80_M_RES(6, state->hl.b.hi); break;  case Z80_RES6_L: Z80_M_RES(6, state->hl.b.lo); break;
case Z80_RES6_xHL: I = Z80_RD(state->hl.w); Z80_M_RES(6, I); Z80_WR(state->hl.w, I); break;
case Z80_RES6_A: Z80_M_RES(6, state->af.b.hi); break;

case Z80_RES7_B: Z80_M_RES(7, state->bc.b.hi); break;  case Z80_RES7_C: Z80_M_RES(7, state->bc.b.lo); break;
case Z80_RES7_D: Z80_M_RES(7, state->de.b.hi); break;  case Z80_RES7_E: Z80_M_RES(7, state->de.b.lo); break;
case Z80_RES7_H: Z80_M_RES(7, state->hl.b.hi); break;  case Z80_RES7_L: Z80_M_RES(7, state->hl.b.lo); break;
case Z80_RES7_xHL: I = Z80_RD(state->hl.w); Z80_M_RES(7, I); Z80_WR(state->hl.w, I); break;
case Z80_RES7_A: Z80_M_RES(7, state->af.b.hi); break;

case Z80_SET0_B: Z80_M_SET(0, state->bc.b.hi); break;  case Z80_SET0_C: Z80_M_SET(0, state->bc.b.lo); break;
case Z80_SET0_D: Z80_M_SET(0, state->de.b.hi); break;  case Z80_SET0_E: Z80_M_SET(0, state->de.b.lo); break;
case Z80_SET0_H: Z80_M_SET(0, state->hl.b.hi); break;  case Z80_SET0_L: Z80_M_SET(0, state->hl.b.lo); break;
case Z80_SET0_xHL: I = Z80_RD(state->hl.w); Z80_M_SET(0, I); Z80_WR(state->hl.w, I); break;
case Z80_SET0_A: Z80_M_SET(0, state->af.b.hi); break;

case Z80_SET1_B: Z80_M_SET(1, state->bc.b.hi); break;  case Z80_SET1_C: Z80_M_SET(1, state->bc.b.lo); break;
case Z80_SET1_D: Z80_M_SET(1, state->de.b.hi); break;  case Z80_SET1_E: Z80_M_SET(1, state->de.b.lo); break;
case Z80_SET1_H: Z80_M_SET(1, state->hl.b.hi); break;  case Z80_SET1_L: Z80_M_SET(1, state->hl.b.lo); break;
case Z80_SET1_xHL: I = Z80_RD(state->hl.w); Z80_M_SET(1, I); Z80_WR(state->hl.w, I); break;
case Z80_SET1_A: Z80_M_SET(1, state->af.b.hi); break;

case Z80_SET2_B: Z80_M_SET(2, state->bc.b.hi); break;  case Z80_SET2_C: Z80_M_SET(2, state->bc.b.lo); break;
case Z80_SET2_D: Z80_M_SET(2, state->de.b.hi); break;  case Z80_SET2_E: Z80_M_SET(2, state->de.b.lo); break;
case Z80_SET2_H: Z80_M_SET(2, state->hl.b.hi); break;  case Z80_SET2_L: Z80_M_SET(2, state->hl.b.lo); break;
case Z80_SET2_xHL: I = Z80_RD(state->hl.w); Z80_M_SET(2, I); Z80_WR(state->hl.w, I); break;
case Z80_SET2_A: Z80_M_SET(2, state->af.b.hi); break;

case Z80_SET3_B: Z80_M_SET(3, state->bc.b.hi); break;  case Z80_SET3_C: Z80_M_SET(3, state->bc.b.lo); break;
case Z80_SET3_D: Z80_M_SET(3, state->de.b.hi); break;  case Z80_SET3_E: Z80_M_SET(3, state->de.b.lo); break;
case Z80_SET3_H: Z80_M_SET(3, state->hl.b.hi); break;  case Z80_SET3_L: Z80_M_SET(3, state->hl.b.lo); break;
case Z80_SET3_xHL: I = Z80_RD(state->hl.w); Z80_M_SET(3, I); Z80_WR(state->hl.w, I); break;
case Z80_SET3_A: Z80_M_SET(3, state->af.b.hi); break;

case Z80_SET4_B: Z80_M_SET(4, state->bc.b.hi); break;  case Z80_SET4_C: Z80_M_SET(4, state->bc.b.lo); break;
case Z80_SET4_D: Z80_M_SET(4, state->de.b.hi); break;  case Z80_SET4_E: Z80_M_SET(4, state->de.b.lo); break;
case Z80_SET4_H: Z80_M_SET(4, state->hl.b.hi); break;  case Z80_SET4_L: Z80_M_SET(4, state->hl.b.lo); break;
case Z80_SET4_xHL: I = Z80_RD(state->hl.w); Z80_M_SET(4, I); Z80_WR(state->hl.w, I); break;
case Z80_SET4_A: Z80_M_SET(4, state->af.b.hi); break;

case Z80_SET5_B: Z80_M_SET(5, state->bc.b.hi); break;  case Z80_SET5_C: Z80_M_SET(5, state->bc.b.lo); break;
case Z80_SET5_D: Z80_M_SET(5, state->de.b.hi); break;  case Z80_SET5_E: Z80_M_SET(5, state->de.b.lo); break;
case Z80_SET5_H: Z80_M_SET(5, state->hl.b.hi); break;  case Z80_SET5_L: Z80_M_SET(5, state->hl.b.lo); break;
case Z80_SET5_xHL: I = Z80_RD(state->hl.w); Z80_M_SET(5, I); Z80_WR(state->hl.w, I); break;
case Z80_SET5_A: Z80_M_SET(5, state->af.b.hi); break;

case Z80_SET6_B: Z80_M_SET(6, state->bc.b.hi); break;  case Z80_SET6_C: Z80_M_SET(6, state->bc.b.lo); break;
case Z80_SET6_D: Z80_M_SET(6, state->de.b.hi); break;  case Z80_SET6_E: Z80_M_SET(6, state->de.b.lo); break;
case Z80_SET6_H: Z80_M_SET(6, state->hl.b.hi); break;  case Z80_SET6_L: Z80_M_SET(6, state->hl.b.lo); break;
case Z80_SET6_xHL: I = Z80_RD(state->hl.w); Z80_M_SET(6, I); Z80_WR(state->hl.w, I); break;
case Z80_SET6_A: Z80_M_SET(6, state->af.b.hi); break;

case Z80_SET7_B: Z80_M_SET(7, state->bc.b.hi); break;  case Z80_SET7_C: Z80_M_SET(7, state->bc.b.lo); break;
case Z80_SET7_D: Z80_M_SET(7, state->de.b.hi); break;  case Z80_SET7_E: Z80_M_SET(7, state->de.b.lo); break;
case Z80_SET7_H: Z80_M_SET(7, state->hl.b.hi); break;  case Z80_SET7_L: Z80_M_SET(7, state->hl.b.lo); break;
case Z80_SET7_xHL: I = Z80_RD(state->hl.w); Z80_M_SET(7, I); Z80_WR(state->hl.w, I); break;
case Z80_SET7_A: Z80_M_SET(7, state->af.b.hi); break;
