//
// msxdisk (fwMSX): dialogos nativos de arquivo do Windows (navegacao de
// pastas de verdade) para Novo/Abrir/Salvar Como -- Fase 5, a pedido do
// autor (as caixas de texto simples viraram diretorio de dialogos
// nativos aqui; Renomear/Nova Pasta continuam com o popup de texto do
// ImGui, ja que sao so um nome, nao um caminho completo).
//
#pragma once

#include <optional>
#include <string>

struct GLFWwindow;

namespace msxdisk::gui {

// GetOpenFileName do Windows, filtrado para *.dsk. Devolve nullopt se o
// usuario cancelar.
std::optional<std::string> ShowOpenDskDialog(GLFWwindow *window);

// GetSaveFileName do Windows, filtrado para *.dsk. 'initial_path' (pode
// ser vazio) preenche o campo de nome inicial. Devolve nullopt se o
// usuario cancelar.
std::optional<std::string> ShowSaveDskDialog(GLFWwindow *window, const std::string &initial_path);

} // namespace msxdisk::gui
