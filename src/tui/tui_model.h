// fwMSX -- TUI de menus: o MODELO (estado e logica), sem FTXUI. Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver doc/tui-spec.md.
//
// A TUI de menus NAO embute a maquina: ela coloca em menus os comandos do console (`emu start`, `cart`,
// `disk`, `romdb`...) e controla o emulador em JANELA pela ponte de controle. Este modelo so' decide o
// que cada tecla faz e QUAIS LINHAS DE COMANDO saem (via o callback `submit`); quem as executa e'
// o ReplSession (ver repl/repl_session.h). Separado do desenho para ser testavel sem terminal.
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace tui {

struct Key {
    enum Type { Char, Enter, Escape, Up, Down, Left, Right, Home, End, Backspace, Delete, Tab, PageUp, PageDown, F1, F10 };
    Type type = Char;
    std::string text; // so' para Char (um caractere UTF-8)
};

// O que se sabe do emulador conectado (consulta periodica de `status`).
struct EmuStatus {
    bool connected = false;
    int port = 0;
    bool paused = false;
    uint64_t frame = 0;
    int screen = 0;
};

class TuiModel {
public:
    enum class Mode { Normal, Menu, Prompt, Browser, Help };

    struct MenuItem {
        std::string label;
        std::function<void()> action; // vazio = separador
    };
    struct MenuDef {
        std::string title;
        std::vector<MenuItem> items;
    };
    struct Step {
        std::string label;
        std::string default_value;
        bool file = false;                // abre o navegador de arquivos
        std::vector<std::string> exts;    // extensoes aceitas no navegador (minusculas, sem ponto); vazio = todas
    };
    struct BrowserEntry {
        std::string name;
        bool dir = false;
    };

    // `submit` recebe cada linha de comando a executar. `start_dir` = onde o navegador de arquivos comeca.
    TuiModel(std::function<void(const std::string &)> submit, std::string start_dir);

    // ---- entrada ----
    void HandleKey(const Key &key);
    // Clique do mouse na celula (x, y) (0,0 = canto superior esquerdo).
    void Click(int x, int y);

    // ---- estado vindo de fora ----
    void SetStatus(const EmuStatus &status) { status_ = status; }
    void AddLog(const std::string &line);
    void SetSize(int cols, int rows) {
        cols_ = cols;
        rows_ = rows;
    }

    // ---- leitura (para desenhar) ----
    Mode mode() const { return mode_; }
    bool quit_requested() const { return quit_; }
    const EmuStatus &status() const { return status_; }
    int slot() const { return slot_; }
    const std::vector<std::string> &log() const { return log_; }
    int log_scroll() const { return log_scroll_; }
    const std::string &input() const { return input_; }
    int cursor() const { return cursor_; }

    std::vector<MenuDef> BuildMenus() const;
    int open_menu() const { return open_menu_; }
    int item_index() const { return item_index_; }
    // Posicao x de cada titulo na barra de menus e largura (para o desenho e para o clique).
    std::vector<std::pair<int, int>> TitleSpans() const;

    const std::string &prompt_title() const { return prompt_title_; }
    const Step &prompt_step() const { return steps_[static_cast<size_t>(step_index_)]; }
    int prompt_step_number() const { return step_index_ + 1; }
    int prompt_step_count() const { return static_cast<int>(steps_.size()); }
    const std::string &prompt_value() const { return prompt_value_; }

    const std::string &browser_dir() const { return browser_dir_; }
    const std::vector<BrowserEntry> &browser_entries() const { return browser_entries_; }
    int browser_index() const { return browser_index_; }

    static const std::vector<std::string> &HelpLines();

private:
    void Submit(const std::string &line);
    void StartWizard(const std::string &title, std::vector<Step> steps,
                     std::function<std::string(const std::vector<std::string> &)> builder);
    void AdvanceWizard(const std::string &value);
    void EnterStep();
    void OpenBrowser(const std::vector<std::string> &exts);
    void RefreshBrowser();
    void KeyNormal(const Key &key);
    void KeyMenu(const Key &key);
    void KeyPrompt(const Key &key);
    void KeyBrowser(const Key &key);
    void Complete();

    std::function<void(const std::string &)> submit_;
    Mode mode_ = Mode::Normal;
    bool quit_ = false;
    EmuStatus status_;
    int slot_ = 1;
    int cols_ = 80;
    int rows_ = 24;

    std::vector<std::string> log_;
    int log_scroll_ = 0;
    std::string input_;
    int cursor_ = 0;
    std::vector<std::string> history_;
    int history_pos_ = 0;

    int open_menu_ = 0;
    int item_index_ = 0;

    std::string prompt_title_;
    std::vector<Step> steps_;
    int step_index_ = 0;
    std::vector<std::string> values_;
    std::string prompt_value_;
    std::function<std::string(const std::vector<std::string> &)> builder_;

    std::string browser_dir_;
    std::vector<BrowserEntry> browser_entries_;
    int browser_index_ = 0;
    std::vector<std::string> browser_exts_;
    std::string start_dir_;
};

} // namespace tui
