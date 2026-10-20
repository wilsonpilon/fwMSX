// fwMSX -- editor de textos em JANELA (Dear ImGui + GLFW + OpenGL3): `fwMSX.exe --edit [arquivo [arquivo2]]` / `--view arquivo`.
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/editor-spec.md (fase E3). Pinta a MESMA grade do `ed::EditorApp` que a TUI
// pinta (mesmos recursos e atalhos, por construcao); aqui o teclado entrega cada tecla com seus modificadores (Ctrl-Enter,
// Ctrl-PgUp, Ctrl-2, Alt sozinho abrindo o menu: tudo funciona como no NE original).
#pragma once

#include <string>
#include <vector>

namespace ed {

// args: os mesmos de RunEditorTui (arquivos, --view, --config, --size CxL e --dump nao se aplicam).
int RunEditorWindow(const std::vector<std::string> &args, const std::string &argv0, bool as_view);

} // namespace ed
