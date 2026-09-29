//
// Modulo Assembly do msxdisk (fwMSX) - NASM, ABI Win64.
//
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Compara 11 bytes fixos de 'name11' (nome de diretorio MSX-DOS: 8 do
// nome + 3 da extensao, sem ponto, preenchido com espaco) contra o padrao
// 'pattern11' no mesmo formato, tratando '?' no padrao como coringa de um
// caractere. Retorna 1 se combina, 0 caso contrario.
int msxdisk_name_match(const char *pattern11, const char *name11);

#ifdef __cplusplus
}
#endif
