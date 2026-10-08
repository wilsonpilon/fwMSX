// fwMSX -- linha de comando do banco de fitas (`fwmsx --fitadb <comando>`).
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/tape-spec.md, secao 11.
#pragma once

#include <string>
#include <vector>

namespace tapedb {

// Executa um comando do banco de fitas. `args` vem depois de "--fitadb". Devolve o
// codigo de saida (0 = ok).
int RunTapeDbCommand(const std::vector<std::string> &args, const std::string &argv0);

} // namespace tapedb
