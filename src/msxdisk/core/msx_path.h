//
// Nucleo C++ do msxdisk (fwMSX): parsing de caminhos MSX-DOS 2.
//
#pragma once

#include <string>
#include <vector>

namespace msxdisk {

// Um caminho estilo MSX-DOS 2 (ex.: "A:\JOGOS\SUBDIR\ARQ.BAS" ou
// "JOGOS/SUBDIR/ARQ.BAS") quebrado em componentes de diretorio, na ordem
// a partir da raiz, e o ultimo componente ("leaf_name" -- normalmente um
// arquivo, mas o chamador decide o que fazer com ele: MakeDirectory usa
// leaf_name como o nome do novo diretorio, por exemplo).
struct MsxPath {
    std::vector<std::string> dir_components;
    std::string leaf_name;
};

// Aceita '\\' e '/' como separador, ignora um prefixo de unidade opcional
// ("A:"), e descarta componentes vazios (barras duplicadas/inicial). Um
// caminho vazio ou "\\" resulta em dir_components vazio e leaf_name vazio
// (referencia a raiz).
MsxPath ParseMsxPath(const std::string &path);

} // namespace msxdisk
