//
// Nucleo C++ do msxdisk (fwMSX): entrada de diretorio FAT12 (32 bytes) e
// nomes 8.3 do MSX-DOS.
//

#include "dir_entry.h"

#include <cctype>

namespace msxdisk {

namespace {

char ToUpperAscii(char c) {
    return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

void SplitNameExt(const std::string &display, std::string *name_part, std::string *ext_part) {
    const auto dot = display.find_last_of('.');
    if (dot == std::string::npos) {
        *name_part = display;
        ext_part->clear();
    } else {
        *name_part = display.substr(0, dot);
        *ext_part = display.substr(dot + 1);
    }
}

} // namespace

std::array<char, 11> ToFat83Name(const std::string &display_name) {
    std::string name_part, ext_part;
    SplitNameExt(display_name, &name_part, &ext_part);

    std::array<char, 11> out{};
    out.fill(' ');
    for (size_t i = 0; i < name_part.size() && i < 8; ++i) {
        out[i] = ToUpperAscii(name_part[i]);
    }
    for (size_t i = 0; i < ext_part.size() && i < 3; ++i) {
        out[8 + i] = ToUpperAscii(ext_part[i]);
    }
    return out;
}

std::array<char, 11> ToFat83Pattern(const std::string &display_pattern) {
    std::string name_part, ext_part;
    SplitNameExt(display_pattern, &name_part, &ext_part);

    std::array<char, 11> out{};
    out.fill(' ');

    // '*' expande preenchendo o restante do campo (nome ou extensao) com
    // '?', igual ao MSX-DOS/CP-M: "*.BAS" -> nome = "????????", ext = "BAS".
    bool star = false;
    size_t out_i = 0;
    for (size_t i = 0; i < name_part.size() && out_i < 8; ++i) {
        if (name_part[i] == '*') {
            star = true;
            break;
        }
        out[out_i++] = ToUpperAscii(name_part[i]);
    }
    if (star) {
        while (out_i < 8) out[out_i++] = '?';
    }

    star = false;
    out_i = 8;
    for (size_t i = 0; i < ext_part.size() && out_i < 11; ++i) {
        if (ext_part[i] == '*') {
            star = true;
            break;
        }
        out[out_i++] = ToUpperAscii(ext_part[i]);
    }
    if (star) {
        while (out_i < 11) out[out_i++] = '?';
    }

    return out;
}

std::string FromFat83Name(const char name[8], const char ext[3]) {
    std::string n(name, 8);
    std::string e(ext, 3);

    const auto rtrim = [](std::string &s) {
        while (!s.empty() && s.back() == ' ') s.pop_back();
    };
    rtrim(n);
    rtrim(e);

    if (e.empty()) return n;
    return n + "." + e;
}

} // namespace msxdisk
