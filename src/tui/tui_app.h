// fwMSX -- TUI de menus (FTXUI). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/tui-spec.md.
//
// `fwmsx --tui`: menus com os comandos do console que iniciam e controlam o emulador em JANELA pela
// ponte de controle. NAO embute a maquina (para isso existe o modo terminal, `--term`).
#pragma once

#include <string>
#include <vector>

namespace tui {

// `args`: opcoes depois de --tui (hoje: --attach <porta> para ja' conectar num emulador aberto).
int RunTui(const std::vector<std::string> &args, const std::string &argv0);

} // namespace tui
