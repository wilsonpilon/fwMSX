// fwMSX -- classificacao das ROMs por tipo (pastas de destino). Codigo ORIGINAL do
// fwMSX (BSD-3-Clause). Ver doc/romdb-spec.md, secao 3.
#pragma once

#include <string>

namespace romdb {

// Categorias: "bios", "interface", "cartucho", "disco", "tabela" ou "outro".
std::string CategoryFor(const std::string &filename);

// Pasta de destino dentro da pasta de ROMs (bios, interfaces, cartuchos, discos,
// tabelas, outros). Ex.: "bios" -> "bios".
std::string FolderForCategory(const std::string &category);

} // namespace romdb
