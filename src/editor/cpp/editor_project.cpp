// fwMSX -- editor: o menu Project (o .msxproject aberto no editor). Ver doc/project-spec.md e doc/editor-spec.md.
//
// O editor continua trabalhando nos arquivos da PASTA do projeto (o modelo, o Build e o Dignified nao mudam); o projeto acompanha: todo arquivo salvo
// dentro da pasta vai tambem para o banco, e "Sync..." compara os dois lados. A copia mais nova e' a sugestao da lista; um conflito pergunta.
#include <algorithm>
#include <filesystem>

#include "editor_ui.h"

namespace ed {

namespace fs = std::filesystem;

namespace {

// A linha da lista de diferencas (a interface do editor e' em ingles).
std::string Describe(const prj::SyncItem &i) {
    switch (i.state) {
    case prj::State::InSync: return "same           " + i.path;
    case prj::State::DiskChanged: return "folder changed -> project   " + i.path;
    case prj::State::ProjectChanged: return "project changed -> folder   " + i.path;
    case prj::State::Conflict: return std::string("CONFLICT (newer: ") + (i.newer == prj::Action::Pull ? "project" : "folder") + ")   " + i.path;
    case prj::State::OnlyDisk: return "new in folder -> project   " + i.path;
    case prj::State::OnlyProject: return "only in project -> folder  " + i.path;
    case prj::State::DiskDeleted: return "deleted in folder (asks)   " + i.path;
    }
    return i.path;
}

long long Stamp(const std::string &name) {
    std::error_code ec;
    if (name.empty() || !fs::exists(name, ec)) return -1;
    return static_cast<long long>(fs::last_write_time(name, ec).time_since_epoch().count());
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------
// abrir / criar / fechar
// ---------------------------------------------------------------------------------------------------------------

bool EditorApp::ProjectNew(const std::string &file, std::string &err) {
    ProjectClose();
    if (!project_.Create(file, std::string(), err)) return false;
    last_project_ = file;
    project_root_ = project_.DefaultRoot();
    for (Editor &w : wins_) w.SetProjectSeen(w.SaveCount()); // o que ja' foi salvo antes nao e' do projeto: entra pelo Sync
    return true;
}

bool EditorApp::ProjectOpen(const std::string &file, std::string &err) {
    ProjectClose();
    if (!project_.Open(file, err)) return false;
    last_project_ = file;
    project_root_ = project_.DefaultRoot();
    for (Editor &w : wins_) w.SetProjectSeen(w.SaveCount());
    return true;
}

void EditorApp::ProjectClose() {
    project_.Close();
    project_root_.clear();
    sync_queue_.clear();
    sync_pos_ = 0;
}

std::string EditorApp::ProjectRel(const std::string &abs_path) const {
    if (abs_path.empty() || !project_.IsOpen()) return {};
    std::error_code ec;
    const fs::path a = fs::absolute(abs_path, ec).lexically_normal();
    const fs::path r = fs::absolute(project_root_, ec).lexically_normal();
    const fs::path rel = a.lexically_relative(r);
    if (rel.empty() || *rel.begin() == ".." || rel.is_absolute()) return {};
    return rel.generic_string();
}

// ---------------------------------------------------------------------------------------------------------------
// salvar: o arquivo da pasta vai tambem para o banco
// ---------------------------------------------------------------------------------------------------------------

void EditorApp::ProjectFlush() {
    if (!project_.IsOpen()) return;
    for (Editor &w : wins_) {
        if (w.SaveCount() == w.ProjectSeen()) continue;
        w.SetProjectSeen(w.SaveCount());
        const std::string rel = ProjectRel(w.Name());
        if (rel.empty() || !prj::PathOk(rel) || prj::IsIgnored(rel)) continue; // fora da pasta, copia de seguranca, .git...
        std::string err;
        if (!project_.Push(rel, project_root_, err)) Msg("Project: " + err);
    }
}

// ---------------------------------------------------------------------------------------------------------------
// sincronizar
// ---------------------------------------------------------------------------------------------------------------

std::vector<std::pair<std::string, long long>> EditorApp::ProjectStamps() const {
    std::vector<std::pair<std::string, long long>> v;
    for (const Editor &w : wins_) v.emplace_back(w.Name(), Stamp(w.Name()));
    return v;
}

// Depois de um Pull que mudou um arquivo aberto: sem alteracoes na tela recarrega (o cursor fica onde estava); com alteracoes avisa.
void EditorApp::ProjectReloadBuffers(const std::vector<std::pair<std::string, long long>> &before) {
    for (size_t i = 0; i < wins_.size() && i < before.size(); ++i) {
        Editor &w = wins_[i];
        if (w.Name().empty() || w.Name() != before[i].first || Stamp(w.Name()) == before[i].second) continue;
        if (w.Modified()) {
            Msg(fs::path(w.Name()).filename().string() + " changed in the folder but has unsaved edits (saving will overwrite the project)");
            continue;
        }
        const Pos old = w.Cursor();
        std::string err;
        if (w.Load(w.Name(), err)) {
            w.SetProjectSeen(w.SaveCount());
            w.SetCursor({std::min(old.line, w.LineCount() - 1), old.col});
        }
    }
}

void EditorApp::ProjectSyncStart() {
    if (!project_.IsOpen()) {
        Msg("No project open (Project > Open project)");
        return;
    }
    sync_queue_.clear();
    sync_pos_ = 0;
    std::vector<prj::SyncItem> items;
    std::string err;
    if (!project_.Status(project_root_, items, err)) {
        Msg("Project: " + err);
        return;
    }
    std::vector<std::string> lines;
    for (const prj::SyncItem &i : items) {
        if (i.state != prj::State::InSync) lines.push_back(Describe(i));
    }
    if (lines.empty()) {
        Msg("Project " + project_.Name() + " is in sync (" + std::to_string(items.size()) + " files)");
        return;
    }
    BuildProjectList(Dialog::ProjectSync, "Project sync: " + std::to_string(lines.size()) + " of " + std::to_string(items.size()) + " differ (OK applies the safe ones)", lines, 0);
}

void EditorApp::ProjectSyncApply() {
    sync_before_ = ProjectStamps();
    prj::SyncSummary s;
    std::string err;
    if (!project_.Sync(project_root_, false, s, err)) {
        Msg("Project: " + err);
        return;
    }
    sync_queue_ = s.conflicts;
    sync_queue_.insert(sync_queue_.end(), s.deleted.begin(), s.deleted.end());
    sync_pos_ = 0;
    ProjectReloadBuffers(sync_before_);
    std::string m = "Sync: " + std::to_string(s.pulled) + " to folder, " + std::to_string(s.pushed) + " to project, " + std::to_string(s.same) + " equal";
    if (!s.errors.empty()) m += "; ERROR " + s.errors.front();
    Msg(m);
    if (!sync_queue_.empty()) ProjectAskNext();
}

void EditorApp::ProjectAskNext() {
    if (sync_pos_ >= sync_queue_.size()) {
        sync_queue_.clear();
        sync_pos_ = 0;
        Msg("Sync done");
        return;
    }
    const prj::SyncItem &it = sync_queue_[sync_pos_];
    const std::string pos = "(" + std::to_string(sync_pos_ + 1) + "/" + std::to_string(sync_queue_.size()) + ")";
    if (it.state == prj::State::Conflict) {
        const bool proj_newer = it.newer == prj::Action::Pull;
        BuildConfirm({it.path + "  " + pos, "Changed in the project AND in the folder.", std::string("The newer copy is the ") + (proj_newer ? "project's" : "folder's") + ".",
                      "Keep which one?"},
                     {kBtnProject, kBtnFolder, kBtnSkip, kBtnCancel}, proj_newer ? kBtnProject : kBtnFolder, Dialog::ProjectConflict);
    } else { // sumiu da pasta
        BuildConfirm({it.path + "  " + pos, "Deleted from the folder.", "Project = restore the file; Folder = remove it from the project."},
                     {kBtnProject, kBtnFolder, kBtnSkip, kBtnCancel}, kBtnProject, Dialog::ProjectConflict);
    }
}

void EditorApp::ProjectAnswer(int button) {
    if (sync_pos_ >= sync_queue_.size()) return;
    const prj::SyncItem it = sync_queue_[sync_pos_++];
    std::string err;
    const auto before = ProjectStamps();
    std::error_code ec;
    const bool disk_exists = fs::exists(fs::path(project_root_) / it.path, ec);
    bool ok = true;
    if (button == kBtnProject) ok = project_.Pull(it.path, project_root_, err);
    else if (button == kBtnFolder) ok = disk_exists ? project_.Push(it.path, project_root_, err) : project_.Drop(it.path, project_root_, err);
    if (!ok) Msg("Project: " + err);
    ProjectReloadBuffers(before);
    ProjectAskNext();
}

// ---------------------------------------------------------------------------------------------------------------
// arquivos do projeto
// ---------------------------------------------------------------------------------------------------------------

void EditorApp::ProjectFilesDialog() {
    if (!project_.IsOpen()) {
        Msg("No project open (Project > Open project)");
        return;
    }
    project_list_ = project_.List();
    if (project_list_.empty()) {
        Msg("The project has no files yet (Project > Sync... brings the files of the folder)");
        return;
    }
    std::vector<std::string> items;
    for (const prj::FileInfo &f : project_list_) {
        std::string kind = f.kind;
        kind.resize(std::max<size_t>(kind.size(), 10), ' ');
        items.push_back(kind + " " + f.path);
    }
    BuildProjectList(Dialog::ProjectFiles, "Project files: " + project_.Name(), items, 0);
}

void EditorApp::ProjectOpenFile(const std::string &path) {
    bool text = false;
    const std::string kind = prj::KindOf(path, &text);
    if (!text) {
        Msg(path + " is a binary file (" + kind + "): not editable here");
        return;
    }
    const fs::path disk = fs::path(project_root_) / path;
    std::error_code ec;
    std::string err;
    if (!fs::exists(disk, ec) && !project_.Pull(path, project_root_, err)) {
        Msg("Project: " + err);
        return;
    }
    pending_path_ = disk.string();
    if (gui_mode_) {
        if (!OpenTab(pending_path_, err, false)) Msg(err);
        return;
    }
    if (Ed().Modified()) {
        confirm_purpose_ = 3;
        BuildConfirm({"This file has been changed.", "Save it before opening another?"}, {kBtnSave, kBtnDontSave, kBtnCancel}, kBtnDontSave, Dialog::ConfirmExit);
    } else if (!Open(pending_path_, err, false)) {
        Msg(err);
    }
}

void EditorApp::ProjectAddCurrent() {
    if (!project_.IsOpen()) {
        Msg("No project open (Project > Open project)");
        return;
    }
    Editor &e = Ed();
    if (e.Name().empty()) {
        Msg("Save the file first (File > Save As), inside the project folder");
        return;
    }
    const std::string rel = ProjectRel(e.Name());
    if (rel.empty() || !prj::PathOk(rel)) {
        Msg("The file is outside the project folder: " + project_root_);
        return;
    }
    if (prj::IsIgnored(rel)) {
        Msg(rel + " is not a project file (hidden folder, backup copy...)");
        return;
    }
    std::string err;
    if (e.Modified() && !e.Save(err)) {
        Msg(err);
        return;
    }
    e.SetProjectSeen(e.SaveCount());
    if (!project_.Push(rel, project_root_, err)) {
        Msg("Project: " + err);
        return;
    }
    Msg("Added to the project: " + rel);
}

} // namespace ed
