//
// Modulo C do msxdisk (fwMSX): empacotamento/leitura de entradas da FAT12.
//
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Le a entrada de 12 bits do cluster 'cluster' dentro de 'fat' (bytes
// crus de uma copia da FAT). A FAT12 guarda cada entrada em 1.5 bytes,
// duas entradas compartilhando 3 bytes consecutivos -- essa aritmetica de
// bits e reescrita aqui a partir do formato publico, sem portar nenhum
// dos fontes de resource/ (ver doc/msxdisk-spec.md, secao 2).
uint16_t msxdisk_fat12_get(const uint8_t *fat, uint16_t cluster);

// Escreve os 12 bits menos significativos de 'value' na entrada do
// cluster 'cluster' dentro de 'fat'.
void msxdisk_fat12_set(uint8_t *fat, uint16_t cluster, uint16_t value);

#ifdef __cplusplus
}
#endif
