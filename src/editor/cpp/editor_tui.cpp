// fwMSX -- editor de textos na TUI (FTXUI). Ver editor_tui.h.
#include "editor_tui.h"
#include "editor_host.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <thread>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/terminal.hpp>

#include "editor_keys.h"
#include "editor_ui.h"

namespace ed {

namespace {

using namespace ftxui;

struct Pal {
    Color fg;
    Color bg;
    bool bold = false;
    bool underline = false;
};

// Cores do destaque de sintaxe: uma para o fundo preto (LCD/mono) e outra para o azul (Color 1/2). `style` e' um kSyn*St.
Pal SynPal(int scheme, uint8_t style) {
    const bool blue = scheme == 1 || scheme == 2;
    const Color bg = blue ? Color::Blue : Color::Black;
    switch (style) {
    case kSynKeywordSt: return {blue ? Color::YellowLight : Color::CyanLight, bg, true};
    case kSynKeyword2St: return {blue ? Color::CyanLight : Color::Cyan, bg};
    case kSynDirectiveSt: return {Color::MagentaLight, bg};
    case kSynCommentSt: return {blue ? Color::GreenLight : Color::GrayDark, bg};
    case kSynStringSt: return {blue ? Color::RedLight : Color::YellowLight, bg};
    case kSynNumberSt: return {blue ? Color::White : Color::GreenLight, bg, blue};
    case kSynLabelSt: return {blue ? Color::GreenLight : Color::YellowLight, bg, true};
    case kSynRegisterSt: return {blue ? Color::CyanLight : Color::BlueLight, bg};
    case kSynOperatorSt: return {Color::White, bg};
    case kSynLineNoSt: return {blue ? Color::GrayLight : Color::GrayDark, bg};
    case kSynVariableSt: return {Color::CyanLight, bg};
    case kSynBiosSt: return {Color::RedLight, bg, true};
    case kSynHeadingSt: return {Color::YellowLight, bg, true};
    case kSynErrorSt: return {Color::RedLight, bg, true, true};
    default: return {Color::White, bg};
    }
}

// Os esquemas de cor do NE 2 (Options > Display): 0 LCD, 1 Color 1 (azul), 2 Color 2 (ciano), 3 VGA mono, 4 mono.
Pal Palette(int scheme, uint8_t style) {
    if (style >= kSynKeywordSt && style < kStyleCount) return SynPal(scheme, style);
    const Color grey = Color::GrayLight;
    const Color dark = Color::GrayDark;
    const Color black = Color::Black;
    const Color white = Color::White;
    switch (scheme) {
    case 1: // azul
    case 2: { // ciano
        const bool cyan = scheme == 2;
        const Color panel = cyan ? Color::Cyan : grey;
        const Color hot = cyan ? Color::Yellow : Color::Red;
        switch (style) {
        case kText: return {white, Color::Blue};
        case kBar: case kTitleBar: return {black, panel};
        case kBarHot: return {hot, panel};
        case kBarSel: return {white, black};
        case kBlockSel: return {black, Color::Cyan};
        case kBlockMark: return {Color::Yellow, Color::Red, true};
        case kScrollTrack: return {Color::Cyan, Color::Blue};
        case kScrollThumb: case kScrollArrow: return {black, panel};
        case kPanel: return {black, panel};
        case kPanelHot: return {hot, panel, true};
        case kPanelSel: return {white, black};
        case kPanelOff: return {dark, panel};
        case kPanelField: return {black, panel, false, true};
        case kShadow: return {dark, black};
        case kBracket: return {black, Color::Yellow};
        default: return {white, Color::Blue};
        }
    }
    default: // LCD / monocromatico: texto claro no preto, barras e caixas cinza
        switch (style) {
        case kText: return {grey, black};
        case kBar: case kTitleBar: return {black, grey};
        case kBarHot: return {black, grey, true};
        case kBarSel: return {grey, black};
        case kBlockSel: return {black, grey};
        case kBlockMark: return {white, black, true};
        case kScrollTrack: return {dark, black};
        case kScrollThumb: case kScrollArrow: return {black, grey};
        case kPanel: return {black, grey};
        case kPanelHot: return {white, black};
        case kPanelSel: return {grey, black};
        case kPanelOff: return {dark, grey};
        case kPanelField: return {black, grey, false, true};
        case kShadow: return {dark, black};
        case kBracket: return {black, white};
        default: return {grey, black};
        }
    }
}

void AppendUtf8(std::string &out, uint16_t cp) {
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

uint16_t Visible(uint16_t ch) {
    if (ch < 32 || ch == 127 || (ch >= 0x80 && ch < 0xA0)) return '.';
    return ch;
}

std::string RowText(const Grid &g, int y) {
    std::string s;
    for (int x = 0; x < g.cols; ++x) AppendUtf8(s, Visible(g.at(x, y).ch));
    return s;
}

Element BuildView(const Grid &g, int scheme, bool have_cursor, int cx, int cy, bool block_cursor) {
    Elements rows;
    for (int y = 0; y < g.rows; ++y) {
        Elements segs;
        int x = 0;
        while (x < g.cols) {
            // o cursor e' uma celula sozinha, para o terminal por o cursor de verdade nela
            if (have_cursor && y == cy && x == cx) {
                std::string t;
                AppendUtf8(t, Visible(g.at(x, y).ch));
                const Pal p = Palette(scheme, g.at(x, y).style);
                Element e = text(t) | color(p.fg) | bgcolor(p.bg);
                segs.push_back(block_cursor ? (e | focusCursorBlock) : (e | focusCursorBar));
                ++x;
                continue;
            }
            const uint8_t st = g.at(x, y).style;
            std::string t;
            int x2 = x;
            while (x2 < g.cols && g.at(x2, y).style == st && !(have_cursor && y == cy && x2 == cx)) {
                AppendUtf8(t, Visible(g.at(x2, y).ch));
                ++x2;
            }
            const Pal p = Palette(scheme, st);
            Element e = text(t) | color(p.fg) | bgcolor(p.bg);
            if (p.bold) e = e | bold;
            if (p.underline) e = e | underlined;
            segs.push_back(e);
            x = x2;
        }
        rows.push_back(hbox(std::move(segs)));
    }
    return vbox(std::move(rows));
}

} // namespace

int RunEditorTui(const std::vector<std::string> &args, const std::string &argv0, bool as_view) {
    EditorApp app;
    ed::AttachManHelp(app);
    std::vector<std::string> files;
    std::string keys;
    std::string cfg;
    bool dump = false;
    int dump_cols = 80;
    int dump_rows = 25;
    bool view = as_view;
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string &a = args[i];
        if (a == "--view") view = true;
        else if (a == "--dump") dump = true;
        else if (a == "--keys" && i + 1 < args.size()) keys = args[++i];
        else if (a == "--config" && i + 1 < args.size()) cfg = args[++i];
        else if (a == "--size" && i + 1 < args.size()) {
            std::sscanf(args[++i].c_str(), "%dx%d", &dump_cols, &dump_rows);
        } else if (!a.empty() && a[0] != '-') files.push_back(a);
    }
    namespace fs = std::filesystem;
    std::error_code ec;
    const fs::path exe = fs::absolute(fs::path(argv0), ec).parent_path();
    app.SetConfigPaths(cfg.empty() ? (exe / "fwedit.cfg").string() : cfg, (fs::current_path(ec) / "fwedit.cfg").string());
    app.SetStartDir(fs::current_path(ec).string());
    app.LoadConfig();
    app.SetSize(dump_cols, dump_rows);

    // sem arquivos na linha de comando: volta a ultima sessao (texto nao salvo inclusive) e o autosave grava nela
    if (files.empty() && !dump) {
        app.SetSessionPath((exe / "fwedit.session").string());
        app.LoadSession(app.SessionPath());
    }
    std::string err;
    for (size_t i = 0; i < files.size() && i < 2; ++i) {
        const bool ok = i == 0 ? app.Open(files[i], err, view) : app.Open2(files[i], err, view);
        if (!ok) {
            // arquivo novo: abre vazio com o nome dado (o NE faz o mesmo) -- so' se nao for "view"
            if (view) {
                std::cerr << "fwmsx --view: " << err << std::endl;
                return 1;
            }
            app.Win(app.Active()).NewFile();
            app.Win(app.Active()).SetName(files[i]);
        }
    }

    if (dump) {
        std::istringstream in(keys);
        std::string k;
        while (in >> k) {
            if (k.size() > 5 && k.rfind("text:", 0) == 0) app.Text(k.substr(5));
            else app.Key(k);
        }
        Grid g;
        app.Render(g);
        for (int y = 0; y < g.rows; ++y) std::cout << RowText(g, y) << "\n";
        return 0;
    }

    auto screen = ScreenInteractive::Fullscreen();
    screen.ForceHandleCtrlC(false);

    auto renderer = Renderer([&] {
        const Dimensions size = Terminal::Size();
        app.SetSize(size.dimx, size.dimy);
        Grid g;
        app.Render(g);
        int cx = 0, cy = 0;
        const bool have = app.CursorScreen(cx, cy);
        const bool block = app.ui().large_cursor || !app.Ed().InsertMode();
        return BuildView(g, app.ui().scheme, have, cx, cy, block);
    });

    auto component = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Custom) { // o relogio do autosave (abaixo): o terminal so' redesenha quando ha' evento
            using namespace std::chrono;
            app.AutosaveTick(duration<double>(steady_clock::now().time_since_epoch()).count());
            return true;
        }
        if (e.is_mouse()) {
            const Mouse &m = e.mouse();
            const Mouse::Button want = app.ui().mouse_button == 0 ? Mouse::Left : Mouse::Right;
            if (m.button == Mouse::WheelUp) app.Wheel(m.x, m.y, -1);
            else if (m.button == Mouse::WheelDown) app.Wheel(m.x, m.y, 1);
            else if (m.button == want && m.motion == Mouse::Pressed) app.MouseDown(m.x, m.y);
            else if (m.button == want && m.motion == Mouse::Released) app.MouseUp(m.x, m.y);
            else if (m.button == want && m.motion == Mouse::Moved) app.MouseDrag(m.x, m.y);
            else if (m.motion == Mouse::Moved && m.button == Mouse::None) app.MouseDrag(m.x, m.y);
            if (app.QuitRequested()) screen.Exit();
            return true;
        }
        const std::string name = KeyNameFromInput(e.input());
        if (name.empty()) return false;
        if (IsTextKey(name)) app.Text(Utf8ToBytes(name));
        else app.Key(name);
        if (app.QuitRequested()) screen.Exit();
        return true;
    });

    std::atomic<bool> ticking{true};
    std::thread ticker([&] {
        while (ticking) {
            for (int i = 0; i < 10 && ticking; ++i) std::this_thread::sleep_for(std::chrono::seconds(1));
            if (ticking) screen.PostEvent(Event::Custom);
        }
    });
    screen.Loop(component);
    ticking = false;
    ticker.join();
    app.DiscardSession(); // saida limpa: o usuario ja' decidiu o que salvar
    return 0;
}

} // namespace ed
