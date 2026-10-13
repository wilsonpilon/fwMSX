// fwMSX -- montador (fwasm): analisador lexico. Ver lexer.h.
#include "lexer.h"

#include <cctype>
#include <cstdlib>

namespace fwasm {

namespace {

bool IsIdentStart(char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_'; }
bool IsIdentChar(char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '@'; }
bool IsHex(char c) { return std::isxdigit(static_cast<unsigned char>(c)) != 0; }

int HexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    return (std::tolower(static_cast<unsigned char>(c)) - 'a') + 10;
}

// Hexadecimal: estoura saturando em 0x7FFFFFFF (como o strtol de 32 bits do programa original).
int32_t ParseHex(const std::string &digits) {
    int64_t v = 0;
    for (const char c : digits) {
        v = v * 16 + HexVal(c);
        if (v > 0x7FFFFFFF) v = 0x7FFFFFFF;
    }
    return static_cast<int32_t>(v);
}

// Decimal, binario e octal acumulam em 32 bits com volta (wrap), como o original.
int32_t ParseRadix(const std::string &digits, int radix) {
    uint32_t v = 0;
    for (const char c : digits) v = v * static_cast<uint32_t>(radix) + static_cast<uint32_t>(c - '0');
    return static_cast<int32_t>(v);
}

bool AllDigits(const std::string &s, size_t from, size_t to, int radix) {
    if (from >= to) return false;
    for (size_t i = from; i < to; ++i) {
        const char c = s[i];
        if (radix == 16) {
            if (!IsHex(c)) return false;
        } else if (c < '0' || c >= '0' + radix) {
            return false;
        }
    }
    return true;
}

} // namespace

LexResult Lex(const std::string &s) {
    LexResult r;
    size_t i = 0;
    const size_t n = s.size();
    auto push = [&](Token t) { r.tokens.push_back(std::move(t)); };
    auto fail = [&](const char *msg) {
        r.error = msg;
        return r;
    };

    while (i < n) {
        const char c = s[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            ++i;
            continue;
        }

        // ---- identificadores (inclui rotulos locais @@x e .x) ----
        if (IsIdentStart(c) || (c == '@' && i + 1 < n && s[i + 1] == '@') || (c == '.' && i + 1 < n && IsIdentStart(s[i + 1]))) {
            size_t j = i;
            if (c == '@') j += 2;
            else if (c == '.') j += 1;
            while (j < n && IsIdentChar(s[j])) ++j;
            Token t;
            t.kind = Token::Ident;
            t.text = s.substr(i, j - i);
            i = j;
            // af' (o apostrofo faz parte do nome do registrador)
            if (i < n && s[i] == '\'' && t.text.size() == 2 && std::tolower(static_cast<unsigned char>(t.text[0])) == 'a' &&
                std::tolower(static_cast<unsigned char>(t.text[1])) == 'f') {
                t.text += '\'';
                ++i;
            }
            push(t);
            continue;
        }

        // ---- numeros ----
        if (std::isdigit(static_cast<unsigned char>(c))) {
            // Peculiaridade observada do asMSX: "0/1" colado (sem espacos) e' erro de sintaxe.
            if (c == '0' && i + 2 < n + 0 && s[i + 1] == '/' && s[i + 2] == '1' && (i == 0 || !IsIdentChar(s[i - 1]))) return fail("syntax error");
            size_t j = i;
            while (j < n && (std::isalnum(static_cast<unsigned char>(s[j])) || s[j] == '_')) ++j;
            // ponto flutuante: digitos '.' digitos [e[+-]digitos]
            if (j < n && s[j] == '.' && j + 1 < n && std::isdigit(static_cast<unsigned char>(s[j + 1])) && AllDigits(s, i, j, 10)) {
                size_t k = j + 1;
                while (k < n && std::isdigit(static_cast<unsigned char>(s[k]))) ++k;
                // (sem expoente: "1.0e3" nao e' aceito pelo asMSX -- "e3" fica sobrando e da' erro de sintaxe)
                Token t;
                t.kind = Token::Float;
                t.fval = std::strtod(s.substr(i, k - i).c_str(), nullptr);
                t.text = s.substr(i, k - i);
                push(t);
                i = k;
                continue;
            }
            const std::string w = s.substr(i, j - i);
            Token t;
            t.kind = Token::Int;
            t.text = w;
            const size_t len = w.size();
            const char last = static_cast<char>(std::tolower(static_cast<unsigned char>(w[len - 1])));
            bool ok = false;
            if (len > 2 && w[0] == '0' && (w[1] == 'x' || w[1] == 'X')) {
                if (AllDigits(w, 2, len, 16)) {
                    t.ival = ParseHex(w.substr(2));
                    ok = true;
                }
            } else if (len > 1 && last == 'h' && AllDigits(w, 0, len - 1, 16)) {
                t.ival = ParseHex(w.substr(0, len - 1));
                ok = true;
            } else if (len > 1 && last == 'b' && AllDigits(w, 0, len - 1, 2)) {
                t.ival = ParseRadix(w.substr(0, len - 1), 2);
                ok = true;
            } else if (len > 1 && last == 'o' && AllDigits(w, 0, len - 1, 8)) {
                t.ival = ParseRadix(w.substr(0, len - 1), 8);
                ok = true;
            } else if (AllDigits(w, 0, len, 10)) {
                if (len > 1 && w[0] == '0') {
                    if (AllDigits(w, 0, len, 8)) {
                        t.ival = ParseRadix(w, 8);
                        ok = true;
                    }
                } else {
                    t.ival = ParseRadix(w, 10);
                    ok = true;
                }
            } else if (len == 1) {
                ok = false;
            }
            // "0b" e "0h" e "0o": valem 0 (digito + sufixo)
            if (!ok) return fail("syntax error");
            push(t);
            i = j;
            continue;
        }

        // ---- $ (endereco atual) e $hex ----
        if (c == '$') {
            if (i + 1 < n && IsHex(s[i + 1])) {
                size_t j = i + 1;
                while (j < n && IsHex(s[j])) ++j;
                Token t;
                t.kind = Token::Int;
                t.text = s.substr(i, j - i);
                t.ival = ParseHex(s.substr(i + 1, j - i - 1));
                push(t);
                i = j;
                continue;
            }
            Token t;
            t.kind = Token::Op;
            t.text = "$";
            push(t);
            ++i;
            continue;
        }

        // ---- caractere 'a' ----
        if (c == '\'') {
            if (i + 2 < n && s[i + 2] == '\'' && s[i + 1] != '\'' && s[i + 1] != '"') {
                Token t;
                t.kind = Token::Char;
                t.ival = static_cast<unsigned char>(s[i + 1]);
                t.text = s.substr(i, 3);
                push(t);
                i += 3;
                continue;
            }
            if (i + 1 < n && s[i + 1] == '"') return fail("Unterminated string"); // '"': o asMSX abre uma string no "
            return fail("syntax error");
        }

        // ---- string "..." ----
        if (c == '"') {
            size_t j = i + 1;
            while (j < n && s[j] != '"') ++j;
            if (j >= n) return fail("Unterminated string");
            Token t;
            t.kind = Token::String;
            t.text = s.substr(i + 1, j - i - 1);
            push(t);
            i = j + 1;
            continue;
        }

        // ---- operadores ----
        static const char *const two[] = {"<<", ">>", "<=", ">=", "==", "!=", "&&", "||", "<>"};
        bool matched = false;
        if (i + 1 < n) {
            for (const char *op : two) {
                if (s[i] == op[0] && s[i + 1] == op[1]) {
                    Token t;
                    t.kind = Token::Op;
                    t.text = op;
                    push(t);
                    i += 2;
                    matched = true;
                    break;
                }
            }
        }
        if (matched) continue;
        if (std::string("+-*/%|&^~!()[],:=<>#").find(c) != std::string::npos) {
            Token t;
            t.kind = Token::Op;
            t.text = std::string(1, c);
            push(t);
            ++i;
            continue;
        }
        return fail("syntax error");
    }
    return r;
}

} // namespace fwasm
