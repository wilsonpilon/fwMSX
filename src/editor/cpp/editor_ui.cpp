// fwMSX -- editor de textos: a aplicacao (menus abertos, dialogos, duas janelas, mouse, saida com confirmacao).
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver editor_ui.h e doc/editor-spec.md. Os desenhos dos dialogos e as
// montagens de cada caixa estao em editor_dialogs.cpp.
#include "editor_ui.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <unordered_map>

namespace ed {

namespace fs = std::filesystem;

namespace {

Cmd LookupCmd(const std::string &canon) {
    static const std::unordered_map<std::string, Cmd> table = [] {
        std::unordered_map<std::string, Cmd> m;
        for (const KeyBinding &b : Bindings()) m[CanonicalKey(b.key)] = b.cmd;
        return m;
    }();
    const auto it = table.find(canon);
    return it == table.end() ? Cmd::None : it->second;
}

// Comandos que a aplicacao trata antes do modelo (envolvem janelas, confirmacao ou sair).
bool IsAppCmd(Cmd c) {
    switch (c) {
    case Cmd::Window2: case Cmd::CloseWindow: case Cmd::DividerUp: case Cmd::DividerDown: case Cmd::WindowDiff:
    case Cmd::New: case Cmd::Exit: case Cmd::Outline: case Cmd::Print: case Cmd::PrintBlock: case Cmd::EjectPage:
    case Cmd::Assemble: case Cmd::RunMsx: case Cmd::NextError: case Cmd::PrevError: case Cmd::ErrorList: case Cmd::HelpWord:
        return true;
    default:
        return false;
    }
}

int MenuX(const Menu &m, int cols) {
    if (m.hot == 0) return cols - 10; // F1=Help: encostado na direita
    return m.col * cols / kDefaultCols;
}

} // namespace

EditorApp::EditorApp() {
    wins_.emplace_back();
    Layout();
}

// ---------------------------------------------------------------------------------------------------------------
// janelas e arquivo
// ---------------------------------------------------------------------------------------------------------------

void EditorApp::SetSize(int cols, int rows) {
    cols_ = std::max(40, cols);
    int r = std::max(8, rows);
    if (ui_.lines > 0) r = std::min(r, ui_.lines);
    rows_ = r;
    Layout();
}

EditorApp::WinRect EditorApp::Rect(int i) const {
    const int body = rows_ - 2;
    if (wins_.size() == 1) return {1, body};
    const int h1 = std::max(2, std::min(divider_, body - 3));
    if (i == 0) return {1, h1};
    return {2 + h1, body - h1 - 1};
}

void EditorApp::Layout() {
    for (size_t i = 0; i < wins_.size(); ++i) {
        const WinRect r = Rect(static_cast<int>(i));
        wins_[i].SetViewSize(cols_, r.h);
    }
}

void EditorApp::LoadConfig() {
    Settings s = Ed().settings();
    LoadConfigFile(cfg_main_, s, ui_);
    LoadConfigFile(cfg_dir_, s, ui_);
    for (Editor &w : wins_) w.settings() = s;
    // snippets do usuario: fwedit.snippets ao lado da configuracao (o da pasta atual, se existir, vale no lugar do outro)
    if (!cfg_main_.empty()) LoadUserSnippets(fs::path(cfg_main_).replace_filename("fwedit.snippets").string());
    if (!cfg_dir_.empty()) LoadUserSnippets(fs::path(cfg_dir_).replace_filename("fwedit.snippets").string());
    Ed().NewFile(); // aplica o modo inicial de Insert/Over
}

bool EditorApp::Open(const std::string &path, std::string &err, bool view) {
    Editor &e = Ed();
    if (!e.Load(path, err)) return false;
    e.SetReadOnly(view);
    cwd_ = fs::path(fs::absolute(path)).parent_path().string();
    Layout();
    return true;
}

bool EditorApp::Open2(const std::string &path, std::string &err, bool view) {
    if (wins_.size() < 2) {
        Editor e;
        e.settings() = Ed().settings();
        wins_.push_back(e);
        divider_ = (rows_ - 3) / 2;
    }
    active_ = 1;
    Layout();
    return Open(path, err, view);
}

// ---------------------------------------------------------------------------------------------------------------
// modo grafico: abas e divisao
// ---------------------------------------------------------------------------------------------------------------

bool EditorApp::OpenTab(const std::string &path, std::string &err, bool view) {
    std::error_code ec;
    const std::string abs = fs::absolute(path, ec).string();
    for (size_t i = 0; i < wins_.size(); ++i) {
        if (!wins_[i].Name().empty() && fs::absolute(wins_[i].Name(), ec).string() == abs) {
            SetActive(static_cast<int>(i));
            return true;
        }
    }
    // uma unica aba vazia e intacta: o arquivo entra nela
    if (wins_.size() == 1 && wins_[0].Name().empty() && !wins_[0].Modified() && wins_[0].LineCount() == 1 && wins_[0].Line(0).empty()) {
        return Open(path, err, view);
    }
    Editor e;
    e.settings() = Ed().settings();
    if (!e.Load(path, err)) return false;
    e.SetReadOnly(view);
    wins_.push_back(e);
    SetActive(static_cast<int>(wins_.size()) - 1);
    cwd_ = fs::path(abs).parent_path().string();
    EnsureUntitled();
    return true;
}

void EditorApp::NewTab() {
    Editor e;
    e.settings() = Ed().settings();
    e.NewFile();
    wins_.push_back(e);
    SetActive(static_cast<int>(wins_.size()) - 1);
    EnsureUntitled();
}

void EditorApp::EnsureUntitled() {
    std::vector<bool> used(wins_.size() + 2, false);
    for (Editor &e : wins_) {
        if (!e.Name().empty()) {
            e.SetUntitled("");
            continue;
        }
        const std::string &u = e.Untitled();
        if (u.rfind("noname", 0) == 0) {
            const size_t n = static_cast<size_t>(std::atoi(u.c_str() + 6));
            if (n >= 1 && n < used.size()) used[n] = true;
        }
    }
    for (Editor &e : wins_) {
        if (!e.Name().empty() || !e.Untitled().empty()) continue;
        size_t n = 1;
        while (n < used.size() && used[n]) ++n;
        if (n >= used.size()) used.resize(n + 1, false);
        used[n] = true;
        char buf[24];
        std::snprintf(buf, sizeof buf, "noname%02zu", n);
        e.SetUntitled(buf);
    }
}

bool EditorApp::AutosaveTick(double now) {
    const int minutes = wins_[0].settings().autosave_minutes;
    if (minutes <= 0) {
        autosave_next_ = 0;
        return false;
    }
    const double period = minutes * 60.0;
    if (autosave_next_ <= 0 || autosave_period_ != period) { // ligou agora, ou o intervalo mudou: recomeca a contar
        autosave_period_ = period;
        autosave_next_ = now + period;
        return false;
    }
    if (now < autosave_next_) return false;
    autosave_next_ = now + period;
    return AutosaveNow();
}

bool EditorApp::AutosaveNow() {
    bool any = false;
    if (wins_[0].settings().autosave_files) {
        for (Editor &e : wins_) {
            if (e.Name().empty() || !e.Modified() || e.ReadOnly()) continue;
            std::string err;
            // a copia { do arquivo ORIGINAL so' e' feita no primeiro autosave (os seguintes nao a trocam pela versao anterior)
            if (e.Save(err, !e.AutosavedOnce())) {
                e.SetAutosavedOnce(true);
                any = true;
            } else {
                Msg(err);
            }
        }
    }
    if (!session_path_.empty() && SaveSession(session_path_)) any = true;
    if (any) Msg("Auto-saved");
    return any;
}

void EditorApp::DiscardSession() {
    if (session_path_.empty()) return;
    std::error_code ec;
    fs::remove(session_path_, ec);
}

void EditorApp::SetActive(int i) {
    if (i < 0 || i >= WindowCount()) return;
    active_ = i;
    if (split_) {
        if (i == pane_[0]) ap_ = 0;
        else if (i == pane_[1]) ap_ = 1;
        else pane_[ap_] = i;
    }
}

void EditorApp::ToggleSplit() {
    if (split_) {
        split_ = false;
        return;
    }
    if (WindowCount() >= 2) {
        const int other = (active_ + 1) % WindowCount();
        pane_[0] = std::min(active_, other); // a aba mais a esquerda fica no painel da esquerda
        pane_[1] = std::max(active_, other);
        ap_ = active_ == pane_[1] ? 1 : 0;
        split_ = true;
    } else {
        ShowDialog(Dialog::Window2);
    }
}

void EditorApp::RemoveTab(int i) {
    if (i < 0 || i >= WindowCount()) return;
    wins_.erase(wins_.begin() + i);
    split_ = false;
    if (wins_.empty()) {
        Editor e;
        wins_.push_back(e);
    }
    EnsureUntitled();
    if (i < active_) --active_;
    active_ = std::max(0, std::min(active_, WindowCount() - 1));
}

void EditorApp::CloseTab(int i) {
    if (i < 0 || i >= WindowCount()) return;
    if (!wins_[static_cast<size_t>(i)].Modified()) {
        RemoveTab(i);
        return;
    }
    active_ = i;
    close_index_ = i;
    confirm_purpose_ = 4;
    BuildConfirm({"This file has been changed.", "Save it before closing?"}, {kBtnSave, kBtnDontSave, kBtnCancel}, kBtnDontSave, Dialog::ConfirmExit);
}

bool EditorApp::SaveSession(const std::string &path) const {
    std::vector<int> keep;
    for (int i = 0; i < WindowCount(); ++i) {
        const Editor &e = wins_[static_cast<size_t>(i)];
        if (!e.Name().empty() || e.Modified() || e.LineCount() > 1 || !e.Line(0).empty()) keep.push_back(i);
    }
    std::error_code ec;
    if (keep.empty()) {
        fs::remove(path, ec);
        return true;
    }
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f) return false;
    f << "FWEDIT-SESSION 1\n";
    int active_pos = 0;
    for (size_t k = 0; k < keep.size(); ++k) {
        if (keep[k] == active_) active_pos = static_cast<int>(k);
    }
    f << "ACTIVE " << active_pos << " " << keep.size() << "\n";
    for (const int i : keep) {
        const Editor &e = wins_[static_cast<size_t>(i)];
        const std::string text = e.Text();
        f << "DOC\n" << "NAME " << e.Name().size() << "\n" << e.Name() << "\n";
        f << "UNT " << e.Untitled() << "\n";
        if (e.LanguageManual()) f << "LANG " << e.Language()->Id() << "\n"; // a escolha manual da linguagem
        f << "MOD " << (e.Modified() ? 1 : 0) << " " << (e.Crlf() ? 1 : 0) << " " << e.Cursor().line << " " << e.Cursor().col << "\n";
        f << "TEXT " << text.size() << "\n";
        f.write(text.data(), static_cast<std::streamsize>(text.size()));
        f << "\n";
    }
    return static_cast<bool>(f);
}

bool EditorApp::LoadSession(const std::string &path) {
    if (!(wins_.size() == 1 && wins_[0].Name().empty() && !wins_[0].Modified() && wins_[0].LineCount() == 1 && wins_[0].Line(0).empty())) return false;
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::string line;
    if (!std::getline(f, line) || line != "FWEDIT-SESSION 1") return false;
    int active_pos = 0;
    size_t count = 0;
    if (!std::getline(f, line) || std::sscanf(line.c_str(), "ACTIVE %d %zu", &active_pos, &count) != 2 || count == 0 || count > 64) return false;
    std::vector<Editor> docs;
    for (size_t k = 0; k < count; ++k) {
        size_t n = 0;
        if (!std::getline(f, line) || line != "DOC") return false;
        if (!std::getline(f, line) || std::sscanf(line.c_str(), "NAME %zu", &n) != 1) return false;
        std::string name(n, '\0');
        if (n) f.read(&name[0], static_cast<std::streamsize>(n));
        std::getline(f, line); // fim da linha do nome
        std::string untitled;
        std::string lang_id;
        if (!std::getline(f, line)) return false;
        while (line.rfind("UNT ", 0) == 0 || line.rfind("LANG ", 0) == 0) { // linhas opcionais (as sessoes da 1.41.4 nao as tem)
            if (line[0] == 'U') untitled = line.substr(4);
            else lang_id = line.substr(5);
            if (!std::getline(f, line)) return false;
        }
        int mod = 0, crlf = 1, cl = 0, cc = 0;
        if (std::sscanf(line.c_str(), "MOD %d %d %d %d", &mod, &crlf, &cl, &cc) != 4) return false;
        size_t tn = 0;
        if (!std::getline(f, line) || std::sscanf(line.c_str(), "TEXT %zu", &tn) != 1) return false;
        std::string text(tn, '\0');
        if (tn) f.read(&text[0], static_cast<std::streamsize>(tn));
        std::getline(f, line);
        if (!f && !f.eof()) return false;
        Editor e;
        e.settings() = wins_[0].settings();
        std::string err;
        if (!mod && !name.empty() && e.Load(name, err)) {
            // arquivo sem alteracoes: vale o que esta' no disco agora
        } else {
            e.SetText(text);
            e.SetName(name);
            e.SetCrlf(crlf != 0);
            if (mod || name.empty()) e.MarkModified();
        }
        if (name.empty()) e.SetUntitled(untitled);
        if (!lang_id.empty() && LangById(lang_id)) e.SetLanguage(LangById(lang_id), true);
        else e.DetectLanguage();
        e.SetCursor({cl, cc});
        docs.push_back(e);
    }
    wins_ = docs;
    split_ = false;
    active_ = std::max(0, std::min(active_pos, WindowCount() - 1));
    EnsureUntitled();
    return true;
}

void EditorApp::RunMenu(int m, int i) {
    const std::vector<Menu> &ms = Menus();
    if (m < 0 || m >= static_cast<int>(ms.size())) return;
    const Menu &menu = ms[static_cast<size_t>(m)];
    if (i < 0 || i >= static_cast<int>(menu.items.size())) return;
    RunMenuItem(menu.items[static_cast<size_t>(i)]);
}

void EditorApp::DialogEnter() {
    if (!HasDialog()) return;
    if (dlg_.kind == Dialog::Open || dlg_.kind == Dialog::SaveAs || dlg_.kind == Dialog::Merge || dlg_.kind == Dialog::Window2) {
        FileDialogEnter();
        return;
    }
    for (const Widget &w : dlg_.ws) {
        if (w.type == Widget::Button && w.is_default) {
            DialogDone(w.id);
            return;
        }
    }
}

bool EditorApp::DoCloseWindow() {
    if (gui_mode_) {
        if (split_) {
            split_ = false;
            return true;
        }
        if (wins_.size() > 1) {
            CloseTab(active_);
            return true;
        }
        Msg("There is only one file");
        return false;
    }
    if (wins_.size() < 2) {
        Msg("There is no second window");
        return false;
    }
    wins_.pop_back();
    active_ = 0;
    Layout();
    return true;
}

bool EditorApp::WindowDifferences() {
    if (wins_.size() < 2 || (gui_mode_ && !split_)) {
        Msg("Open a second window first (F2)");
        return false;
    }
    Editor &a = wins_[static_cast<size_t>(gui_mode_ ? pane_[0] : 0)];
    Editor &b = wins_[static_cast<size_t>(gui_mode_ ? pane_[1] : 1)];
    Pos pa = a.Cursor();
    Pos pb = b.Cursor();
    while (pa.line < a.LineCount() && pb.line < b.LineCount()) {
        const std::string &la = a.Line(pa.line);
        const std::string &lb = b.Line(pb.line);
        const size_t n = std::min(la.size(), lb.size());
        size_t i = static_cast<size_t>(std::min(pa.col, pb.col));
        // mesma coluna nas duas na primeira linha; depois a partir do inicio
        while (i < n && la[i] == lb[i]) ++i;
        if (i < n || la.size() != lb.size()) {
            a.SetCursor({pa.line, static_cast<int>(i)});
            b.SetCursor({pb.line, static_cast<int>(i)});
            Msg("Difference found");
            return true;
        }
        ++pa.line;
        ++pb.line;
        pa.col = pb.col = 0;
    }
    if (pa.line < a.LineCount() || pb.line < b.LineCount()) {
        if (pa.line < a.LineCount()) a.SetCursor({pa.line, 0});
        if (pb.line < b.LineCount()) b.SetCursor({pb.line, 0});
        Msg("One file ends first");
        return true;
    }
    Msg("No differences");
    return false;
}

// ---------------------------------------------------------------------------------------------------------------
// comandos
// ---------------------------------------------------------------------------------------------------------------

void EditorApp::AfterEditorCommand() {
    const Dialog d = Ed().TakeDialog();
    if (Ed().TakeMenuRequest()) OpenMenu(0);
    if (Ed().ExitRequested()) quit_ = true;
    if (d != Dialog::None) ShowDialog(d, Ed().HelpTopic());
}

bool EditorApp::ExecuteCmd(Cmd cmd, int arg) {
    Editor &e = Ed();
    switch (cmd) {
    case Cmd::Window2:
        if (gui_mode_) {
            if (split_) {
                ap_ = 1 - ap_; // F2 com a divisao aberta: passa para o outro painel
                active_ = pane_[ap_];
            } else {
                ToggleSplit();
            }
            return true;
        }
        if (wins_.size() == 2) {
            active_ = 1 - active_;
        } else {
            ShowDialog(Dialog::Window2);
        }
        return true;
    case Cmd::CloseWindow:
        return DoCloseWindow();
    case Cmd::DividerUp:
    case Cmd::DividerDown:
        if (wins_.size() == 2) {
            divider_ += cmd == Cmd::DividerUp ? -1 : 1;
            divider_ = std::max(2, std::min(divider_, rows_ - 5));
            Layout();
        }
        return true;
    case Cmd::WindowDiff:
        return WindowDifferences();
    case Cmd::New:
        if (gui_mode_) {
            NewTab(); // abas: o arquivo novo abre ao lado, nada e' perdido
            return true;
        }
        if (e.ReadOnly()) return true;
        if (e.Modified()) {
            confirm_purpose_ = 2;
            BuildConfirm({"This file has been changed.", "Save it before starting a new file?"}, {kBtnSave, kBtnDontSave, kBtnCancel}, kBtnDontSave, Dialog::ConfirmExit);
        } else {
            e.NewFile();
        }
        return true;
    case Cmd::Exit:
        if (embedded_) {
            hide_request_ = true; // so' esconde: o texto fica para a proxima vez
            return true;
        }
        exit_index_ = 0;
        confirm_purpose_ = 1;
        NextExitStep();
        return true;
    case Cmd::Assemble:
        BuildAssemble();
        return true;
    case Cmd::RunMsx:
        BuildRun();
        return true;
    case Cmd::NextError:
        BuildStep(+1);
        return true;
    case Cmd::PrevError:
        BuildStep(-1);
        return true;
    case Cmd::ErrorList:
        BuildErrors();
        return true;
    case Cmd::HelpWord:
        HelpOnWord();
        return true;
    case Cmd::Outline:
    case Cmd::Print:
    case Cmd::PrintBlock:
    case Cmd::EjectPage:
        Msg(std::string(Info(cmd).name) + ": not available yet");
        return false;
    default: {
        const bool r = e.Execute(cmd, arg);
        AfterEditorCommand();
        return r;
    }
    }
}

bool EditorApp::Enabled(const MenuItem &it) const {
    if (it.cmd == Cmd::CloseWindow) return gui_mode_ ? (split_ || wins_.size() > 1) : wins_.size() > 1;
    if (it.cmd == Cmd::WindowDiff) return gui_mode_ ? split_ : wins_.size() > 1;
    if (it.cmd == Cmd::Window2) return true;
    if (it.cmd == Cmd::RunMsx) return static_cast<bool>(run_hook_);
    if (it.cmd == Cmd::HelpWord) return static_cast<bool>(help_hook_);
    if (it.cmd == Cmd::NextError || it.cmd == Cmd::PrevError) return !build_msgs_.empty();
    if (it.cmd == Cmd::New || it.cmd == Cmd::Exit) return !(Ed().ReadOnly() && it.cmd == Cmd::New);
    if (it.cmd == Cmd::Outline || it.cmd == Cmd::Print || it.cmd == Cmd::PrintBlock || it.cmd == Cmd::EjectPage) return false;
    return Ed().ItemEnabled(it);
}

bool EditorApp::Key(const std::string &canon) {
    if (canon.empty()) return false;
    Ed().ClearMessage();
    for (Editor &w : wins_) { // a lista de completar e' do editor que esta' recebendo as teclas
        if (&w != &Ed()) w.CompletionClose();
    }
    if (HasDialog()) {
        if (dlg_.kind == Dialog::Help && canon == "F1") return true;
        int b = 0;
        if ((dlg_.kind == Dialog::Open || dlg_.kind == Dialog::SaveAs || dlg_.kind == Dialog::Merge || dlg_.kind == Dialog::Window2) && canon == "Enter") {
            const Widget &fw = dlg_.ws[static_cast<size_t>(dlg_.fields[static_cast<size_t>(dlg_.focus)])];
            if (fw.type == Widget::Button && fw.id == kBtnCancel) {
                DialogDone(kBtnCancel);
            } else {
                FileDialogEnter();
            }
            return true;
        }
        if ((dlg_.kind == Dialog::Find || dlg_.kind == Dialog::Replace) && canon == "F1") {
            b = 0;
            return true;
        }
        b = dlg_.Key(canon);
        if (b) DialogDone(b);
        return true;
    }
    if (menu_ >= 0) return MenuKey(canon);

    if (canon == "F10") {
        OpenMenu(0);
        return true;
    }
    Cmd cmd = LookupCmd(canon);
    if (cmd == Cmd::None && canon.size() == 3 && canon[0] == 'A' && canon[1] == '-') {
        const char l = canon[2];
        const std::vector<Menu> &ms = Menus();
        for (size_t i = 0; i < ms.size(); ++i) {
            if (ms[i].hot == l) {
                OpenMenu(static_cast<int>(i));
                return true;
            }
        }
    }
    if (cmd != Cmd::None && IsAppCmd(cmd)) return ExecuteCmd(cmd, 0);
    if (cmd == Cmd::Help) {
        OpenMenu(static_cast<int>(Menus().size()) - 1); // o menu F1=Help
        return true;
    }
    const bool r = Ed().Key(canon);
    AfterEditorCommand();
    return r;
}

bool EditorApp::Text(const std::string &bytes) {
    if (HasDialog()) {
        for (const char c : bytes) {
            const int b = dlg_.Key(std::string(1, c));
            if (b) {
                DialogDone(b);
                break;
            }
        }
        return true;
    }
    if (menu_ >= 0) {
        for (const char c : bytes) MenuKey(std::string(1, c));
        return true;
    }
    Ed().ClearMessage();
    return Ed().Type(bytes);
}

// ---------------------------------------------------------------------------------------------------------------
// menus
// ---------------------------------------------------------------------------------------------------------------

namespace {

std::vector<int> VisibleItems(const Menu &m) {
    std::vector<int> v;
    for (size_t i = 0; i < m.items.size(); ++i) {
        if (!m.items[i].hidden && m.items[i].label) v.push_back(static_cast<int>(i));
    }
    return v;
}

} // namespace

void EditorApp::OpenMenu(int m) {
    Ed().CompletionClose();
    const std::vector<Menu> &ms = Menus();
    menu_ = (m + static_cast<int>(ms.size())) % static_cast<int>(ms.size());
    bar_active_ = true;
    item_ = 0;
    const Menu &menu = ms[static_cast<size_t>(menu_)];
    for (const int i : VisibleItems(menu)) {
        if (Enabled(menu.items[static_cast<size_t>(i)])) {
            item_ = i;
            break;
        }
    }
}

void EditorApp::CloseMenu() {
    menu_ = -1;
    bar_active_ = false;
}

void EditorApp::RunMenuItem(const MenuItem &it) {
    CloseMenu();
    if (!Enabled(it)) return;
    ExecuteCmd(it.cmd, it.arg);
}

bool EditorApp::MenuKey(const std::string &canon) {
    const std::vector<Menu> &ms = Menus();
    const Menu &menu = ms[static_cast<size_t>(menu_)];
    const std::vector<int> vis = VisibleItems(menu);
    auto step = [&](int dir) {
        if (vis.empty()) return;
        size_t pos = 0;
        for (size_t k = 0; k < vis.size(); ++k) {
            if (vis[k] == item_) pos = k;
        }
        for (size_t tries = 0; tries < vis.size(); ++tries) {
            pos = (pos + vis.size() + static_cast<size_t>(dir)) % vis.size();
            if (Enabled(menu.items[static_cast<size_t>(vis[pos])])) {
                item_ = vis[pos];
                return;
            }
        }
    };
    if (canon == "Esc" || canon == "F10") {
        CloseMenu();
    } else if (canon == "Left") {
        OpenMenu(menu_ - 1);
    } else if (canon == "Right") {
        OpenMenu(menu_ + 1);
    } else if (canon == "Up") {
        step(-1);
    } else if (canon == "Down") {
        step(1);
    } else if (canon == "Home") {
        if (!vis.empty()) item_ = vis.front();
    } else if (canon == "End") {
        if (!vis.empty()) item_ = vis.back();
    } else if (canon == "Enter") {
        RunMenuItem(menu.items[static_cast<size_t>(item_)]);
    } else {
        char letter = 0;
        if (canon == "Del") {
            letter = 0x7F;
        } else if (canon.size() == 1) {
            letter = static_cast<char>(std::tolower(static_cast<unsigned char>(canon[0])));
        } else if (canon.size() == 3 && canon[0] == 'A' && canon[1] == '-') {
            // Alt+letra de OUTRO menu troca de menu; a mesma letra do item tambem vale (Alt-F, depois Alt-X)
            const char l = canon[2];
            for (size_t i = 0; i < ms.size(); ++i) {
                if (ms[i].hot == l) {
                    OpenMenu(static_cast<int>(i));
                    return true;
                }
            }
        }
        if (letter) {
            for (const MenuItem &it : menu.items) {
                if (it.hot == letter) {
                    RunMenuItem(it);
                    return true;
                }
            }
        }
    }
    return true;
}

void EditorApp::RenderMenu(Grid &g) const {
    const Menu &m = Menus()[static_cast<size_t>(menu_)];
    const std::vector<int> vis = VisibleItems(m);
    size_t lw = 0, sw = 0;
    int seps = 0;
    for (const int i : vis) {
        lw = std::max(lw, std::string(m.items[static_cast<size_t>(i)].label).size());
        sw = std::max(sw, std::string(m.items[static_cast<size_t>(i)].shortcut).size());
        if (m.items[static_cast<size_t>(i)].separator_after) ++seps;
    }
    const int w = static_cast<int>(lw + sw) + (sw ? 3 : 0) + 4;
    const int h = static_cast<int>(vis.size()) + seps + 2;
    int x = MenuX(m, cols_);
    if (x + w + 2 > cols_) x = std::max(0, cols_ - w - 2);
    const int y = 1;
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            if (x + i >= cols_ || y + j >= rows_) continue;
            g.at(x + i, y + j).ch = ' ';
            g.at(x + i, y + j).style = kPanel;
        }
    }
    auto put = [&](int xx, int yy, uint16_t ch, Style st) {
        if (xx < 0 || yy < 0 || xx >= cols_ || yy >= rows_) return;
        g.at(xx, yy).ch = ch;
        g.at(xx, yy).style = st;
    };
    for (int i = 1; i < w - 1; ++i) {
        put(x + i, y, 0x2500, kPanel);
        put(x + i, y + h - 1, 0x2500, kPanel);
    }
    for (int j = 1; j < h - 1; ++j) {
        put(x, y + j, 0x2502, kPanel);
        put(x + w - 1, y + j, 0x2502, kPanel);
    }
    put(x, y, 0x250C, kPanel);
    put(x + w - 1, y, 0x2510, kPanel);
    put(x, y + h - 1, 0x2514, kPanel);
    put(x + w - 1, y + h - 1, 0x2518, kPanel);
    int row = y + 1;
    for (const int i : vis) {
        const MenuItem &it = m.items[static_cast<size_t>(i)];
        const bool enabled = Enabled(it);
        const bool sel = i == item_;
        const Style base = !enabled ? kPanelOff : (sel ? kPanelSel : kPanel);
        for (int k = 1; k < w - 1; ++k) put(x + k, row, ' ', base);
        const std::string label = it.label;
        for (size_t k = 0; k < label.size(); ++k) {
            Style st = base;
            if (enabled && !sel && it.hot && std::tolower(static_cast<unsigned char>(label[k])) == it.hot &&
                label.find_first_of(label[k]) == k) {
                st = kPanelHot;
            }
            put(x + 2 + static_cast<int>(k), row, static_cast<unsigned char>(label[k]), st);
        }
        const std::string sc = it.shortcut;
        for (size_t k = 0; k < sc.size(); ++k) put(x + w - 2 - static_cast<int>(sc.size()) + static_cast<int>(k), row, static_cast<unsigned char>(sc[k]), base);
        ++row;
        if (it.separator_after) {
            for (int k = 1; k < w - 1; ++k) put(x + k, row, 0x2500, kPanel);
            ++row;
        }
    }
    for (int j = 1; j <= h; ++j) {
        put(x + w, y + j, 0x2591, kShadow);
        put(x + w + 1, y + j, 0x2591, kShadow);
    }
    for (int i = 2; i < w + 2; ++i) put(x + i, y + h, 0x2591, kShadow);
}

// ---------------------------------------------------------------------------------------------------------------
// dialogos: abrir e concluir
// ---------------------------------------------------------------------------------------------------------------

void EditorApp::ShowDialog(Dialog d, int arg) {
    switch (d) {
    case Dialog::Open: BuildFileDialog(d, "Open file"); break;
    case Dialog::SaveAs: BuildFileDialog(d, "Save file as"); break;
    case Dialog::Merge: BuildFileDialog(d, "Merge file"); break;
    case Dialog::Window2: BuildFileDialog(d, "Open file"); break;
    case Dialog::Find: BuildFind(false); break;
    case Dialog::Replace: BuildFind(true); break;
    case Dialog::LineNumber: BuildSimpleInput(d, "Find line", "Line number:", "", 8); break;
    case Dialog::BlockIndent: BuildSimpleInput(d, "Indent", "Amount:", "4", 4); break;
    case Dialog::BlockUnindent: BuildSimpleInput(d, "Unindent", "Amount:", "4", 4); break;
    case Dialog::BlockOutput: BuildSimpleInput(d, "Output to file", "File name:", "", 40); break;
    case Dialog::TextSetup: BuildTextSetup(); break;
    case Dialog::PrintSetup: BuildPrintSetup(); break;
    case Dialog::Information: BuildInfo(); break;
    case Dialog::About: BuildAbout(); break;
    case Dialog::DisplaySetup: BuildDisplay(); break;
    case Dialog::MouseSetup: BuildMouse(); break;
    case Dialog::Language: BuildLanguage(); break;
    case Dialog::Errors: BuildMessagesDialog(); break;
    case Dialog::EditorSetup: BuildEditorSetup(); break;
    case Dialog::SaveConfig: BuildSaveConfig(); break;
    case Dialog::Help: BuildHelp(arg, 0); break;
    case Dialog::InsertPageBreaks: BuildPageBreakPrompt(d, "Insert page breaks starting at page..."); break;
    case Dialog::RemovePageBreaks: BuildPageBreakPrompt(d, "Remove page breaks starting at..."); break;
    case Dialog::ExpandTabs: BuildPageBreakPrompt(d, "Expand tabs to spaces starting at..."); break;
    case Dialog::PutInTabs: BuildPageBreakPrompt(d, "Replace left-hand spaces with tabs starting at..."); break;
    case Dialog::FormatDocument: Msg("Document format: not available yet"); break;
    case Dialog::Outline: Msg("Outline display: not available yet"); break;
    default: break;
    }
}

void EditorApp::NextExitStep() {
    while (exit_index_ < static_cast<int>(wins_.size())) {
        if (wins_[static_cast<size_t>(exit_index_)].Modified()) {
            active_ = exit_index_;
            confirm_purpose_ = 1;
            BuildConfirm({"This file has been changed.", "Save it before exiting?"}, {kBtnSave, kBtnDontSave, kBtnCancel}, kBtnDontSave, Dialog::ConfirmExit);
            return;
        }
        ++exit_index_;
    }
    if (config_dirty_ && exit_index_ == static_cast<int>(wins_.size())) {
        ++exit_index_;
        confirm_purpose_ = 5;
        BuildConfirm({"The editor's configuration has been changed.", "Save this configuration?"}, {kBtnSave, kBtnDontSave}, kBtnSave, Dialog::ConfirmExit);
        return;
    }
    quit_ = true;
}

void EditorApp::SaveConfigNow(bool this_dir) {
    std::string err;
    const std::string path = this_dir ? cfg_dir_ : cfg_main_;
    if (path.empty()) {
        Msg("No configuration file path");
        return;
    }
    if (SaveConfigFile(path, Ed().settings(), ui_, err)) {
        config_dirty_ = false;
        Msg("Configuration saved");
    } else {
        Msg(err);
    }
}

bool EditorApp::SaveActive() {
    Editor &e = Ed();
    if (e.Name().empty()) {
        after_save_ = confirm_purpose_;
        ShowDialog(Dialog::SaveAs);
        return false;
    }
    std::string err;
    if (!e.Save(err)) {
        Msg(err);
        return false;
    }
    return true;
}

void EditorApp::ContinueAfterConfirm() {
    const int purpose = confirm_purpose_;
    after_save_ = 0;
    confirm_purpose_ = 0;
    if (purpose == 1) {
        ++exit_index_;
        confirm_purpose_ = 1;
        NextExitStep();
    } else if (purpose == 2) {
        Ed().NewFile();
    } else if (purpose == 3) {
        std::string err;
        if (!Open(pending_path_, err, false)) Msg(err);
    } else if (purpose == 4) {
        RemoveTab(close_index_);
        close_index_ = -1;
    }
}

void EditorApp::ReplacePrompt() {
    Editor &e = Ed();
    if (!e.Find(find_text_, find_case_, find_back_)) {
        Msg(repl_count_ == 0 ? "Text not found" : std::to_string(repl_count_) + " replaced");
        return;
    }
    BuildConfirm({"Replace this occurrence?"}, {kBtnYes, kBtnNo, kBtnAll, kBtnCancel}, kBtnYes, Dialog::Replace);
    dlg_.tag = 1;
    dlg_.box_y = std::min(rows_ - dlg_.h - 1, std::max(1, Rect(active_).y0 + (e.Cursor().line - e.TopLine()) + 2));
}

void EditorApp::DialogDone(int button) {
    if (!HasDialog()) return;
    const Dialog kind = dlg_.kind;
    const bool is_file = kind == Dialog::Open || kind == Dialog::SaveAs || kind == Dialog::Merge || kind == Dialog::Window2;
    if (is_file && button == kBtnOk) {
        FileDialogEnter();
        return;
    }
    const bool file_chosen = button == kBtnOk + 100;
    if (file_chosen) button = kBtnOk;
    const DialogBox done = dlg_;
    dlg_ = DialogBox();
    Editor &e = Ed();
    std::string err;

    // --- ajuda: paginas ---
    if (kind == Dialog::Help) {
        if (button == kBtnNext) BuildHelp(done.help_topic, done.page + 1);
        else if (button == kBtnPrev) BuildHelp(done.help_topic, done.page - 1);
        return;
    }
    // --- confirmacoes (ConfirmExit) ---
    if (kind == Dialog::ConfirmExit) {
        if (button == kBtnCancel) {
            confirm_purpose_ = 0;
            exit_index_ = -1;
            close_index_ = -1;
            return;
        }
        if (confirm_purpose_ == 5) {
            if (button == kBtnSave) SaveConfigNow(false);
            confirm_purpose_ = 0;
            quit_ = true;
            return;
        }
        if (button == kBtnSave) {
            if (!SaveActive()) return; // sem nome: o dialogo Save As continua o fluxo
        }
        ContinueAfterConfirm();
        return;
    }
    // --- busca interativa ---
    if (kind == Dialog::Replace && done.tag == 1) {
        if (button == kBtnYes) {
            e.ReplaceHere(static_cast<int>(find_text_.size()), repl_text_);
            ++repl_count_;
            if (find_back_) e.SetCursor(Pos{e.Cursor().line, std::max(0, e.Cursor().col - static_cast<int>(repl_text_.size()))});
            ReplacePrompt();
        } else if (button == kBtnNo) {
            e.Execute(find_back_ ? Cmd::Left : Cmd::Right);
            ReplacePrompt();
        } else if (button == kBtnAll) {
            const int n = e.ReplaceAll(find_text_, repl_text_, find_case_);
            Msg(std::to_string(repl_count_ + n) + " replaced");
        }
        return;
    }
    if (button == kBtnCancel) {
        if (after_save_) { // o Save As do fluxo de saida foi cancelado
            after_save_ = 0;
            confirm_purpose_ = 0;
            exit_index_ = -1;
        }
        return;
    }
    switch (kind) {
    case Dialog::Open:
        if (!file_chosen) return;
        if (gui_mode_) {
            if (!OpenTab(pending_path_, err, false)) Msg(err);
            break;
        }
        if (e.Modified()) {
            confirm_purpose_ = 3;
            BuildConfirm({"This file has been changed.", "Save it before opening another?"}, {kBtnSave, kBtnDontSave, kBtnCancel}, kBtnDontSave, Dialog::ConfirmExit);
        } else if (!Open(pending_path_, err, false)) {
            Msg(err);
        }
        break;
    case Dialog::SaveAs:
        if (!file_chosen) return;
        if (!e.SaveAs(pending_path_, err)) {
            Msg(err);
        } else if (after_save_) {
            ContinueAfterConfirm();
        }
        break;
    case Dialog::Merge:
        if (file_chosen && !e.Merge(pending_path_, err)) Msg(err);
        break;
    case Dialog::Window2:
        if (gui_mode_) {
            if (!file_chosen) break;
            const int prev = active_;
            if (OpenTab(pending_path_, err, false)) {
                pane_[0] = prev;
                pane_[1] = active_;
                ap_ = 1;
                split_ = prev != active_;
            } else {
                Msg(err);
            }
            break;
        }
        if (file_chosen && !Open2(pending_path_, err, false)) Msg(err);
        break;
    case Dialog::Find:
    case Dialog::Replace: {
        find_text_ = done.Text(1);
        find_case_ = done.Checked(10);
        find_back_ = done.Checked(11);
        if (find_text_.empty()) return;
        if (kind == Dialog::Find) {
            e.Find(find_text_, find_case_, find_back_);
        } else {
            repl_text_ = done.Text(2);
            repl_count_ = 0;
            if (!e.ReadOnly()) ReplacePrompt();
        }
        break;
    }
    case Dialog::LineNumber: {
        const std::string t = done.Text(1);
        if (!t.empty()) e.GoToLine(std::atoi(t.c_str()));
        break;
    }
    case Dialog::BlockIndent:
        e.Indent(std::max(1, std::atoi(done.Text(1).c_str())));
        break;
    case Dialog::BlockUnindent:
        e.Unindent(std::max(1, std::atoi(done.Text(1).c_str())));
        break;
    case Dialog::BlockOutput:
        if (!e.WriteBlock(done.Text(1), err)) Msg(err);
        break;
    case Dialog::TextSetup: {
        Settings &s = e.settings();
        s.line_length = std::max(10, std::atoi(done.Text(1).c_str()));
        s.first_margin = std::atoi(done.Text(2).c_str());
        s.left_margin = std::atoi(done.Text(3).c_str());
        ui_.format_begin = done.Group(0);
        config_dirty_ = true;
        for (Editor &w : wins_) w.settings() = s;
        break;
    }
    case Dialog::PrintSetup: {
        Settings &s = e.settings();
        s.top_margin = std::atoi(done.Text(1).c_str());
        ui_.print_left_margin = std::atoi(done.Text(2).c_str());
        s.page_length = std::max(1, std::atoi(done.Text(3).c_str()));
        ui_.formfeed_between = done.Checked(10);
        ui_.formfeed_after = done.Group(0);
        config_dirty_ = true;
        for (Editor &w : wins_) w.settings() = s;
        break;
    }
    case Dialog::DisplaySetup: {
        static const int radio_to_scheme[] = {1, 2, 3, 4, 0};
        static const int radio_to_lines[] = {25, 35, 43, 50, 0};
        ui_.scheme = radio_to_scheme[done.Group(0)];
        ui_.lines = radio_to_lines[done.Group(1)];
        ui_.scrollbar = done.Checked(10);
        ui_.blinking_bracket = done.Checked(11);
        ui_.zoom_effects = done.Checked(12);
        ui_.large_cursor = done.Checked(13);
        Settings &s = e.settings();
        s.tab_width = std::max(1, std::min(32, std::atoi(done.Text(14).c_str())));
        s.show_bracket = ui_.blinking_bracket;
        s.syntax = done.Checked(15);
        s.line_numbers = done.Checked(16);
        for (Editor &w : wins_) w.settings() = s;
        config_dirty_ = true;
        SetSize(cols_, rows_);
        break;
    }
    case Dialog::Errors: {
        const Widget *l = done.Find(2);
        if (button == kBtnOk && l) BuildGoto(l->sel);
        break;
    }
    case Dialog::Reference:
        break;
    case Dialog::Language: {
        const Widget *l = done.Find(2);
        const int sel = l ? l->sel : 0;
        if (sel <= 0) {
            e.SetLanguage(nullptr, false);
            e.DetectLanguage();
        } else {
            e.SetLanguage(AllLangs()[static_cast<size_t>(sel - 1)], true);
        }
        break;
    }
    case Dialog::MouseSetup:
        ui_.mouse_button = done.Group(0);
        ui_.scroll_delay = done.Group(1);
        ui_.adj_mouse_speed = done.Checked(10);
        config_dirty_ = true;
        break;
    case Dialog::EditorSetup: {
        Settings &s = e.settings();
        s.over_mode = static_cast<OverMode>(done.Group(0));
        s.tab_key = static_cast<TabKey>(done.Group(1));
        s.backup_char = done.Group(2) == 1 ? '~' : done.Group(2) == 2 ? '@' : '{';
        s.delete_backup = done.Checked(10);
        s.alt_selects_menu = done.Checked(11);
        s.autosave_minutes = std::max(0, std::min(1440, std::atoi(done.Text(12).c_str())));
        s.autosave_files = done.Checked(13);
        s.auto_complete = done.Checked(15);
        for (Editor &w : wins_) w.settings() = s;
        config_dirty_ = true;
        break;
    }
    case Dialog::SaveConfig:
        SaveConfigNow(done.Group(0) == 1);
        break;
    case Dialog::InsertPageBreaks:
        e.ApplyInsertPageBreaks(button == kBtnTop);
        break;
    case Dialog::RemovePageBreaks:
        e.ApplyRemovePageBreaks(button == kBtnTop);
        break;
    case Dialog::ExpandTabs:
        e.ApplyExpandTabs(button == kBtnTop);
        break;
    case Dialog::PutInTabs:
        e.ApplyPutInTabs(button == kBtnTop);
        break;
    default:
        break;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// mouse
// ---------------------------------------------------------------------------------------------------------------

void EditorApp::MouseDown(int x, int y) {
    dragging_ = false;
    if (HasDialog()) {
        const int bx = dlg_.box_x >= 0 ? dlg_.box_x : std::max(0, (cols_ - dlg_.w) / 2);
        const int by = dlg_.box_y >= 0 ? dlg_.box_y : std::max(1, (rows_ - dlg_.h) / 2);
        if (x >= bx && x < bx + dlg_.w && y >= by && y < by + dlg_.h) {
            const int b = dlg_.ClickAt(x - bx, y - by);
            if (b) DialogDone(b);
        }
        return;
    }
    if (menu_ >= 0) {
        const Menu &m = Menus()[static_cast<size_t>(menu_)];
        // mesma conta do desenho
        const std::vector<int> vis = VisibleItems(m);
        size_t lw = 0, sw = 0;
        for (const int i : vis) {
            lw = std::max(lw, std::string(m.items[static_cast<size_t>(i)].label).size());
            sw = std::max(sw, std::string(m.items[static_cast<size_t>(i)].shortcut).size());
        }
        const int w = static_cast<int>(lw + sw) + (sw ? 3 : 0) + 4;
        int mx = MenuX(m, cols_);
        if (mx + w + 2 > cols_) mx = std::max(0, cols_ - w - 2);
        if (y >= 2 && x > mx && x < mx + w - 1) {
            int row = 2;
            for (const int i : vis) {
                if (y == row) {
                    RunMenuItem(m.items[static_cast<size_t>(i)]);
                    return;
                }
                ++row;
                if (m.items[static_cast<size_t>(i)].separator_after) ++row;
            }
            return;
        }
        if (y != 0) {
            CloseMenu();
            return;
        }
    }
    if (y == 0) {
        const std::vector<Menu> &ms = Menus();
        for (size_t i = 0; i < ms.size(); ++i) {
            const int mx = MenuX(ms[i], cols_);
            if (x >= mx - 1 && x <= mx + static_cast<int>(std::string(ms[i].title).size())) {
                OpenMenu(static_cast<int>(i));
                return;
            }
        }
        CloseMenu();
        return;
    }
    if (y >= rows_ - 1) return;
    for (int i = 0; i < static_cast<int>(wins_.size()); ++i) {
        const WinRect r = Rect(i);
        if (y < r.y0 || y >= r.y0 + r.h) continue;
        active_ = i;
        Editor &e = wins_[static_cast<size_t>(i)];
        const int row = y - r.y0;
        if (ui_.scrollbar && x == cols_ - 1) {
            if (row == 0) e.ScrollView(-1);
            else if (row == r.h - 1) e.ScrollView(1);
            else e.ScrollView(row < r.h / 2 ? -(r.h - 1) : (r.h - 1));
            return;
        }
        const Pos p = e.ScreenToPos(x, row);
        e.SetCursor(p);
        e.Execute(Cmd::BlockRemoveMarkers);
        dragging_ = true;
        drag_anchor_ = p;
        drag_win_ = i;
        return;
    }
}

void EditorApp::MouseDrag(int x, int y) {
    if (!dragging_ || HasDialog() || menu_ >= 0) return;
    Editor &e = wins_[static_cast<size_t>(drag_win_)];
    const WinRect r = Rect(drag_win_);
    int row = y - r.y0;
    if (row < 0) {
        e.ScrollView(-1);
        row = 0;
    } else if (row >= r.h) {
        e.ScrollView(1);
        row = r.h - 1;
    }
    const Pos p = e.ScreenToPos(x, row);
    e.SelectBlock(drag_anchor_, p);
    e.SetCursor(p);
}

void EditorApp::MouseUp(int, int) {
    dragging_ = false;
}

void EditorApp::Wheel(int x, int y, int dy) {
    (void)x;
    if (HasDialog() || menu_ >= 0) return;
    for (int i = 0; i < static_cast<int>(wins_.size()); ++i) {
        const WinRect r = Rect(i);
        if (y >= r.y0 && y < r.y0 + r.h) {
            wins_[static_cast<size_t>(i)].ScrollView(dy * 3);
            return;
        }
    }
    Ed().ScrollView(dy * 3);
}

// ---------------------------------------------------------------------------------------------------------------
// tela
// ---------------------------------------------------------------------------------------------------------------

void EditorApp::Render(Grid &g) const {
    g.Resize(cols_, rows_);
    g.PaintRow(0, kBar);
    const std::vector<Menu> &ms = Menus();
    for (size_t i = 0; i < ms.size(); ++i) {
        g.Put(MenuX(ms[i], cols_), 0, ms[i].title, static_cast<int>(i) == menu_ ? kBarSel : kBar);
    }

    for (int i = 0; i < static_cast<int>(wins_.size()); ++i) {
        const WinRect r = Rect(i);
        wins_[static_cast<size_t>(i)].RenderBody(g, r.y0, ui_.scrollbar);
    }
    if (wins_.size() == 2) {
        const int y = Rect(0).y0 + Rect(0).h;
        g.PaintRow(y, kTitleBar);
        const std::string name = wins_[0].Name().empty() ? "Untitled" : wins_[0].Name();
        g.Put(std::max(0, (cols_ - static_cast<int>(name.size())) / 2), y, name, kTitleBar);
    }
    Ed().RenderStatus(g, rows_ - 1);
    if (menu_ >= 0) RenderMenu(g);
    if (HasDialog()) RenderDialog(g);
}

} // namespace ed
