// fwMSX -- classificacao das ROMs por tipo. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "classify.h"

#include <algorithm>
#include <cctype>

namespace romdb {
namespace {

std::string Upper(std::string s) {
    for (char &c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

bool EndsWith(const std::string &s, const std::string &suffix) {
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool Contains(const std::string &s, const char *part) { return s.find(part) != std::string::npos; }

} // namespace

std::string CategoryFor(const std::string &filename) {
    // Nome sem pasta, em maiusculas, para comparar.
    const size_t slash = filename.find_last_of("/\\");
    const std::string name = Upper(slash == std::string::npos ? filename : filename.substr(slash + 1));

    if (EndsWith(name, ".SHA") || EndsWith(name, ".DB") || EndsWith(name, ".JSON") || EndsWith(name, ".SQL")) {
        return "tabela";
    }
    if (EndsWith(name, ".DSK")) return "disco";
    if (EndsWith(name, ".TXT") || EndsWith(name, ".HTML") || EndsWith(name, ".MD") || EndsWith(name, ".BAS")) {
        return "outro";
    }

    // Interfaces e sub-ROMs: disco, extensoes de MSX2, FM-PAC, Painter.
    if (Contains(name, "DISK") || Contains(name, "FDC") || Contains(name, "EXT.") || Contains(name, "EXT_") ||
        name.rfind("FMPAC", 0) == 0 || name.rfind("PAINTER", 0) == 0 || name == "DISK.ROM") {
        return "interface";
    }
    // BIOS: MSX.ROM, MSX2.ROM, MSX2P.ROM (e variantes de BIOS como MSX2+).
    if (name.rfind("MSX", 0) == 0 && (EndsWith(name, ".ROM")) && !Contains(name, "EXT")) return "bios";

    if (EndsWith(name, ".ROM") || EndsWith(name, ".BIN") || EndsWith(name, ".MX1") || EndsWith(name, ".MX2") ||
        EndsWith(name, ".EPROM") || EndsWith(name, ".DAT")) {
        return "cartucho";
    }
    return "outro";
}

std::string FolderForCategory(const std::string &category) {
    if (category == "bios") return "bios";
    if (category == "interface") return "interfaces";
    if (category == "cartucho") return "cartuchos";
    if (category == "disco") return "discos";
    if (category == "tabela") return "tabelas";
    return "outros";
}

} // namespace romdb
