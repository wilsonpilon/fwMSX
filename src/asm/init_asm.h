//
// Modulo Assembly do fwMSX.
//
// Created by barney on 27/09/2026.
//
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Carrega o modulo Assembly (implementado em src/asm/init_asm.asm).
// Monta e executa, em NASM puro, uma chamada a printf() respeitando a
// ABI Win64 (shadow space, alinhamento de pilha de 16 bytes e a ordem de
// registradores RCX/RDX/R8/R9), imprimindo
// "Loading module...Assembly [v MAJOR.MINOR.PATCH]" e devolvendo em EAX a
// assinatura hexadecimal do modulo (0x0003).
uint16_t init_asm(int major, int minor, int patch);

#ifdef __cplusplus
}
#endif
