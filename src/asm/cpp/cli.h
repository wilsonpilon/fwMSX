// fwMSX -- montador (fwasm): linha de comando (fwmsx --asm) e comando "asm" do console. Mesmas opcoes do asMSX.
#pragma once

#include <string>
#include <vector>

namespace fwasm {

// tokens = argumentos depois de --asm: [-z] [-s] [-vv] [-r] [-o <caminho>] <arquivo.asm>. Devolve o codigo de saida
// do montador (0 = ok; 2 = uso incorreto ou arquivo ilegivel).
int RunAsmCommand(const std::vector<std::string> &tokens);

} // namespace fwasm
