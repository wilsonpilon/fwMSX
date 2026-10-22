/* fwMSX -- downloads: comparacao de texto sem diferenciar maiusculas. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
 * A busca no indice do File Hunter passa por ~59 mil caminhos a cada tecla digitada; em C simples e sem alocar. */
#include "dl_match.h"

static int fold(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }

int dl_contains_ci(const char *hay, int hlen, const char *needle, int nlen) {
    int i, j;
    if (nlen <= 0) return 1;
    if (hlen < nlen) return 0;
    for (i = 0; i + nlen <= hlen; ++i) {
        if (fold((unsigned char)hay[i]) != fold((unsigned char)needle[0])) continue;
        for (j = 1; j < nlen; ++j)
            if (fold((unsigned char)hay[i + j]) != fold((unsigned char)needle[j])) break;
        if (j == nlen) return 1;
    }
    return 0;
}
