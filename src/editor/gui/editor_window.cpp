// fwMSX -- editor de textos em janela. Ver editor_window.h.
#include "editor_window.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../cpp/editor_gui_input.h"
#include "../cpp/editor_keys.h"
#include "../cpp/editor_ui.h"

namespace ed {

namespace {

namespace fs = std::filesystem;

ImU32 Col(Rgb c, float a = 1.0f) {
    return IM_COL32(c.r, c.g, c.b, static_cast<int>(a * 255));
}

void AppendUtf8(std::string &out, uint32_t cp) {
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

// A fonte de largura fixa: Consolas / Courier New (Windows) ou DejaVu/Liberation Mono (Linux); sem nenhuma delas, a embutida.
// Os ornamentos (caixas, setas, sombras) NAO dependem da fonte: sao desenhados a mao.
void LoadMonoFont(ImGuiIO &io, float size) {
    static const char *kCandidates[] = {
        "C:\\Windows\\Fonts\\consola.ttf",
        "C:\\Windows\\Fonts\\cour.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/System/Library/Fonts/Menlo.ttc",
    };
    for (const char *path : kCandidates) {
        std::error_code ec;
        if (fs::exists(path, ec) && io.Fonts->AddFontFromFileTTF(path, size) != nullptr) return;
    }
    io.Fonts->AddFontDefault();
}

struct Metrics {
    float cw = 9;
    float ch = 18;
    float fs = 16;
};

GuiKey MapKey(ImGuiKey k) {
    if (k >= ImGuiKey_A && k <= ImGuiKey_Z) return static_cast<GuiKey>(static_cast<int>(GuiKey::A) + (k - ImGuiKey_A));
    if (k >= ImGuiKey_0 && k <= ImGuiKey_9) return static_cast<GuiKey>(static_cast<int>(GuiKey::D0) + (k - ImGuiKey_0));
    if (k >= ImGuiKey_F1 && k <= ImGuiKey_F12) return static_cast<GuiKey>(static_cast<int>(GuiKey::F1) + (k - ImGuiKey_F1));
    switch (k) {
    case ImGuiKey_LeftArrow: return GuiKey::Left;
    case ImGuiKey_RightArrow: return GuiKey::Right;
    case ImGuiKey_UpArrow: return GuiKey::Up;
    case ImGuiKey_DownArrow: return GuiKey::Down;
    case ImGuiKey_Home: return GuiKey::Home;
    case ImGuiKey_End: return GuiKey::End;
    case ImGuiKey_PageUp: return GuiKey::PgUp;
    case ImGuiKey_PageDown: return GuiKey::PgDn;
    case ImGuiKey_Insert: return GuiKey::Ins;
    case ImGuiKey_Delete: return GuiKey::Del;
    case ImGuiKey_Backspace: return GuiKey::Backspace;
    case ImGuiKey_Enter: case ImGuiKey_KeypadEnter: return GuiKey::Enter;
    case ImGuiKey_Tab: return GuiKey::Tab;
    case ImGuiKey_Escape: return GuiKey::Escape;
    case ImGuiKey_Space: return GuiKey::Space;
    case ImGuiKey_LeftBracket: return GuiKey::LBracket;
    case ImGuiKey_RightBracket: return GuiKey::RBracket;
    case ImGuiKey_Minus: return GuiKey::Minus;
    case ImGuiKey_Equal: return GuiKey::Equal;
    case ImGuiKey_Slash: return GuiKey::Slash;
    case ImGuiKey_Backslash: return GuiKey::Backslash;
    case ImGuiKey_Comma: return GuiKey::Comma;
    case ImGuiKey_Period: return GuiKey::Period;
    case ImGuiKey_Semicolon: return GuiKey::Semicolon;
    case ImGuiKey_Apostrophe: return GuiKey::Apostrophe;
    case ImGuiKey_GraveAccent: return GuiKey::Grave;
    default: return GuiKey::None;
    }
}

bool IsModifier(ImGuiKey k) {
    return k == ImGuiKey_LeftCtrl || k == ImGuiKey_RightCtrl || k == ImGuiKey_LeftShift || k == ImGuiKey_RightShift || k == ImGuiKey_LeftAlt ||
           k == ImGuiKey_RightAlt || k == ImGuiKey_LeftSuper || k == ImGuiKey_RightSuper || k == ImGuiKey_ReservedForModCtrl ||
           k == ImGuiKey_ReservedForModShift || k == ImGuiKey_ReservedForModAlt || k == ImGuiKey_ReservedForModSuper || k == ImGuiKey_Menu;
}

// --- desenho de ornamentos ----------------------------------------------------------------------------------------

void DrawBoxGlyph(ImDrawList *dl, uint16_t ch, float x0, float y0, float x1, float y1, ImU32 c) {
    const float cx = std::floor((x0 + x1) / 2), cy = std::floor((y0 + y1) / 2);
    const float d = 2.0f;
    auto hline = [&](float xa, float xb, float y) { dl->AddRectFilled(ImVec2(xa, y), ImVec2(xb, y + 1), c); };
    auto vline = [&](float ya, float yb, float x) { dl->AddRectFilled(ImVec2(x, ya), ImVec2(x + 1, yb), c); };
    switch (ch) {
    case 0x2500: hline(x0, x1, cy); break;
    case 0x2502: vline(y0, y1, cx); break;
    case 0x250C: hline(cx, x1, cy); vline(cy, y1, cx); break;
    case 0x2510: hline(x0, cx + 1, cy); vline(cy, y1, cx); break;
    case 0x2514: hline(cx, x1, cy); vline(y0, cy + 1, cx); break;
    case 0x2518: hline(x0, cx + 1, cy); vline(y0, cy + 1, cx); break;
    case 0x2550: hline(x0, x1, cy - d); hline(x0, x1, cy + d); break;
    case 0x2551: vline(y0, y1, cx - d); vline(y0, y1, cx + d); break;
    case 0x2554: // ╔
        hline(cx - d, x1, cy - d); vline(cy - d, y1, cx - d);
        hline(cx + d, x1, cy + d); vline(cy + d, y1, cx + d);
        break;
    case 0x2557: // ╗
        hline(x0, cx + d + 1, cy - d); vline(cy - d, y1, cx + d);
        hline(x0, cx - d + 1, cy + d); vline(cy + d, y1, cx - d);
        break;
    case 0x255A: // ╚
        hline(cx - d, x1, cy + d); vline(y0, cy + d + 1, cx - d);
        hline(cx + d, x1, cy - d); vline(y0, cy - d + 1, cx + d);
        break;
    case 0x255D: // ╝
        hline(x0, cx + d + 1, cy + d); vline(y0, cy + d + 1, cx + d);
        hline(x0, cx - d + 1, cy - d); vline(y0, cy - d + 1, cx - d);
        break;
    default: break;
    }
}

bool IsOrnament(uint16_t ch) {
    return (ch >= 0x2190 && ch <= 0x2193) || ch == 0x2022 || ch == 0x2588 || ch == 0x2591 || ch == 0x25AA || (ch >= 0x2500 && ch <= 0x255D);
}

void DrawOrnament(ImDrawList *dl, uint16_t ch, float x0, float y0, float x1, float y1, ImU32 c) {
    const float w = x1 - x0, h = y1 - y0;
    const float cx = (x0 + x1) / 2, cy = (y0 + y1) / 2;
    if (ch == 0x2588) {
        dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), c);
    } else if (ch == 0x2591) { // sombra pontilhada: 1 pixel em cada 4
        for (float y = y0; y < y1; y += 2) {
            for (float x = x0 + (static_cast<int>(y - y0) % 4 == 0 ? 0 : 1); x < x1; x += 2) dl->AddRectFilled(ImVec2(x, y), ImVec2(x + 1, y + 1), c);
        }
    } else if (ch == 0x2022) {
        dl->AddCircleFilled(ImVec2(cx, cy), std::max(2.0f, h * 0.18f), c, 12);
    } else if (ch == 0x25AA) {
        const float s = std::max(2.0f, h * 0.2f);
        dl->AddRectFilled(ImVec2(cx - s, cy - s), ImVec2(cx + s, cy + s), c);
    } else if (ch == 0x2191) {
        dl->AddTriangleFilled(ImVec2(cx, y0 + h * 0.2f), ImVec2(cx - w * 0.35f, y1 - h * 0.25f), ImVec2(cx + w * 0.35f, y1 - h * 0.25f), c);
    } else if (ch == 0x2193) {
        dl->AddTriangleFilled(ImVec2(cx, y1 - h * 0.2f), ImVec2(cx - w * 0.35f, y0 + h * 0.25f), ImVec2(cx + w * 0.35f, y0 + h * 0.25f), c);
    } else if (ch == 0x2190) {
        dl->AddTriangleFilled(ImVec2(x0 + w * 0.15f, cy), ImVec2(x1 - w * 0.2f, cy - h * 0.22f), ImVec2(x1 - w * 0.2f, cy + h * 0.22f), c);
    } else if (ch == 0x2192) {
        dl->AddTriangleFilled(ImVec2(x1 - w * 0.15f, cy), ImVec2(x0 + w * 0.2f, cy - h * 0.22f), ImVec2(x0 + w * 0.2f, cy + h * 0.22f), c);
    } else {
        DrawBoxGlyph(dl, ch, x0, y0, x1, y1, c);
    }
}

void DrawGrid(ImDrawList *dl, ImFont *font, const Metrics &m, const Grid &g, int scheme, float ox, float oy) {
    for (int y = 0; y < g.rows; ++y) {
        // fundos: um retangulo por trecho de mesmo estilo
        int x = 0;
        while (x < g.cols) {
            const uint8_t st = g.at(x, y).style;
            int x2 = x;
            while (x2 < g.cols && g.at(x2, y).style == st) ++x2;
            const PalRgb p = PaletteRgb(scheme, st);
            dl->AddRectFilled(ImVec2(ox + x * m.cw, oy + y * m.ch), ImVec2(ox + x2 * m.cw, oy + (y + 1) * m.ch), Col(p.bg));
            x = x2;
        }
        // caracteres
        x = 0;
        while (x < g.cols) {
            const Cell &c = g.at(x, y);
            const PalRgb p = PaletteRgb(scheme, c.style);
            const float px = ox + x * m.cw;
            const float py = oy + y * m.ch;
            if (IsOrnament(c.ch)) {
                DrawOrnament(dl, c.ch, px, py, px + m.cw, py + m.ch, Col(p.fg));
                ++x;
                continue;
            }
            // trecho de texto comum com o mesmo estilo
            std::string run;
            int x2 = x;
            while (x2 < g.cols && g.at(x2, y).style == c.style && !IsOrnament(g.at(x2, y).ch)) {
                uint16_t ch = g.at(x2, y).ch;
                if (ch < 32 || ch == 127 || (ch >= 0x80 && ch < 0xA0)) ch = '.';
                AppendUtf8(run, ch);
                ++x2;
            }
            // so' desenha se houver algo alem de espacos
            if (run.find_first_not_of(' ') != std::string::npos) {
                dl->AddText(font, m.fs, ImVec2(px, py + (m.ch - m.fs) / 2), Col(p.fg), run.c_str(), run.c_str() + run.size());
                if (p.bold) dl->AddText(font, m.fs, ImVec2(px + 1, py + (m.ch - m.fs) / 2), Col(p.fg), run.c_str(), run.c_str() + run.size());
                if (p.underline) dl->AddRectFilled(ImVec2(px, py + m.ch - 2), ImVec2(px + (x2 - x) * m.cw, py + m.ch - 1), Col(p.fg));
            }
            x = x2;
        }
    }
}

std::string NormalizeNewlines(const char *s) {
    std::string out;
    if (!s) return out;
    for (const char *p = s; *p; ++p) {
        if (*p == '\r') continue;
        out.push_back(*p);
    }
    return out;
}

void GlfwError(int code, const char *desc) {
    std::fprintf(stderr, "fwmsx editor: GLFW erro %d: %s\n", code, desc);
}

} // namespace

int RunEditorWindow(const std::vector<std::string> &args, const std::string &argv0, bool as_view) {
    EditorApp app;
    std::vector<std::string> files;
    std::string cfg;
    bool view = as_view;
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string &a = args[i];
        if (a == "--view") view = true;
        else if (a == "--config" && i + 1 < args.size()) cfg = args[++i];
        else if (a == "--keys" || a == "--size") ++i; // opcoes do --dump (so' terminal)
        else if (!a.empty() && a[0] != '-') files.push_back(a);
    }
    std::error_code ec;
    const fs::path exe = fs::absolute(fs::path(argv0), ec).parent_path();
    app.SetConfigPaths(cfg.empty() ? (exe / "fwedit.cfg").string() : cfg, (fs::current_path(ec) / "fwedit.cfg").string());
    app.SetStartDir(fs::current_path(ec).string());
    app.LoadConfig();
    std::string err;
    for (size_t i = 0; i < files.size() && i < 2; ++i) {
        const bool ok = i == 0 ? app.Open(files[i], err, view) : app.Open2(files[i], err, view);
        if (!ok) {
            if (view) {
                std::fprintf(stderr, "fwmsx --view: %s\n", err.c_str());
                return 1;
            }
            app.Win(app.Active()).NewFile();
            app.Win(app.Active()).SetName(files[i]);
        }
    }

    glfwSetErrorCallback(GlfwError);
    if (!glfwInit()) {
        std::fprintf(stderr, "fwmsx: falha ao inicializar o GLFW\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    int start_w = 960, start_h = 640;
    int wx = 0, wy = 0, ww = 0, wh = 0;
    glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &wx, &wy, &ww, &wh);
    if (ww > 0 && wh > 0) {
        start_w = std::min(start_w, ww * 9 / 10);
        start_h = std::min(start_h, wh * 8 / 10);
    }
    GLFWwindow *window = glfwCreateWindow(start_w, start_h, "fwEdit", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "fwmsx: falha ao criar a janela (GLFW)\n");
        glfwTerminate();
        return 1;
    }
    if (ww > 0) glfwSetWindowPos(window, wx + (ww - start_w) / 2, wy + 60);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    const float font_size = 18.0f;
    LoadMonoFont(io, font_size);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    Metrics m;
    bool metrics_ready = false;
    bool alt_down = false, alt_used = false;
    std::string last_clip;
    std::string last_title;
    bool mouse_down = false;
    int last_cx = -1, last_cy = -1;

    while (!app.QuitRequested()) {
        glfwWaitEventsTimeout(0.25); // acorda para piscar o cursor
        // fechar a janela: o mesmo caminho de Alt-F X (pergunta por arquivos alterados)
        if (glfwWindowShouldClose(window)) {
            glfwSetWindowShouldClose(window, GLFW_FALSE);
            app.RequestExit();
            if (app.QuitRequested()) break;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiIO &fio = ImGui::GetIO();
        ImFont *font = ImGui::GetFont();
        if (!metrics_ready || m.fs != font_size) {
            const ImVec2 sz = ImGui::CalcTextSize("M");
            m.fs = font_size;
            m.cw = std::max(1.0f, std::floor(sz.x));
            m.ch = std::ceil(font_size * 1.2f);
            metrics_ready = true;
        }

        int fbw = 0, fbh = 0;
        glfwGetWindowSize(window, &fbw, &fbh);
        app.SetSize(std::max(40, static_cast<int>(fbw / m.cw)), std::max(8, static_cast<int>(fbh / m.ch)));

        // ---- teclado ----
        const bool ctrl = fio.KeyCtrl, alt = fio.KeyAlt, shift = fio.KeyShift;
        const bool altgr = ctrl && alt; // AltGr no Windows aparece como Ctrl+Alt: o caractere vem pela fila de texto
        bool any_key = false;
        for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; ++k) {
            const ImGuiKey key = static_cast<ImGuiKey>(k);
            if (IsModifier(key) || !ImGui::IsKeyPressed(key, true)) continue;
            any_key = true;
            const GuiKey gk = MapKey(key);
            if (gk == GuiKey::None || altgr) continue;
            const std::string name = GuiKeyName(gk, ctrl, alt, shift);
            if (name.empty()) continue;
            if (name == "S-Ins") app.Ed().SetClipboard(NormalizeNewlines(ImGui::GetClipboardText()));
            app.Key(name);
        }
        if (any_key && alt) alt_used = true;
        const bool combo = (ctrl && !altgr) || (alt && !altgr); // Ctrl+letra / Alt+letra: ja' tratados como tecla acima
        if (!combo) {
            std::string text;
            for (int i = 0; i < fio.InputQueueCharacters.Size; ++i) {
                const ImWchar c = fio.InputQueueCharacters[i];
                if (c >= 32 && c != 127 && c < 256) text.push_back(static_cast<char>(c));
            }
            if (!text.empty()) app.Text(text);
        }
        // Alt sozinho (apertar e soltar sem outra tecla) abre a barra de menus -- "Alt alone selects menu" do NE
        if (alt && !alt_down) {
            alt_down = true;
            alt_used = false;
        } else if (!alt && alt_down) {
            alt_down = false;
            if (!alt_used && app.Ed().settings().alt_selects_menu && !app.HasDialog()) app.Key(app.MenuIndex() >= 0 ? "Esc" : "F10");
        }

        // ---- mouse ----
        const ImVec2 mp = fio.MousePos;
        const int cx = static_cast<int>(std::floor(mp.x / m.cw));
        const int cy = static_cast<int>(std::floor(mp.y / m.ch));
        const int btn = app.ui().mouse_button == 0 ? 0 : 1;
        if (ImGui::IsMouseClicked(btn)) {
            app.MouseDown(cx, cy);
            mouse_down = true;
            last_cx = cx;
            last_cy = cy;
            if (any_key) alt_used = true;
            alt_used = alt_used || alt;
        } else if (mouse_down && ImGui::IsMouseDown(btn)) {
            if (cx != last_cx || cy != last_cy) {
                app.MouseDrag(cx, cy);
                last_cx = cx;
                last_cy = cy;
            }
        } else if (mouse_down) {
            app.MouseUp(cx, cy);
            mouse_down = false;
        }
        if (fio.MouseWheel != 0.0f) app.Wheel(cx, cy, fio.MouseWheel > 0 ? -1 : 1);

        // ---- clipboard e titulo ----
        if (app.Ed().Clipboard() != last_clip) {
            last_clip = app.Ed().Clipboard();
            ImGui::SetClipboardText(last_clip.c_str());
        }
        std::string title = "fwEdit - " + (app.Ed().Name().empty() ? std::string("Untitled") : fs::path(app.Ed().Name()).filename().string());
        if (app.Ed().Modified()) title += " *";
        if (app.Ed().ReadOnly()) title += " [view]";
        if (title != last_title) {
            glfwSetWindowTitle(window, title.c_str());
            last_title = title;
        }

        // ---- desenho ----
        Grid g;
        app.Render(g);
        ImDrawList *dl = ImGui::GetBackgroundDrawList();
        DrawGrid(dl, font, m, g, app.ui().scheme, 0.0f, 0.0f);
        int curx = 0, cury = 0;
        if (app.CursorScreen(curx, cury) && static_cast<int>(glfwGetTime() * 2) % 2 == 0) {
            const float px = curx * m.cw, py = cury * m.ch;
            const PalRgb p = PaletteRgb(app.ui().scheme, g.at(curx, cury).style);
            const bool block = app.ui().large_cursor || !app.Ed().InsertMode();
            if (block) {
                dl->AddRectFilled(ImVec2(px, py), ImVec2(px + m.cw, py + m.ch), Col(p.fg, 0.7f));
            } else {
                dl->AddRectFilled(ImVec2(px, py), ImVec2(px + 2, py + m.ch), Col(p.fg));
            }
        }

        ImGui::Render();
        int dw = 0, dh = 0;
        glfwGetFramebufferSize(window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

} // namespace ed
