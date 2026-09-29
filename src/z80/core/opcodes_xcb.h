// Adaptado de fMSX (resource/fMSX/Z80/CodesXCB.h), Copyright (C) Marat
// Fayzullin 1994-2021. O fwMSX evolui a partir do fMSX com o aval do
// autor original para adaptar/estudar seu codigo (ver README.md) -- isso
// nao e uma relicenciacao: este arquivo continua sob os termos originais
// dele (nao-comercial, aviso ao autor em caso de mudanca), nao o
// BSD-3-Clause do restante do fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// Corpo dos opcodes prefixados por DD CB/FD CB -- incluso dentro do
// switch(I) de z80_exec_ddcb()/z80_exec_fdcb() em z80_core.c, que ja
// calculam o endereco efetivo (IX+d)/(IY+d) em J antes do switch. Ver
// opcodes_base.h para a nota completa sobre `state`/`bus`/`I`/`J`.
//
// Fiel ao original: os opcodes BIT0..BIT7 tem um "case" por registrador
// (B/C/D/E/H/L/A) mas todos caem no mesmo corpo -- o hardware real (e o
// fMSX) sempre testa o byte em (IX+d)/(IY+d), nunca o registrador
// nomeado, para esses opcodes indocumentados. Os opcodes RES/SET
// indocumentados que tambem escreveriam no registrador nomeado (alem da
// memoria) NAO estao implementados aqui -- exatamente como no fMSX (so
// existe o "case" da variante xHL; a variante "nomeada" cai no default
// do switch que inclui este arquivo). Isso e' uma limitacao conhecida,
// herdada de proposito para bater com o comportamento do fMSX, nao um
// erro de transcricao.

case Z80_RLC_xHL: I = Z80_RD(J.w); Z80_M_RLC(I); Z80_WR(J.w, I); break;
case Z80_RRC_xHL: I = Z80_RD(J.w); Z80_M_RRC(I); Z80_WR(J.w, I); break;
case Z80_RL_xHL:  I = Z80_RD(J.w); Z80_M_RL(I);  Z80_WR(J.w, I); break;
case Z80_RR_xHL:  I = Z80_RD(J.w); Z80_M_RR(I);  Z80_WR(J.w, I); break;
case Z80_SLA_xHL: I = Z80_RD(J.w); Z80_M_SLA(I); Z80_WR(J.w, I); break;
case Z80_SRA_xHL: I = Z80_RD(J.w); Z80_M_SRA(I); Z80_WR(J.w, I); break;
case Z80_SLL_xHL: I = Z80_RD(J.w); Z80_M_SLL(I); Z80_WR(J.w, I); break;
case Z80_SRL_xHL: I = Z80_RD(J.w); Z80_M_SRL(I); Z80_WR(J.w, I); break;

case Z80_BIT0_B: case Z80_BIT0_C: case Z80_BIT0_D: case Z80_BIT0_E:
case Z80_BIT0_H: case Z80_BIT0_L: case Z80_BIT0_A:
case Z80_BIT0_xHL: I = Z80_RD(J.w); Z80_M_BIT(0, I); break;
case Z80_BIT1_B: case Z80_BIT1_C: case Z80_BIT1_D: case Z80_BIT1_E:
case Z80_BIT1_H: case Z80_BIT1_L: case Z80_BIT1_A:
case Z80_BIT1_xHL: I = Z80_RD(J.w); Z80_M_BIT(1, I); break;
case Z80_BIT2_B: case Z80_BIT2_C: case Z80_BIT2_D: case Z80_BIT2_E:
case Z80_BIT2_H: case Z80_BIT2_L: case Z80_BIT2_A:
case Z80_BIT2_xHL: I = Z80_RD(J.w); Z80_M_BIT(2, I); break;
case Z80_BIT3_B: case Z80_BIT3_C: case Z80_BIT3_D: case Z80_BIT3_E:
case Z80_BIT3_H: case Z80_BIT3_L: case Z80_BIT3_A:
case Z80_BIT3_xHL: I = Z80_RD(J.w); Z80_M_BIT(3, I); break;
case Z80_BIT4_B: case Z80_BIT4_C: case Z80_BIT4_D: case Z80_BIT4_E:
case Z80_BIT4_H: case Z80_BIT4_L: case Z80_BIT4_A:
case Z80_BIT4_xHL: I = Z80_RD(J.w); Z80_M_BIT(4, I); break;
case Z80_BIT5_B: case Z80_BIT5_C: case Z80_BIT5_D: case Z80_BIT5_E:
case Z80_BIT5_H: case Z80_BIT5_L: case Z80_BIT5_A:
case Z80_BIT5_xHL: I = Z80_RD(J.w); Z80_M_BIT(5, I); break;
case Z80_BIT6_B: case Z80_BIT6_C: case Z80_BIT6_D: case Z80_BIT6_E:
case Z80_BIT6_H: case Z80_BIT6_L: case Z80_BIT6_A:
case Z80_BIT6_xHL: I = Z80_RD(J.w); Z80_M_BIT(6, I); break;
case Z80_BIT7_B: case Z80_BIT7_C: case Z80_BIT7_D: case Z80_BIT7_E:
case Z80_BIT7_H: case Z80_BIT7_L: case Z80_BIT7_A:
case Z80_BIT7_xHL: I = Z80_RD(J.w); Z80_M_BIT(7, I); break;

case Z80_RES0_xHL: I = Z80_RD(J.w); Z80_M_RES(0, I); Z80_WR(J.w, I); break;
case Z80_RES1_xHL: I = Z80_RD(J.w); Z80_M_RES(1, I); Z80_WR(J.w, I); break;
case Z80_RES2_xHL: I = Z80_RD(J.w); Z80_M_RES(2, I); Z80_WR(J.w, I); break;
case Z80_RES3_xHL: I = Z80_RD(J.w); Z80_M_RES(3, I); Z80_WR(J.w, I); break;
case Z80_RES4_xHL: I = Z80_RD(J.w); Z80_M_RES(4, I); Z80_WR(J.w, I); break;
case Z80_RES5_xHL: I = Z80_RD(J.w); Z80_M_RES(5, I); Z80_WR(J.w, I); break;
case Z80_RES6_xHL: I = Z80_RD(J.w); Z80_M_RES(6, I); Z80_WR(J.w, I); break;
case Z80_RES7_xHL: I = Z80_RD(J.w); Z80_M_RES(7, I); Z80_WR(J.w, I); break;

case Z80_SET0_xHL: I = Z80_RD(J.w); Z80_M_SET(0, I); Z80_WR(J.w, I); break;
case Z80_SET1_xHL: I = Z80_RD(J.w); Z80_M_SET(1, I); Z80_WR(J.w, I); break;
case Z80_SET2_xHL: I = Z80_RD(J.w); Z80_M_SET(2, I); Z80_WR(J.w, I); break;
case Z80_SET3_xHL: I = Z80_RD(J.w); Z80_M_SET(3, I); Z80_WR(J.w, I); break;
case Z80_SET4_xHL: I = Z80_RD(J.w); Z80_M_SET(4, I); Z80_WR(J.w, I); break;
case Z80_SET5_xHL: I = Z80_RD(J.w); Z80_M_SET(5, I); Z80_WR(J.w, I); break;
case Z80_SET6_xHL: I = Z80_RD(J.w); Z80_M_SET(6, I); Z80_WR(J.w, I); break;
case Z80_SET7_xHL: I = Z80_RD(J.w); Z80_M_SET(7, I); Z80_WR(J.w, I); break;
