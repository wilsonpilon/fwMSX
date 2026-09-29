//
// Modulo C do fwMSX.
//
// Created by barney on 27/09/2026.
//
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Carrega o modulo C: imprime "Loading module...C [v MAJOR.MINOR.PATCH]"
// e retorna a assinatura hexadecimal do modulo (0x0002).
uint16_t init_c(int major, int minor, int patch);

#ifdef __cplusplus
}
#endif
