//
// msxdisk-tui (fwMSX): aplicacao FTXUI -- Fase 4.
//
#pragma once

#include <string>

#include "../config/config_store.h"
#include "pane_state.h"

namespace msxdisk::tui {

// Que dialogo modal esta ativo (None = nenhum, navegacao normal).
// TextInput* pede uma linha de texto (Enter confirma, Esc cancela);
// Confirm pede uma confirmacao de uma tecla (s/n).
enum class DialogKind {
    None,
    NewImage,
    OpenImage,
    SaveAsImage,
    RenameEntry,
    MakeDirectory,
    ConfirmDelete,
};

struct AppState {
    LocalPaneState local;
    DiskPaneState disk;
    ActivePane active = ActivePane::Local;
    std::string status_message;
    msxdisk::config::Theme theme;
    msxdisk::config::ConfigStore *config = nullptr; // nao possui; dono e o main()

    DialogKind dialog = DialogKind::None;
    std::string dialog_title;
    std::string dialog_input;
};

// Roda o loop principal da TUI ate o usuario sair (F10/Esc). 'state' deve
// vir com local.cwd/disk.image (se houver) e theme ja preenchidos.
void RunApp(AppState &state);

// Ponto de entrada de alto nivel da TUI: abre o ConfigStore, monta o
// AppState (diretorio local atual, tema salvo, imagem de 'image_path' se
// nao vazio) e roda RunApp. Usado tanto por "msxdisk --tui" quanto pelo
// comando "tui"/"call tui" de dentro do shell interativo -- ver
// src/msxdisk/shell/shell.cpp.
int LaunchTui(const std::string &image_path = "");

} // namespace msxdisk::tui
