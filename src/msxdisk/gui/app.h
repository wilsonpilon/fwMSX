//
// msxdisk (fwMSX): GUI (Dear ImGui + GLFW + OpenGL3) -- Fase 5.
//
// Sempre declarado, independente de FWMSX_MSXDISK_GUI: app.cpp (GUI real)
// ou app_stub.cpp (aviso "nao compilado nesta build") fornecem a mesma
// funcao, escolhidos no CMakeLists.txt -- assim tools/msxdisk/main.cpp e
// src/msxdisk/shell/shell.cpp nao precisam de #ifdef nenhum.
//
#pragma once

#include <string>

namespace msxdisk::gui {

int LaunchGui(const std::string &image_path = "");

} // namespace msxdisk::gui
