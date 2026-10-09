// fwMSX -- TUI do emulador (FTXUI): a maquina rodando dentro do terminal. Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver doc/tui-spec.md.
//
// A maquina roda numa thread propria (60 quadros/s); a interface so' desenha um instantaneo e manda
// teclas e comandos. TODO comando passa pelo mesmo control::Commander da janela, do console e da ponte
// de controle -- o menu e a linha de comando da TUI sao so' jeitos diferentes de digitar os mesmos
// comandos.
#pragma once

#include <string>

#include "../machine/machine.h"

namespace tui {

struct TuiOptions {
    machine::MachineConfig machine;
    bool audio = true;
    // Ponte de controle externa (como na janela): -1 = desligada, 0 = porta livre.
    int ctl_port = -1;
    // Pasta do executavel (onde tambem se grava o fwmsx.port).
    std::string exe_dir;
};

// Roda a TUI ate o usuario sair. Devolve o codigo de saida do processo.
int RunTui(const TuiOptions &options);

} // namespace tui
