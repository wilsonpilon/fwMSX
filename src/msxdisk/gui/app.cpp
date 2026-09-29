//
// msxdisk (fwMSX): GUI (Dear ImGui + GLFW + OpenGL3) -- ver app.h.
//
// Fase 5b: interface de verdade -- duas colunas (local <-> imagem MSX),
// marcar com checkbox, duplo-clique navega, Novo/Abrir/Salvar/SalvarComo/
// Ejetar/Renomear/NovaPasta/Excluir via menu ou tecla de funcao (mesmo
// esquema da TUI). Novo/Abrir/SalvarComo usam o dialogo nativo de
// arquivo do Windows (file_dialog.h, navegacao de pastas de verdade);
// Renomear/NovaPasta/Excluir/Ejetar usam dialogos modais do proprio
// ImGui (so pedem um nome/confirmacao, nao um caminho completo).
//
// Reaproveita o estado/navegacao de src/msxdisk/tui/pane_state.h (ja era
// independente de framework de UI) -- so a apresentacao e as acoes de
// alto nivel (Novo/Abrir/Enviar/etc.) sao proprias daqui, no mesmo
// espirito das de src/msxdisk/tui/app.cpp (duplicadas de proposito, para
// nao criar uma dependencia cruzada entre os modulos gui/ e tui/ so por
// causa de uma struct -- mesma logica de "aceitar duplicacao pela
// simplicidade" ja documentada para os fontes de linguagem em
// doc/msxdisk-spec.md).
//
// Visual: a GUI tem sua PROPRIA identidade (src/msxdisk/gui/style.h) --
// nao usa mais o Theme retro compartilhado com a TUI (ficava "anos 90"
// numa janela grafica de verdade, a pedido do autor). A TUI continua
// com o visual Norton Commander/XTree de proposito.
//

#include "app.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../cli/format.h"
#include "../config/config_store.h"
#include "../core/dir_entry.h"
#include "../core/disk_image.h"
#include "../tui/pane_state.h"
#include "file_dialog.h"
#include "style.h"

namespace msxdisk::gui {

namespace {

using msxdisk::tui::ActivePane;
using msxdisk::tui::DiskPaneState;
using msxdisk::tui::LocalPaneState;
using msxdisk::tui::NavigateDisk;
using msxdisk::tui::NavigateLocal;
using msxdisk::tui::PaneEntry;
using msxdisk::tui::RefreshDiskEntries;
using msxdisk::tui::RefreshLocalEntries;

struct GuiState {
    LocalPaneState local;
    DiskPaneState disk;
    ActivePane active = ActivePane::Local;
    std::string status_message;
    bool dark_mode = true; // estilo moderno proprio da GUI -- ver style.h
    config::ConfigStore *config = nullptr;
};

void GlfwErrorCallback(int error, const char *description) {
    std::fprintf(stderr, "msxdisk: erro do GLFW %d: %s\n", error, description);
}

std::string FormatSize(uint64_t size) {
    if (size < 1024) return std::to_string(size);
    if (size < 1024ull * 1024) return std::to_string(size / 1024) + "K";
    return std::to_string(size / (1024 * 1024)) + "M";
}

// ---------------------------------------------------------------------
// Persistencia (SQLite) -- mesmas chaves da TUI, pra lembrar estado
// independente de qual dos dois modos foi usado por ultimo.
// ---------------------------------------------------------------------

void PersistDiskState(GuiState &state) {
    if (state.config == nullptr) return;
    state.config->SetSetting("last_image_path", state.disk.image_path);
    state.config->RecordImageSeen(state.disk.image_path);
    state.config->AddRecentImage(state.disk.image_path);
}

void PersistLocalDir(GuiState &state) {
    if (state.config == nullptr) return;
    state.config->SetSetting("last_local_dir", state.local.cwd.string());
}

// ---------------------------------------------------------------------
// Acoes -- mesma logica de src/msxdisk/tui/app.cpp, adaptada pra nao
// depender do AppState/DialogKind proprios da TUI.
// ---------------------------------------------------------------------

void ActionNew(GuiState &state, const std::string &path) {
    if (path.empty()) return;
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

void ActionOpen(GuiState &state, const std::string &path) {
    if (path.empty()) return;
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

void ActionSave(GuiState &state) {
    if (!state.disk.image.has_value()) {
        state.status_message = "Nenhuma imagem carregada.";
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

void ActionSaveAs(GuiState &state, const std::string &path) {
    if (!state.disk.image.has_value() || path.empty()) return;
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

void ActionEject(GuiState &state) {
    const std::string path = state.disk.image_path;
    state.disk = DiskPaneState{};
    state.status_message = path.empty() ? "Nenhuma imagem carregada." : ("Disco ejetado: " + path);
}

void ActionSend(GuiState &state) {
    if (!state.disk.image.has_value()) {
        state.status_message = "Nenhuma imagem carregada -- use Novo/Abrir antes de enviar.";
        return;
    }
    std::vector<int> targets;
    for (size_t i = 0; i < state.local.entries.size(); ++i) {
        if (state.local.entries[i].marked) targets.push_back(static_cast<int>(i));
    }
    if (targets.empty() && !state.local.entries.empty() && !state.local.entries[state.local.cursor].is_parent &&
        !state.local.entries[state.local.cursor].is_dir) {
        targets.push_back(state.local.cursor);
    }

    int sent = 0;
    for (int idx : targets) {
        const auto &entry = state.local.entries[idx];
        if (entry.is_dir) continue;
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
        state.status_message = std::to_string(sent) + " arquivo(s) enviado(s) (Salvar grava no disco).";
    } else if (targets.empty()) {
        state.status_message = "Nada marcado/selecionado para enviar.";
    }
}

void ActionReceive(GuiState &state) {
    if (!state.disk.image.has_value()) {
        state.status_message = "Nenhuma imagem carregada -- use Novo/Abrir antes de receber.";
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
        if (entry.is_dir) continue;
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

void ActionRename(GuiState &state, const std::string &new_name) {
    auto &entries = (state.active == ActivePane::Local) ? state.local.entries : state.disk.entries;
    const int cursor = (state.active == ActivePane::Local) ? state.local.cursor : state.disk.cursor;
    if (new_name.empty() || entries.empty() || entries[cursor].is_parent) return;
    const std::string old_name = entries[cursor].name;

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
        if (!state.disk.image.has_value()) return;
        DiskError error = DiskError::None;
        const std::string old_path = cli::JoinMsxPath(state.disk.current_dir, old_name);
        if (!state.disk.image->RenameFile(old_path, new_name, &error)) {
            state.status_message = "Falha ao renomear: " + std::string(cli::DiskErrorMessage(error));
            return;
        }
        state.disk.dirty = true;
        RefreshDiskEntries(state.disk);
        state.status_message = "Renomeado (Salvar grava no disco): " + old_name + " -> " + new_name;
    }
}

void ActionMakeDirectory(GuiState &state, const std::string &name) {
    if (name.empty()) return;
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
        if (!state.disk.image.has_value()) return;
        DiskError error = DiskError::None;
        const std::string path = cli::JoinMsxPath(state.disk.current_dir, name);
        if (!state.disk.image->MakeDirectory(path, &error)) {
            state.status_message = "Falha ao criar pasta: " + std::string(cli::DiskErrorMessage(error));
            return;
        }
        state.disk.dirty = true;
        RefreshDiskEntries(state.disk);
        state.status_message = "Pasta criada (Salvar grava no disco): " + name;
    }
}

void ActionDeleteConfirmed(GuiState &state) {
    auto &entries = (state.active == ActivePane::Local) ? state.local.entries : state.disk.entries;
    const int cursor = (state.active == ActivePane::Local) ? state.local.cursor : state.disk.cursor;

    std::vector<int> targets;
    for (size_t i = 0; i < entries.size(); ++i) {
        if (entries[i].marked) targets.push_back(static_cast<int>(i));
    }
    if (targets.empty() && !entries.empty() && !entries[cursor].is_parent) targets.push_back(cursor);

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
        if (!state.disk.image.has_value()) return;
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
                                (state.active == ActivePane::Disk ? " (Salvar grava no disco)." : ".");
    }
}

void SetDarkMode(GuiState &state, bool dark) {
    state.dark_mode = dark;
    ApplyModernStyle(dark);
    if (state.config != nullptr) state.config->SetSetting("gui_theme", dark ? "dark" : "light");
    state.status_message = std::string("Tema: ") + (dark ? "escuro" : "claro");
}

void ActionCycleTheme(GuiState &state) { SetDarkMode(state, !state.dark_mode); }

// ---------------------------------------------------------------------
// Dialogos modais (ImGui nativo)
// ---------------------------------------------------------------------

enum class ModalResult { None, Confirmed, Cancelled };

ModalResult TextInputModal(const char *popup_id, const char *label, char *buf, size_t buf_size) {
    ModalResult result = ModalResult::None;
    ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal(popup_id, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", label);
        ImGui::SetItemDefaultFocus();
        if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
        const bool enter = ImGui::InputText("##input", buf, buf_size, ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::Separator();
        if (ImGui::Button("OK") || enter) {
            result = ModalResult::Confirmed;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancelar")) {
            result = ModalResult::Cancelled;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return result;
}

// ---------------------------------------------------------------------
// Renderizacao de um painel (local ou disco)
// ---------------------------------------------------------------------

void RenderPane(const char *id, float width, const std::string &title, std::vector<PaneEntry> &entries, int &cursor,
                bool active, GuiState &state, ActivePane pane_kind,
                const std::function<void(const PaneEntry &)> &on_activate) {
    ImGui::BeginChild(id, ImVec2(width, -ImGui::GetFrameHeightWithSpacing() * 2), true,
                       ImGuiWindowFlags_HorizontalScrollbar);

    if (active) {
        ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive), "%s", title.c_str());
    } else {
        ImGui::TextDisabled("%s", title.c_str());
    }
    ImGui::Separator();

    if (ImGui::BeginTable((std::string(id) + "_tbl").c_str(), 3,
                           ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 22);
        ImGui::TableSetupColumn("Nome", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Tamanho", ImGuiTableColumnFlags_WidthFixed, 70);

        for (size_t i = 0; i < entries.size(); ++i) {
            PaneEntry &entry = entries[i];
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            if (!entry.is_parent) {
                bool marked = entry.marked;
                ImGui::PushID(static_cast<int>(i));
                if (ImGui::Checkbox("##mark", &marked)) {
                    entry.marked = marked;
                    // Marcar tambem ativa o painel e seleciona a linha --
                    // sem isso, marcar so pela caixinha (sem clicar no
                    // nome) deixava o painel "ativo" desatualizado, e
                    // F4/F7/F8/F11/Del acabavam operando no painel/item
                    // errado (bug relatado pelo autor: renomear/criar
                    // pasta/excluir pareciam nao fazer nada, ou faziam no
                    // lado errado).
                    cursor = static_cast<int>(i);
                    state.active = pane_kind;
                }
                ImGui::PopID();
            }

            ImGui::TableSetColumnIndex(1);
            const std::string label = entry.is_dir ? ("[" + entry.name + "]") : entry.name;
            const bool selected = (static_cast<int>(i) == cursor) && active;
            ImGui::PushID(static_cast<int>(i) + 100000);
            if (ImGui::Selectable(label.c_str(), selected,
                                   ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                cursor = static_cast<int>(i);
                state.active = pane_kind;
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && entry.is_dir) {
                    on_activate(entry);
                }
            }
            ImGui::PopID();

            ImGui::TableSetColumnIndex(2);
            if (!entry.is_dir) ImGui::Text("%s", FormatSize(entry.size).c_str());
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}

} // namespace

int LaunchGui(const std::string &image_path) {
    auto store = config::ConfigStore::Open();
    if (!store) {
        std::fprintf(stderr, "msxdisk: falha ao abrir o banco de configuracao (%s)\n",
                     config::ConfigStore::DefaultPath().string().c_str());
        return 1;
    }

    GuiState state;
    state.config = &*store;

    const std::string last_dir = store->GetSetting("last_local_dir").value_or("");
    std::error_code cwd_ec;
    state.local.cwd = (!last_dir.empty() && std::filesystem::is_directory(last_dir, cwd_ec))
                           ? std::filesystem::path(last_dir)
                           : std::filesystem::current_path();
    RefreshLocalEntries(state.local);

    state.dark_mode = store->GetSetting("gui_theme").value_or("dark") != "light";

    std::string effective_image_path = image_path.empty() ? store->GetSetting("last_image_path").value_or("") : image_path;
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
            state.status_message = "Falha ao abrir '" + image_path + "': " + cli::DiskErrorMessage(error);
        }
    }

    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) {
        std::fprintf(stderr, "msxdisk: falha ao inicializar o GLFW\n");
        return 1;
    }
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow *window = glfwCreateWindow(1100, 700, "msxdisk", nullptr, nullptr);
    if (window == nullptr) {
        std::fprintf(stderr, "msxdisk: falha ao criar a janela (GLFW)\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    LoadModernFont(io);
    ApplyModernStyle(state.dark_mode);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    char rename_buf[260] = "";
    char mkdir_buf[260] = "";

    // Novo/Abrir/SalvarComo usam o dialogo nativo do Windows (navegacao
    // de pastas de verdade), que ja e modal por conta propria -- nao
    // precisam do popup do ImGui nem do cuidado de escopo de ID abaixo.
    const auto do_new = [&] {
        if (const auto path = ShowSaveDskDialog(window, "")) ActionNew(state, *path);
    };
    const auto do_open = [&] {
        if (const auto path = ShowOpenDskDialog(window)) ActionOpen(state, *path);
    };
    const auto do_saveas = [&] {
        if (const auto path = ShowSaveDskDialog(window, state.disk.image_path)) ActionSaveAs(state, *path);
    };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // IMPORTANTE: OpenPopup()/BeginPopupModal() calculam o ID do popup
        // usando a JANELA ATUAL no momento da chamada (ImGui::GetID() usa
        // g.CurrentWindow como parte do hash). Chamar OpenPopup() de
        // dentro de um BeginMenu() (que e uma sub-janela) ou antes do
        // Begin("msxdisk") deste frame (sem nenhuma janela ainda aberta)
        // gera um ID diferente do que o BeginPopupModal("...") mais abaixo
        // vai procurar -- o popup nunca aparecia. Por isso os atalhos de
        // teclado e os itens de menu so marcam a INTENCAO aqui; o
        // ImGui::OpenPopup(...) de verdade so roda logo apos o
        // Begin("msxdisk"), no mesmo nivel de janela do BeginPopupModal.
        bool want_open_rename = false, want_open_mkdir = false, want_open_delete = false, want_open_eject = false;

        // Atalhos de teclado (mesmo esquema de teclas da TUI), so quando
        // nenhum campo de texto esta com foco.
        if (!ImGui::GetIO().WantTextInput) {
            if (ImGui::IsKeyPressed(ImGuiKey_F2)) do_new();
            if (ImGui::IsKeyPressed(ImGuiKey_F3)) do_open();
            if (ImGui::IsKeyPressed(ImGuiKey_F4)) {
                auto &entries = (state.active == ActivePane::Local) ? state.local.entries : state.disk.entries;
                const int cursor = (state.active == ActivePane::Local) ? state.local.cursor : state.disk.cursor;
                if (!entries.empty() && !entries[cursor].is_parent) {
                    std::snprintf(rename_buf, sizeof(rename_buf), "%s", entries[cursor].name.c_str());
                    want_open_rename = true;
                }
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F5)) ActionSave(state);
            if (ImGui::IsKeyPressed(ImGuiKey_F6)) do_saveas();
            if (ImGui::IsKeyPressed(ImGuiKey_F7)) ActionSend(state);
            if (ImGui::IsKeyPressed(ImGuiKey_F8)) ActionReceive(state);
            if (ImGui::IsKeyPressed(ImGuiKey_F9)) ActionCycleTheme(state);
            if (ImGui::IsKeyPressed(ImGuiKey_F11)) { mkdir_buf[0] = '\0'; want_open_mkdir = true; }
            if (ImGui::IsKeyPressed(ImGuiKey_F12)) want_open_eject = true;
            if (ImGui::IsKeyPressed(ImGuiKey_Delete)) want_open_delete = true;
        }

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("msxdisk", nullptr,
                      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Arquivo")) {
                if (ImGui::MenuItem("Novo...", "F2")) do_new();
                if (ImGui::MenuItem("Abrir...", "F3")) do_open();
                if (ImGui::MenuItem("Salvar", "F5")) ActionSave(state);
                if (ImGui::MenuItem("Salvar como...", "F6")) do_saveas();
                if (ImGui::MenuItem("Ejetar", "F12", false, state.disk.image.has_value())) want_open_eject = true;
                ImGui::Separator();
                if (ImGui::MenuItem("Sair")) glfwSetWindowShouldClose(window, true);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Acao")) {
                if (ImGui::MenuItem("Enviar (local -> disco)", "F7")) ActionSend(state);
                if (ImGui::MenuItem("Receber (disco -> local)", "F8")) ActionReceive(state);
                ImGui::Separator();
                if (ImGui::MenuItem("Renomear...", "F4")) {
                    auto &entries = (state.active == ActivePane::Local) ? state.local.entries : state.disk.entries;
                    const int cursor = (state.active == ActivePane::Local) ? state.local.cursor : state.disk.cursor;
                    if (!entries.empty() && !entries[cursor].is_parent) {
                        std::snprintf(rename_buf, sizeof(rename_buf), "%s", entries[cursor].name.c_str());
                        want_open_rename = true;
                    }
                }
                if (ImGui::MenuItem("Nova pasta...", "F11")) { mkdir_buf[0] = '\0'; want_open_mkdir = true; }
                if (ImGui::MenuItem("Excluir", "Del")) want_open_delete = true;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tema")) {
                if (ImGui::MenuItem("Escuro", nullptr, state.dark_mode)) SetDarkMode(state, true);
                if (ImGui::MenuItem("Claro", nullptr, !state.dark_mode)) SetDarkMode(state, false);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Dispara os OpenPopup() de verdade aqui -- mesmo nivel de janela
        // (direto dentro de "msxdisk", fora de qualquer BeginMenu) que os
        // BeginPopupModal() mais abaixo, senao o ID nao bate (ver nota
        // grande acima).
        if (want_open_rename) ImGui::OpenPopup("Renomear");
        if (want_open_mkdir) ImGui::OpenPopup("Nova pasta");
        if (want_open_delete) ImGui::OpenPopup("Confirmar exclusao");
        if (want_open_eject) {
            // So pergunta se ha algo a perder; sem alteracoes pendentes,
            // ejeta na hora.
            if (state.disk.dirty) {
                ImGui::OpenPopup("Confirmar ejecao");
            } else {
                ActionEject(state);
            }
        }

        const std::string local_title = "LOCAL: " + state.local.cwd.string();
        const std::string disk_title =
            state.disk.image.has_value()
                ? ("DISCO: " + state.disk.image_path +
                   (state.disk.current_dir.empty() ? "" : ("\\" + state.disk.current_dir)) +
                   (state.disk.dirty ? " *" : ""))
                : std::string("DISCO: (nenhuma imagem -- Arquivo > Novo/Abrir)");

        const float pane_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;

        RenderPane("local_pane", pane_width, local_title, state.local.entries, state.local.cursor,
                   state.active == ActivePane::Local, state, ActivePane::Local,
                   [&](const PaneEntry &e) { NavigateLocal(state.local, e); });
        ImGui::SameLine();
        RenderPane("disk_pane", pane_width, disk_title, state.disk.entries, state.disk.cursor,
                   state.active == ActivePane::Disk, state, ActivePane::Disk,
                   [&](const PaneEntry &e) { NavigateDisk(state.disk, e); });

        ImGui::Separator();
        ImGui::TextWrapped("%s", state.status_message.empty() ? " " : state.status_message.c_str());

        // Dialogos modais restantes (o popup so aparece quando aberto via
        // OpenPopup acima).
        if (TextInputModal("Renomear", "Novo nome:", rename_buf, sizeof(rename_buf)) == ModalResult::Confirmed) {
            ActionRename(state, rename_buf);
        }
        if (TextInputModal("Nova pasta", "Nome da nova pasta:", mkdir_buf, sizeof(mkdir_buf)) ==
            ModalResult::Confirmed) {
            ActionMakeDirectory(state, mkdir_buf);
        }
        if (ImGui::BeginPopupModal("Confirmar exclusao", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Excluir os itens marcados (ou o selecionado)?");
            if (ImGui::Button("Sim")) {
                ActionDeleteConfirmed(state);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Nao")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        if (ImGui::BeginPopupModal("Confirmar ejecao", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Ha alteracoes nao salvas nesta imagem. Ejetar mesmo assim?");
            if (ImGui::Button("Sim")) {
                ActionEject(state);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Nao")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImGui::End();

        ImGui::Render();
        int display_w = 0;
        int display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        float clear_r = 0.0f, clear_g = 0.0f, clear_b = 0.0f;
        ModernClearColor(state.dark_mode, &clear_r, &clear_g, &clear_b);
        glClearColor(clear_r, clear_g, clear_b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    PersistLocalDir(state);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

} // namespace msxdisk::gui
