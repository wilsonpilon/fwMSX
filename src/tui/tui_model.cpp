// fwMSX -- TUI de menus: modelo. Ver tui_model.h e doc/tui-spec.md.
#include "tui_model.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace tui {

namespace fs = std::filesystem;

namespace {

std::string Quote(const std::string &s) {
    std::string out = "\"";
    for (const char c : s) {
        if (c == '"') out += '\\';
        out += c;
    }
    return out + "\"";
}

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::vector<std::string> SplitLines(const std::string &text) {
    std::vector<std::string> out;
    size_t start = 0;
    while (start <= text.size()) {
        const size_t nl = text.find('\n', start);
        if (nl == std::string::npos) {
            if (start < text.size()) out.push_back(text.substr(start));
            break;
        }
        out.push_back(text.substr(start, nl - start));
        start = nl + 1;
    }
    return out;
}

const std::vector<std::string> kWords = {
    "help", "exit", "quit", "clear", "emu start", "emu attach", "emu detach", "emu stop", "emu status", "newdisk", "romdb",
    "cas", "fitadb", "msxdisk", "version", "status", "reset", "pause", "resume", "step", "type", "peek", "poke", "regs",
    "cart", "disk", "eject", "tape", "state save", "state load", "screenshot"};

const std::vector<std::string> kRomExts = {"rom", "mx1", "mx2", "bin"};
const std::vector<std::string> kDskExts = {"dsk"};
const std::vector<std::string> kTapeExts = {"cas", "tsx", "tzx"};
const std::vector<std::string> kSstExts = {"sst"};

} // namespace

TuiModel::TuiModel(std::function<void(const std::string &)> submit, std::string start_dir)
    : submit_(std::move(submit)), start_dir_(std::move(start_dir)) {
    browser_dir_ = start_dir_;
}

const std::vector<std::string> &TuiModel::HelpLines() {
    static const std::vector<std::string> lines = {
        "TUI de menus do fwMSX -- controla o emulador em janela pela ponte de controle",
        "",
        "  F10          abre/fecha a barra de menus (setas, Enter; Esc fecha); o mouse tambem funciona",
        "  F1           esta ajuda",
        "  Enter        executa a linha de comando digitada embaixo (os mesmos comandos do console)",
        "  Seta cima/baixo  historico        Tab  completa o comando        PgUp/PgDn  rola o historico",
        "  Home/End/setas/Delete/Backspace  editam a linha",
        "  exit ou quit sai da TUI (o emulador continua aberto); clear limpa o historico",
        "",
        "Primeiro passo: menu Emulador > Iniciar MSX1/MSX2/MSX2+ (abre a janela do emulador).",
        "Depois: Arquivo (estado, captura), Maquina, Midia (cartucho, disco, fita), Ferramentas.",
        "Itens com '...' pedem dados (e arquivos, com um navegador: setas, Enter, Backspace sobe,",
        "'t' digita o caminho). Esc cancela.",
        "",
        "  (qualquer tecla fecha esta ajuda)",
    };
    return lines;
}

void TuiModel::AddLog(const std::string &line) {
    log_.push_back(line);
    if (log_.size() > 1000) log_.erase(log_.begin(), log_.begin() + 200);
    if (log_scroll_ > 0) ++log_scroll_; // quem rolou para cima nao e' puxado para baixo
}

void TuiModel::Submit(const std::string &line) {
    AddLog("> " + line);
    if (submit_) submit_(line);
}

// ---------------------------------------------------------------- menus

std::vector<TuiModel::MenuDef> TuiModel::BuildMenus() const {
    TuiModel *self = const_cast<TuiModel *>(this); // as acoes alteram o modelo; os menus sao reconstruidos a cada uso
    const std::string slot_file = "fwmsx-estado-" + std::to_string(slot_) + ".sst";
    const bool paused = status_.paused;
    auto cmd = [self](const std::string &line) { return [self, line] { self->Submit(line); }; };
    auto wiz = [self](const std::string &title, std::vector<Step> steps,
                      std::function<std::string(const std::vector<std::string> &)> b) {
        return [self, title, steps, b] { self->StartWizard(title, steps, b); };
    };
    auto sep = [] { return MenuItem{"", nullptr}; };

    std::vector<MenuDef> m;

    m.push_back({"Emulador",
                 {{"Iniciar MSX1", cmd("emu start")},
                  {"Iniciar MSX2", cmd("emu start --msx2")},
                  {"Iniciar MSX2+", cmd("emu start --msx2p")},
                  {"Iniciar com opcoes...", wiz("Iniciar o emulador", {{"Opcoes do --msx (ex.: --msx2p --cart jogo.rom)", "", false, {}}},
                                                [](const std::vector<std::string> &v) { return "emu start " + v[0]; })},
                  sep(),
                  {"Conectar a um emulador aberto...", wiz("Conectar", {{"Porta do --ctl-port", "", false, {}}},
                                                           [](const std::vector<std::string> &v) { return "emu attach " + v[0]; })},
                  {"Desconectar (o emulador continua aberto)", cmd("emu detach")},
                  {"Encerrar o emulador", cmd("emu stop")},
                  {"Estado do emulador", cmd("emu status")}}});

    m.push_back({"Arquivo",
                 {{"Salvar estado no slot " + std::to_string(slot_), cmd("state save " + slot_file)},
                  {"Carregar estado do slot " + std::to_string(slot_), cmd("state load " + slot_file)},
                  {"Slot anterior", [self] {
                       self->slot_ = (self->slot_ + 7) % 9 + 1;
                       self->AddLog("slot de estado " + std::to_string(self->slot_));
                   }},
                  {"Proximo slot", [self] {
                       self->slot_ = self->slot_ % 9 + 1;
                       self->AddLog("slot de estado " + std::to_string(self->slot_));
                   }},
                  sep(),
                  {"Salvar estado em arquivo...", wiz("Salvar estado", {{"Arquivo .sst", "meu.sst", false, {}}},
                                                      [](const std::vector<std::string> &v) { return "state save " + Quote(v[0]); })},
                  {"Carregar estado de arquivo...", wiz("Carregar estado", {{"Arquivo .sst", "", true, kSstExts}},
                                                        [](const std::vector<std::string> &v) { return "state load " + Quote(v[0]); })},
                  {"Capturar tela (PNG)...", wiz("Capturar tela", {{"Arquivo .png", "tela.png", false, {}}},
                                                 [](const std::vector<std::string> &v) { return "screenshot " + Quote(v[0]); })},
                  sep(),
                  {"Sair da TUI", [self] { self->quit_ = true; }}}});

    m.push_back({"Maquina",
                 {{"Reiniciar", cmd("reset")},
                  {paused ? "Retomar" : "Pausar", cmd(paused ? "resume" : "pause")},
                  {"Avancar 1 quadro (pausado)", cmd("step 1")},
                  {"Avancar 10 quadros (pausado)", cmd("step 10")},
                  sep(),
                  {"Registradores do Z80", cmd("regs")},
                  {"Ler memoria...", wiz("Ler memoria", {{"Endereco (ex.: 0xC000)", "0x0000", false, {}}, {"Quantidade de bytes", "16", false, {}}},
                                         [](const std::vector<std::string> &v) { return "peek " + v[0] + " " + v[1]; })},
                  {"Gravar memoria...", wiz("Gravar memoria", {{"Endereco (ex.: 0xC000)", "0xC000", false, {}}, {"Valores (ex.: 0x41 0x42)", "", false, {}}},
                                            [](const std::vector<std::string> &v) { return "poke " + v[0] + " " + v[1]; })},
                  {"Digitar texto no MSX...", wiz("Digitar", {{"Texto (um Enter e' acrescentado no fim)", "", false, {}}},
                                                  [](const std::vector<std::string> &v) {
                                                      std::string t;
                                                      for (const char c : v[0]) {
                                                          if (c == '"') t += '\\';
                                                          t += c;
                                                      }
                                                      return "type \"" + t + "\\n\"";
                                                  })}}});

    m.push_back({"Midia",
                 {{"Inserir cartucho...", wiz("Inserir cartucho", {{"Arquivo da ROM", "", true, kRomExts}, {"Mapper (auto, gen8, gen16, konami5, konami4, ascii8, ascii16, msxdos2)", "auto", false, {}}},
                                              [](const std::vector<std::string> &v) { return "cart " + Quote(v[0]) + (v[1].empty() ? "" : " " + v[1]); })},
                  {"Retirar cartucho", cmd("cart -")},
                  sep(),
                  {"Inserir disco em A:...", wiz("Disco A:", {{"Arquivo .dsk", "", true, kDskExts}},
                                                 [](const std::vector<std::string> &v) { return "disk A " + Quote(v[0]); })},
                  {"Inserir disco em B:...", wiz("Disco B:", {{"Arquivo .dsk", "", true, kDskExts}},
                                                 [](const std::vector<std::string> &v) { return "disk B " + Quote(v[0]); })},
                  {"Ejetar disco A:", cmd("eject A")},
                  {"Ejetar disco B:", cmd("eject B")},
                  {"Novo disco em branco...", wiz("Novo disco", {{"Arquivo .dsk a criar", "novo.dsk", false, {}}, {"Formato (ss525=180K, ds525=360K, ss35=360K, ds35=720K)", "ds35", false, {}}},
                                                  [](const std::vector<std::string> &v) { return "newdisk " + Quote(v[0]) + " " + v[1]; })},
                  {"Novo disco e inserir em A:...", wiz("Novo disco em A:", {{"Arquivo .dsk a criar", "novo.dsk", false, {}}, {"Formato (ss525, ds525, ss35, ds35)", "ds35", false, {}}},
                                                        [](const std::vector<std::string> &v) {
                                                            return "newdisk " + Quote(v[0]) + " " + v[1] + "\ndisk A " + Quote(v[0]);
                                                        })},
                  sep(),
                  {"Inserir fita...", wiz("Inserir fita", {{"Arquivo .cas/.tsx/.tzx", "", true, kTapeExts}},
                                          [](const std::vector<std::string> &v) { return "tape " + Quote(v[0]); })},
                  {"Ejetar fita", cmd("tape eject")},
                  {"Rebobinar fita", cmd("tape rewind")}}});

    m.push_back({"Ferramentas",
                 {{"Banco de ROMs: buscar...", wiz("Buscar ROM", {{"Termo", "", false, {}}},
                                                   [](const std::vector<std::string> &v) { return "romdb search " + Quote(v[0]); })},
                  {"Banco de ROMs: mostrar por id...", wiz("Mostrar ROM", {{"Id", "", false, {}}},
                                                           [](const std::vector<std::string> &v) { return "romdb show " + v[0]; })},
                  {"Banco de ROMs: verificar SHA-1", cmd("romdb verify")},
                  {"Banco de ROMs: identificar", cmd("romdb identify")},
                  {"Banco de ROMs: estatisticas", cmd("romdb stats")},
                  {"Banco de ROMs: baixar fMSX", cmd("romdb fmsx")},
                  {"Banco de ROMs: comando livre...", wiz("romdb", {{"Argumentos do romdb", "", false, {}}},
                                                          [](const std::vector<std::string> &v) { return "romdb " + v[0]; })},
                  sep(),
                  {"Fita: listar arquivos...", wiz("Listar fita", {{"Arquivo .cas/.tsx/.tzx", "", true, kTapeExts}},
                                                   [](const std::vector<std::string> &v) { return "cas list " + Quote(v[0]); })},
                  {"Fita: comando livre (cas)...", wiz("cas", {{"Argumentos do cas (ex.: pack ...)", "", false, {}}},
                                                       [](const std::vector<std::string> &v) { return "cas " + v[0]; })},
                  {"Banco de fitas: listar", cmd("fitadb list")},
                  {"Banco de fitas: buscar...", wiz("Buscar fita", {{"Termo", "", false, {}}},
                                                    [](const std::vector<std::string> &v) { return "fitadb search " + Quote(v[0]); })},
                  {"Banco de fitas: estatisticas", cmd("fitadb stats")},
                  sep(),
                  {"Utilitario de discos (msxdisk)...", wiz("msxdisk", {{"Argumentos do msxdisk (ex.: list disco.dsk)", "", false, {}}},
                                                            [](const std::vector<std::string> &v) { return "msxdisk " + v[0]; })}}});

    m.push_back({"Ajuda",
                 {{"Teclas e uso (F1)", [self] { self->mode_ = Mode::Help; }},
                  {"Comandos do console", cmd("help")},
                  {"Versao do emulador", cmd("version")}}});
    return m;
}

std::vector<std::pair<int, int>> TuiModel::TitleSpans() const {
    std::vector<std::pair<int, int>> spans;
    int x = 0;
    for (const MenuDef &m : BuildMenus()) {
        const int w = static_cast<int>(m.title.size()) + 2;
        spans.push_back({x, w});
        x += w;
    }
    return spans;
}

// ---------------------------------------------------------------- assistente (prompts em sequencia)

void TuiModel::StartWizard(const std::string &title, std::vector<Step> steps,
                           std::function<std::string(const std::vector<std::string> &)> builder) {
    prompt_title_ = title;
    steps_ = std::move(steps);
    builder_ = std::move(builder);
    values_.clear();
    step_index_ = 0;
    EnterStep();
}

void TuiModel::EnterStep() {
    const Step &step = steps_[static_cast<size_t>(step_index_)];
    if (step.file) {
        OpenBrowser(step.exts);
    } else {
        prompt_value_ = step.default_value;
        mode_ = Mode::Prompt;
    }
}

void TuiModel::AdvanceWizard(const std::string &value) {
    values_.push_back(value);
    ++step_index_;
    if (step_index_ >= static_cast<int>(steps_.size())) {
        mode_ = Mode::Normal;
        for (const std::string &line : SplitLines(builder_(values_))) Submit(line);
        return;
    }
    EnterStep();
}

// ---------------------------------------------------------------- navegador de arquivos

void TuiModel::OpenBrowser(const std::vector<std::string> &exts) {
    browser_exts_ = exts;
    mode_ = Mode::Browser;
    RefreshBrowser();
}

void TuiModel::RefreshBrowser() {
    browser_entries_.clear();
    browser_index_ = 0;
    std::error_code ec;
    const fs::path dir(browser_dir_);
    if (dir.has_parent_path() && dir.parent_path() != dir) browser_entries_.push_back({"..", true});
    std::vector<BrowserEntry> dirs, files;
    for (fs::directory_iterator it(dir, fs::directory_options::skip_permission_denied, ec), end; !ec && it != end; it.increment(ec)) {
        std::error_code e2;
        const std::string name = it->path().filename().string();
        if (it->is_directory(e2)) {
            dirs.push_back({name, true});
        } else {
            std::string ext = Lower(it->path().extension().string());
            if (!ext.empty() && ext[0] == '.') ext.erase(0, 1);
            const bool ok = browser_exts_.empty() || std::find(browser_exts_.begin(), browser_exts_.end(), ext) != browser_exts_.end();
            if (ok) files.push_back({name, false});
        }
    }
    auto by_name = [](const BrowserEntry &a, const BrowserEntry &b) { return Lower(a.name) < Lower(b.name); };
    std::sort(dirs.begin(), dirs.end(), by_name);
    std::sort(files.begin(), files.end(), by_name);
    browser_entries_.insert(browser_entries_.end(), dirs.begin(), dirs.end());
    browser_entries_.insert(browser_entries_.end(), files.begin(), files.end());
}

void TuiModel::KeyBrowser(const Key &key) {
    const int n = static_cast<int>(browser_entries_.size());
    switch (key.type) {
    case Key::Escape:
        mode_ = Mode::Normal;
        return;
    case Key::Up:
        if (browser_index_ > 0) --browser_index_;
        return;
    case Key::Down:
        if (browser_index_ + 1 < n) ++browser_index_;
        return;
    case Key::PageUp:
        browser_index_ = std::max(0, browser_index_ - 10);
        return;
    case Key::PageDown:
        browser_index_ = std::min(std::max(0, n - 1), browser_index_ + 10);
        return;
    case Key::Home:
        browser_index_ = 0;
        return;
    case Key::End:
        browser_index_ = std::max(0, n - 1);
        return;
    case Key::Backspace: {
        const fs::path parent = fs::path(browser_dir_).parent_path();
        if (!parent.empty() && parent != fs::path(browser_dir_)) {
            browser_dir_ = parent.string();
            RefreshBrowser();
        }
        return;
    }
    case Key::Enter: {
        if (n == 0) return;
        const BrowserEntry &e = browser_entries_[static_cast<size_t>(browser_index_)];
        if (e.dir) {
            browser_dir_ = (e.name == ".." ? fs::path(browser_dir_).parent_path() : fs::path(browser_dir_) / e.name).string();
            RefreshBrowser();
        } else {
            AdvanceWizard((fs::path(browser_dir_) / e.name).string());
        }
        return;
    }
    case Key::Char:
        if (key.text == "t" || key.text == "T") { // digitar o caminho a mao
            prompt_value_ = (fs::path(browser_dir_) / "").string();
            mode_ = Mode::Prompt;
        }
        return;
    default:
        return;
    }
}

// ---------------------------------------------------------------- teclas

void TuiModel::KeyPrompt(const Key &key) {
    switch (key.type) {
    case Key::Escape:
        mode_ = Mode::Normal;
        return;
    case Key::Enter:
        AdvanceWizard(prompt_value_);
        return;
    case Key::Backspace:
        if (!prompt_value_.empty()) prompt_value_.pop_back();
        return;
    case Key::Char:
        prompt_value_ += key.text;
        return;
    default:
        return;
    }
}

void TuiModel::KeyMenu(const Key &key) {
    std::vector<MenuDef> menus = BuildMenus();
    const int count = static_cast<int>(menus.size());
    auto skip = [&](int dir) {
        const auto &items = menus[static_cast<size_t>(open_menu_)].items;
        const int n = static_cast<int>(items.size());
        for (int i = 0; i < n && !items[static_cast<size_t>(item_index_)].action; ++i) item_index_ = (item_index_ + dir + n) % n;
    };
    switch (key.type) {
    case Key::Escape:
    case Key::F10:
        mode_ = Mode::Normal;
        return;
    case Key::Left:
        open_menu_ = (open_menu_ + count - 1) % count;
        item_index_ = 0;
        return;
    case Key::Right:
        open_menu_ = (open_menu_ + 1) % count;
        item_index_ = 0;
        return;
    case Key::Up: {
        const int n = static_cast<int>(menus[static_cast<size_t>(open_menu_)].items.size());
        item_index_ = (item_index_ + n - 1) % n;
        skip(-1);
        return;
    }
    case Key::Down: {
        const int n = static_cast<int>(menus[static_cast<size_t>(open_menu_)].items.size());
        item_index_ = (item_index_ + 1) % n;
        skip(1);
        return;
    }
    case Key::Enter: {
        const MenuItem &item = menus[static_cast<size_t>(open_menu_)].items[static_cast<size_t>(item_index_)];
        if (!item.action) return;
        mode_ = Mode::Normal; // a acao pode abrir um assistente ou a ajuda
        item.action();
        return;
    }
    default:
        return;
    }
}

void TuiModel::Complete() {
    // So' completa a linha inteira (sem argumentos): o prefixo comum dos comandos que comecam com ela.
    std::vector<std::string> hits;
    for (const std::string &w : kWords)
        if (w.compare(0, input_.size(), input_) == 0) hits.push_back(w);
    if (hits.empty()) return;
    std::string common = hits[0];
    for (const std::string &h : hits) {
        size_t i = 0;
        while (i < common.size() && i < h.size() && common[i] == h[i]) ++i;
        common.resize(i);
    }
    if (common.size() > input_.size()) {
        input_ = common;
        cursor_ = static_cast<int>(input_.size());
    } else if (hits.size() > 1) {
        std::string list = "  ";
        for (const std::string &h : hits) list += h + "  ";
        AddLog(list);
    }
}

void TuiModel::KeyNormal(const Key &key) {
    switch (key.type) {
    case Key::Enter: {
        if (input_.empty()) return;
        const std::string line = input_;
        input_.clear();
        cursor_ = 0;
        history_.push_back(line);
        history_pos_ = static_cast<int>(history_.size());
        log_scroll_ = 0;
        if (line == "exit" || line == "quit") {
            quit_ = true;
        } else if (line == "clear") {
            log_.clear();
        } else {
            Submit(line);
        }
        return;
    }
    case Key::Backspace:
        if (cursor_ > 0) {
            input_.erase(static_cast<size_t>(cursor_ - 1), 1);
            --cursor_;
        }
        return;
    case Key::Delete:
        if (cursor_ < static_cast<int>(input_.size())) input_.erase(static_cast<size_t>(cursor_), 1);
        return;
    case Key::Left:
        if (cursor_ > 0) --cursor_;
        return;
    case Key::Right:
        if (cursor_ < static_cast<int>(input_.size())) ++cursor_;
        return;
    case Key::Home:
        cursor_ = 0;
        return;
    case Key::End:
        cursor_ = static_cast<int>(input_.size());
        return;
    case Key::Up:
        if (!history_.empty() && history_pos_ > 0) {
            input_ = history_[static_cast<size_t>(--history_pos_)];
            cursor_ = static_cast<int>(input_.size());
        }
        return;
    case Key::Down:
        if (history_pos_ + 1 < static_cast<int>(history_.size())) {
            input_ = history_[static_cast<size_t>(++history_pos_)];
        } else {
            history_pos_ = static_cast<int>(history_.size());
            input_.clear();
        }
        cursor_ = static_cast<int>(input_.size());
        return;
    case Key::PageUp:
        log_scroll_ = std::min(static_cast<int>(log_.size()), log_scroll_ + 5);
        return;
    case Key::PageDown:
        log_scroll_ = std::max(0, log_scroll_ - 5);
        return;
    case Key::Tab:
        Complete();
        return;
    case Key::Char:
        input_.insert(static_cast<size_t>(cursor_), key.text);
        cursor_ += static_cast<int>(key.text.size());
        return;
    default:
        return;
    }
}

void TuiModel::HandleKey(const Key &key) {
    if (mode_ == Mode::Help) {
        mode_ = Mode::Normal;
        return;
    }
    if (key.type == Key::F1) {
        mode_ = Mode::Help;
        return;
    }
    if (key.type == Key::F10) {
        if (mode_ == Mode::Menu) {
            mode_ = Mode::Normal;
        } else if (mode_ == Mode::Normal) {
            mode_ = Mode::Menu;
            open_menu_ = 0; // sempre abre no primeiro menu (previsivel)
            item_index_ = 0;
        }
        return;
    }
    switch (mode_) {
    case Mode::Normal: KeyNormal(key); break;
    case Mode::Menu: KeyMenu(key); break;
    case Mode::Prompt: KeyPrompt(key); break;
    case Mode::Browser: KeyBrowser(key); break;
    case Mode::Help: break;
    }
}

void TuiModel::Click(int x, int y) {
    if (mode_ == Mode::Help) {
        mode_ = Mode::Normal;
        return;
    }
    const std::vector<std::pair<int, int>> spans = TitleSpans();
    if (y == 0) {
        for (size_t i = 0; i < spans.size(); ++i) {
            if (x >= spans[i].first && x < spans[i].first + spans[i].second) {
                if (mode_ == Mode::Menu && open_menu_ == static_cast<int>(i)) {
                    mode_ = Mode::Normal;
                } else if (mode_ == Mode::Normal || mode_ == Mode::Menu) {
                    mode_ = Mode::Menu;
                    open_menu_ = static_cast<int>(i);
                    item_index_ = 0;
                }
                return;
            }
        }
        if (mode_ == Mode::Menu) mode_ = Mode::Normal;
        return;
    }
    if (mode_ != Mode::Menu) return;
    const std::vector<MenuDef> menus = BuildMenus();
    const MenuDef &menu = menus[static_cast<size_t>(open_menu_)];
    size_t widest = 0;
    for (const MenuItem &it : menu.items) widest = std::max(widest, it.label.size());
    const int left = spans[static_cast<size_t>(open_menu_)].first;
    const int width = static_cast<int>(widest) + 4; // borda + espaco de cada lado
    const int index = y - 2;                        // linha 1 = borda de cima do menu suspenso
    if (x >= left && x < left + width && index >= 0 && index < static_cast<int>(menu.items.size()) &&
        menu.items[static_cast<size_t>(index)].action) {
        mode_ = Mode::Normal;
        menu.items[static_cast<size_t>(index)].action();
        return;
    }
    mode_ = Mode::Normal; // clicou fora: fecha
}

} // namespace tui
