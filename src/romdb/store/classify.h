// fwMSX -- classificacao das ROMs por tipo (pastas de destino). Codigo ORIGINAL do
// fwMSX (BSD-3-Clause). Ver doc/romdb-spec.md, secao 3.
#pragma once

#include <string>

namespace romdb {

// Categorias: "bios", "interface", "cartucho", "disco", "tabela" ou "outro".
std::string CategoryFor(const std::string &filename);

// A categoria que uma assinatura do openMSX sugere (bios, interface ou outro): ROM de maquina = bios (menos disco/IDE, que sao interface); ROM de extensao =
// interface (disco, som, MSX-DOS...), menos ferramentas de depuracao e processadores de texto.
std::string CategoryForSignature(const std::string &role, const std::string &family, const std::string &source, const std::string &filename);

// Pasta de destino dentro da pasta de ROMs (bios, interfaces, cartuchos, discos,
// tabelas, outros). Ex.: "bios" -> "bios".
std::string FolderForCategory(const std::string &category);

} // namespace romdb
