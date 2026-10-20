// fwMSX -- editor de textos: traducao da entrada do terminal (bytes e sequencias de escape) para o nome da tecla que
// o modelo entende. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Fica fora do FTXUI para ser testado com texto puro.
// Ver doc/editor-spec.md, secao 3 (teclas no console).
#pragma once

#include <string>

namespace ed {

// `input` e' o que o terminal entregou para UMA tecla ("\x17" = Ctrl-W, "\x1b" "x" = Alt-X, "\x1b[1;5D" = Ctrl-Esquerda,
// "\x1b[3~" = Del...). Devolve o nome canonico ("C-w", "A-x", "C-Left", "Del"), o proprio texto se for um caractere
// imprimivel (ou UTF-8), ou "" se a tecla nao tem significado. Convencoes do console:
//   0x08 = Ctrl-Backspace ("C-BS"), 0x7F = Backspace ("BS"), 0x00 = Ctrl-2 ("C-2"), 0x1D = Ctrl-] ("C-]").
std::string KeyNameFromInput(const std::string &input);

// true se `name` (devolvido por KeyNameFromInput) e' texto digitado, e nao o nome de uma tecla.
bool IsTextKey(const std::string &name);

// UTF-8 -> bytes do editor (Latin-1: o que o terminal digita e cabe em um byte; o resto e' descartado).
std::string Utf8ToBytes(const std::string &utf8);

} // namespace ed
