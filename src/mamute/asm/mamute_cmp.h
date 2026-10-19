/* fwMSX -- Mamute: comparacao de blocos (Assembly, mamute_cmp.asm). Codigo ORIGINAL do fwMSX (BSD-3-Clause). */
#ifndef FWMSX_MAMUTE_CMP_H
#define FWMSX_MAMUTE_CMP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Compara a[0..n) com b[0..n). want_equal = 0 lista os indices em que os bytes DIFEREM, != 0 os que BATEM. Os primeiros `max`
 * indices vao para out[] (max pode ser 0); o retorno e' o TOTAL de indices que casam, mesmo alem de `max`. */
int mamute_diff(const uint8_t *a, const uint8_t *b, int n, int want_equal, int *out, int max);

#ifdef __cplusplus
}
#endif

#endif
