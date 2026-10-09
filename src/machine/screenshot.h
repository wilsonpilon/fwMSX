// fwMSX -- captura da tela da maquina em PNG (usa o miniz, que so' o executavel principal linka).
// Compartilhada pela janela (F12) e pela TUI.
#pragma once

#include <string>

#include "machine.h"

namespace machine {

// Grava a tela atual (com a proporcao certa: o MSX2 de 512 colunas ganha linhas duplicadas) em `path`
// (vazio = "fwmsx-AAAAMMDD-HHMMSS.png" na pasta de trabalho). Devolve o nome gravado, ou "" com a razao
// em `error`.
std::string SaveScreenshotPng(const Machine &machine, std::string &error, const std::string &path = std::string());

} // namespace machine
