/* fwMSX -- obi: nomes 8.3 do MSX-DOS e o tipo de um fonte pela extensao. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/obi-spec.md.
 *
 * Em C porque e' texto byte a byte sem estado, o tipo de rotina que o resto do projeto (prj_kind.c, ed_text.c) ja' deixa em C. */
#include "obi_name.h"

#include <string.h>

static int up(int c) { return (c >= 'a' && c <= 'z') ? c - 32 : c; }

/* os caracteres que o MSX-DOS 1 nao deixa num nome: controle, espaco e  " * + , / : ; < = > ? \ [ ] | */
static int bad_char(int c) {
    if (c <= 32 || c >= 127) return 1;
    return strchr("\"*+,/:;<=>?\[]|", c) != NULL;
}

int obi_msx_name(const char *in, char *out, int cap) {
    int base = 0, ext = 0, in_ext = 0;
    char tmp[13];
    if (cap > 0) out[0] = 0;
    if (!in || !*in) return OBI_NAME_EMPTY;
    for (const char *p = in; *p; ++p) {
        if (*p == '.' && !in_ext && p != in) {
            in_ext = 1;
            tmp[base++] = '.';
            if (base > 9) return OBI_NAME_BASE_LONG;
            continue;
        }
        if (bad_char((unsigned char)*p) || *p == '.') return OBI_NAME_BAD_CHAR;
        if (in_ext) {
            if (++ext > 3) return OBI_NAME_EXT_LONG;
        } else if (base > 7) {
            return OBI_NAME_BASE_LONG;
        }
        if (base >= 12) return OBI_NAME_BASE_LONG;
        tmp[base++] = (char)up((unsigned char)*p);
    }
    if (base == 0 || tmp[0] == '.') return OBI_NAME_EMPTY;
    if (in_ext && ext == 0) return OBI_NAME_BAD_CHAR; /* "ABC." */
    if (cap < base + 1) return OBI_NAME_BASE_LONG;
    memcpy(out, tmp, (size_t)base);
    out[base] = 0;
    return OBI_NAME_OK;
}

int obi_kind(const char *path) {
    const char *dot = strrchr(path, '.');
    char e[8];
    size_t n, i;
    if (!dot) return OBI_KIND_RAW;
    n = strlen(dot + 1);
    if (n == 0 || n >= sizeof e) return OBI_KIND_RAW;
    for (i = 0; i < n; ++i) e[i] = (char)up((unsigned char)dot[1 + i]);
    e[n] = 0;
    if (strcmp(e, "DMX") == 0) return OBI_KIND_DMX;
    if (strcmp(e, "ASM") == 0) return OBI_KIND_ASM;
    return OBI_KIND_RAW;
}
