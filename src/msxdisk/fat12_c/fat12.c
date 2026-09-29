//
// Modulo C do msxdisk (fwMSX): empacotamento/leitura de entradas da FAT12.
//

#include "fat12.h"

uint16_t msxdisk_fat12_get(const uint8_t *fat, uint16_t cluster) {
    const size_t offset = (size_t)cluster + (size_t)(cluster / 2);

    if (cluster & 1u) {
        /* cluster impar: 4 bits altos do 1o byte + 8 bits do 2o byte */
        return (uint16_t)((fat[offset] >> 4) | ((uint16_t)fat[offset + 1] << 4));
    }
    /* cluster par: 8 bits do 1o byte + 4 bits baixos do 2o byte */
    return (uint16_t)(fat[offset] | (((uint16_t)fat[offset + 1] & 0x0Fu) << 8));
}

void msxdisk_fat12_set(uint8_t *fat, uint16_t cluster, uint16_t value) {
    const size_t offset = (size_t)cluster + (size_t)(cluster / 2);
    value &= 0x0FFFu;

    if (cluster & 1u) {
        fat[offset] = (uint8_t)((fat[offset] & 0x0Fu) | (uint8_t)((value << 4) & 0xF0u));
        fat[offset + 1] = (uint8_t)(value >> 4);
    } else {
        fat[offset] = (uint8_t)(value & 0xFFu);
        fat[offset + 1] = (uint8_t)((fat[offset + 1] & 0xF0u) | (uint8_t)((value >> 8) & 0x0Fu));
    }
}
