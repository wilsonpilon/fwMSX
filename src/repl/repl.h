// fwMSX -- "fwmsx --cli": console interativo (REPL) com historico e TAB. Ver doc/repl-spec.md.
#pragma once

#include <string>
#include <vector>

namespace repl {

// Abre o console. `args` sao as opcoes depois de --cli (hoje: --attach <porta> para ja' conectar
// num emulador aberto). Devolve o codigo de saida.
int RunReplCommand(const std::vector<std::string> &args, const std::string &argv0);

} // namespace repl
