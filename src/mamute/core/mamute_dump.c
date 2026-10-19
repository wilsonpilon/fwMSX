/* fwMSX -- Mamute: formatacao de despejos de memoria (C). Codigo ORIGINAL do fwMSX (BSD-3-Clause). */
#include "mamute_dump.h"

#include <stdio.h>

static const char kHex[] = "0123456789ABCDEF";

int mamute_dump_width(int mode) {
    switch (mode) {
    case 0: return 4;
    case 1: return 16;
    case 2: return 8;
    case 3: return 8;
    default: return 0;
    }
}

/* ASCII imprimivel ou '.', como o bloco de texto do DM do Mamute. */
static char printable(unsigned char b) { return (b >= 0x20 && b < 0x7F) ? (char)b : '.'; }

int mamute_dump_line(int mode, unsigned addr, const unsigned char *bytes, int n, char *out, int out_size) {
    const int width = mamute_dump_width(mode);
    int pos = 0, i;
    if (width == 0 || out_size < 8) {
        if (out_size > 0) out[0] = '\0';
        return 0;
    }
    if (n > width) n = width;
    /* o endereco sempre tem 4 digitos; o VRAM de 128/192 KB usa 5 (addr > FFFF) */
    pos += snprintf(out + pos, (size_t)(out_size - pos), addr > 0xFFFFu ? "%05X " : "%04X ", addr);
    pos += snprintf(out + pos, (size_t)(out_size - pos), " ");
    for (i = 0; i < width && pos < out_size - 4; ++i) {
        if (i > 0) out[pos++] = ' ';
        if (i < n) {
            out[pos++] = kHex[bytes[i] >> 4];
            out[pos++] = kHex[bytes[i] & 15];
        } else {
            out[pos++] = ' ';
            out[pos++] = ' ';
        }
    }
    if (mode == 0 || mode == 1) {
        if (pos < out_size - 3) {
            out[pos++] = ' ';
            out[pos++] = ' ';
        }
        for (i = 0; i < n && pos < out_size - 1; ++i) out[pos++] = printable(bytes[i]);
    } else {
        /* modos 2 e 3: o checksum fecha a linha (3 = so' a soma dos bytes; 2 = soma + byte baixo do endereco) */
        signed char tmp[16];
        int cs;
        for (i = 0; i < n && i < 16; ++i) tmp[i] = (signed char)bytes[i];
        cs = mamute_checksum8(tmp, n, (int)(addr & 0xFF), mode == 2);
        if (pos < out_size - 5) {
            out[pos++] = ' ';
            out[pos++] = ' ';
            out[pos++] = kHex[(cs >> 4) & 15];
            out[pos++] = kHex[cs & 15];
        }
    }
    while (pos > 0 && out[pos - 1] == ' ') --pos; /* sem espacos sobrando no fim */
    out[pos] = '\0';
    return pos;
}
