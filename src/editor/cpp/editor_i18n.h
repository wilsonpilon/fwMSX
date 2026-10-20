// fwMSX -- editor de textos: os textos da interface grafica em PORTUGUES. A TUI mantem os textos do Norton Editor 2 (em ingles);
// a janela grafica traduz na hora de desenhar com `Tr`. Sem acento (convencao do projeto). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#pragma once

#include <string>

namespace ed {

// Traduz um texto da interface (rotulo de menu, titulo/rotulo/botao de dialogo, ajuda). Texto desconhecido volta como veio.
std::string Tr(const std::string &en);

} // namespace ed
