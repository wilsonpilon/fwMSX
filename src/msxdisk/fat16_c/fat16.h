//
// Modulo C do msxdisk (fwMSX): leitura/escrita de entradas da FAT16.
//
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Le a entrada de 16 bits do cluster 'cluster' dentro de 'fat' (bytes
// crus de uma copia da FAT). Diferente da FAT12, a FAT16 guarda cada
// entrada em 2 bytes inteiros (little-endian), sem compartilhar bits
// com a entrada vizinha -- por isso nao ha' aritmetica de bits aqui,
// so' a leitura/escrita little-endian (formato publico; ver
// doc/msxdisk-spec.md). 'cluster' cabe em 16 bits (FAT16 vai so' a
// 65524 clusters de dados).
uint16_t msxdisk_fat16_get(const uint8_t *fat, uint16_t cluster);

// Escreve 'value' na entrada do cluster 'cluster' dentro de 'fat'.
void msxdisk_fat16_set(uint8_t *fat, uint16_t cluster, uint16_t value);

#ifdef __cplusplus
}
#endif
