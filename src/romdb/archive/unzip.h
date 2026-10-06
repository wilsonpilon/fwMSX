// fwMSX -- extracao de arquivos ZIP (biblioteca miniz, dominio publico/MIT). Codigo
// ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/romdb-spec.md, secao 4.
#pragma once

#include <string>
#include <utility>
#include <vector>

namespace romdb {

// Extrai o ZIP em `dest_dir` (cria as pastas). Devolve em `files` o caminho de cada
// arquivo extraido. Caminhos com "..", absolutos ou com letra de unidade sao ignorados
// (protecao contra escrita fora da pasta). false com `error` se o ZIP nao abrir.
bool ExtractZip(const std::string &zip_path, const std::string &dest_dir, std::vector<std::string> &files,
                std::string &error);

// Cria um ZIP com `entries` (nome dentro do ZIP -> conteudo). Usado pelos testes.
bool WriteZip(const std::string &zip_path, const std::vector<std::pair<std::string, std::string>> &entries,
              std::string &error);

} // namespace romdb
