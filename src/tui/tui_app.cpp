// fwMSX -- TUI de menus (FTXUI): desenho e ligacao com o console. Ver tui_app.h e doc/tui-spec.md.
#include "tui_app.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

#include "../common/version.h"
#include "../repl/repl.h"
#include "tui_model.h"

namespace tui {

namespace {

using namespace ftxui;

// Traduz o evento do FTXUI para a tecla do modelo. false = tecla que o modelo ignora.
bool ToKey(const Event &e, Key &key) {
    key = Key{};
    if (e == Event::Return) key.type = Key::Enter;
    else if (e == Event::Escape) key.type = Key::Escape;
    else if (e == Event::ArrowUp) key.type = Key::Up;
    else if (e == Event::ArrowDown) key.type = Key::Down;
    else if (e == Event::ArrowLeft) key.type = Key::Left;
    else if (e == Event::ArrowRight) key.type = Key::Right;
    else if (e == Event::Home) key.type = Key::Home;
    else if (e == Event::End) key.type = Key::End;
    else if (e == Event::Backspace) key.type = Key::Backspace;
    else if (e == Event::Delete) key.type = Key::Delete;
    else if (e == Event::Tab) key.type = Key::Tab;
    else if (e == Event::PageUp) key.type = Key::PageUp;
    else if (e == Event::PageDown) key.type = Key::PageDown;
    else if (e == Event::F1) key.type = Key::F1;
    else if (e == Event::F10) key.type = Key::F10;
    else if (e == Event::CtrlS) key.type = Key::CtrlS;
    else if (e.is_character()) {
        key.type = Key::Char;
        key.text = e.character();
    } else {
        return false;
    }
    return true;
}

// Le "frame=N paused=P screen=S" da resposta do `status` do emulador.
void ParseStatus(const std::string &reply, EmuStatus &st) {
    auto num = [&](const std::string &name) -> long long {
        const size_t p = reply.find(name + "=");
        return p == std::string::npos ? 0 : std::atoll(reply.c_str() + p + name.size() + 1);
    };
    st.frame = static_cast<uint64_t>(num("frame"));
    st.paused = num("paused") != 0;
    st.stopped = num("stopped") != 0;
    st.screen = static_cast<int>(num("screen"));
}

// Texto que a thread corrente escreve em std::cout/std::cerr vai para ca' (nullptr = segue para o terminal). O FTXUI desenha a
// tela escrevendo em std::cout; se o worker trocasse o rdbuf global para capturar a saida de um comando, qualquer redesenho
// feito pela thread da interface nesse intervalo cairia no historico como um monte de codigos ANSI.
thread_local std::string *g_capture = nullptr;

class ThreadCaptureBuf : public std::streambuf {
public:
    explicit ThreadCaptureBuf(std::streambuf *orig) : orig_(orig) {}

protected:
    int_type overflow(int_type c) override {
        if (traits_type::eq_int_type(c, traits_type::eof())) return traits_type::not_eof(c);
        if (g_capture) {
            g_capture->push_back(static_cast<char>(c));
            return c;
        }
        return orig_->sputc(static_cast<char>(c));
    }
    std::streamsize xsputn(const char *s, std::streamsize n) override {
        if (g_capture) {
            g_capture->append(s, static_cast<size_t>(n));
            return n;
        }
        return orig_->sputn(s, n);
    }
    int sync() override { return g_capture ? 0 : orig_->pubsync(); }

private:
    std::streambuf *orig_;
};

class App {
public:
    App(std::unique_ptr<repl::ReplSession> session, std::string start_dir)
        : session_(std::move(session)),
          model_([this](const std::string &line) { Enqueue(line); }, std::move(start_dir)) {
        // `man <topico>` abre a janelinha de leitura (o worker chama isto no meio do Execute, sem segurar o mutex do modelo).
        session_->SetViewer([this](const std::string &title, const std::string &text) {
            std::lock_guard<std::mutex> lock(mu_);
            model_.OpenMan(title, text);
        });
        session_->EnablePanels(true); // o monitor Mamute abre DM, M, S, ZAP e SCR como paineis desta TUI
    }

    ~App() { StopWorker(); }

    void StartWorker() {
        out_router_ = std::make_unique<ThreadCaptureBuf>(std::cout.rdbuf());
        err_router_ = std::make_unique<ThreadCaptureBuf>(std::cerr.rdbuf());
        old_out_ = std::cout.rdbuf(out_router_.get());
        old_err_ = std::cerr.rdbuf(err_router_.get());
        running_ = true;
        worker_ = std::thread(&App::WorkerLoop, this);
    }

    void StopWorker() {
        if (running_.exchange(false)) {
            cv_.notify_all();
            if (worker_.joinable()) worker_.join();
            std::cout.rdbuf(old_out_);
            std::cerr.rdbuf(old_err_);
        }
    }

    std::mutex &model_mutex() { return mu_; }
    TuiModel &model() { return model_; }
    repl::ReplSession &session() { return *session_; }
    bool busy() const { return busy_; }
    // A tela so' e' redesenhada quando algo mudou (tecla, resposta do emulador, estado novo) ou ha' um comando rodando.
    bool TakeDirty() { return dirty_.exchange(false); }

    void Enqueue(const std::string &line) {
        {
            std::lock_guard<std::mutex> lock(jobs_mu_);
            jobs_.push_back(line);
        }
        cv_.notify_all();
    }

private:
    // Executa os comandos na ordem, fora da thread da interface (um `romdb` que baixa arquivos demora).
    // As ferramentas escrevem em std::cout/std::cerr; enquanto uma roda, a saida vai para o historico.
    void WorkerLoop() {
        while (running_) {
            std::string line;
            bool have = false;
            {
                std::unique_lock<std::mutex> lock(jobs_mu_);
                cv_.wait_for(lock, std::chrono::milliseconds(400), [&] { return !jobs_.empty() || !running_; });
                if (!running_) break;
                if (!jobs_.empty()) {
                    line = jobs_.front();
                    jobs_.pop_front();
                    have = true;
                }
            }
            if (have) {
                busy_ = true;
                std::string captured;
                std::ostringstream out;
                {
                    std::lock_guard<std::mutex> lock(mu_);
                    session_->SetWidth(std::max(30, std::min(100, model_.cols() - 6))); // help/man cabem na caixa
                }
                g_capture = &captured;
                session_->Execute(line, out);
                g_capture = nullptr;
                const std::string text = captured + out.str();
                {
                    std::lock_guard<std::mutex> lock(mu_);
                    if (text.rfind("action=", 0) == 0 && text.substr(0, text.find(10)).find("panel=mamute") != std::string::npos) {
                        // Painel do Mamute (DM, M, S, ZAP, SCR): a resposta e' o painel (ou o fechamento dele).
                        model_.SetMamutePanel(text);
                    } else if (line.rfind("dbgui ", 0) == 0) {
                        // Painel Z80 Debugger: a resposta inteira (acao + linhas) vai para o painel.
                        std::string panel;
                        std::istringstream in(text);
                        std::string l;
                        while (std::getline(in, l)) {
                            if (!l.empty() && l.back() == '\r') l.pop_back();
                            panel += (panel.empty() ? "" : "\n") + l;
                        }
                        model_.SetPanel(panel);
                    } else {
                        std::istringstream in(text);
                        std::string l;
                        while (std::getline(in, l)) {
                            if (!l.empty() && l.back() == '\r') l.pop_back();
                            model_.AddLog(l);
                        }
                    }
                }
                busy_ = false;
            }
            PollStatus();
            dirty_ = true; // chegou resposta/estado novo: a tela precisa ser redesenhada
        }
    }

    void PollStatus() {
        EmuStatus st;
        st.connected = session_->connected();
        st.port = st.connected ? session_->link().port() : 0;
        st.monitor_prompt = session_->MonitorPrompt();
        if (st.connected) {
            std::string reply, error;
            if (session_->link().Send("status", reply, error)) ParseStatus(reply, st);
            else st.connected = false;
        }
        // Eventos assincronos do emulador (breakpoint atingido...) vao para o historico.
        std::ostringstream events;
        session_->PrintEvents(events, 0);
        std::lock_guard<std::mutex> lock(mu_);
        model_.SetStatus(st);
        std::istringstream in(events.str());
        std::string l;
        bool stop_event = false;
        while (std::getline(in, l)) {
            model_.AddLog(l);
            stop_event = stop_event || l.find("break id=") != std::string::npos || l.find("watch id=") != std::string::npos ||
                         l.find("until pc=") != std::string::npos;
        }
        // O emulador parou num breakpoint/watchpoint/until: abre o painel Z80 Debugger (como a janela faz).
        if (stop_event && model_.mode() == TuiModel::Mode::Normal) model_.OpenPanel();
    }

    std::unique_ptr<repl::ReplSession> session_;
    TuiModel model_;
    std::mutex mu_;
    std::mutex jobs_mu_;
    std::condition_variable cv_;
    std::deque<std::string> jobs_;
    std::thread worker_;
    std::unique_ptr<ThreadCaptureBuf> out_router_, err_router_;
    std::streambuf *old_out_ = nullptr, *old_err_ = nullptr;
    std::atomic<bool> running_{false};
    std::atomic<bool> busy_{false};
    std::atomic<bool> dirty_{true};
};

// ---- desenho ----------------------------------------------------------------------------------

Element MenuBar(const TuiModel &m) {
    Elements titles;
    const auto menus = m.BuildMenus();
    for (size_t i = 0; i < menus.size(); ++i) {
        Element t = text(" " + menus[i].title + " ");
        if (m.mode() == TuiModel::Mode::Menu && m.open_menu() == static_cast<int>(i)) t = t | inverted;
        titles.push_back(t);
    }
    titles.push_back(filler());
    titles.push_back(text(" F10 menus  F1 ajuda ") | dim);
    return hbox(titles) | bgcolor(Color::Blue) | color(Color::White);
}

Element Dropdown(const TuiModel &m) {
    const auto menus = m.BuildMenus();
    const auto &menu = menus[static_cast<size_t>(m.open_menu())];
    size_t widest = 0;
    for (const auto &it : menu.items) widest = std::max(widest, it.label.size());
    Elements items;
    for (size_t i = 0; i < menu.items.size(); ++i) {
        if (!menu.items[i].action) {
            items.push_back(text(std::string(widest + 2, '-')) | dim);
            continue;
        }
        std::string label = " " + menu.items[i].label;
        label.resize(widest + 2, ' ');
        Element line = text(label);
        if (static_cast<int>(i) == m.item_index()) line = line | inverted;
        items.push_back(line);
    }
    const auto spans = m.TitleSpans();
    const int left = spans[static_cast<size_t>(m.open_menu())].first;
    Element box = vbox(items) | border | clear_under;
    return vbox({text(""), hbox({text(std::string(static_cast<size_t>(left), ' ')), box}), filler()});
}

Element Dialog(const std::string &title, Elements body) {
    Element box = window(text(" " + title + " "), vbox(std::move(body))) | clear_under | size(WIDTH, GREATER_THAN, 50);
    return vbox({filler(), hbox({filler(), box, filler()}), filler()});
}

Element PromptDialog(const TuiModel &m) {
    const auto &step = m.prompt_step();
    return Dialog(m.prompt_title() + "  (" + std::to_string(m.prompt_step_number()) + "/" + std::to_string(m.prompt_step_count()) + ")",
                  {text(" " + step.label),
                   text(" > " + m.prompt_value() + "_") | bold,
                   text(" Enter confirma - Esc cancela") | dim});
}

Element BrowserDialog(const TuiModel &m) {
    const auto &entries = m.browser_entries();
    const int total = static_cast<int>(entries.size());
    const int visible = 14;
    int first = std::max(0, m.browser_index() - visible / 2);
    first = std::min(first, std::max(0, total - visible));
    Elements lines;
    lines.push_back(text(" " + m.browser_dir()) | bold);
    for (int i = first; i < std::min(total, first + visible); ++i) {
        const auto &e = entries[static_cast<size_t>(i)];
        Element line = text(" " + (e.dir ? "[" + e.name + "]" : e.name));
        if (i == m.browser_index()) line = line | inverted;
        lines.push_back(line);
    }
    if (total == 0) lines.push_back(text(" (nenhum arquivo aqui)") | dim);
    lines.push_back(text(" Enter abre/escolhe - Backspace sobe - t digita o caminho - Esc cancela") | dim);
    return Dialog(m.prompt_title() + ": " + m.prompt_step().label, std::move(lines));
}

// Painel Z80 Debugger: as linhas vem do emulador com uma letra de estilo na frente (T titulo, R registradores,
// C cursor, P pergunta, D ajuda, N normal).
// `max_rows`: linhas de terminal disponiveis para a caixa. O painel inteiro tem 32 linhas; num terminal menor mostra
// o cabecalho (registradores), o rodape (se couber) e uma janela do corpo que contem a linha do cursor.
// Janelinha do `man`: o texto rolavel, com o titulo do topico e a posicao no canto. O texto ja' vem quebrado na largura certa.
Element ManDialog(const TuiModel &m, const Dimensions &dims) {
    const auto &all = m.man_lines();
    const int page = std::max(1, dims.dimy - 6);
    const int first = std::min(m.man_scroll(), std::max(0, static_cast<int>(all.size()) - page));
    Elements lines;
    for (int i = first; i < std::min<int>(static_cast<int>(all.size()), first + page); ++i) {
        const std::string &l = all[static_cast<size_t>(i)];
        Element e = text(" " + (l.empty() ? std::string(" ") : l));
        // titulos das secoes (MAIUSCULAS na primeira coluna) em destaque
        if (!l.empty() && l[0] >= 'A' && l[0] <= 'Z' && l.find_first_of("abcdefghijklmnopqrstuvwxyz") == std::string::npos) e = e | bold | color(Color::Yellow);
        lines.push_back(e);
    }
    const int shown_to = std::min<int>(static_cast<int>(all.size()), first + page);
    const std::string pos = " " + std::to_string(all.empty() ? 0 : first + 1) + "-" + std::to_string(shown_to) + "/" + std::to_string(all.size()) + " ";
    lines.push_back(filler());
    lines.push_back(text(" setas/PgUp/PgDn/espaco rolam - Home/End - Q ou Esc fecham") | dim);
    return window(text(" man " + m.man_title() + " " + pos) | bold, vbox(std::move(lines))) | size(WIDTH, GREATER_THAN, std::min(dims.dimx - 2, 40)) |
           size(HEIGHT, EQUAL, std::max(5, dims.dimy - 3)) | clear_under;
}

// Painel do Mamute: a primeira linha e' o titulo da caixa, a ultima e' a ajuda; o que esta' entre  e  (o cursor) sai invertido.
Element MamuteDialog(const TuiModel &m, const Dimensions &dims) {
    const auto &all = m.mamute_lines();
    Elements lines;
    for (size_t i = 0; i < all.size(); ++i) {
        const std::string &l = all[i];
        Elements parts;
        std::string cur;
        bool inv = false;
        for (const char c : l) {
            if (c == 1 || c == 2) {
                if (!cur.empty()) parts.push_back(inv ? text(cur) | inverted : text(cur));
                cur.clear();
                inv = c == 1;
            } else {
                cur += c;
            }
        }
        if (!cur.empty() || parts.empty()) parts.push_back(inv ? text(cur) | inverted : text(cur.empty() ? std::string(" ") : cur));
        Element line = hbox(std::move(parts));
        if (i + 1 == all.size() && all.size() > 1) line = line | dim;
        lines.push_back(line);
    }
    lines.push_back(filler());
    return window(text(" " + m.mamute_title() + " ") | bold, vbox(std::move(lines))) | size(WIDTH, GREATER_THAN, std::min(dims.dimx - 2, 30)) | clear_under;
}

Element PanelDialog(const TuiModel &m, int max_rows) {
    const auto &p = m.panel();
    std::string title = "Z80 Debugger";
    Elements lines;
    std::vector<size_t> shown;
    constexpr size_t kHeaderEnd = 6, kFooterBegin = 28; // p[0] = titulo; p[1..5] = registradores; p[6..27] = corpo; p[28..31] = rodape
    const int room = std::max(4, max_rows - 2); // menos as bordas da caixa
    if (p.size() != 32 || static_cast<int>(p.size()) - 1 <= room) {
        for (size_t i = 0; i < p.size(); ++i) shown.push_back(i);
    } else {
        for (size_t i = 0; i < kHeaderEnd; ++i) shown.push_back(i);
        bool footer = room >= static_cast<int>(kHeaderEnd) - 1 + 8 + 4; // so' mostra a ajuda se sobrar espaco para o corpo
        const int body_room = std::max(1, room - (static_cast<int>(kHeaderEnd) - 1) - (footer ? 4 : 0));
        int cur = 0; // indice do cursor dentro do corpo
        for (size_t i = kHeaderEnd; i < kFooterBegin; ++i) {
            if (!p[i].empty() && p[i][0] == 'C') cur = static_cast<int>(i - kHeaderEnd);
        }
        const int body_n = static_cast<int>(kFooterBegin - kHeaderEnd);
        const int start = std::max(0, std::min(cur - body_room / 2, body_n - body_room));
        for (int k = 0; k < std::min(body_room, body_n); ++k) shown.push_back(kHeaderEnd + static_cast<size_t>(start + k));
        if (footer) {
            for (size_t i = kFooterBegin; i < p.size(); ++i) shown.push_back(i);
        }
    }
    for (const size_t i : shown) {
        if (p[i].empty()) continue;
        const char tag = p[i][0];
        std::string t = p[i].substr(1);
        while (!t.empty() && t.back() == ' ') t.pop_back();
        if (i == 0) {
            while (!t.empty() && t.front() == ' ') t.erase(t.begin());
            if (!t.empty()) title = t;
            continue;
        }
        Element e = text(" " + (t.empty() ? std::string(" ") : t));
        if (tag == 'R') e = e | color(Color::Yellow);
        else if (tag == 'C') e = e | inverted | color(Color::Cyan);
        else if (tag == 'P') e = e | inverted | color(Color::Yellow);
        else if (tag == 'D') e = e | dim;
        lines.push_back(e);
    }
    if (p.empty()) lines.push_back(text(" aguardando o emulador...") | dim);
    return Dialog(title, std::move(lines));
}

Element HelpDialog() {
    Elements lines;
    for (const std::string &l : TuiModel::HelpLines()) lines.push_back(text(" " + l));
    return Dialog("Ajuda", std::move(lines));
}

// Monta o quadro inteiro da TUI a partir do modelo (usado pelo desenho normal e por --dump).
Element BuildView(App &app, const Dimensions &size) {
    std::lock_guard<std::mutex> lock(app.model_mutex());
    TuiModel &m = app.model();
    m.SetSize(size.dimx, size.dimy);

    // historico (toda a area entre a barra de menus e a linha de status)
    const int body_h = std::max(1, size.dimy - 3);
    const auto &log = m.log();
    const int total = static_cast<int>(log.size());
    const int end = std::max(0, total - m.log_scroll());
    const int begin = std::max(0, end - body_h);
    Elements body;
    for (int i = begin; i < end; ++i) {
        const std::string &l = log[static_cast<size_t>(i)];
        Element line = text(l);
        if (l.rfind("> ", 0) == 0) line = line | color(Color::Cyan);
        else if (l.rfind("erro", 0) == 0) line = line | color(Color::Red);
        body.push_back(line);
    }
    body.push_back(filler());

    const EmuStatus &st = m.status();
    std::string status;
    if (st.connected) {
        status = " Emulador conectado :" + std::to_string(st.port) + " | SCREEN " + std::to_string(st.screen) + " | quadro " +
                 std::to_string(st.frame) + (st.paused ? " | PAUSADO" : "") + (st.stopped ? " | PARADO (depurador)" : "") + " | slot " + std::to_string(m.slot());
    } else {
        status = " Emulador: nao conectado (menu Emulador > Iniciar)";
    }
    if (app.busy()) status += " | executando...";

    // linha de comando com cursor
    const std::string &in = m.input();
    const size_t cur = static_cast<size_t>(m.cursor());
    Element input_line = hbox({text(st.monitor_prompt.empty() ? std::string("> ") : st.monitor_prompt) | bold, text(in.substr(0, cur)),
                               text(cur < in.size() ? in.substr(cur, 1) : std::string(" ")) | inverted,
                               text(cur < in.size() ? in.substr(cur + 1) : std::string())});

    Element base = vbox({MenuBar(m), vbox(body) | flex, text(status) | inverted, input_line});
    switch (m.mode()) {
    case TuiModel::Mode::Menu: return dbox({base, Dropdown(m)});
    case TuiModel::Mode::Prompt: return dbox({base, PromptDialog(m)});
    case TuiModel::Mode::Browser: return dbox({base, BrowserDialog(m)});
    case TuiModel::Mode::Help: return dbox({base, HelpDialog()});
    case TuiModel::Mode::Debugger: return dbox({base, PanelDialog(m, size.dimy - 2)});
    case TuiModel::Mode::Man: return dbox({base, ManDialog(m, size)});
    case TuiModel::Mode::Mamute: return dbox({base, MamuteDialog(m, size)});
    default: return base;
    }
}

} // namespace

int RunTui(const std::vector<std::string> &args, const std::string &argv0) {
    std::error_code ec;
    App app(repl::BuildSession(argv0), std::filesystem::current_path(ec).string());
    for (size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == "--attach") app.Enqueue("emu attach " + args[i + 1]);
        if (args[i] == "--run") app.Enqueue(args[i + 1]); // executa uma linha ao abrir (testes / automacao)
    }
    {
        std::lock_guard<std::mutex> lock(app.model_mutex());
        app.model().AddLog(std::string("fwMSX TUI de menus - ") + FWMSX_COMPANY + ": " + FWMSX_CODENAME + ". F1 = ajuda, F10 = menus.");
        app.model().AddLog("Primeiro passo: menu Emulador > Iniciar MSX1/MSX2/MSX2+ (abre a janela do emulador).");
    }
    app.StartWorker();

    // --dump: desenha UM quadro (80x24), depois de dar tempo ao que foi pedido (--attach), e sai. Serve de
    // teste de fumaca sem terminal.
    for (const std::string &a : args) {
        if (a != "--dump") continue;
        std::this_thread::sleep_for(std::chrono::milliseconds(1800));
        Dimensions size{80, 24};
        Screen shot = Screen::Create(Dimension::Fixed(size.dimx), Dimension::Fixed(size.dimy));
        Render(shot, BuildView(app, size));
        std::cout << shot.ToString() << std::endl;
        app.StopWorker();
        return 0;
    }

    auto screen = ScreenInteractive::Fullscreen();
    screen.ForceHandleCtrlC(false);

    auto renderer = Renderer([&] { return BuildView(app, Terminal::Size()); });

    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Custom) {
            std::lock_guard<std::mutex> lock(app.model_mutex());
            if (app.model().quit_requested()) screen.Exit();
            return true;
        }
        std::lock_guard<std::mutex> lock(app.model_mutex());
        if (e.is_mouse()) {
            const Mouse &mouse = e.mouse();
            if (mouse.button == Mouse::Left && mouse.motion == Mouse::Pressed) app.model().Click(mouse.x, mouse.y);
            return true;
        }
        Key key;
        if (ToKey(e, key)) app.model().HandleKey(key);
        if (app.model().quit_requested()) screen.Exit();
        return true;
    });

    // Redesenha so' quando ha' o que mostrar: antes era 12 vezes por segundo SEMPRE, mesmo parado, e cada quadro cheio no
    // terminal (mais as teclas) disputava o terminal com a digitacao. Agora: a cada mudanca de estado (o estado do emulador
    // e' lido a cada 0,4 s), enquanto um comando roda (o aviso "executando...") e a cada tecla/clique, que ja' redesenham.
    std::atomic<bool> ui_running{true};
    std::thread refresher([&] {
        while (ui_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (app.TakeDirty() || app.busy()) screen.PostEvent(Event::Custom);
        }
    });
    screen.Loop(component);
    ui_running = false;
    refresher.join();
    app.StopWorker();
    return 0;
}

} // namespace tui
