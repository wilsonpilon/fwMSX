// fwMSX -- editor de textos: nucleo de varredura de linha (C). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver ed_text.h e doc/editor-spec.md.
#include "ed_text.h"

int ed_is_word_char(int c) {
    c &= 0xFF;
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c >= 128;
}

int ed_word_left(const char *s, int len, int col) {
    if (col > len) col = len;
    while (col > 0 && !ed_is_word_char((unsigned char)s[col - 1])) --col;
    while (col > 0 && ed_is_word_char((unsigned char)s[col - 1])) --col;
    return col;
}

int ed_word_right(const char *s, int len, int col) {
    if (col < 0) col = 0;
    while (col < len && ed_is_word_char((unsigned char)s[col])) ++col;
    while (col < len && !ed_is_word_char((unsigned char)s[col])) ++col;
    return col;
}

static int fold(int c, int match_case) {
    if (!match_case && c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

static int match_at(const char *hay, const char *nd, int nlen, int at, int match_case) {
    for (int k = 0; k < nlen; ++k) {
        if (fold((unsigned char)hay[at + k], match_case) != fold((unsigned char)nd[k], match_case)) return 0;
    }
    return 1;
}

int ed_find(const char *hay, int hlen, const char *nd, int nlen, int start, int fwd, int match_case) {
    if (nlen <= 0 || nlen > hlen) return -1;
    const int last = hlen - nlen;
    if (fwd) {
        for (int i = start < 0 ? 0 : start; i <= last; ++i) {
            if (match_at(hay, nd, nlen, i, match_case)) return i;
        }
    } else {
        for (int i = start > last ? last : start; i >= 0; --i) {
            if (match_at(hay, nd, nlen, i, match_case)) return i;
        }
    }
    return -1;
}

void ed_flip_case(char *s, int from, int to) {
    for (int i = from; i < to; ++i) {
        const char c = s[i];
        if (c >= 'a' && c <= 'z') {
            s[i] = (char)(c - ('a' - 'A'));
        } else if (c >= 'A' && c <= 'Z') {
            s[i] = (char)(c + ('a' - 'A'));
        }
    }
}
