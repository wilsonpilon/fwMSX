// fwMSX -- MAN/HELP: quebra de linhas longas em colunas (C). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "man_wrap.h"

int man_wrap_break(const char *s, int len, int width) {
    int i, last_space = -1;
    if (len <= 0) return 0;
    if (width < 1) width = 1;
    for (i = 0; i < len && i < width; ++i) {
        if (s[i] == '\n') return i == 0 ? 0 : i; // linha vazia: o chamador pula so' o '\n'
        if (s[i] == ' ' && i > 0) last_space = i;
    }
    if (i >= len) return len;                 // cabe tudo
    if (s[i] == '\n' || s[i] == ' ') return i; // a linha acaba exatamente na coluna
    if (last_space > 0) return last_space;     // quebra na ultima palavra inteira
    return width;                              // palavra maior que a linha: corta
}
