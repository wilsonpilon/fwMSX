// fwMSX -- "fwmsx --cli": console interativo (REPL) com historico e TAB. Ver doc/repl-spec.md.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "repl_session.h"

namespace repl {

// Cria uma sessao com as ferramentas registradas (newdisk, romdb, cas, fitadb, msxdisk) e o `emu start`
// apontando para a JANELA (no Windows, o fwMSX.exe ao lado do fwMSXc.exe). Usada pelo console e pela TUI.
std::unique_ptr<ReplSession> BuildSession(const std::string &argv0);

// Abre o console. `args` sao as opcoes depois de --cli (hoje: --attach <porta> para ja' conectar
// num emulador aberto). Devolve o codigo de saida.
int RunReplCommand(const std::vector<std::string> &args, const std::string &argv0);

} // namespace repl
