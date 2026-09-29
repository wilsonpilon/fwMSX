//
// msxdisk (fwMSX): roteamento comum dos 4 modos (CLI one-shot, shell,
// TUI, GUI) a partir de uma lista de tokens sem o nome do programa.
//
// Usado por tools/msxdisk/main.cpp (executavel standalone) e por
// src/cpp/main.cpp (fwMSX --msxdisk ..., Fase 5c) -- os dois pontos de
// entrada chamam a mesma funcao, entao o comportamento e identico nos
// dois binarios. Ver doc/msxdisk-spec.md, Fase 5c, e doc/SPEC.md, secao
// 5.1.
//
#pragma once

#include <string>
#include <vector>

namespace msxdisk {

int RunEntryPoint(const std::vector<std::string> &tokens);

} // namespace msxdisk
