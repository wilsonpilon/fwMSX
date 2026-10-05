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

// Seletor generico: `title` na barra, `filter_name` e `patterns` (ex.: "*.rom;*.mx1")
// para o filtro principal, alem de "Todos os arquivos". Devolve nullopt se cancelar
// ou se a plataforma nao tem seletor nativo.
std::optional<std::string> ShowOpenFileDialog(GLFWwindow *window, const std::string &title,
                                              const std::string &filter_name, const std::string &patterns);

} // namespace msxdisk::gui
