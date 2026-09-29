//
// msxdisk (fwMSX) - ponto de entrada do executavel standalone.
//
// Um so binario para os quatro modos (a pedido do autor):
//   msxdisk <comando> ...   modo CLI one-shot (Fase 1/2)
//   msxdisk                 shell interativo (Fase 3)
//   msxdisk --cli           mesma coisa, de forma explicita
//   msxdisk --tui [disco]   TUI estilo Norton Commander/XTree (Fase 4)
//   msxdisk --gui [disco]   GUI Dear ImGui (Fase 5)
// De dentro do shell interativo, "tui"/"call tui" e "gui"/"call gui"
// tambem abrem a TUI/GUI sem sair do processo (ver
// src/msxdisk/shell/shell.cpp). O roteamento em si (msxdisk::RunEntryPoint)
// e compartilhado com "fwmsx --msxdisk ..." -- ver src/cpp/main.cpp e
// doc/msxdisk-spec.md, Fase 5c.
//

#include <string>
#include <vector>

#include "msxdisk/entry.h"

int main(int argc, char **argv) {
    const std::vector<std::string> tokens(argv + 1, argv + argc);
    return msxdisk::RunEntryPoint(tokens);
}
