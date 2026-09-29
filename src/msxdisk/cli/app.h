//
// msxdisk (fwMSX): definicao da CLI11 App e despacho de comandos.
//
// Usado tanto pelo modo CLI one-shot (main() chama Dispatch uma vez)
// quanto pelo shell interativo da Fase 3 (RunShell chama Dispatch a cada
// linha digitada) -- a App e reconstruida do zero em cada chamada para
// nao vazar valores de uma invocacao pra outra.
//
#pragma once

#include <string>
#include <vector>

namespace msxdisk::cli {

// 'tokens' e a linha de comando ja tokenizada, SEM o nome do programa
// (ex.: {"list", "disco.dsk", "*.BAS"}). Devolve o codigo de saida do
// comando (0 = sucesso).
int Dispatch(const std::vector<std::string> &tokens);

} // namespace msxdisk::cli
