// fwMSX -- Basic Dignified: arredondamento de digitos (Fortran). Ver badig_num.f90.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Corta digits[0, n) em `precision` digitos arredondando pelo seguinte; escreve em out[0, outcap) e devolve quantos (0 = nao coube/invalido).
int bd_round_digits(const char *digits, int n, int precision, char *out, int outcap);

#ifdef __cplusplus
}
#endif
