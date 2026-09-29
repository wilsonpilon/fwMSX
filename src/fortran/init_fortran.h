//
// Modulo Fortran do fwMSX.
//
// Created by barney on 27/09/2026.
//
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Carrega o modulo Fortran (implementado em src/fortran/init_fortran.f90).
// Imprime "Loading module Fortran [v MAJOR.MINOR.PATCH]" usando E/S nativa
// do Fortran (WRITE em string interna + PRINT) e retorna a assinatura
// hexadecimal do modulo (0x0004).
uint16_t init_fortran(int major, int minor, int patch);

#ifdef __cplusplus
}
#endif
