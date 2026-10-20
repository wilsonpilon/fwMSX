// fwMSX -- editor de textos: nucleo de varredura do destaque de sintaxe (C). Ver ed_syntax.h.
#include "ed_syntax.h"

static int is_digit(int c) {
    return c >= '0' && c <= '9';
}

static int is_hex(int c) {
    return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int ed_hl_ident_start(int c) {
    c &= 0xFF;
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c >= 128;
}

int ed_hl_ident_char(int c) {
    return ed_hl_ident_start(c) || is_digit(c & 0xFF);
}

int ed_hl_ident_end(const char *s, int len, int pos, int flags) {
    int i = pos;
    if (i >= len || !ed_hl_ident_start((unsigned char)s[i])) return pos;
    while (i < len) {
        const unsigned char c = (unsigned char)s[i];
        if (ed_hl_ident_char(c)) {
            ++i;
        } else if ((flags & ED_ID_DOT) && c == '.' && i + 1 < len && ed_hl_ident_char((unsigned char)s[i + 1])) {
            ++i;
        } else if ((flags & ED_ID_AT) && c == '@') {
            ++i;
        } else {
            break;
        }
    }
    if ((flags & ED_ID_SUFFIX) && i < len && (s[i] == '$' || s[i] == '%' || s[i] == '!' || s[i] == '#')) ++i;
    if ((flags & ED_ID_QUOTE) && i < len && s[i] == '\'') ++i;
    return i;
}

int ed_hl_number_end(const char *s, int len, int pos) {
    int i = pos;
    if (i >= len) return pos;
    const unsigned char c = (unsigned char)s[i];
    // &H1F  &B101  &O17  &17
    if (c == '&' && i + 1 < len) {
        const char k = s[i + 1];
        int j = i + 1;
        if (k == 'H' || k == 'h' || k == 'B' || k == 'b' || k == 'O' || k == 'o') ++j;
        const int start = j;
        while (j < len && is_hex((unsigned char)s[j])) ++j;
        return j > start ? j : pos;
    }
    // $FF  #FF  %101 -- so' quando vem logo um digito valido
    if ((c == '$' || c == '#' || c == '%') && i + 1 < len && is_hex((unsigned char)s[i + 1])) {
        int j = i + 1;
        while (j < len && is_hex((unsigned char)s[j])) ++j;
        return j;
    }
    // .5
    if (c == '.' && i + 1 < len && is_digit((unsigned char)s[i + 1])) {
        int j = i + 1;
        while (j < len && is_digit((unsigned char)s[j])) ++j;
        return j;
    }
    if (!is_digit(c)) return pos;
    // 0x1F  123  12.5  1E5  1E+5  0FFh  101b
    int j = i + 1;
    if (c == '0' && j < len && (s[j] == 'x' || s[j] == 'X')) {
        ++j;
        while (j < len && is_hex((unsigned char)s[j])) ++j;
        return j;
    }
    for (;;) {
        if (j >= len) break;
        const unsigned char d = (unsigned char)s[j];
        if (is_hex(d) || d == 'h' || d == 'H') { // digitos, e hexa/expoente/sufixo juntos (a classificacao fina e' do Fortran)
            ++j;
        } else if (d == '.' && j + 1 < len && is_digit((unsigned char)s[j + 1])) {
            ++j;
        } else if ((d == '+' || d == '-') && (s[j - 1] == 'E' || s[j - 1] == 'e') && j + 1 < len && is_digit((unsigned char)s[j + 1])) {
            ++j;
        } else if (d == '.' && j == i + 1) { // "1."
            ++j;
        } else {
            break;
        }
    }
    // sufixos de tipo do BASIC
    if (j < len && (s[j] == '!' || s[j] == '#' || s[j] == '%')) ++j;
    return j;
}

int ed_hl_string_end(const char *s, int len, int pos, char quote) {
    int i = pos + 1;
    while (i < len) {
        if (s[i] == quote) {
            if (i + 1 < len && s[i + 1] == quote) {
                i += 2;
                continue;
            }
            return i + 1;
        }
        ++i;
    }
    return len;
}
