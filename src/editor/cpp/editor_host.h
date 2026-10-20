// fwMSX -- editor de textos: ligacoes com o resto do programa (so' no executavel, nunca nos testes do editor). Ver editor_build.cpp.
#pragma once

#include "editor_ui.h"

namespace ed {

// Liga a ajuda da palavra (Help > On word, Ctrl-F1) ao `man` do fwMSX.
void AttachManHelp(EditorApp &app);

} // namespace ed
