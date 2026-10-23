// fwMSX -- janela "Downloads" (ImGui). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver download_window.h e doc/download-spec.md.
#include "download_window.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>

#include <imgui.h>

#include "../../romdb/net/web.h"

namespace machine::gui {

namespace fs = std::filesystem;

void DrawDownloadProgress() {
    const romdb::DownloadProgress p = romdb::CurrentProgress();
    if (!p.active) return;
    char text[96];
    const double mb = 1048576.0;
    if (p.items) std::snprintf(text, sizeof(text), "%d / %d arquivos", static_cast<int>(p.done), static_cast<int>(p.total));
    else if (p.total > 0) std::snprintf(text, sizeof(text), "%.1f / %.1f MB (%d%%)", static_cast<double>(p.done) / mb, static_cast<double>(p.total) / mb, static_cast<int>(p.done * 100 / p.total));
    else std::snprintf(text, sizeof(text), "%.1f MB", static_cast<double>(p.done) / mb);
    ImGui::TextUnformatted(p.name.c_str());
    // sem tamanho anunciado a barra anda de um lado a outro (fracao negativa = indeterminada no ImGui)
    ImGui::ProgressBar(p.total > 0 ? static_cast<float>(p.done) / static_cast<float>(p.total) : -1.0f * static_cast<float>(ImGui::GetTime()), ImVec2(-1.0f, 0.0f), text);
}

DownloadWindow::DownloadWindow() { paths_.exe_dir = dl::ExeDir(); }

DownloadWindow::~DownloadWindow() { Join(); }

void DownloadWindow::Join() {
    if (worker_.joinable()) worker_.join();
}

void DownloadWindow::Log(const std::string &line) {
    std::lock_guard<std::mutex> lock(mutex_);
    log_.push_back(line);
    if (log_.size() > 40) log_.erase(log_.begin());
}

// Uma tarefa de fundo por vez: os arquivos de `jobs` saem um depois do outro (com o intervalo minimo do dl::Fetch), nunca em paralelo.
void DownloadWindow::Start(std::vector<Job> jobs, bool refresh_index) {
    if (busy_) return;
    Join();
    busy_ = true;
    worker_ = std::thread([this, jobs = std::move(jobs), refresh_index]() {
        if (refresh_index) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                status_ = "Baixando o indice do File Hunter (3,5 MB)...";
            }
            bool fresh = false;
            std::string err;
            if (dl::EnsureIndex(paths_, true, fresh, err)) {
                Log("Indice do File Hunter atualizado.");
                reload_index_ = true;
            } else {
                Log("ERRO no indice: " + err);
            }
        }
        for (const Job &j : jobs) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                status_ = "Baixando " + j.title + "...";
            }
            std::string out, err;
            bool skipped = false;
            if (dl::Fetch(paths_, j.url, j.dest, j.install, j.overwrite, out, skipped, err)) Log((skipped ? "Ja existia: " : "Baixado: ") + out);
            else Log("ERRO em " + j.title + ": " + err);
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            status_.clear();
        }
        busy_ = false;
    });
}

void DownloadWindow::LoadIndexNow() {
    need_load_ = false;
    index_ = dl::Index();
    index_loaded_ = dl::LoadIndex(paths_, index_, index_error_);
    dir_.clear();
    applied_query_ = "\x01"; // forca refazer a lista
    Rebuild();
}

void DownloadWindow::Rebuild() {
    rows_.clear();
    truncated_ = false;
    if (!index_loaded_) return;
    const std::string q = query_;
    applied_query_ = q;
    if (!q.empty()) {
        for (const size_t i : index_.Search(q, 400, &truncated_)) rows_.push_back({false, i, index_.path(i)});
        return;
    }
    std::vector<std::string> folders;
    std::vector<size_t> files;
    index_.List(dir_, folders, files);
    for (const std::string &f : folders) rows_.push_back({true, 0, f});
    for (const size_t i : files) {
        const std::string &p = index_.path(i);
        rows_.push_back({false, i, p.substr(p.find_last_of('/') == std::string::npos ? 0 : p.find_last_of('/') + 1)});
    }
}

bool DownloadWindow::Downloaded(const std::string &rel) const {
    std::error_code ec;
    return fs::is_regular_file(fs::path(paths_.root()) / fs::path(rel), ec);
}

void DownloadWindow::DrawFileHunter() {
    if (!index_loaded_) {
        ImGui::TextWrapped("O indice do File Hunter (a lista de todos os arquivos do site, ~3,5 MB) ainda nao foi baixado. Com ele voce navega pelas pastas e procura "
                           "sem fazer uma requisicao por tela: o site so' e' acessado quando voce clica em Baixar.");
        if (!index_error_.empty()) ImGui::TextDisabled("%s", index_error_.c_str());
        if (busy_) ImGui::BeginDisabled();
        if (ImGui::Button("Baixar o indice")) Start({}, true);
        if (busy_) ImGui::EndDisabled();
        return;
    }
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 260);
    ImGui::InputTextWithHint("##busca", "procurar (todas as palavras, ex.: nemesis konami)", query_, sizeof(query_));
    if (applied_query_ != query_) Rebuild();
    ImGui::SameLine();
    if (ImGui::Button("Limpar")) {
        query_[0] = '\0';
        Rebuild();
    }
    ImGui::SameLine();
    if (busy_) ImGui::BeginDisabled();
    if (ImGui::Button("Atualizar indice")) Start({}, true);
    if (busy_) ImGui::EndDisabled();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Baixa de novo o allfiles.txt (3,5 MB)");

    const bool searching = query_[0] != '\0';
    if (!searching) {
        ImGui::Text("/%s", dir_.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton("Raiz")) {
            dir_.clear();
            Rebuild();
        }
        ImGui::SameLine();
        if (dir_.empty()) ImGui::BeginDisabled();
        if (ImGui::SmallButton("Subir")) {
            const size_t s = dir_.find_last_of('/');
            dir_ = s == std::string::npos ? std::string() : dir_.substr(0, s);
            Rebuild();
        }
        if (dir_.empty()) ImGui::EndDisabled();
    } else {
        ImGui::TextDisabled("%zu resultado(s)%s", rows_.size(), truncated_ ? " (mostrando os primeiros: refine a busca)" : "");
    }

    const float footer = ImGui::GetTextLineHeightWithSpacing() * 5.5f;
    if (ImGui::BeginChild("##fhlista", ImVec2(0, -footer), ImGuiChildFlags_Borders)) {
        ImGuiListClipper clip;
        clip.Begin(static_cast<int>(rows_.size()));
        while (clip.Step()) {
            for (int r = clip.DisplayStart; r < clip.DisplayEnd; ++r) {
                const Row &row = rows_[static_cast<size_t>(r)];
                ImGui::PushID(r);
                if (row.folder) {
                    if (ImGui::Selectable(("[pasta] " + row.name).c_str(), false)) {
                        dir_ = dir_.empty() ? row.name : dir_ + "/" + row.name;
                        Rebuild();
                        ImGui::PopID();
                        ImGui::EndChild();
                        return;
                    }
                } else {
                    const std::string &path = index_.path(row.index);
                    const bool have = Downloaded(dl::FileHunterRel(path));
                    if (busy_) ImGui::BeginDisabled();
                    if (ImGui::SmallButton(have ? "De novo" : "Baixar")) {
                        Job j;
                        j.title = row.name;
                        j.url = dl::FileHunterUrl(path);
                        j.dest = dl::FileHunterRel(path);
                        j.overwrite = have;
                        Start({j});
                    }
                    if (busy_) ImGui::EndDisabled();
                    ImGui::SameLine();
                    if (have) ImGui::TextDisabled("(baixado)");
                    else ImGui::TextUnformatted("");
                    ImGui::SameLine();
                    ImGui::TextUnformatted(row.name.c_str());
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", path.c_str());
                }
                ImGui::PopID();
            }
        }
    }
    ImGui::EndChild();
}

void DownloadWindow::DrawUtilities() {
    ImGui::TextWrapped("Utilitarios de sites especificos. Cada arquivo vem do site do autor, direto para downloads/konamiman/<utilitario>/; "
                       "o fwMSX nao os distribui.");
    ImGui::Separator();
    for (const dl::Entry &e : dl::Catalog()) {
        ImGui::PushID(e.id.c_str());
        if (ImGui::CollapsingHeader(e.title.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextWrapped("%s", e.note.c_str());
            if (busy_) ImGui::BeginDisabled();
            if (ImGui::Button("Baixar todos os arquivos")) {
                std::vector<Job> jobs;
                for (const dl::Item &it : e.items) {
                    Job j;
                    j.title = it.name;
                    j.url = it.url;
                    j.dest = it.dest;
                    j.install = it.install;
                    jobs.push_back(j);
                }
                Start(std::move(jobs));
            }
            if (busy_) ImGui::EndDisabled();
            for (const dl::Item &it : e.items) {
                ImGui::PushID(it.name.c_str());
                const bool have = Downloaded(it.dest);
                if (busy_) ImGui::BeginDisabled();
                if (ImGui::SmallButton(have ? "De novo" : "Baixar")) {
                    Job j;
                    j.title = it.name;
                    j.url = it.url;
                    j.dest = it.dest;
                    j.install = it.install;
                    j.overwrite = have;
                    Start({j});
                }
                if (busy_) ImGui::EndDisabled();
                ImGui::SameLine();
                ImGui::Text("%s%s%s", it.name.c_str(), have ? "  (baixado)" : "", it.install.empty() ? "" : "  -> tambem em badig/");
                ImGui::PopID();
            }
        }
        ImGui::PopID();
    }
}

void DownloadWindow::Draw() {
    if (!busy_ && worker_.joinable()) Join();
    if (!open_) return;
    if (!busy_ && reload_index_) {
        reload_index_ = false;
        need_load_ = true;
    }
    if (need_load_ && !busy_) LoadIndexNow();
    ImGui::SetNextWindowSize(ImVec2(820, 560), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Downloads", &open_)) {
        ImGui::End();
        return;
    }
    if (ImGui::BeginTabBar("##dltabs")) {
        if (ImGui::BeginTabItem("File Hunter")) {
            DrawFileHunter();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Utilitarios")) {
            DrawUtilities();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::Separator();
    if (busy_) DrawDownloadProgress();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!status_.empty()) ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "%s", status_.c_str());
        else ImGui::TextDisabled("Pasta de downloads: %s", paths_.root().c_str());
        const size_t from = log_.size() > 3 ? log_.size() - 3 : 0;
        for (size_t i = from; i < log_.size(); ++i) ImGui::TextUnformatted(log_[i].c_str());
    }
    ImGui::TextDisabled("Um arquivo por vez, com intervalo entre eles: o File Hunter bane quem baixa em massa.");
    ImGui::End();
}

} // namespace machine::gui
