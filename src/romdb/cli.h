// fwMSX -- linha de comando do banco de ROMs (`fwmsx --romdb <comando>`). Codigo ORIGINAL
// do fwMSX (BSD-3-Clause). Ver doc/romdb-spec.md, secao 5.
#pragma once

#include <string>
#include <vector>

namespace romdb {

// Executa um comando do banco de ROMs. `args` vem depois de "--romdb". Devolve o codigo
// de saida (0 = ok).
int RunRomDbCommand(const std::vector<std::string> &args, const std::string &argv0);

} // namespace romdb
