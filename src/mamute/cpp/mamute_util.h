// fwMSX -- Mamute: pequenas funcoes de texto compartilhadas (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md.
//
// Os comandos do SUPER-X (Fase 4) vivem em varios arquivos; em vez de copiar mais uma vez o Trim/Upper/ParseHex de cada .cpp, ficam aqui.
#pragma once

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace mamute {
namespace util {

inline std::string Upper(std::string s) {
    for (char &c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

inline std::string Trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

inline std::string Hex(unsigned v, int digits) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%0*X", digits, v);
    return buf;
}

// Hexadecimal puro, 1 a `max_digits` digitos (sem prefixo/sufixo: o SUPER-X nao os usa nos enderecos).
inline bool ParseHexN(const std::string &str, uint32_t &out, int max_digits) {
    const std::string s = Trim(str);
    if (s.empty() || static_cast<int>(s.size()) > max_digits) return false;
    uint32_t v = 0;
    for (const char c : s) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
        v = v * 16 + static_cast<uint32_t>(std::isdigit(static_cast<unsigned char>(c)) ? c - '0' : std::toupper(static_cast<unsigned char>(c)) - 'A' + 10);
    }
    out = v;
    return true;
}

inline bool ParseDec(const std::string &str, int &out) {
    const std::string s = Trim(str);
    if (s.empty() || s.size() > 9) return false;
    int v = 0;
    for (const char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        v = v * 10 + (c - '0');
    }
    out = v;
    return true;
}

// Divide por virgula GUARDANDO os campos vazios (o "XD 4000,,arq" tem o fim vazio); campos aparados.
inline std::vector<std::string> SplitCommas(const std::string &r) {
    std::vector<std::string> out;
    size_t pos = 0;
    while (true) {
        const size_t c = r.find(',', pos);
        out.push_back(Trim(r.substr(pos, c == std::string::npos ? std::string::npos : c - pos)));
        if (c == std::string::npos) break;
        pos = c + 1;
    }
    return out;
}

inline char Printable(int b) { return (b >= 0x20 && b < 0x7F) ? static_cast<char>(b) : '.'; }

inline std::string Join(const std::vector<std::string> &v) {
    std::string out;
    for (const std::string &l : v) out += (out.empty() ? "" : "\n") + l;
    return out;
}

} // namespace util
} // namespace mamute
