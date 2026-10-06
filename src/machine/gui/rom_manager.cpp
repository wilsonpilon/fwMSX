// fwMSX -- interface do banco de ROMs. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "rom_manager.h"

#include <cstring>

#include <imgui.h>

#include "../../msxdisk/gui/file_dialog.h"

namespace machine::gui {
namespace {

const char *kCategories[] = {"todas", "bios", "interface", "cartucho", "disco", "tabela", "outro"};

// Nome para mostrar: o nome do jogo, ou o do arquivo.
std::string Shown(const romdb::RomRecord &r) {
    if (!r.name.empty()) return r.name;
    const size_t slash = r.path.find_last_of("/\\");
    return slash == std::string::npos ? r.path : r.path.substr(slash + 1);
}

void CopyText(char *dst, size_t size, const std::string &src) {
    std::strncpy(dst, src.c_str(), size - 1);
    dst[size - 1] = '\0';
}

} // namespace

RomManager::RomManager(const std::string &root) {
    paths_.root = root;
    browser_url_ = romdb::kFileHunterSystemRoms;
    if (!romdb::OpenRomDb(paths_, db_, db_error_)) db_error_ = db_error_.empty() ? "banco indisponivel" : db_error_;
}

RomManager::~RomManager() {
    if (worker_.joinable()) worker_.join();
}

void RomManager::StartTask(const std::string &title, Work work) {
    if (running_) return;
    if (worker_.joinable()) worker_.join();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        task_title_ = title;
        task_log_.clear();
        task_result_.clear();
        task_has_result_ = false;
    }
    show_status_ = true;
    running_ = true;
    worker_ = std::thread([this, work] {
        // Conexao propria da tarefa: a interface nao compartilha a do banco com a thread.
        romdb::RomDb db;
        std::string error;
        bool ok = romdb::OpenRomDb(paths_, db, error);
        romdb::Log log = [this](const std::string &line) {
            std::lock_guard<std::mutex> lock(mutex_);
            task_log_.push_back(line);
        };
        if (ok) ok = work(paths_, db, log, error);
        std::lock_guard<std::mutex> lock(mutex_);
        task_result_ = ok ? std::string() : error;
        task_has_result_ = true;
        running_ = false;
    });
}

void RomManager::FinishTask() {
    if (worker_.joinable()) worker_.join();
    results_dirty_ = true;
    edit_loaded_ = false;
}

void RomManager::Refresh() {
    results_ = db_.Search(search_, kCategories[category_] == std::string("todas") ? std::string() : kCategories[category_]);
    results_dirty_ = false;
}

void RomManager::DrawMenu(GLFWwindow *window) {
    (void)window;
    const bool busy = running_;
    if (ImGui::MenuItem("Baixar fMSX 6.0 (Windows)", nullptr, false, !busy)) {
        StartTask("fMSX 6.0", [](const romdb::RomPaths &p, romdb::RomDb &d, const romdb::Log &l, std::string &e) {
            int64_t added = 0;
            return romdb::DownloadFmsx(p, d, l, added, e);
        });
    }
    if (ImGui::MenuItem("Baixar System ROMs (file-hunter, mais recente)", nullptr, false, !busy)) {
        StartTask("System ROMs (file-hunter)", [](const romdb::RomPaths &p, romdb::RomDb &d, const romdb::Log &l, std::string &e) {
            int64_t added = 0;
            return romdb::DownloadFileHunterFullSet(p, d, "", l, added, e);
        });
    }
    if (ImGui::MenuItem("Navegar file-hunter...")) {
        show_browser_ = true;
        if (!browser_loaded_) {
            browser_url_ = romdb::kFileHunterSystemRoms;
            browser_loaded_ = false;
        }
    }
    if (ImGui::MenuItem("Baixar banco do Vampier (SQL)", nullptr, false, !busy)) {
        StartTask("Banco do Vampier", [](const romdb::RomPaths &p, romdb::RomDb &d, const romdb::Log &l, std::string &e) {
            int64_t rows = 0;
            return romdb::ImportVampier(p, d, l, rows, e);
        });
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Banco de ROMs (busca e CRUD)...")) {
        show_db_ = true;
        results_dirty_ = true;
    }
    if (ImGui::MenuItem("Escanear pasta de ROMs", nullptr, false, !busy)) {
        StartTask("Escanear pasta", [](const romdb::RomPaths &p, romdb::RomDb &d, const romdb::Log &l, std::string &e) {
            int64_t added = 0, updated = 0;
            l("Lendo " + p.root + " ...");
            return d.ScanDirectory(p.root, p.root, "scan", added, updated, e);
        });
    }
    if (ImGui::MenuItem("Identificar ROMs pelo Vampier", nullptr, false, !busy)) {
        StartTask("Identificar", [](const romdb::RomPaths &, romdb::RomDb &d, const romdb::Log &l, std::string &e) {
            const int64_t named = d.IdentifyWithVampier(e);
            if (!e.empty()) return false;
            l(std::to_string(named) + " ROM(s) receberam nome.");
            return true;
        });
    }
    ImGui::Separator();
    ImGui::TextDisabled("Pasta: %s", paths_.root.c_str());
    if (!db_error_.empty()) ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.4f, 1.0f), "%s", db_error_.c_str());
}

void RomManager::DrawStatus() {
    if (!show_status_) return;
    ImGui::SetNextWindowSize(ImVec2(520, 0), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Banco de ROMs: tarefa", &show_status_)) {
        std::lock_guard<std::mutex> lock(mutex_);
        ImGui::Text("%s", task_title_.c_str());
        if (running_) {
            ImGui::TextDisabled("em andamento... (a maquina continua rodando)");
        }
        const size_t from = task_log_.size() > 8 ? task_log_.size() - 8 : 0;
        for (size_t i = from; i < task_log_.size(); ++i) ImGui::TextWrapped("%s", task_log_[i].c_str());
        if (task_has_result_) {
            if (task_result_.empty()) {
                ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Concluido.");
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.4f, 1.0f), "Erro: %s", task_result_.c_str());
            }
            if (ImGui::Button("Fechar")) show_status_ = false;
        }
    }
    ImGui::End();
    if (!running_ && worker_.joinable()) FinishTask();
}

void RomManager::DrawDatabase(GLFWwindow *window) {
    if (!show_db_) return;
    ImGui::SetNextWindowSize(ImVec2(900, 560), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Banco de ROMs", &show_db_)) {
        ImGui::End();
        return;
    }
    if (!db_.is_open()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.4f, 1.0f), "%s", db_error_.c_str());
        ImGui::End();
        return;
    }

    // Busca e categoria.
    ImGui::SetNextItemWidth(320);
    if (ImGui::InputText("Buscar", search_, sizeof search_)) results_dirty_ = true;
    ImGui::SameLine();
    ImGui::SetNextItemWidth(140);
    if (ImGui::Combo("Categoria", &category_, kCategories, IM_ARRAYSIZE(kCategories))) results_dirty_ = true;
    ImGui::SameLine();
    if (ImGui::Button("Atualizar")) results_dirty_ = true;
    ImGui::SameLine();
    if (ImGui::Button("Adicionar arquivo...")) {
        if (const auto chosen = msxdisk::gui::ShowOpenFileDialog(window, "Adicionar ROM ao banco", "ROMs e discos MSX",
                                                                  "*.rom;*.bin;*.mx1;*.mx2;*.dat;*.dsk;*.sha")) {
            romdb::RomRecord rec;
            std::string error;
            if (!db_.ScanFile(*chosen, paths_.root, "manual", rec, error)) {
                db_message_ = error;
            } else {
                db_message_.clear();
                selected_id_ = rec.id;
                edit_loaded_ = false;
            }
            results_dirty_ = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Identificar (Vampier)")) {
        StartTask("Identificar", [](const romdb::RomPaths &, romdb::RomDb &d, const romdb::Log &l, std::string &e) {
            const int64_t named = d.IdentifyWithVampier(e);
            if (!e.empty()) return false;
            l(std::to_string(named) + " ROM(s) receberam nome.");
            return true;
        });
    }
    ImGui::SameLine();
    ImGui::TextDisabled("%lld ROM(s) no banco", static_cast<long long>(db_.Count()));

    if (results_dirty_) Refresh();

    // Lista (esquerda) e edicao (direita).
    ImGui::BeginChild("##lista", ImVec2(560, 0), true);
    if (ImGui::BeginTable("##roms", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                                          ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Nome");
        ImGui::TableSetupColumn("Categoria", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Hardware", ImGuiTableColumnFlags_WidthFixed, 130.0f);
        ImGui::TableSetupColumn("Arquivo");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        size_t shown = 0;
        for (const romdb::RomRecord &r : results_) {
            if (++shown > 1500) break;  // a lista so' mostra 1500 linhas; refine a busca
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(static_cast<int>(r.id));
            const bool selected = r.id == selected_id_;
            if (ImGui::Selectable(Shown(r).c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
                selected_id_ = r.id;
                edit_loaded_ = false;
            }
            ImGui::PopID();
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(r.category.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(r.hardware.c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(r.path.c_str());
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("##edicao", ImVec2(0, 0), true);
    if (selected_id_ > 0) {
        if (!edit_loaded_) {
            if (db_.Get(selected_id_, edit_)) {
                edit_loaded_ = true;
            } else {
                selected_id_ = 0;
            }
        }
    }
    if (selected_id_ > 0 && edit_loaded_) {
        ImGui::Text("id %lld  -  sha1 %s", static_cast<long long>(edit_.id), edit_.sha1.c_str());
        ImGui::TextDisabled("crc32 %s  -  %lld bytes  -  origem %s", edit_.crc32.c_str(),
                            static_cast<long long>(edit_.size), edit_.source.c_str());
        ImGui::TextDisabled("%s", edit_.path.c_str());
        ImGui::Separator();
        char name[256], hw[256], notes[512];
        CopyText(name, sizeof name, edit_.name);
        CopyText(hw, sizeof hw, edit_.hardware);
        CopyText(notes, sizeof notes, edit_.notes);
        if (ImGui::InputText("Nome", name, sizeof name)) edit_.name = name;
        int cat = 0;
        for (int i = 1; i < IM_ARRAYSIZE(kCategories); ++i)
            if (edit_.category == kCategories[i]) cat = i - 1;
        // A combo de categoria usa as categorias sem "todas".
        int cat_index = cat;
        if (ImGui::Combo("Categoria##edit", &cat_index, kCategories + 1, IM_ARRAYSIZE(kCategories) - 1)) {
            edit_.category = kCategories[cat_index + 1];
        }
        if (ImGui::InputText("Hardware", hw, sizeof hw)) edit_.hardware = hw;
        ImGui::InputInt("Mapper (-1 = nenhum)", &edit_.mapper);
        if (ImGui::InputTextMultiline("Notas", notes, sizeof notes, ImVec2(0, 80))) edit_.notes = notes;
        std::string error;
        if (ImGui::Button("Salvar")) {
            db_message_ = db_.Update(edit_, error) ? "Salvo." : error;
            results_dirty_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Excluir do banco")) {
            if (db_.Delete(edit_.id, error)) {
                selected_id_ = 0;
                edit_loaded_ = false;
                db_message_ = "Removido do banco.";
                results_dirty_ = true;
            } else {
                db_message_ = error;
            }
        }
        if (!db_message_.empty()) ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "%s", db_message_.c_str());
        ImGui::TextDisabled("Excluir remove so' o cadastro; o arquivo no disco nao e' apagado.");
    } else {
        ImGui::TextDisabled("Escolha uma ROM na lista para ver e editar.");
    }

    // Banco do Vampier: busca no banco de referencia.
    ImGui::Separator();
    ImGui::Text("Banco do Vampier");
    ImGui::SetNextItemWidth(260);
    ImGui::InputText("buscar jogo ou SHA-1", vsearch_, sizeof vsearch_);
    ImGui::SameLine();
    if (ImGui::Button("Buscar no Vampier")) vhits_ = db_.VampierSearch(vsearch_);
    if (!db_.HasVampier()) {
        ImGui::TextDisabled("Nao importado: use o menu ROMs > Baixar banco do Vampier.");
    } else if (ImGui::BeginTable("##vam", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Jogo");
        ImGui::TableSetupColumn("Ano", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Empresa", ImGuiTableColumnFlags_WidthFixed, 130.0f);
        ImGui::TableSetupColumn("ROM");
        ImGui::TableHeadersRow();
        size_t shown = 0;
        for (const romdb::VampierHit &h : vhits_) {
            if (++shown > 300) break;
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(h.game.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(h.year.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(h.company.c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s %s", h.rom_type.c_str(), h.dump.c_str());
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::End();
}

void RomManager::DrawBrowser() {
    if (!show_browser_) return;
    ImGui::SetNextWindowSize(ImVec2(760, 500), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Navegar file-hunter", &show_browser_)) {
        ImGui::End();
        return;
    }
    if (!browser_loaded_ && !running_) {
        // Primeira abertura (ou subpasta escolhida): le a pasta em segundo plano.
        const std::string url = browser_url_;
        browser_loaded_ = true;
        StartTask("Lendo " + url, [this, url](const romdb::RomPaths &, romdb::RomDb &, const romdb::Log &, std::string &e) {
            std::vector<romdb::ListingEntry> entries;
            if (!romdb::ListFileHunter(url, entries, e)) return false;
            std::lock_guard<std::mutex> lock(mutex_);
            listing_ = entries;
            listing_ready_ = true;
            return true;
        });
    }
    if (listing_ready_) {
        std::lock_guard<std::mutex> lock(mutex_);
        browser_entries_ = listing_;
        listing_ready_ = false;
    }

    ImGui::TextWrapped("%s", browser_url_.c_str());
    const bool at_root = browser_url_ == romdb::kFileHunterSystemRoms;
    if (ImGui::Button("Subir") && !at_root && !running_) {
        std::string up = browser_url_;
        if (!up.empty() && up.back() == '/') up.pop_back();
        up = up.substr(0, up.find_last_of('/') + 1);
        browser_url_ = up;
        browser_loaded_ = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Raiz")) {
        browser_url_ = romdb::kFileHunterSystemRoms;
        browser_loaded_ = false;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Arquivos vao para %s/filehunter/", paths_.root.c_str());
    ImGui::Separator();

    if (ImGui::BeginTable("##fh", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Nome");
        ImGui::TableSetupColumn("Tamanho", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Tipo", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Acao", ImGuiTableColumnFlags_WidthFixed, 110.0f);
        ImGui::TableHeadersRow();
        for (const romdb::ListingEntry &e : browser_entries_) {
            ImGui::TableNextRow();
            ImGui::PushID(e.url.c_str());
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(e.is_dir ? (e.name + "/").c_str() : e.name.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(e.size.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(e.type.c_str());
            ImGui::TableSetColumnIndex(3);
            if (e.is_dir) {
                if (ImGui::SmallButton("Abrir") && !running_) {
                    browser_url_ = e.url;
                    browser_loaded_ = false;
                }
            } else if (ImGui::SmallButton("Baixar") && !running_) {
                const std::string url = e.url;
                StartTask("Baixar " + e.name, [url](const romdb::RomPaths &p, romdb::RomDb &d, const romdb::Log &l, std::string &err) {
                    int64_t added = 0;
                    return romdb::DownloadFileHunterEntry(p, d, url, l, added, err);
                });
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void RomManager::DrawWindows(GLFWwindow *window) {
    DrawStatus();
    DrawDatabase(window);
    DrawBrowser();
    if (!running_ && worker_.joinable()) FinishTask();
}

} // namespace machine::gui
