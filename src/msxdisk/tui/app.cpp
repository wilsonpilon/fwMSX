//
// msxdisk-tui (fwMSX): aplicacao FTXUI -- ver app.h.
//

#include "app.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "../cli/format.h"
#include "../core/dir_entry.h"

namespace msxdisk::tui {

namespace {

using namespace ftxui;

Color ToFtxuiColor(const config::RgbColor &c) { return Color::RGB(c.r, c.g, c.b); }

std::string FormatSize(uint64_t size) {
    if (size < 1024) return std::to_string(size);
    if (size < 1024ull * 1024) return std::to_string(size / 1024) + "K";
    return std::to_string(size / (1024 * 1024)) + "M";
}

// ---------------------------------------------------------------------
// Renderizacao
// ---------------------------------------------------------------------

Element RenderEntryRow(const PaneEntry &e, bool is_cursor, bool active_pane, const config::Theme &theme) {
    const std::string mark = e.marked ? "*" : " ";
    const std::string label = e.is_dir ? ("[" + e.name + "]") : e.name;
    const std::string size_str = e.is_dir ? std::string() : FormatSize(e.size);

    Element row = hbox({
        text(mark),
        text(label) | flex,
        text(size_str),
    });

    if (e.marked) row = row | color(ToFtxuiColor(theme.fg_marked));
    if (is_cursor) {
        row = row | bgcolor(ToFtxuiColor(theme.bg_selected)) | color(ToFtxuiColor(theme.fg_selected));
        if (active_pane) row = focus(row);
    }
    return row;
}

Element RenderPaneBox(const std::string &title, const std::vector<PaneEntry> &entries, int cursor, bool active,
                       const config::Theme &theme) {
    Elements rows;
    for (size_t i = 0; i < entries.size(); ++i) {
        rows.push_back(RenderEntryRow(entries[i], static_cast<int>(i) == cursor, active, theme));
    }
    if (rows.empty()) rows.push_back(text("(vazio)"));

    Element title_line = text(title) | bold | center;
    if (active) {
        title_line = title_line | bgcolor(ToFtxuiColor(theme.bg_selected)) | color(ToFtxuiColor(theme.fg_selected));
    }

    Element body = vbox({
        title_line,
        separator(),
        frame(vbox(rows)) | flex,
    });

    return body | border | bgcolor(ToFtxuiColor(theme.bg_normal)) | color(ToFtxuiColor(theme.fg_normal)) | flex;
}

Element RenderTopBar(const config::Theme &theme) {
    // Duas linhas para caber em terminais de 80 colunas (um hbox so nao
    // quebra linha sozinho no FTXUI).
    const auto item = [](const std::string &key, const std::string &label) {
        return hbox({text(key) | bold, text(" " + label + "  ")});
    };
    Element row1 = hbox({
        item("F2", "Novo"),
        item("F3", "Abrir"),
        item("F4", "Renomear"),
        item("F5", "Salvar"),
        item("F6", "SalvarComo"),
    });
    Element row2 = hbox({
        item("F7", "Enviar>"),
        item("F8", "<Receber"),
        item("F9", "Tema"),
        item("F10", "Sair"),
        item("F11", "NovaPasta"),
        item("Del", "Excluir"),
    });
    return vbox({row1, row2}) | bgcolor(ToFtxuiColor(theme.bg_titlebar)) | color(ToFtxuiColor(theme.fg_titlebar));
}

Element RenderStatusBar(const std::string &message, const config::Theme &theme) {
    return text(message.empty() ? std::string(" ") : message) | bgcolor(ToFtxuiColor(theme.bg_statusbar)) |
           color(ToFtxuiColor(theme.fg_statusbar));
}

Element RenderDialog(const AppState &state) {
    Element body;
    if (state.dialog == DialogKind::ConfirmDelete) {
        body = vbox({
            text(state.dialog_title) | bold,
            separator(),
            text("Confirma? (s/n)"),
        });
    } else {
        body = vbox({
            text(state.dialog_title) | bold,
            separator(),
            hbox({text("> "), text(state.dialog_input), text("_")}),
        });
    }
    return body | size(WIDTH, GREATER_THAN, 44) | border | bgcolor(ToFtxuiColor(state.theme.bg_titlebar)) |
           color(ToFtxuiColor(state.theme.fg_titlebar));
}

// ---------------------------------------------------------------------
// Persistencia (SQLite) -- Fase 4d
// ---------------------------------------------------------------------

void PersistDiskState(AppState &state) {
    if (state.config == nullptr) return;
    state.config->SetSetting("last_image_path", state.disk.image_path);
    state.config->RecordImageSeen(state.disk.image_path);
    state.config->AddRecentImage(state.disk.image_path);
}

void PersistLocalDir(AppState &state) {
    if (state.config == nullptr) return;
    state.config->SetSetting("last_local_dir", state.local.cwd.string());
}

// ---------------------------------------------------------------------
// Acoes de imagem (Novo/Abrir/Salvar/SalvarComo) -- Fase 4c
// ---------------------------------------------------------------------

void ActionNew(AppState &state, const std::string &path) {
    if (path.empty()) {
        state.status_message = "Novo: caminho vazio, cancelado.";
        return;
    }
    DiskImage image = DiskImage::CreateBlank();
    DiskError error = DiskError::None;
    if (!image.Save(path, &error)) {
        state.status_message = "Falha ao criar '" + path + "': " + cli::DiskErrorMessage(error);
        return;
    }

    std::error_code ec;
    const auto abs = std::filesystem::absolute(path, ec);
    state.disk.image = std::move(image);
    state.disk.image_path = ec ? path : abs.string();
    state.disk.current_dir.clear();
    state.disk.cursor = 0;
    state.disk.dirty = false;
    RefreshDiskEntries(state.disk);
    PersistDiskState(state);
    state.status_message = "Imagem criada: " + path;
}

void ActionOpen(AppState &state, const std::string &path) {
    if (path.empty()) {
        state.status_message = "Abrir: caminho vazio, cancelado.";
        return;
    }
    DiskError error = DiskError::None;
    auto image = DiskImage::Load(path, &error);
    if (!image) {
        state.status_message = "Falha ao abrir '" + path + "': " + cli::DiskErrorMessage(error);
        return;
    }

    std::error_code ec;
    const auto abs = std::filesystem::absolute(path, ec);
    state.disk.image = std::move(image);
    state.disk.image_path = ec ? path : abs.string();
    state.disk.current_dir.clear();
    state.disk.cursor = 0;
    state.disk.dirty = false;
    RefreshDiskEntries(state.disk);
    PersistDiskState(state);
    state.status_message = "Imagem carregada: " + path;
}

void ActionSave(AppState &state) {
    if (!state.disk.image.has_value()) {
        state.status_message = "Nenhuma imagem carregada -- use F2/F3.";
        return;
    }
    DiskError error = DiskError::None;
    if (!state.disk.image->Save(state.disk.image_path, &error)) {
        state.status_message = "Falha ao salvar: " + std::string(cli::DiskErrorMessage(error));
        return;
    }
    state.disk.dirty = false;
    state.status_message = "Salvo: " + state.disk.image_path;
}

void ActionSaveAs(AppState &state, const std::string &path) {
    if (!state.disk.image.has_value()) {
        state.status_message = "Nenhuma imagem carregada -- use F2/F3.";
        return;
    }
    if (path.empty()) {
        state.status_message = "SalvarComo: caminho vazio, cancelado.";
        return;
    }
    DiskError error = DiskError::None;
    if (!state.disk.image->Save(path, &error)) {
        state.status_message = "Falha ao salvar '" + path + "': " + cli::DiskErrorMessage(error);
        return;
    }
    std::error_code ec;
    const auto abs = std::filesystem::absolute(path, ec);
    state.disk.image_path = ec ? path : abs.string();
    state.disk.dirty = false;
    PersistDiskState(state);
    state.status_message = "Salvo como: " + path;
}

// ---------------------------------------------------------------------
// Transferencia (Enviar/Receber marcados) -- Fase 4c
// ---------------------------------------------------------------------

void ActionSend(AppState &state) {
    if (!state.disk.image.has_value()) {
        state.status_message = "Nenhuma imagem carregada -- use F2/F3 antes de enviar.";
        return;
    }

    std::vector<int> targets;
    for (size_t i = 0; i < state.local.entries.size(); ++i) {
        if (state.local.entries[i].marked) targets.push_back(static_cast<int>(i));
    }
    if (targets.empty() && !state.local.entries.empty() && !state.local.entries[state.local.cursor].is_parent &&
        !state.local.entries[state.local.cursor].is_dir) {
        targets.push_back(state.local.cursor); // nada marcado: usa o arquivo sob o cursor
    }

    int sent = 0;
    for (int idx : targets) {
        const auto &entry = state.local.entries[idx];
        if (entry.is_dir) continue; // Fase 4c nao envia pastas inteiras
        const auto host_path = state.local.cwd / entry.name;
        DiskError error = DiskError::None;
        if (state.disk.image->AddFile(host_path, entry.name, msxdisk::kAttrArchive, &error)) {
            state.local.entries[idx].marked = false;
            ++sent;
        } else {
            state.status_message = "Falha ao enviar '" + entry.name + "': " + cli::DiskErrorMessage(error);
        }
    }
    if (sent > 0) {
        state.disk.dirty = true;
        RefreshDiskEntries(state.disk);
        state.status_message = std::to_string(sent) + " arquivo(s) enviado(s) (F5 salva no disco).";
    } else if (targets.empty()) {
        state.status_message = "Nada marcado/selecionado para enviar.";
    }
}

void ActionReceive(AppState &state) {
    if (!state.disk.image.has_value()) {
        state.status_message = "Nenhuma imagem carregada -- use F2/F3 antes de receber.";
        return;
    }

    std::vector<int> targets;
    for (size_t i = 0; i < state.disk.entries.size(); ++i) {
        if (state.disk.entries[i].marked) targets.push_back(static_cast<int>(i));
    }
    if (targets.empty() && !state.disk.entries.empty() && !state.disk.entries[state.disk.cursor].is_parent &&
        !state.disk.entries[state.disk.cursor].is_dir) {
        targets.push_back(state.disk.cursor);
    }

    int received = 0;
    for (int idx : targets) {
        const auto &entry = state.disk.entries[idx];
        if (entry.is_dir) continue; // Fase 4c nao recebe pastas inteiras
        const std::string msx_path = cli::JoinMsxPath(state.disk.current_dir, entry.name);
        const auto host_path = state.local.cwd / entry.name;
        DiskError error = DiskError::None;
        if (state.disk.image->ExtractFile(msx_path, host_path, &error)) {
            state.disk.entries[idx].marked = false;
            ++received;
        } else {
            state.status_message = "Falha ao receber '" + entry.name + "': " + cli::DiskErrorMessage(error);
        }
    }
    if (received > 0) {
        RefreshLocalEntries(state.local);
        state.status_message = std::to_string(received) + " arquivo(s) recebido(s) em " + state.local.cwd.string();
    } else if (targets.empty()) {
        state.status_message = "Nada marcado/selecionado para receber.";
    }
}

// ---------------------------------------------------------------------
// Renomear / criar pasta / excluir -- Fase 4d
// ---------------------------------------------------------------------

void ActionRename(AppState &state, const std::string &new_name) {
    auto &pane_entries = (state.active == ActivePane::Local) ? state.local.entries : state.disk.entries;
    const int cursor = (state.active == ActivePane::Local) ? state.local.cursor : state.disk.cursor;
    if (new_name.empty() || pane_entries.empty() || pane_entries[cursor].is_parent) {
        state.status_message = "Renomear: cancelado.";
        return;
    }
    const std::string old_name = pane_entries[cursor].name;

    if (state.active == ActivePane::Local) {
        std::error_code ec;
        std::filesystem::rename(state.local.cwd / old_name, state.local.cwd / new_name, ec);
        if (ec) {
            state.status_message = "Falha ao renomear: " + ec.message();
            return;
        }
        RefreshLocalEntries(state.local);
        state.status_message = "Renomeado: " + old_name + " -> " + new_name;
    } else {
        if (!state.disk.image.has_value()) {
            state.status_message = "Nenhuma imagem carregada.";
            return;
        }
        DiskError error = DiskError::None;
        const std::string old_path = cli::JoinMsxPath(state.disk.current_dir, old_name);
        if (!state.disk.image->RenameFile(old_path, new_name, &error)) {
            state.status_message = "Falha ao renomear: " + std::string(cli::DiskErrorMessage(error));
            return;
        }
        state.disk.dirty = true;
        RefreshDiskEntries(state.disk);
        state.status_message = "Renomeado (use F5 para salvar): " + old_name + " -> " + new_name;
    }
}

void ActionMakeDirectory(AppState &state, const std::string &name) {
    if (name.empty()) {
        state.status_message = "NovaPasta: cancelado.";
        return;
    }

    if (state.active == ActivePane::Local) {
        std::error_code ec;
        std::filesystem::create_directory(state.local.cwd / name, ec);
        if (ec) {
            state.status_message = "Falha ao criar pasta: " + ec.message();
            return;
        }
        RefreshLocalEntries(state.local);
        state.status_message = "Pasta criada: " + name;
    } else {
        if (!state.disk.image.has_value()) {
            state.status_message = "Nenhuma imagem carregada.";
            return;
        }
        DiskError error = DiskError::None;
        const std::string path = cli::JoinMsxPath(state.disk.current_dir, name);
        if (!state.disk.image->MakeDirectory(path, &error)) {
            state.status_message = "Falha ao criar pasta: " + std::string(cli::DiskErrorMessage(error));
            return;
        }
        state.disk.dirty = true;
        RefreshDiskEntries(state.disk);
        state.status_message = "Pasta criada (use F5 para salvar): " + name;
    }
}

void ActionDeleteConfirmed(AppState &state) {
    auto &pane_entries = (state.active == ActivePane::Local) ? state.local.entries : state.disk.entries;

    std::vector<int> targets;
    for (size_t i = 0; i < pane_entries.size(); ++i) {
        if (pane_entries[i].marked) targets.push_back(static_cast<int>(i));
    }
    const int cursor = (state.active == ActivePane::Local) ? state.local.cursor : state.disk.cursor;
    if (targets.empty() && !pane_entries.empty() && !pane_entries[cursor].is_parent) {
        targets.push_back(cursor);
    }

    int deleted = 0;
    if (state.active == ActivePane::Local) {
        for (int idx : targets) {
            const auto &entry = state.local.entries[idx];
            if (entry.is_dir) {
                state.status_message = "'" + entry.name + "' e uma pasta -- exclusao de pasta local nao suportada.";
                continue;
            }
            std::error_code ec;
            if (std::filesystem::remove(state.local.cwd / entry.name, ec) && !ec) ++deleted;
        }
        if (deleted > 0) RefreshLocalEntries(state.local);
    } else {
        if (!state.disk.image.has_value()) {
            state.status_message = "Nenhuma imagem carregada.";
            return;
        }
        for (int idx : targets) {
            const auto &entry = state.disk.entries[idx];
            const std::string path = cli::JoinMsxPath(state.disk.current_dir, entry.name);
            DiskError error = DiskError::None;
            const bool ok = entry.is_dir ? state.disk.image->RemoveDirectory(path, &error)
                                          : state.disk.image->DeleteFile(path, &error);
            if (ok) {
                ++deleted;
            } else {
                state.status_message = "Falha ao excluir '" + entry.name + "': " + cli::DiskErrorMessage(error);
            }
        }
        if (deleted > 0) {
            state.disk.dirty = true;
            RefreshDiskEntries(state.disk);
        }
    }

    if (deleted > 0) {
        state.status_message = std::to_string(deleted) + " item(ns) excluido(s)" +
                                (state.active == ActivePane::Disk ? " (use F5 para salvar)." : ".");
    } else if (targets.empty()) {
        state.status_message = "Nada marcado/selecionado para excluir.";
    }
}

// ---------------------------------------------------------------------
// Eventos
// ---------------------------------------------------------------------

void OpenDialog(AppState &state, DialogKind kind, const std::string &title, const std::string &prefill = "") {
    state.dialog = kind;
    state.dialog_title = title;
    state.dialog_input = prefill;
}

void ApplyDialogSubmit(AppState &state, DialogKind kind, const std::string &value) {
    switch (kind) {
        case DialogKind::NewImage: ActionNew(state, value); break;
        case DialogKind::OpenImage: ActionOpen(state, value); break;
        case DialogKind::SaveAsImage: ActionSaveAs(state, value); break;
        case DialogKind::RenameEntry: ActionRename(state, value); break;
        case DialogKind::MakeDirectory: ActionMakeDirectory(state, value); break;
        case DialogKind::ConfirmDelete:
        case DialogKind::None:
            break;
    }
}

bool HandleDialogEvent(AppState &state, Event event) {
    if (state.dialog == DialogKind::ConfirmDelete) {
        if (event.is_character()) {
            const std::string ch = event.character();
            const bool yes = !ch.empty() && (ch[0] == 's' || ch[0] == 'S' || ch[0] == 'y' || ch[0] == 'Y');
            state.dialog = DialogKind::None;
            if (yes) {
                ActionDeleteConfirmed(state);
            } else {
                state.status_message = "Exclusao cancelada.";
            }
        } else if (event == Event::Escape) {
            state.dialog = DialogKind::None;
            state.status_message = "Exclusao cancelada.";
        }
        return true;
    }

    if (event == Event::Escape) {
        state.dialog = DialogKind::None;
        state.status_message = "Cancelado.";
        return true;
    }
    if (event == Event::Return) {
        const DialogKind kind = state.dialog;
        const std::string value = state.dialog_input;
        state.dialog = DialogKind::None;
        state.dialog_input.clear();
        ApplyDialogSubmit(state, kind, value);
        return true;
    }
    if (event == Event::Backspace) {
        if (!state.dialog_input.empty()) state.dialog_input.pop_back();
        return true;
    }
    if (event.is_character()) {
        state.dialog_input += event.character();
        return true;
    }
    return true;
}

bool HandleNormalEvent(AppState &state, ScreenInteractive &screen, Event event) {
    auto &entries = (state.active == ActivePane::Local) ? state.local.entries : state.disk.entries;
    int &cursor = (state.active == ActivePane::Local) ? state.local.cursor : state.disk.cursor;

    if (event == Event::Tab) {
        state.active = (state.active == ActivePane::Local) ? ActivePane::Disk : ActivePane::Local;
        return true;
    }
    if (event == Event::ArrowUp) {
        if (cursor > 0) --cursor;
        return true;
    }
    if (event == Event::ArrowDown) {
        if (cursor + 1 < static_cast<int>(entries.size())) ++cursor;
        return true;
    }
    if (event == Event::Return) {
        if (!entries.empty() && entries[cursor].is_dir) {
            if (state.active == ActivePane::Local) {
                NavigateLocal(state.local, entries[cursor]);
            } else {
                NavigateDisk(state.disk, entries[cursor]);
            }
            state.status_message.clear();
        }
        return true;
    }
    if (event == Event::Character(' ')) {
        if (!entries.empty() && !entries[cursor].is_parent) {
            entries[cursor].marked = !entries[cursor].marked;
        }
        return true;
    }
    if (event == Event::F2) {
        OpenDialog(state, DialogKind::NewImage, "Novo disco -- caminho do .dsk:");
        return true;
    }
    if (event == Event::F3) {
        OpenDialog(state, DialogKind::OpenImage, "Abrir imagem -- caminho do .dsk:");
        return true;
    }
    if (event == Event::F4) {
        if (!entries.empty() && !entries[cursor].is_parent) {
            OpenDialog(state, DialogKind::RenameEntry, "Renomear '" + entries[cursor].name + "' para:",
                       entries[cursor].name);
        }
        return true;
    }
    if (event == Event::F5) {
        ActionSave(state);
        return true;
    }
    if (event == Event::F6) {
        OpenDialog(state, DialogKind::SaveAsImage, "Salvar como -- novo caminho do .dsk:", state.disk.image_path);
        return true;
    }
    if (event == Event::F7) {
        ActionSend(state);
        return true;
    }
    if (event == Event::F8) {
        ActionReceive(state);
        return true;
    }
    if (event == Event::F9) {
        const std::string next_name = (state.theme.name == "classic") ? "dark" : "classic";
        state.theme = config::BuiltinThemeByName(next_name);
        if (state.config != nullptr) state.config->SetSetting("active_theme", next_name);
        state.status_message = "Tema: " + state.theme.name;
        return true;
    }
    if (event == Event::F10 || event == Event::Escape) {
        PersistLocalDir(state);
        screen.Exit();
        return true;
    }
    if (event == Event::F11) {
        OpenDialog(state, DialogKind::MakeDirectory, "Nova pasta -- nome:");
        return true;
    }
    if (event == Event::Delete) {
        const bool any_marked = std::any_of(entries.begin(), entries.end(), [](const PaneEntry &e) { return e.marked; });
        const std::string label = any_marked ? "os itens marcados" : (entries.empty() ? "" : ("'" + entries[cursor].name + "'"));
        if (!label.empty()) {
            OpenDialog(state, DialogKind::ConfirmDelete, "Excluir " + label + "?");
        }
        return true;
    }
    return false;
}

} // namespace

void RunApp(AppState &state) {
    auto screen = ScreenInteractive::Fullscreen();

    auto renderer = Renderer([&] {
        const std::string local_title = "LOCAL: " + state.local.cwd.string();
        const std::string disk_title =
            state.disk.image.has_value()
                ? ("DISCO: " + state.disk.image_path +
                   (state.disk.current_dir.empty() ? "" : ("\\" + state.disk.current_dir)) +
                   (state.disk.dirty ? " *" : ""))
                : std::string("DISCO: (nenhuma imagem carregada -- F2/F3)");

        auto local_box =
            RenderPaneBox(local_title, state.local.entries, state.local.cursor, state.active == ActivePane::Local,
                          state.theme);
        auto disk_box =
            RenderPaneBox(disk_title, state.disk.entries, state.disk.cursor, state.active == ActivePane::Disk,
                          state.theme);

        Element base = vbox({
            RenderTopBar(state.theme),
            hbox({local_box, disk_box}) | flex,
            RenderStatusBar(state.status_message, state.theme),
        });

        if (state.dialog == DialogKind::None) return base;
        return dbox({base, RenderDialog(state) | clear_under | center});
    });

    auto component = CatchEvent(renderer, [&](Event event) -> bool {
        if (state.dialog != DialogKind::None) return HandleDialogEvent(state, event);
        return HandleNormalEvent(state, screen, event);
    });

    screen.Loop(component);
}

int LaunchTui(const std::string &image_path) {
    auto store = config::ConfigStore::Open();
    if (!store) {
        std::cerr << "msxdisk: falha ao abrir o banco de configuracao ("
                   << config::ConfigStore::DefaultPath().string() << ")" << std::endl;
        return 1;
    }

    AppState state;
    state.config = &*store;

    const std::string last_dir = store->GetSetting("last_local_dir").value_or("");
    std::error_code cwd_ec;
    if (!last_dir.empty() && std::filesystem::is_directory(last_dir, cwd_ec)) {
        state.local.cwd = last_dir;
    } else {
        state.local.cwd = std::filesystem::current_path();
    }
    RefreshLocalEntries(state.local);

    const std::string active_theme_name = store->GetSetting("active_theme").value_or("classic");
    state.theme = store->LoadTheme(active_theme_name).value_or(config::ClassicTheme());

    std::string effective_image_path = image_path;
    if (effective_image_path.empty()) {
        effective_image_path = store->GetSetting("last_image_path").value_or("");
    }

    if (!effective_image_path.empty()) {
        DiskError error = DiskError::None;
        auto image = DiskImage::Load(effective_image_path, &error);
        if (image) {
            state.disk.image = std::move(image);
            std::error_code ec;
            const auto abs = std::filesystem::absolute(effective_image_path, ec);
            state.disk.image_path = ec ? effective_image_path : abs.string();
            store->RecordImageSeen(state.disk.image_path);
            store->AddRecentImage(state.disk.image_path);
            RefreshDiskEntries(state.disk);
        } else if (!image_path.empty()) {
            // So mostra erro se o caminho veio explicito do chamador --
            // uma ultima-imagem lembrada que sumiu do disco e silenciosa.
            state.status_message =
                "Falha ao abrir '" + image_path + "': " + cli::DiskErrorMessage(error);
        }
    }

    RunApp(state);
    return 0;
}

} // namespace msxdisk::tui
