// fwMSX -- obi: o parser do Obifile (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/obi-spec.md.
#include "obi.h"

#include <cctype>

namespace obi {

namespace {

struct Line {
    int indent = 0;
    std::string text;
    int num = 0;
};

std::string Trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// "campo: valor"; o valor pode vir entre aspas.
bool SplitKV(const std::string &text, std::string &key, std::string &value) {
    const size_t c = text.find(':');
    if (c == std::string::npos) return false;
    key = Trim(text.substr(0, c));
    value = Trim(text.substr(c + 1));
    if (value.size() >= 2 && (value.front() == '"' || value.front() == '\'') && value.back() == value.front()) value = value.substr(1, value.size() - 2);
    return !key.empty();
}

// So' linhas inteiras de comentario (#): um '#' no meio de um caminho ou nome vale como texto.
std::vector<Line> Tokenize(const std::string &content) {
    std::vector<Line> out;
    int num = 0;
    size_t pos = 0;
    while (pos <= content.size()) {
        const size_t nl = content.find('\n', pos);
        std::string raw = content.substr(pos, nl == std::string::npos ? std::string::npos : nl - pos);
        pos = nl == std::string::npos ? content.size() + 1 : nl + 1;
        ++num;
        while (!raw.empty() && (raw.back() == '\r' || raw.back() == ' ' || raw.back() == '\t')) raw.pop_back();
        size_t ind = 0;
        while (ind < raw.size() && raw[ind] == ' ') ++ind;
        const std::string t = raw.substr(ind);
        if (Trim(t).empty() || t[0] == '#') continue;
        out.push_back({static_cast<int>(ind), t, num});
    }
    return out;
}

bool Fail(std::string &err, int line, const std::string &msg) {
    err = "Obifile, line " + std::to_string(line) + ": " + msg;
    return false;
}

bool ParseBool(const std::string &v, bool &out) {
    const std::string l = Lower(v);
    if (l == "true" || l == "yes" || l == "sim" || l == "1") out = true;
    else if (l == "false" || l == "no" || l == "nao" || l == "0") out = false;
    else return false;
    return true;
}

// Um campo de um item da lista `files:`.
bool SetFileField(FileSpec &f, const std::string &key, const std::string &value, std::string &err, int line) {
    const std::string k = Lower(key);
    if (k == "source") f.source = value;
    else if (k == "name") f.name = value;
    else return Fail(err, line, "unknown file field '" + k + "'");
    return true;
}

} // namespace

bool ParseRecipe(const std::string &text, Recipe &out, std::string &err) {
    out = Recipe();
    const std::vector<Line> lines = Tokenize(text);
    size_t i = 0;
    while (i < lines.size()) {
        const Line &ln = lines[i];
        if (ln.indent != 0) return Fail(err, ln.num, "unexpected indentation");
        std::string key, value;
        if (!SplitKV(ln.text, key, value)) return Fail(err, ln.num, "expected 'field: value', got '" + ln.text + "'");
        key = Lower(key);
        ++i;
        if (key == "target") {
            if (value.empty()) return Fail(err, ln.num, "'target' cannot be empty");
            out.target = value;
        } else if (key == "boot") {
            out.boot = value;
        } else if (key == "dos") {
            if (value == "1" || value == "2") {
                out.dos_version = value[0] - '0';
                out.dos = true;
            } else if (!ParseBool(value, out.dos)) {
                return Fail(err, ln.num, "'dos' must be 1, 2, true or false");
            }
        } else if (key == "dos_tools") {
            if (!ParseBool(value, out.dos_tools)) return Fail(err, ln.num, "'dos_tools' must be true or false");
        } else if (key == "dos_disk") {
            out.dos_disk = value;
        } else if (key == "files") {
            // itens "  - source: x" com campos extras indentados mais fundo; tambem vale a forma curta "  - x.dat"
            int item_indent = -1;
            while (i < lines.size() && lines[i].indent != 0) {
                const Line &it = lines[i];
                const bool dash = it.text == "-" || it.text.rfind("- ", 0) == 0;
                if (dash) {
                    if (item_indent < 0) item_indent = it.indent;
                    else if (it.indent != item_indent) return Fail(err, it.num, "inconsistent indentation in the list");
                    out.files.emplace_back();
                    out.files.back().line = it.num;
                    const std::string rest = Trim(it.text.substr(1));
                    if (!rest.empty()) {
                        std::string k, v;
                        if (SplitKV(rest, k, v)) {
                            if (!SetFileField(out.files.back(), k, v, err, it.num)) return false;
                        } else {
                            out.files.back().source = rest;
                        }
                    }
                } else {
                    if (out.files.empty() || item_indent < 0) return Fail(err, it.num, "field outside a list item (expected '- source: file')");
                    std::string k, v;
                    if (!SplitKV(it.text, k, v)) return Fail(err, it.num, "expected 'field: value'");
                    if (!SetFileField(out.files.back(), k, v, err, it.num)) return false;
                }
                ++i;
            }
        } else {
            return Fail(err, ln.num, "unknown key '" + key + "'");
        }
    }
    if (out.target.empty()) {
        err = "Obifile: required field 'target' is missing";
        return false;
    }
    for (const FileSpec &f : out.files) {
        if (f.source.empty()) return Fail(err, f.line, "file item without 'source'");
    }
    if (out.files.empty()) {
        err = "Obifile: no files listed (files:)";
        return false;
    }
    return true;
}

} // namespace obi
