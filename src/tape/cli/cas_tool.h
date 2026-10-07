// fwMSX -- "fwmsx --cas <comando>": ferramenta de linha de comando para
// empacotar um .BIN/.BAS solto num .TSX/.CAS valido, sem passar pelo
// emulador, e para listar o conteudo de uma fita existente. Codigo
// ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/tape-spec.md, secao 8.
#pragma once

#include <string>
#include <vector>

namespace tape {

int RunCasToolCommand(const std::vector<std::string> &args, const std::string &argv0);

} // namespace tape
