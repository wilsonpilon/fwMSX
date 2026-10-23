/* fwMSX -- tokenizador minimo de XML. Ver xml_scan.h. */
#include "xml_scan.h"

#include <string.h>

static int is_space(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }

static const char *find_str(const char *p, const char *end, const char *needle) {
    size_t n = strlen(needle);
    for (; p + n <= end; ++p) {
        if (memcmp(p, needle, n) == 0) return p;
    }
    return NULL;
}

int xml_next(XmlScan *s, XmlTok *tok) {
    const char *p = s->pos;
    const char *end = s->end;
    memset(tok, 0, sizeof(*tok));
    if (s->pend_name) { /* o CLOSE de uma tag vazia (<a/>) */
        tok->kind = XML_CLOSE;
        tok->name = s->pend_name;
        tok->name_len = s->pend_len;
        s->pend_name = NULL;
        return XML_CLOSE;
    }
    for (;;) {
        if (p >= end) {
            s->pos = end;
            return XML_END;
        }
        if (*p != '<') { /* texto ate a proxima tag */
            const char *ts = p;
            while (p < end && *p != '<') ++p;
            const char *te = p;
            while (ts < te && is_space(*ts)) ++ts;
            while (te > ts && is_space(te[-1])) --te;
            if (te > ts) {
                tok->kind = XML_TEXT;
                tok->text = ts;
                tok->text_len = (int)(te - ts);
                s->pos = p;
                return XML_TEXT;
            }
            continue;
        }
        if (p + 4 <= end && memcmp(p, "<!--", 4) == 0) {
            const char *c = find_str(p + 4, end, "-->");
            p = c ? c + 3 : end;
            continue;
        }
        if (p + 2 <= end && (p[1] == '?' || p[1] == '!')) { /* <?xml ...?> e <!DOCTYPE ...> */
            while (p < end && *p != '>') ++p;
            if (p < end) ++p;
            continue;
        }
        break;
    }
    /* uma tag */
    ++p; /* '<' */
    int closing = 0;
    if (p < end && *p == '/') {
        closing = 1;
        ++p;
    }
    const char *name = p;
    while (p < end && !is_space(*p) && *p != '>' && *p != '/') ++p;
    int name_len = (int)(p - name);
    const char *attrs = p;
    const char *q = p;
    char quote = 0; /* um '>' dentro de aspas nao fecha a tag */
    while (q < end && (quote || *q != '>')) {
        if (quote) {
            if (*q == quote) quote = 0;
        } else if (*q == '"' || *q == '\'') {
            quote = *q;
        }
        ++q;
    }
    const char *tag_end = q; /* aponta o '>' */
    int attrs_len = (int)(tag_end - attrs);
    int empty = 0;
    while (attrs_len > 0 && is_space(attrs[attrs_len - 1])) --attrs_len;
    if (attrs_len > 0 && attrs[attrs_len - 1] == '/') {
        empty = 1;
        --attrs_len;
    }
    s->pos = tag_end < end ? tag_end + 1 : end;
    tok->name = name;
    tok->name_len = name_len;
    if (closing) {
        tok->kind = XML_CLOSE;
        return XML_CLOSE;
    }
    tok->kind = XML_OPEN;
    tok->attrs = attrs;
    tok->attrs_len = attrs_len;
    if (empty) {
        s->pend_name = name;
        s->pend_len = name_len;
    }
    return XML_OPEN;
}

int xml_attr(const char *attrs, int attrs_len, const char *key, char *out, int cap) {
    size_t klen = strlen(key);
    const char *p = attrs, *end = attrs + attrs_len;
    while (p < end) {
        while (p < end && is_space(*p)) ++p;
        const char *k = p;
        while (p < end && *p != '=' && !is_space(*p)) ++p;
        size_t len = (size_t)(p - k);
        while (p < end && is_space(*p)) ++p;
        if (p >= end || *p != '=') continue;
        ++p;
        while (p < end && is_space(*p)) ++p;
        if (p >= end || (*p != '"' && *p != '\'')) continue;
        char quote = *p++;
        const char *v = p;
        while (p < end && *p != quote) ++p;
        if (len == klen && memcmp(k, key, klen) == 0) {
            int n = (int)(p - v);
            if (n > cap - 1) n = cap - 1;
            if (n < 0) n = 0;
            memcpy(out, v, (size_t)n);
            out[n] = '\0';
            return n;
        }
        if (p < end) ++p;
    }
    if (cap > 0) out[0] = '\0';
    return -1;
}
