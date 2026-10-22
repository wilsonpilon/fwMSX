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

// Quebra `text` em linhas de ate' `width` colunas: a 1a com `first` na frente, as demais com `rest`.
void Wrap(const std::string &text, int width, const std::string &first, const std::string &rest, std::vector<std::string> &out) {
    std::vector<std::string> words;
    size_t i = 0;
    while (i < text.size()) {
        while (i < text.size() && IsSpace(text[i])) ++i;
        size_t j = i;
        while (j < text.size() && !IsSpace(text[j])) ++j;
        if (j > i) words.push_back(text.substr(i, j - i));
        i = j;
    }
    std::string cur = first;
    bool empty = true;
    for (const std::string &w : words) {
        const size_t room = static_cast<size_t>(std::max(10, width));
        if (!empty && cur.size() + 1 + w.size() > room) {
            out.push_back(cur);
            cur = rest;
            empty = true;
        }
        if (!empty) cur.push_back(' ');
        cur += w;
        empty = false;
    }
    if (!empty || words.empty()) out.push_back(cur);
}

struct Para {
    std::string first, rest, text;
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

std::string MarkdownInline(const std::string &s) {
    std::string out;
    const size_t n = s.size();
    size_t i = 0;
    while (i < n) {
        const char c = s[i];
        if (c == '`') { // codigo: o conteudo vai como esta'
            const size_t e = s.find('`', i + 1);
            if (e != std::string::npos) {
                out += s.substr(i + 1, e - i - 1);
                i = e + 1;
                continue;
            }
        }
        if (c == '\\' && i + 1 < n && std::ispunct(static_cast<unsigned char>(s[i + 1]))) { // \* \_ \# ...
            out.push_back(s[i + 1]);
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
                        const std::string label = MarkdownInline(s.substr(open + 1, close - open - 1));
                        const std::string url = s.substr(close + 2, end - close - 2);
                        out += c == '!' ? "[imagem: " + label + "]" : (url.empty() || url == label ? label : label + " (" + url + ")");
                        i = end + 1;
                        continue;
                    }
                }
            }
        }
        if (c == '*') { // negrito/italico: some com os asteriscos, menos o "a * b" solto
            const bool before = i > 0 && !IsSpace(s[i - 1]);
            const bool after = i + 1 < n && !IsSpace(s[i + 1]);
            if (before || after) {
                ++i;
                continue;
            }
        }
        if (c == '_') { // so' nas bordas da palavra: snake_case fica
            const bool inside = i > 0 && i + 1 < n && IsAlnum(s[i - 1]) && IsAlnum(s[i + 1]);
            const bool lone = (i == 0 || IsSpace(s[i - 1])) && (i + 1 >= n || IsSpace(s[i + 1]));
            if (!inside && !lone) {
                ++i;
                continue;
            }
        }
        out.push_back(c);
        ++i;
    }
    return out;
}

std::vector<std::string> RenderMarkdown(const std::vector<std::string> &lines, int width) {
    width = std::max(20, width);
    std::vector<std::string> out;
    Para para;
    bool fence = false;

    auto blank = [&]() {
        if (!out.empty() && !out.back().empty()) out.push_back(std::string());
    };
    auto flush = [&]() {
        if (para.active) Wrap(MarkdownInline(para.text), width, para.first, para.rest, out);
        para = Para();
    };
    auto start = [&](const std::string &first, const std::string &rest, const std::string &text) {
        flush();
        para.first = first;
        para.rest = rest;
        para.text = text;
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
            out.push_back(l);
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
            const std::string h = MarkdownInline(text);
            if (level == 1) {
                std::string up = h;
                for (char &c : up) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                out.push_back(up);
                out.push_back(std::string(std::min<size_t>(up.size(), static_cast<size_t>(width)), '='));
            } else if (level == 2) {
                out.push_back(h);
                out.push_back(std::string(std::min<size_t>(h.size(), static_cast<size_t>(width)), '-'));
            } else {
                out.push_back(std::string(static_cast<size_t>(level - 2), ' ') + h);
            }
            out.push_back(std::string());
            continue;
        }
        if (IsHr(t)) {
            flush();
            out.push_back(std::string(static_cast<size_t>(width), '-'));
            continue;
        }
        if (t[0] == '|') { // tabela: as linhas seguem como estao; a de "---|---" vira um traco
            flush();
            bool sep = true;
            for (const char c : t)
                if (c != '|' && c != '-' && c != ':' && c != ' ') sep = false;
            out.push_back(sep ? std::string(static_cast<size_t>(std::min<size_t>(t.size(), static_cast<size_t>(width))), '-') : MarkdownInline(t));
            continue;
        }
        if (t[0] == '>') {
            std::string q = t.substr(1);
            if (!q.empty() && q[0] == ' ') q.erase(0, 1);
            if (para.active && para.first == "| ") para.text += " " + q;
            else start("| ", "| ", q);
            continue;
        }
        int indent = 0;
        std::string marker, item;
        if (ListItem(raw, indent, marker, item)) {
            const std::string pad(static_cast<size_t>(std::min(indent, 12)), ' ');
            start(pad + marker + " ", pad + std::string(marker.size() + 1, ' '), item);
            continue;
        }
        if (para.active) para.text += " " + t; // continuacao do paragrafo, do item ou da citacao
        else start(std::string(), std::string(), t);
    }
    flush();
    while (!out.empty() && out.back().empty()) out.pop_back();
    return out;
}

} // namespace ed
