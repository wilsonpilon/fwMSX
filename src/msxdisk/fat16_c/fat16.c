//
// Modulo C do msxdisk (fwMSX): leitura/escrita de entradas da FAT16.
//

#include "fat16.h"

uint16_t msxdisk_fat16_get(const uint8_t *fat, uint16_t cluster) {
    const size_t offset = (size_t)cluster * 2u;
    return (uint16_t)(fat[offset] | ((uint16_t)fat[offset + 1] << 8));
}

void msxdisk_fat16_set(uint8_t *fat, uint16_t cluster, uint16_t value) {
    const size_t offset = (size_t)cluster * 2u;
    fat[offset] = (uint8_t)(value & 0xFFu);
    fat[offset + 1] = (uint8_t)(value >> 8);
}
