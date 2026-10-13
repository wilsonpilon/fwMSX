// fwMSX -- montador (fwasm): pre-processador de fonte. Ver preproc.h.
#include "preproc.h"

#include <algorithm>
#include <cctype>
#include <map>

namespace fwasm {

namespace {

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string Trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r')) --b;
    return s.substr(a, b - a);
}

bool IsIdStart(char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_'; }
bool IsIdChar(char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }

std::string DirOf(const std::string &path) {
    const size_t p = path.find_last_of("/\\");
    return p == std::string::npos ? std::string() : path.substr(0, p + 1);
}

// "file, line N: msg"
std::string Where(const std::string &file, int line, const std::string &msg) {
    return file + ", line " + std::to_string(line) + ": " + msg;
}

struct Pre {
    FileProvider *files = nullptr;
    PreResult out;

    void Fail(int rc, const std::string &file, int line, const std::string &msg, bool syntax = false) {
        if (out.rc != 0) return;
        out.rc = rc;
        out.error = Where(file, line, msg);
        out.syntax_error = syntax;
    }

    // ---- 1: leitura, comentarios e INCLUDE ----
    std::vector<std::string> include_stack;

    void Read(const std::string &name, const std::string &text, int depth, std::vector<SrcLine> &dst) {
        if (depth > 32) return;
        if (depth == 0) include_stack.assign(1, name);
        int block = 0;
        int block_line = 0;
        int no = 0;
        size_t pos = 0;
        while (pos <= text.size() && out.rc == 0) {
            size_t nl = text.find('\n', pos);
            const bool last = nl == std::string::npos;
            std::string raw = text.substr(pos, last ? std::string::npos : nl - pos);
            if (!raw.empty() && raw.back() == '\r') raw.pop_back();
            ++no;
            if (last && raw.empty()) break;
            pos = last ? text.size() + 1 : nl + 1;
            const int block_before = block;
            const std::string stripped = StripComments(raw, block);
            if (block_before == 0 && block == 1) block_line = no;
            const std::string t = Trim(stripped);
            // include "arquivo"
            if (t.size() > 7) {
                size_t k = 0;
                if (t[0] == '.') k = 1;
                if (Lower(t.substr(k, 7)) == "include" && (t.size() == k + 7 || t[k + 7] == ' ' || t[k + 7] == '\t' || t[k + 7] == '"')) {
                    const size_t q1 = t.find('"', k + 7);
                    const size_t q2 = q1 == std::string::npos ? std::string::npos : t.find('"', q1 + 1);
                    if (q2 == std::string::npos) {
                        Fail(1, name, no, "Unterminated string");
                        return;
                    }
                    if (q1 != std::string::npos && q2 != std::string::npos && Trim(t.substr(q2 + 1)).empty()) {
                        const std::string inc = t.substr(q1 + 1, q2 - q1 - 1);
                        std::vector<uint8_t> data;
                        const std::string full = (inc.find(':') != std::string::npos || (!inc.empty() && (inc[0] == '/' || inc[0] == '\\')))
                                                      ? inc
                                                      : DirOf(name) + inc;
                        if (std::find(include_stack.begin(), include_stack.end(), full) != include_stack.end()) {
                            Fail(4, name, no, "Recursive include");
                            return;
                        }
                        if (!files || !files->Read(full, data)) {
                            Fail(3, name, no, "Include file not found");
                            return;
                        }
                        include_stack.push_back(full);
                        Read(full, std::string(data.begin(), data.end()), depth + 1, dst);
                        include_stack.pop_back();
                        continue;
                    }
                }
            }
            dst.push_back({name, no, stripped, t != Trim(raw)});
        }
        if (block == 1 && out.rc == 0) Fail(1, name, block_line, "syntax error", true); // /* sem */
    }

    // ---- 2: macros ----
    struct Macro {
        std::string name;
        std::vector<std::string> params; // sem o '#'
        std::vector<std::string> body;
        int line = 0;
    };

    static std::vector<std::string> SplitArgs(const std::string &s) {
        std::vector<std::string> args;
        std::string cur;
        int depth = 0;
        bool in_str = false;
        for (size_t i = 0; i < s.size(); ++i) {
            const char c = s[i];
            if (in_str) {
                cur += c;
                if (c == '"') in_str = false;
                continue;
            }
            if (c == '"') {
                in_str = true;
                cur += c;
            } else if (c == '(' || c == '[') {
                ++depth;
                cur += c;
            } else if (c == ')' || c == ']') {
                --depth;
                cur += c;
            } else if (c == ',' && depth == 0) {
                args.push_back(Trim(cur));
                cur.clear();
            } else {
                cur += c;
            }
        }
        if (!Trim(cur).empty() || !args.empty()) args.push_back(Trim(cur));
        return args;
    }

    // Troca textual, um parametro por vez e em ordem (sem respeitar limite de identificador: "#a" tambem
    // atinge o prefixo de "#ab", como no asMSX). Argumento ausente vale "10".
    static std::string Substitute(const std::string &line, const std::vector<std::string> &params, const std::vector<std::string> &args) {
        std::string out = line;
        for (size_t k = 0; k < params.size(); ++k) {
            const std::string key = "#" + params[k];
            const std::string val = k < args.size() ? args[k] : "10";
            size_t at = out.find(key);
            while (at != std::string::npos) {
                out.replace(at, key.size(), val);
                at = out.find(key, at + val.size());
            }
        }
        return out;
    }

    // "nome: macro #a,#b" -> true e preenche
    static bool ParseMacroHeader(const std::string &text, bool commented, std::string &name, std::vector<std::string> &params, bool &bad) {
        bad = false;
        const std::string t = Trim(text);
        size_t i = 0;
        if (i >= t.size() || !IsIdStart(t[i])) return false;
        size_t j = i;
        while (j < t.size() && IsIdChar(t[j])) ++j;
        const std::string first = t.substr(i, j - i);
        size_t k = j;
        while (k < t.size() && (t[k] == ' ' || t[k] == '\t')) ++k;
        bool colon = false;
        if (k < t.size() && t[k] == ':') {
            colon = true;
            ++k;
            while (k < t.size() && (t[k] == ' ' || t[k] == '\t')) ++k;
        }
        if (Lower(t.substr(k, 5)) != "macro" || (k + 5 < t.size() && IsIdChar(t[k + 5]))) return false;
        if (!colon || commented) { // "mac macro #a" (sem os dois pontos): erro de sintaxe no asMSX
            bad = true;
            return true;
        }
        name = first;
        params.clear();
        const std::string rest = Trim(t.substr(k + 5));
        if (!rest.empty()) {
            for (const std::string &p : SplitArgs(rest)) {
                if (p.size() < 2 || p[0] != '#') {
                    bad = true;
                    return true;
                }
                params.push_back(p.substr(1));
            }
        }
        return true;
    }

    static bool IsEndm(const SrcLine &l) { return !l.commented && Lower(Trim(l.text)) == "endm"; }

    // O asMSX troca "#param" por argumento um parametro por vez, em ordem: "#a" tambem come o prefixo de "#ab".
    // Devolve os parametros que nao acharam nada no corpo.
    static std::vector<std::string> UnusedParams(const std::vector<std::string> &params, const std::vector<std::string> &body) {
        std::string text;
        for (const std::string &b : body) text += b + "\n";
        std::vector<std::string> res;
        for (const std::string &p : params) {
            const std::string key = "#" + p;
            size_t at = text.find(key);
            if (at == std::string::npos) {
                res.push_back(p);
                continue;
            }
            while (at != std::string::npos) {
                text.erase(at, key.size());
                at = text.find(key, at);
            }
        }
        return res;
    }

    std::vector<SrcLine> Macros(const std::vector<SrcLine> &in) {
        std::map<std::string, Macro> macros;
        std::vector<SrcLine> res;
        for (size_t n = 0; n < in.size() && out.rc == 0; ++n) {
            const SrcLine &l = in[n];
            std::string name;
            std::vector<std::string> params;
            bool bad = false;
            if (ParseMacroHeader(l.text, l.commented, name, params, bad)) {
                if (bad) {
                    Fail(1, l.file, l.line, "syntax error", true);
                    break;
                }
                Macro m;
                m.name = name;
                m.params = params;
                m.line = l.line;
                size_t e = n + 1;
                while (e < in.size() && !IsEndm(in[e])) m.body.push_back(in[e++].text);
                if (e >= in.size()) {
                    Fail(2, l.file, l.line, "memory overflow");
                    break;
                }
                if (macros.count(name)) {
                    Fail(6, l.file, l.line, "Macro redefinition.");
                    break;
                }
                for (const std::string &p : UnusedParams(m.params, m.body)) out.warnings.push_back(Where(l.file, l.line, "Warning: Parameter #" + p + " of macro " + name + " unused."));
                macros[name] = m;
                n = e; // pula ate' o ENDM
                continue;
            }
            // invocacao: a primeira palavra da linha e' o nome de uma macro (maiusculas/minusculas contam)
            const std::string t = Trim(l.text);
            if (!t.empty() && IsIdStart(t[0])) {
                size_t j = 0;
                while (j < t.size() && IsIdChar(t[j])) ++j;
                const auto it = macros.find(t.substr(0, j));
                if (it != macros.end() && (j == t.size() || t[j] == ' ' || t[j] == '\t')) {
                    const Macro &m = it->second;
                    const std::vector<std::string> args = SplitArgs(Trim(t.substr(j)));
                    if (args.size() > m.params.size()) {
                        Fail(5, l.file, l.line, "Wrong number of parameters for the macro. Given more than required.");
                        break;
                    }
                    if (!args.empty() && args.size() < m.params.size()) {
                        Fail(4, l.file, l.line, "Wrong number of parameters for the macro. Given less than required.");
                        break;
                    }
                    for (const std::string &p : UnusedParams(m.params, m.body)) out.warnings.push_back(Where(l.file, l.line, "Warning: Parameter #" + p + " of macro " + m.name + " unused."));
                    for (const std::string &b : m.body) res.push_back({l.file, l.line, Substitute(b, m.params, args)});
                    continue;
                }
            }
            res.push_back(l);
        }
        return res;
    }

    // ---- 3: REPT ----
    static bool IsRept(const std::string &text, std::string &arg) {
        std::string t = Trim(text);
        if (!t.empty() && t[0] == '.') t = t.substr(1);
        if (Lower(t.substr(0, 4)) != "rept" || (t.size() > 4 && IsIdChar(t[4]))) return false;
        arg = Trim(t.substr(4));
        return true;
    }
    static bool IsEndr(const std::string &text) {
        const std::string t = Lower(Trim(text));
        return t == "endr" || t == ".endr";
    }

    std::vector<SrcLine> Rept(const std::vector<SrcLine> &in) {
        std::vector<SrcLine> res;
        for (size_t n = 0; n < in.size() && out.rc == 0; ++n) {
            std::string arg;
            if (!IsRept(in[n].text, arg)) {
                res.push_back(in[n]);
                continue;
            }
            const SrcLine &l = in[n];
            // acha o ENDR correspondente
            int depth = 1;
            size_t e = n + 1;
            for (; e < in.size(); ++e) {
                std::string dummy;
                if (IsRept(in[e].text, dummy)) ++depth;
                else if (IsEndr(in[e].text) && --depth == 0) break;
            }
            if (e >= in.size()) {
                Fail(3, l.file, l.line, "wrong register combination");
                break;
            }
            // numero: comeca com digito e segue so' com letras/digitos (0x3 vale 0 repeticoes: strtol decimal)
            bool digits = !arg.empty() && std::isdigit(static_cast<unsigned char>(arg[0]));
            for (const char c : arg) digits = digits && (std::isalnum(static_cast<unsigned char>(c)) || c == '_');
            if (!digits) {
                if (arg.empty() || arg == "-" || !(IsIdStart(arg[0]) || arg[0] == '-')) {
                    Fail(1, l.file, (!arg.empty() && std::isdigit(static_cast<unsigned char>(arg[0]))) ? 0 : l.line, "syntax error", true);
                } else {
                    Fail(1, l.file, l.line, "number expected in REPT");
                }
                break;
            }
            const long count = std::strtol(arg.c_str(), nullptr, 10);
            const std::vector<SrcLine> body(in.begin() + static_cast<std::ptrdiff_t>(n) + 1, in.begin() + static_cast<std::ptrdiff_t>(e));
            const std::vector<SrcLine> inner = Rept(body);
            for (long k = 0; k < count && out.rc == 0; ++k) res.insert(res.end(), inner.begin(), inner.end());
            n = e;
        }
        return res;
    }
};

} // namespace

std::string StripComments(const std::string &s, int &block) {
    std::string out;
    size_t i = 0;
    const size_t n = s.size();
    while (i < n) {
        if (block == 1) {
            const size_t e = s.find("*/", i);
            if (e == std::string::npos) return out;
            i = e + 2;
            block = 0;
            out += ' ';
            continue;
        }
        if (block == 2) {
            const size_t e = s.find('}', i);
            if (e == std::string::npos) return out;
            i = e + 1;
            block = 0;
            out += ' ';
            continue;
        }
        const char c = s[i];
        if (c == '"') {
            const size_t e = s.find('"', i + 1);
            if (e == std::string::npos) {
                out += s.substr(i);
                return out;
            }
            out += s.substr(i, e - i + 1);
            i = e + 1;
            continue;
        }
        if (c == ';') return out;
        if (c == '/' && i + 1 < n && s[i + 1] == '/') return out;
        if (c == '-' && i + 1 < n && s[i + 1] == '-') return out;
        if (c == '/' && i + 1 < n && s[i + 1] == '*') {
            block = 1;
            i += 2;
            continue;
        }
        if (c == '{') {
            block = 2;
            ++i;
            continue;
        }
        out += c;
        ++i;
    }
    return out;
}

PreResult Preprocess(const std::string &name, const std::string &text, FileProvider *files) {
    Pre p;
    p.files = files;
    std::vector<SrcLine> lines;
    p.Read(name, text, 0, lines);
    if (p.out.rc == 0) lines = p.Macros(lines);
    if (p.out.rc == 0) lines = p.Rept(lines);
    p.out.lines = std::move(lines);
    return p.out;
}

} // namespace fwasm
