// fwMSX -- janela do emulador (Dear ImGui + GLFW + OpenGL3). Codigo ORIGINAL
// do fwMSX (BSD-3-Clause). Ver doc/machine-spec.md.
//
// Sempre declarada, independente de FWMSX_MSXDISK_GUI (mesma ideia do
// msxdisk::gui::LaunchGui): emu_window.cpp (janela real) ou
// emu_window_stub.cpp (aviso "nao compilado nesta build") fornecem a mesma
// funcao, escolhidos no CMakeLists.txt.
#pragma once

#include "../machine.h"

namespace machine::gui {

struct WindowOptions {
    MachineConfig machine;
    // Fecha sozinha apos N quadros emulados (0 = so' quando o usuario fechar).
    // Existe para validar a janela sem interacao (CI/automacao).
    int autoquit_frames = 0;
    // false = nao abre dispositivo de audio (emulador mudo).
    bool audio = true;
    // Ponte de controle externa (TCP em 127.0.0.1, doc/control-spec.md): -1 = desligada,
    // 0 = porta escolhida pelo sistema. A porta aberta e' escrita em fwmsx.port.
    int ctl_port = -1;
    // Pasta do executavel: o fwmsx.port tambem e' gravado ali (lugar previsivel, mesmo quando o
    // emulador foi aberto por um atalho com outra pasta de trabalho).
    std::string exe_dir;
    // Pasta de ROMs e do banco (menu ROMs). Vazio = roms/ ao lado do executavel.
    std::string rom_root;
};

// Abre a janela e roda a maquina em tempo real ate' ser fechada. Devolve o
// codigo de saida do processo.
int RunEmulatorWindow(const WindowOptions &options);

} // namespace machine::gui
