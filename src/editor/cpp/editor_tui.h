// fwMSX -- editor de textos na TUI (FTXUI): `fwMSXc --edit [arquivo [arquivo2]]` e `fwMSXc --view arquivo`. Codigo ORIGINAL do
// fwMSX (BSD-3-Clause). Ver doc/editor-spec.md. So' traduz a entrada do terminal para teclas/mouse do `ed::EditorApp` e pinta
// a grade dele; toda a logica esta' no modelo (testado sem terminal).
#pragma once

#include <string>
#include <vector>

namespace ed {

// args: arquivos (ate' 2) e opcoes: --view (somente leitura), --dump (imprime a tela e sai, sem terminal), --keys "A-f o ..."
// (teclas a apertar antes do --dump), --size CxL (tamanho do --dump), --config <arquivo>.
// `as_view`: veio de `--view` (todos os arquivos em somente leitura).
int RunEditorTui(const std::vector<std::string> &args, const std::string &argv0, bool as_view);

} // namespace ed
