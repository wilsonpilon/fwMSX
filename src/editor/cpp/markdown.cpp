// fwMSX -- editor de textos: o preview do Markdown (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver markdown.h.
#include "markdown.h"

#include <algorithm>
#include <cctype>

namespace ed {

namespace {

bool IsSpace(char c) { return c == ' ' || c == '\t'; }
bool IsAlnum(char c) { return std::isalnum(static_cast<unsigned char>(c)) != 0; }

std::string Trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && IsSpace(s[a])) ++a;
    while (b > a && (IsSpace(s[b - 1]) || s[b - 1] == '\r')) --b;
    return s.substr(a, b - a);
}

// Texto com a classe de cada byte.
struct Styled {
    std::string t;
    std::vector<uint8_t> c;
    void Add(char ch, uint8_t cls) {
        t.push_back(ch);
        c.push_back(cls);
    }
    void Add(const Styled &o) {
        t += o.t;
        c.insert(c.end(), o.c.begin(), o.c.end());
    }
    void Add(const std::string &s, uint8_t cls) {
        for (const char ch : s) Add(ch, cls);
    }
};

// Texto corrido de uma linha: tira as marcas (**, *, _, `, \) e troca links/imagens; `base` e' a classe do texto simples (a do titulo, da
// citacao...). Negrito/italico so' mudam a classe quando `base` e' o texto comum: dentro de um titulo valem as cores do titulo.
Styled Inline(const std::string &s, uint8_t base) {
    Styled out;
    const size_t n = s.size();
    bool bold = false, italic = false;
    auto cur = [&]() -> uint8_t {
        if (base != kMdText) return base;
        return bold ? kMdBold : (italic ? kMdItalic : kMdText);
    };
    size_t i = 0;
    while (i < n) {
        const char c = s[i];
        if (c == '`') { // codigo: o conteudo vai como esta'
            const size_t e = s.find('`', i + 1);
            if (e != std::string::npos) {
                out.Add(s.substr(i + 1, e - i - 1), kMdCode);
                i = e + 1;
                continue;
            }
        }
        if (c == '\\' && i + 1 < n && std::ispunct(static_cast<unsigned char>(s[i + 1]))) { // \* \_ \# ...
            out.Add(s[i + 1], cur());
            i += 2;
            continue;
        }
        if (c == '!' || c == '[') {
            const size_t open = c == '!' ? i + 1 : i;
            if (open < n && s[open] == '[') {
                const size_t close = s.find(']', open + 1);
                if (close != std::string::npos && close + 1 < n && s[close + 1] == '(') {
                    const size_t end = s.find(')', close + 2);
                    if (end != std::string::npos) {
                        const Styled label = Inline(s.substr(open + 1, close - open - 1), kMdLink);
                        const std::string url = s.substr(close + 2, end - close - 2);
                        if (c == '!') {
                            out.Add("[imagem: ", kMdImage);
                            for (size_t k = 0; k < label.t.size(); ++k) out.Add(label.t[k], kMdImage);
                            out.Add(']', kMdImage);
                        } else {
                            out.Add(label);
                            if (!url.empty() && url != label.t) {
                                out.Add(' ', cur());
                                out.Add("(" + url + ")", kMdUrl);
                            }
                        }
                        i = end + 1;
                        continue;
                    }
                }
            }
        }
        if (c == '*' || c == '_') { // negrito/italico: sequencias de 1, 2 ou 3 marcas; "a * b" solto e snake_case ficam
            size_t j = i;
            while (j < n && s[j] == c) ++j;
            const size_t run = j - i;
            const bool before = i > 0 && !IsSpace(s[i - 1]);
            const bool after = j < n && !IsSpace(s[j]);
            bool marker = before || after;
            if (c == '_') {
                const bool inside = i > 0 && j < n && IsAlnum(s[i - 1]) && IsAlnum(s[j]);
                marker = marker && !inside;
            }
            if (marker && run <= 3) {
                if (run == 1) italic = !italic;
                else if (run == 2) bold = !bold;
                else bold = italic = !bold;
                i = j;
                continue;
            }
        }
        out.Add(c, cur());
        ++i;
    }
    return out;
}

// Quebra `text` em linhas de ate' `width` colunas: a 1a com o prefixo `first`, as demais com `rest`; `pcls` e' a classe dos prefixos.
void Wrap(const Styled &text, int width, const std::string &first, const std::string &rest, uint8_t pcls, std::vector<MdLine> &out) {
    struct Word {
        size_t from, to;
    };
    std::vector<Word> words;
    size_t i = 0;
    while (i < text.t.size()) {
        while (i < text.t.size() && IsSpace(text.t[i])) ++i;
        size_t j = i;
        while (j < text.t.size() && !IsSpace(text.t[j])) ++j;
        if (j > i) words.push_back({i, j});
        i = j;
    }
    const size_t room = static_cast<size_t>(std::max(10, width));
    MdLine cur;
    auto start = [&](const std::string &prefix) {
        cur = MdLine();
        for (const char ch : prefix) {
            cur.text.push_back(ch);
            cur.cls.push_back(pcls);
        }
    };
    start(first);
    size_t base = first.size(); // o prefixo da linha atual
    bool empty = true;
    for (const Word &w : words) {
        const size_t len = w.to - w.from;
        if (!empty && cur.text.size() + 1 + len > room) {
            out.push_back(cur);
            start(rest);
            base = rest.size();
            empty = true;
        }
        if (!empty) {
            cur.text.push_back(' ');
            cur.cls.push_back(kMdText);
        }
        cur.text += text.t.substr(w.from, len);
        cur.cls.insert(cur.cls.end(), text.c.begin() + static_cast<std::ptrdiff_t>(w.from), text.c.begin() + static_cast<std::ptrdiff_t>(w.to));
        empty = false;
    }
    (void)base;
    if (!empty || words.empty()) out.push_back(cur);
}

MdLine Plain(const std::string &s, uint8_t cls) {
    MdLine l;
    l.text = s;
    l.cls.assign(s.size(), cls);
    return l;
}

struct Para {
    std::string first, rest, text;
    uint8_t pcls = kMdText, tcls = kMdText;
    bool active = false;
};

// ---- reconhecimento de linhas ----

bool IsHr(const std::string &t) {
    if (t.size() < 3) return false;
    const char c = t[0];
    if (c != '-' && c != '*' && c != '_') return false;
    int n = 0;
    for (const char x : t) {
        if (x == c) ++n;
        else if (!IsSpace(x)) return false;
    }
    return n >= 3;
}

bool Heading(const std::string &t, int &level, std::string &text) {
    size_t i = 0;
    while (i < t.size() && t[i] == '#') ++i;
    if (i == 0 || i > 6 || (i < t.size() && t[i] != ' ')) return false;
    level = static_cast<int>(i);
    text = Trim(t.substr(i));
    while (!text.empty() && text.back() == '#') text.pop_back(); // "## Titulo ##"
    text = Trim(text);
    return true;
}

// "- item", "* item", "+ item", "1. item", "1) item": devolve o recuo, o marcador e o texto
bool ListItem(const std::string &raw, int &indent, std::string &marker, std::string &text) {
    size_t i = 0;
    while (i < raw.size() && IsSpace(raw[i])) ++i;
    indent = static_cast<int>(i);
    if (i + 1 < raw.size() && (raw[i] == '-' || raw[i] == '*' || raw[i] == '+') && IsSpace(raw[i + 1])) {
        marker = indent >= 2 ? "-" : "*";
        text = Trim(raw.substr(i + 2));
        return true;
    }
    size_t j = i;
    while (j < raw.size() && std::isdigit(static_cast<unsigned char>(raw[j]))) ++j;
    if (j > i && j + 1 < raw.size() && (raw[j] == '.' || raw[j] == ')') && IsSpace(raw[j + 1])) {
        marker = raw.substr(i, j - i + 1);
        text = Trim(raw.substr(j + 2));
        return true;
    }
    return false;
}

} // namespace

std::string MarkdownInline(const std::string &s) { return Inline(s, kMdText).t; }

std::vector<MdLine> RenderMarkdownStyled(const std::vector<std::string> &lines, int width) {
    width = std::max(20, width);
    std::vector<MdLine> out;
    Para para;
    bool fence = false;

    auto blank = [&]() {
        if (!out.empty() && !out.back().text.empty()) out.push_back(MdLine());
    };
    auto flush = [&]() {
        if (para.active) Wrap(Inline(para.text, para.tcls), width, para.first, para.rest, para.pcls, out);
        para = Para();
    };
    auto start = [&](const std::string &first, const std::string &rest, const std::string &text, uint8_t pcls, uint8_t tcls) {
        flush();
        para.first = first;
        para.rest = rest;
        para.text = text;
        para.pcls = pcls;
        para.tcls = tcls;
        para.active = true;
    };

    for (const std::string &raw : lines) {
        const std::string t = Trim(raw);
        if (t.rfind("```", 0) == 0 || t.rfind("~~~", 0) == 0) {
            flush();
            if (!fence) blank();
            fence = !fence;
            continue;
        }
        if (fence) {
            std::string l = "    " + raw;
            while (!l.empty() && l.back() == '\r') l.pop_back();
            if (static_cast<int>(l.size()) > width) l.resize(static_cast<size_t>(width));
            out.push_back(Plain(l, kMdBlock));
            continue;
        }
        if (t.empty()) {
            flush();
            blank();
            continue;
        }
        int level = 0;
        std::string text;
        if (Heading(t, level, text)) {
            flush();
            blank();
            const uint8_t hc = level == 1 ? kMdH1 : (level == 2 ? kMdH2 : kMdH3);
            Styled h = Inline(text, hc);
            if (level == 1)
                for (char &ch : h.t) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
            MdLine hl;
            hl.text = (level >= 3 ? std::string(static_cast<size_t>(level - 2), ' ') : std::string()) + h.t;
            hl.cls.assign(hl.text.size() - h.t.size(), hc);
            hl.cls.insert(hl.cls.end(), h.c.begin(), h.c.end());
            out.push_back(hl);
            if (level <= 2) out.push_back(Plain(std::string(std::min<size_t>(h.t.size(), static_cast<size_t>(width)), level == 1 ? '=' : '-'), hc));
            out.push_back(MdLine());
            continue;
        }
        if (IsHr(t)) {
            flush();
            out.push_back(Plain(std::string(static_cast<size_t>(width), '-'), kMdRule));
            continue;
        }
        if (t[0] == '|') { // tabela: as linhas seguem como estao; a de "---|---" vira um traco
            flush();
            bool sep = true;
            for (const char c : t)
                if (c != '|' && c != '-' && c != ':' && c != ' ') sep = false;
            if (sep) {
                out.push_back(Plain(std::string(std::min<size_t>(t.size(), static_cast<size_t>(width)), '-'), kMdRule));
            } else {
                const Styled row = Inline(t, kMdTable);
                MdLine l;
                l.text = row.t;
                l.cls = row.c;
                out.push_back(l);
            }
            continue;
        }
        if (t[0] == '>') {
            std::string q = t.substr(1);
            if (!q.empty() && q[0] == ' ') q.erase(0, 1);
            if (para.active && para.first == "| ") para.text += " " + q;
            else start("| ", "| ", q, kMdQuote, kMdQuote);
            continue;
        }
        int indent = 0;
        std::string marker, item;
        if (ListItem(raw, indent, marker, item)) {
            const std::string pad(static_cast<size_t>(std::min(indent, 12)), ' ');
            start(pad + marker + " ", pad + std::string(marker.size() + 1, ' '), item, kMdBullet, kMdText);
            continue;
        }
        if (para.active) para.text += " " + t; // continuacao do paragrafo, do item ou da citacao
        else start(std::string(), std::string(), t, kMdText, kMdText);
    }
    flush();
    while (!out.empty() && out.back().text.empty()) out.pop_back();
    return out;
}

std::vector<std::string> RenderMarkdown(const std::vector<std::string> &lines, int width) {
    std::vector<std::string> out;
    for (const MdLine &l : RenderMarkdownStyled(lines, width)) out.push_back(l.text);
    return out;
}

} // namespace ed
