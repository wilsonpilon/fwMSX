// fwMSX -- editor de textos: entrada do terminal -> nome de tecla. Ver editor_keys.h.
#include "editor_keys.h"

#include <cctype>
#include <cstdlib>

namespace ed {

namespace {

// m = o parametro de modificador do xterm (1 + bits: 1 shift, 2 alt, 4 ctrl).
std::string Mods(int m) {
    if (m <= 1) return {};
    const int b = m - 1;
    std::string s;
    if (b & 4) s += "C-";
    if (b & 2) s += "A-";
    if (b & 1) s += "S-";
    return s;
}

std::string FromLetter(char c) {
    switch (c) {
    case 'A': return "Up";
    case 'B': return "Down";
    case 'C': return "Right";
    case 'D': return "Left";
    case 'H': return "Home";
    case 'F': return "End";
    case 'P': return "F1";
    case 'Q': return "F2";
    case 'R': return "F3";
    case 'S': return "F4";
    default: return {};
    }
}

std::string FromTilde(int n) {
    switch (n) {
    case 1: case 7: return "Home";
    case 2: return "Ins";
    case 3: return "Del";
    case 4: case 8: return "End";
    case 5: return "PgUp";
    case 6: return "PgDn";
    case 11: return "F1";
    case 12: return "F2";
    case 13: return "F3";
    case 14: return "F4";
    case 15: return "F5";
    case 17: return "F6";
    case 18: return "F7";
    case 19: return "F8";
    case 20: return "F9";
    case 21: return "F10";
    case 23: return "F11";
    case 24: return "F12";
    default: return {};
    }
}

std::string Control(unsigned char c) {
    switch (c) {
    case 0x00: return "C-2";
    case 0x08: return "C-BS";
    case 0x09: return "Tab";
    case 0x0A: case 0x0D: return "Enter";
    case 0x1B: return "Esc";
    case 0x1D: return "C-]";
    case 0x7F: return "BS";
    default: break;
    }
    if (c >= 1 && c <= 26) return std::string("C-") + static_cast<char>('a' + c - 1);
    return {};
}

} // namespace

std::string KeyNameFromInput(const std::string &in) {
    if (in.empty()) return {};
    const unsigned char c0 = static_cast<unsigned char>(in[0]);
    if (in.size() == 1) {
        if (c0 >= 32 && c0 < 127) return in;
        return Control(c0);
    }
    if (c0 >= 0x80) return in; // UTF-8: texto
    if (c0 != 0x1B) return {};

    // ESC + um caractere = Alt+caractere
    if (in.size() == 2) {
        const unsigned char c = static_cast<unsigned char>(in[1]);
        if (c >= 32 && c < 127) return std::string("A-") + in[1];
        if (c == 0x7F) return "A-BS";
        if (c == 0x0D || c == 0x0A) return "A-Enter";
        const std::string ctl = Control(c);
        if (ctl.size() == 3 && ctl[0] == 'C' && ctl[1] == '-') return "C-A-" + ctl.substr(2);
        return {};
    }
    // ESC O x (F1-F4, Home/End em alguns terminais)
    if (in[1] == 'O' && in.size() == 3) return FromLetter(in[2]);
    if (in[1] != '[') return {};
    if (in == "\x1b[Z") return "S-Tab";
    const char last = in.back();
    // ESC [ [ params ] final: parametros "n" ou "n;m" ou "1;m"
    const std::string params = in.substr(2, in.size() - 3);
    int n = 0;
    int m = 1;
    const size_t semi = params.find(';');
    if (!params.empty()) n = std::atoi(params.c_str());
    if (semi != std::string::npos) m = std::atoi(params.c_str() + semi + 1);
    if (last == '~') {
        const std::string name = FromTilde(n);
        return name.empty() ? std::string() : Mods(m) + name;
    }
    const std::string name = FromLetter(last);
    return name.empty() ? std::string() : Mods(m) + name;
}

bool IsTextKey(const std::string &name) {
    if (name.empty()) return false;
    if (static_cast<unsigned char>(name[0]) >= 0x80) return true;
    return name.size() == 1;
}

std::string Utf8ToBytes(const std::string &s) {
    std::string out;
    for (size_t i = 0; i < s.size();) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c < 0x80) {
            out.push_back(static_cast<char>(c));
            ++i;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < s.size()) {
            const unsigned cp = ((c & 0x1Fu) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3Fu);
            if (cp < 256) out.push_back(static_cast<char>(cp));
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            i += 4;
        } else {
            ++i;
        }
    }
    return out;
}

} // namespace ed
