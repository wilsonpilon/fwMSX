/* fwMSX -- Mamute: busca com curinga (Assembly, mamute_scan.asm). Codigo ORIGINAL do fwMSX (BSD-3-Clause). */
#ifndef FWMSX_MAMUTE_SCAN_H
#define FWMSX_MAMUTE_SCAN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Menor indice i em `hay` tal que ((hay[i+k] ^ pat[k]) & mask[k]) == 0 para todo k < pat_len; -1 se nao ha'.
 * mask[k] = 0xFF exige o byte, 0x00 e' curinga. */
int mamute_find(const uint8_t *hay, int hay_len, const uint8_t *pat, const uint8_t *mask, int pat_len);

#ifdef __cplusplus
}
#endif

#endif
