// fwMSX -- editor: o menu Project (o .msxproject aberto no editor). Ver doc/project-spec.md e doc/editor-spec.md.
//
// O editor continua trabalhando nos arquivos da PASTA do projeto (o modelo, o Build e o Dignified nao mudam); o projeto acompanha: todo arquivo salvo
// dentro da pasta vai tambem para o banco, e "Sync..." compara os dois lados. A copia mais nova e' a sugestao da lista; um conflito pergunta.
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <set>

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
    case prj::State::ProjectRemoved: return "removed from project (asks)   " + i.path;
    case prj::State::RemovedChanged: return "removed from project, folder copy CHANGED (asks)   " + i.path;
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
    ProjectClose(false);
    if (!project_.Create(file, std::string(), err)) return false;
    last_project_ = file;
    project_root_ = project_.DefaultRoot();
    for (Editor &w : wins_) w.SetProjectSeen(w.SaveCount()); // o que ja' foi salvo antes nao e' do projeto: entra pelo Sync
    ProjectRemember();
    return true;
}

bool EditorApp::ProjectOpen(const std::string &file, std::string &err) {
    ProjectClose(false);
    if (!project_.Open(file, err)) return false;
    last_project_ = file;
    project_root_ = project_.DefaultRoot();
    for (Editor &w : wins_) w.SetProjectSeen(w.SaveCount());
    ProjectRemember();
    return true;
}

void EditorApp::ProjectClose(bool forget) {
    project_.Close();
    project_root_.clear();
    sync_queue_.clear();
    sync_pos_ = 0;
    if (forget) { // quem fechou de proposito nao quer o projeto de volta na proxima sessao (o Open project... ainda o propoe nesta)
        std::error_code ec;
        if (!ProjectMemoPath().empty()) fs::remove(ProjectMemoPath(), ec);
    }
}

// O ultimo projeto fica num arquivinho proprio (e nao no fwedit.cfg): a configuracao so' e' gravada quando o usuario pede, e abrir um projeto e'
// um fato que deve valer na proxima sessao sem precisar de "Save configuration".
std::string EditorApp::ProjectMemoPath() const {
    return cfg_main_.empty() ? std::string() : fs::path(cfg_main_).replace_filename("fwedit.project").string();
}

void EditorApp::ProjectRemember() const {
    const std::string p = ProjectMemoPath();
    if (p.empty()) return;
    std::error_code ec;
    std::ofstream f(p, std::ios::trunc);
    if (f) f << fs::absolute(last_project_, ec).string() << "\n";
}

bool EditorApp::ProjectRestore() {
    const std::string p = ProjectMemoPath();
    std::ifstream f(p);
    std::string line;
    if (p.empty() || !f || !std::getline(f, line)) return false;
    while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
    std::error_code ec;
    if (line.empty() || !fs::exists(line, ec)) return false; // sumiu (outro PC, pasta apagada): fica sem projeto, sem reclamar
    std::string err;
    if (!ProjectOpen(line, err)) {
        Msg("Project: " + err);
        return false;
    }
    Msg("Project " + project_.Name() + " reopened (Project > Sync... to compare)");
    return true;
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
    sync_queue_.insert(sync_queue_.end(), s.removed.begin(), s.removed.end());
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
    } else if (it.state == prj::State::ProjectRemoved || it.state == prj::State::RemovedChanged) { // tirado do projeto (talvez em outro PC)
        const bool changed = it.state == prj::State::RemovedChanged;
        BuildConfirm({it.path + "  " + pos, changed ? "Removed from the project, but the folder copy changed since." : "Removed from the project (on another PC?).",
                      "Project = delete the folder copy (it stays in the history).", "Folder = keep it and add it back to the project."},
                     {kBtnProject, kBtnFolder, kBtnSkip, kBtnCancel}, kBtnFolder, Dialog::ProjectConflict); // o padrao nunca apaga
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
    if (it.state == prj::State::ProjectRemoved || it.state == prj::State::RemovedChanged) {
        if (button == kBtnProject) ok = project_.DeleteLoose(it.path, project_root_, err);
        else if (button == kBtnFolder) ok = project_.Push(it.path, project_root_, err);
    } else if (button == kBtnProject) ok = project_.Pull(it.path, project_root_, err);
    else if (button == kBtnFolder) ok = disk_exists ? project_.Push(it.path, project_root_, err) : project_.Drop(it.path, project_root_, err);
    if (!ok) Msg("Project: " + err);
    ProjectReloadBuffers(before);
    ProjectAskNext();
}

// ---------------------------------------------------------------------------------------------------------------
// arquivos do projeto
// ---------------------------------------------------------------------------------------------------------------

// A lista de arquivos como ARVORE: cada pasta e' uma linha "[-] pasta/" (Enter recolhe, "[+]" expande) e cada arquivo, recuado sob a sua pasta,
// abre com Enter. Como o dialogo fecha no Enter, recolher/expandir reconstroi a lista no mesmo item (ver DialogDone).
void EditorApp::ProjectFilesDialog(int sel) {
    if (!project_.IsOpen()) {
        Msg("No project open (Project > Open project)");
        return;
    }
    project_list_ = project_.List();
    if (project_list_.empty()) {
        Msg("The project has no files yet (Project > Sync... brings the files of the folder)");
        return;
    }
    // '/' antes de qualquer outro caractere: os arquivos de uma pasta ficam juntos ("a/x", "a.b", "a/y" abririam a pasta "a" duas vezes)
    auto key = [](std::string s) {
        std::replace(s.begin(), s.end(), '/', '\x01');
        return s;
    };
    std::vector<const prj::FileInfo *> sorted;
    for (const prj::FileInfo &f : project_list_) sorted.push_back(&f);
    std::sort(sorted.begin(), sorted.end(), [&](const prj::FileInfo *a, const prj::FileInfo *b) { return key(a->path) < key(b->path); });

    std::vector<std::string> items;
    tree_rows_.clear();
    std::set<std::string> seen;
    for (const prj::FileInfo *f : sorted) {
        std::vector<std::string> parts;
        for (size_t p = 0, q; p <= f->path.size(); p = q + 1) {
            q = f->path.find('/', p);
            if (q == std::string::npos) q = f->path.size();
            parts.push_back(f->path.substr(p, q - p));
        }
        const size_t ndirs = parts.size() - 1;
        std::string prefix;
        bool hidden = false; // algum ancestral recolhido: nada abaixo dele aparece
        for (size_t d = 0; d < ndirs; ++d) {
            prefix += parts[d] + "/";
            const bool folded = tree_folded_.count(prefix) != 0;
            if (seen.insert(prefix).second && !hidden) {
                tree_rows_.emplace_back(prefix, true);
                items.push_back(std::string(d * 2, ' ') + (folded ? "[+] " : "[-] ") + parts[d] + "/");
            }
            hidden = hidden || folded;
        }
        if (hidden) continue;
        tree_rows_.emplace_back(f->path, false);
        items.push_back(std::string(ndirs * 2, ' ') + "    " + parts.back() + "   (" + f->kind + ")");
    }
    BuildProjectList(Dialog::ProjectFiles, "Project files: " + project_.Name() + "  (Enter: open / fold)", items, sel);
}

// Enter numa linha da arvore: pasta recolhe/expande e a lista volta no mesmo item; arquivo abre.
void EditorApp::ProjectTreeEnter(int row) {
    if (row < 0 || row >= static_cast<int>(tree_rows_.size())) return;
    const auto [path, is_dir] = tree_rows_[static_cast<size_t>(row)];
    if (!is_dir) {
        ProjectOpenFile(path);
        return;
    }
    if (!tree_folded_.erase(path)) tree_folded_.insert(path);
    ProjectFilesDialog(row);
}

// ---------------------------------------------------------------------------------------------------------------
// menu de contexto da arvore
// ---------------------------------------------------------------------------------------------------------------

// No TUI o "clique direito" e' o botao Actions... do Files...: uma lista com o que se pode fazer com a linha marcada.
void EditorApp::ProjectActionsDialog(int row) {
    if (row < 0 || row >= static_cast<int>(tree_rows_.size())) return;
    act_row_ = row;
    act_path_ = tree_rows_[static_cast<size_t>(row)].first;
    act_from_files_ = true;
    const bool dir = tree_rows_[static_cast<size_t>(row)].second;
    act_list_.clear();
    std::vector<std::string> items;
    auto add = [&](ProjAct a, const char *text) {
        act_list_.push_back(a);
        items.push_back(text);
    };
    if (!dir) {
        add(ProjAct::Open, "Open");
        add(ProjAct::Push, "Update the project from the folder copy");
        add(ProjAct::Pull, "Restore the folder copy from the project");
        add(ProjAct::Rename, "Rename...");
        add(ProjAct::History, "History (previous versions)...");
    }
    add(ProjAct::Add, "Add a file from the folder...");
    add(ProjAct::Removed, "Removed files (bring one back)...");
    add(ProjAct::Remove, dir ? "Remove the folder from the project (the folder copies stay)" : "Remove from the project (the folder copy stays)");
    BuildProjectList(Dialog::ProjectActions, "Actions: " + act_path_, items, 0);
}

void EditorApp::ProjectDo(ProjAct a, const std::string &path, bool from_files) {
    if (!project_.IsOpen()) return;
    act_path_ = path;
    act_from_files_ = from_files;
    const bool dir = !path.empty() && path.back() == '/';
    std::string err;
    switch (a) {
    case ProjAct::Open:
        if (!dir) ProjectOpenFile(path);
        return; // abrir sai da arvore
    case ProjAct::Push: {
        if (dir) break;
        if (!project_.Push(path, project_root_, err)) Msg("Project: " + err);
        else Msg("Project updated from the folder: " + path);
        break;
    }
    case ProjAct::Pull: {
        if (dir) break;
        const auto before = ProjectStamps();
        if (!project_.Pull(path, project_root_, err)) Msg("Project: " + err);
        else Msg("Folder copy restored from the project: " + path);
        ProjectReloadBuffers(before);
        break;
    }
    case ProjAct::Rename:
        if (!dir) {
            BuildSimpleInput(Dialog::ProjectRename, "Rename", "New path:", path, 44);
            return;
        }
        break;
    case ProjAct::Add: {
        const size_t slash = path.rfind('/'); // a pasta do alvo: o arquivo novo costuma morar ao lado
        BuildSimpleInput(Dialog::ProjectAddFile, "Add file", "File (in the project folder):", slash == std::string::npos ? std::string() : path.substr(0, slash + 1), 44);
        return;
    }
    case ProjAct::History: {
        if (dir) break;
        hist_path_ = path;
        hist_revs_ = project_.Revisions(path);
        if (hist_revs_.empty()) {
            Msg("No history for " + path + " yet (it keeps the last " + project_.GetConfig("revisions.keep", "10") + " versions)");
            break;
        }
        std::vector<std::string> items;
        for (const prj::Revision &r : hist_revs_) {
            char when[32] = "?";
            const std::time_t t = static_cast<std::time_t>(r.saved);
            if (const std::tm *tm = std::localtime(&t)) std::strftime(when, sizeof(when), "%Y-%m-%d %H:%M", tm);
            items.push_back(std::string(when) + "   " + std::to_string(r.size) + " bytes   #" + std::to_string(r.id));
        }
        BuildProjectList(Dialog::ProjectHistory, "History of " + path + "  (Enter: go back to it)", items, 0);
        return;
    }
    case ProjAct::Removed: {
        removed_list_ = project_.Removed();
        if (removed_list_.empty()) {
            Msg("No removed files");
            break;
        }
        BuildProjectList(Dialog::ProjectRemoved, "Removed files  (Enter: see their history)", removed_list_, 0);
        return;
    }
    case ProjAct::Remove: {
        int n = 0;
        for (const prj::FileInfo &f : project_.List()) {
            if (f.path == path || (dir && f.path.compare(0, path.size(), path) == 0)) n += project_.Drop(f.path, project_root_, err) ? 1 : 0;
        }
        Msg("Removed from the project: " + std::to_string(n) + (n == 1 ? " file" : " files"));
        break;
    }
    }
    ProjectBackToFiles();
}

// Voltar o arquivo a uma versao do historico: o projeto e a pasta; abas abertas e intactas recarregam. A versao de agora entra no historico.
void EditorApp::ProjectRestoreRevision(int64_t id) {
    std::string err;
    const auto before = ProjectStamps();
    if (!project_.Restore(hist_path_, id, project_root_, err)) Msg("Project: " + err);
    else Msg("Restored " + hist_path_ + " (the previous content is in the history)");
    ProjectReloadBuffers(before);
}

// Renomear leva a copia solta junto (se existir) e as abas abertas; o projeto tira o nome antigo e recebe o novo, ja' em sincronia.
void EditorApp::ProjectRenameDo(const std::string &from, const std::string &to) {
    std::string err;
    if (to.empty() || to == from) return;
    if (!prj::PathOk(to) || prj::IsIgnored(to)) {
        Msg("Project: invalid name " + to);
        return;
    }
    if (project_.Has(to)) {
        Msg("Project: " + to + " already exists");
        return;
    }
    std::error_code ec;
    const fs::path a = fs::path(project_root_) / from, b = fs::path(project_root_) / to;
    if (fs::exists(a, ec)) {
        fs::create_directories(b.parent_path(), ec);
        fs::rename(a, b, ec);
        if (ec) {
            Msg("Project: cannot rename the folder copy (" + ec.message() + ")");
            return;
        }
    }
    for (Editor &w : wins_) {
        if (!w.Name().empty() && ProjectRel(w.Name()) == from) w.SetName(b.string());
    }
    if (!project_.Drop(from, project_root_, err) || (fs::exists(b, ec) && !project_.Push(to, project_root_, err))) {
        Msg("Project: " + err);
        return;
    }
    Msg("Renamed: " + from + " -> " + to);
}

void EditorApp::ProjectAddDo(const std::string &rel_in) {
    std::string rel = rel_in;
    std::replace(rel.begin(), rel.end(), '\\', '/');
    if (rel.empty()) return;
    std::error_code ec;
    if (!prj::PathOk(rel) || prj::IsIgnored(rel) || !fs::is_regular_file(fs::path(project_root_) / rel, ec)) {
        Msg("Project: " + rel + " is not a file of the project folder");
        return;
    }
    std::string err;
    if (!project_.Push(rel, project_root_, err)) Msg("Project: " + err);
    else Msg("Added to the project: " + rel);
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
