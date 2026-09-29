//
// Nucleo C++ do msxdisk (fwMSX): parsing de caminhos MSX-DOS 2.
//

#include "msx_path.h"

namespace msxdisk {

MsxPath ParseMsxPath(const std::string &path) {
    std::string trimmed = path;

    // Prefixo de unidade opcional ("A:", "B:" etc.) -- o msxdisk sempre
    // trabalha numa unica imagem por vez, entao o prefixo e apenas
    // descartado.
    if (trimmed.size() >= 2 && trimmed[1] == ':') {
        trimmed = trimmed.substr(2);
    }

    std::vector<std::string> parts;
    std::string current;
    for (char c : trimmed) {
        if (c == '\\' || c == '/') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }

    MsxPath result;
    if (!parts.empty()) {
        result.leaf_name = parts.back();
        parts.pop_back();
        result.dir_components = std::move(parts);
    }
    return result;
}

} // namespace msxdisk
