//
// msxdisk-tui (fwMSX): estado dos dois paineis (local/disco) -- Fase 4.
//
// Deliberadamente sem nenhuma dependencia do FTXUI aqui -- so dados e a
// logica de navegacao/listagem, pra poder ser testado/entendido sem
// precisar de terminal. A camada de renderizacao fica em app.cpp.
//
#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "../core/disk_image.h"

namespace msxdisk::tui {

// Uma linha da listagem, ja no formato comum aos dois paineis (local e
// disco tem estruturas de diretorio bem diferentes por baixo, mas para a
// UI ambos viram a mesma coisa: nome, se e pasta, tamanho, se esta
// marcado para transferencia).
struct PaneEntry {
    std::string name;
    bool is_dir = false;
    uint64_t size = 0;
    bool marked = false;
    bool is_parent = false; // entrada sintetica ".." (nao pode ser marcada)
};

struct LocalPaneState {
    std::filesystem::path cwd;
    std::vector<PaneEntry> entries;
    int cursor = 0;
};

struct DiskPaneState {
    std::optional<msxdisk::DiskImage> image;
    std::string image_path;  // absoluto (mesma licao do shell -- ver session.cpp)
    std::string current_dir; // caminho MSX atual, "" = raiz
    std::vector<PaneEntry> entries;
    int cursor = 0;
    bool dirty = false;
};

enum class ActivePane { Local, Disk };

// Recarrega 'entries' a partir do diretorio atual (dirs primeiro, depois
// arquivos, ambos em ordem alfabetica; ".." no topo se nao for a raiz).
void RefreshLocalEntries(LocalPaneState &pane);
void RefreshDiskEntries(DiskPaneState &pane);

// Aplica Enter sobre 'entry' (deve ser um diretorio, incluindo "..").
void NavigateLocal(LocalPaneState &pane, const PaneEntry &entry);
void NavigateDisk(DiskPaneState &pane, const PaneEntry &entry);

} // namespace msxdisk::tui
