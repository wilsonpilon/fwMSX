// fwMSX -- o editor de textos como componente grafico (ImGui). Ver editor_panel.h.
#include "editor_panel.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>

#include "../cpp/editor_gui_input.h"
#include "../cpp/editor_i18n.h"

namespace ed::gui {

namespace {

namespace fs = std::filesystem;

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

bool IsModifierKey(ImGuiKey k) {
    return k == ImGuiKey_LeftCtrl || k == ImGuiKey_RightCtrl || k == ImGuiKey_LeftShift || k == ImGuiKey_RightShift || k == ImGuiKey_LeftAlt ||
           k == ImGuiKey_RightAlt || k == ImGuiKey_LeftSuper || k == ImGuiKey_RightSuper || k == ImGuiKey_ReservedForModCtrl ||
           k == ImGuiKey_ReservedForModShift || k == ImGuiKey_ReservedForModAlt || k == ImGuiKey_ReservedForModSuper || k == ImGuiKey_Menu;
}

std::string NormalizeNewlines(const char *s) {
    std::string out;
    if (!s) return out;
    for (const char *p = s; *p; ++p) {
        if (*p != '\r') out.push_back(*p);
    }
    return out;
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

// Linha com as tabulacoes expandidas e os caracteres de controle/altos trocados por algo desenhavel (UTF-8).
std::string DisplayLine(const std::string &s, int tab) {
    std::string out;
    int col = 0;
    for (const char ch : s) {
        const unsigned char c = static_cast<unsigned char>(ch);
        if (c == '\t') {
            const int n = tab - col % tab;
            out.append(static_cast<size_t>(n), ' ');
            col += n;
        } else {
            uint32_t cp = c;
            if (c < 32 || c == 127 || (c >= 0x80 && c < 0xA0)) cp = 0xB7; // ponto medio
            AppendUtf8(out, cp);
            ++col;
        }
    }
    return out;
}

ImU32 WithAlpha(ImU32 c, float a) {
    const int alpha = static_cast<int>(((c >> IM_COL32_A_SHIFT) & 0xFF) * a);
    return (c & ~IM_COL32_A_MASK) | (static_cast<ImU32>(alpha) << IM_COL32_A_SHIFT);
}

std::string MnemonicText(const Menu &m, const MenuItem &it) {
    if (it.shortcut && it.shortcut[0]) return it.shortcut;
    if (!m.hot || !it.hot) return {};
    std::string s = "Alt+";
    s += static_cast<char>(std::toupper(m.hot));
    s += ", ";
    s += it.hot == 0x7F ? std::string("Del") : std::string(1, static_cast<char>(std::toupper(it.hot)));
    return s;
}

} // namespace

ImFont *LoadMonoFont(ImGuiIO &io, float size) {
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
        if (fs::exists(path, ec)) {
            if (ImFont *f = io.Fonts->AddFontFromFileTTF(path, size)) return f;
        }
    }
    return io.Fonts->AddFontDefault();
}

EditorPanel::EditorPanel() {
    app_.SetGuiMode(true);
}

void EditorPanel::LoadFont(ImGuiIO &io, float size) {
    mono_size_ = size;
    mono_ = LoadMonoFont(io, size);
}

void EditorPanel::Configure(const std::string &exe_dir, const std::string &cwd) {
    app_.SetConfigPaths((fs::path(exe_dir) / "fwedit.cfg").string(), (fs::path(cwd) / "fwedit.cfg").string());
    app_.SetStartDir(cwd);
    app_.LoadConfig();
}

void EditorPanel::Tick() {
    if (app_.AutosaveTick(ImGui::GetTime())) blink_reset_ = ImGui::GetTime();
}

void EditorPanel::ShowOpenDialog() {
    request_focus_ = true;
    app_.RunMenu(0, 1); // Arquivo > Abrir...
}

// ---------------------------------------------------------------------------------------------------------------
// area de transferencia do sistema
// ---------------------------------------------------------------------------------------------------------------

void EditorPanel::SyncClipboardIn() {
    app_.Ed().SetClipboard(NormalizeNewlines(ImGui::GetClipboardText()));
}

void EditorPanel::SyncClipboardOut() {
    const std::string &c = app_.Ed().Clipboard();
    if (c != last_clip_) {
        last_clip_ = c;
        ImGui::SetClipboardText(c.c_str());
    }
}

void EditorPanel::Press(const std::string &name) {
    if (name == "S-Ins" || name == "C-v") SyncClipboardIn();
    app_.Key(name);
    SyncClipboardOut();
    blink_reset_ = ImGui::GetTime();
}

// ---------------------------------------------------------------------------------------------------------------
// teclado: o MESMO conjunto de atalhos do Norton Editor 2, mais as alternativas
// ---------------------------------------------------------------------------------------------------------------

void EditorPanel::HandleKeyboard() {
    ImGuiIO &io = ImGui::GetIO();
    const bool ctrl = io.KeyCtrl, alt = io.KeyAlt, shift = io.KeyShift;
    const bool altgr = ctrl && alt; // AltGr (teclados internacionais): o caractere vem pela fila de texto
    bool any_key = false;
    for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; ++k) {
        const ImGuiKey key = static_cast<ImGuiKey>(k);
        if (IsModifierKey(key) || !ImGui::IsKeyPressed(key, true)) continue;
        any_key = true;
        const GuiKey gk = MapKey(key);
        if (gk == GuiKey::None || altgr) continue;
        const std::string name = GuiKeyName(gk, ctrl, alt, shift);
        if (!name.empty()) Press(name);
    }
    if (any_key && alt) alt_used_ = true;
    if (!((ctrl && !altgr) || (alt && !altgr))) {
        std::string text;
        for (int i = 0; i < io.InputQueueCharacters.Size; ++i) {
            const ImWchar c = io.InputQueueCharacters[i];
            if (c >= 32 && c != 127 && c < 256) text.push_back(static_cast<char>(c));
        }
        if (!text.empty()) {
            app_.Text(text);
            blink_reset_ = ImGui::GetTime();
        }
    }
    // Alt sozinho (apertar e soltar sem outra tecla) abre a barra de menus: "Alt alone selects menu" do NE
    if (alt && !alt_down_) {
        alt_down_ = true;
        alt_used_ = false;
    } else if (!alt && alt_down_) {
        alt_down_ = false;
        if (!alt_used_ && app_.Ed().settings().alt_selects_menu && !app_.HasDialog()) Press(app_.MenuIndex() >= 0 ? "Esc" : "F10");
    }
}

// ---------------------------------------------------------------------------------------------------------------
// moldura: menus, abas, status
// ---------------------------------------------------------------------------------------------------------------

void EditorPanel::DrawMenuBar() {
    if (!ImGui::BeginMenuBar()) return;
    const std::vector<Menu> &menus = Menus();
    anchors_.assign(menus.size(), ImVec2(0, 0));
    bar_min_ = ImGui::GetCursorScreenPos();
    bar_max_ = ImVec2(bar_min_.x + ImGui::GetContentRegionAvail().x, bar_min_.y + ImGui::GetFrameHeight());
    for (int i = 0; i < static_cast<int>(menus.size()); ++i) {
        const std::string label = Tr(menus[static_cast<size_t>(i)].title);
        const bool open = app_.MenuIndex() == i;
        const ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::PushID(i);
        if (ImGui::Selectable(label.c_str(), open, ImGuiSelectableFlags_None, ImVec2(ImGui::CalcTextSize(label.c_str()).x + 16, 0))) {
            if (open) app_.CloseMenuNow();
            else app_.OpenMenuAt(i);
        }
        if (ImGui::IsItemHovered() && app_.MenuIndex() >= 0 && !open) app_.OpenMenuAt(i); // com um menu aberto, passar o mouse troca
        ImGui::PopID();
        anchors_[static_cast<size_t>(i)] = ImVec2(p.x, p.y + ImGui::GetFrameHeight());
        ImGui::SameLine(0, 0);
    }
    ImGui::EndMenuBar();
}

void EditorPanel::DrawMenuPopup() {
    // O menu aberto e' uma janela COMUM do ImGui (nao um popup): se o editor for escondido com ele aberto, nao sobra estado preso na pilha de popups.
    const int m = app_.MenuIndex();
    if (m < 0) {
        menu_popup_live_ = false;
        return;
    }
    if (static_cast<size_t>(m) < anchors_.size()) ImGui::SetNextWindowPos(anchors_[static_cast<size_t>(m)], ImGuiCond_Always);
    if (!menu_popup_live_) {
        ImGui::SetNextWindowFocus();
        menu_popup_live_ = true;
    }
    int run = -1;
    bool hovered = false;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    if (ImGui::Begin("##edmenu", nullptr, flags)) {
        const Menu &menu = Menus()[static_cast<size_t>(m)];
        float lw = 0, sw = 0;
        for (const MenuItem &it : menu.items) {
            if (it.hidden || !it.label) continue;
            lw = std::max(lw, ImGui::CalcTextSize(Tr(it.label).c_str()).x);
            sw = std::max(sw, ImGui::CalcTextSize(MnemonicText(menu, it).c_str()).x);
        }
        const float total = lw + sw + 56.0f;
        for (int idx = 0; idx < static_cast<int>(menu.items.size()); ++idx) {
            const MenuItem &it = menu.items[static_cast<size_t>(idx)];
            if (it.hidden || !it.label) continue;
            const bool enabled = app_.MenuItemEnabled(it);
            ImGui::PushID(idx);
            ImGui::BeginDisabled(!enabled);
            if (ImGui::Selectable(Tr(it.label).c_str(), idx == app_.MenuItemIndex(), ImGuiSelectableFlags_None, ImVec2(total, 0))) run = idx;
            if (ImGui::IsItemHovered() && enabled) app_.SetMenuItem(idx);
            const std::string sc = MnemonicText(menu, it);
            if (!sc.empty()) {
                ImGui::SameLine(total - ImGui::CalcTextSize(sc.c_str()).x - 4);
                ImGui::TextDisabled("%s", sc.c_str());
            }
            ImGui::EndDisabled();
            ImGui::PopID();
            if (it.separator_after) ImGui::Separator();
        }
        // AllowWhenBlockedByActiveItem: no frame do clique o item de menu ja' esta' ativo e, sem esta opcao, o ImGui diria
        // "nao esta' sobre a janela" -- e o menu fecharia ANTES de o clique virar um item executado.
        hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    }
    ImGui::End();
    ImGui::PopStyleVar();
    if (run >= 0) {
        app_.RunMenu(m, run);
        return;
    }
    // clique fora do menu e fora da barra de titulos fecha o menu (como um popup)
    if (ImGui::IsMouseClicked(0) && !hovered) {
        const ImVec2 mp = ImGui::GetIO().MousePos;
        const bool on_bar = mp.x >= bar_min_.x && mp.x <= bar_max_.x && mp.y >= bar_min_.y && mp.y <= bar_max_.y;
        if (!on_bar) app_.CloseMenuNow();
    }
}

void EditorPanel::DrawTabs() {
    const int n = app_.WindowCount();
    if (!ImGui::BeginTabBar("##edtabs", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_AutoSelectNewTabs)) return;
    if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) app_.NewTab();
    int to_close = -1;
    for (int i = 0; i < n; ++i) {
        const Editor &e = app_.Win(i);
        std::string label = e.Title(); // o arquivo, ou noname01, noname02...
        label += "###tab" + std::to_string(i);
        ImGuiTabItemFlags flags = ImGuiTabItemFlags_None;
        if (e.Modified()) flags |= ImGuiTabItemFlags_UnsavedDocument;
        if (i == app_.Active() && last_active_ != i) flags |= ImGuiTabItemFlags_SetSelected;
        bool open = true;
        if (ImGui::BeginTabItem(label.c_str(), &open, flags)) ImGui::EndTabItem();
        if (ImGui::IsItemClicked(0) && i != app_.Active()) app_.SetActive(i);
        if (!e.Name().empty() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", e.Name().c_str());
        if (!open) to_close = i;
    }
    last_active_ = app_.Active();
    ImGui::EndTabBar();
    if (to_close >= 0) app_.CloseTab(to_close);
}

void EditorPanel::DrawStatus() {
    const Editor &e = app_.Ed();
    ImGui::Separator();
    std::string left = "Lin " + std::to_string(e.Cursor().line + 1) + ", Col " + std::to_string(e.DisplayCol(e.Cursor()) + 1);
    ImGui::TextUnformatted(left.c_str());
    ImGui::SameLine(0, 24);
    ImGui::TextDisabled("%s", e.ReadOnly() ? "Somente leitura" : (e.InsertMode() ? "Insert" : "Sobrescrever"));
    if (e.settings().word_wrap) {
        ImGui::SameLine(0, 16);
        ImGui::TextDisabled("Quebra automatica");
    }
    if (e.settings().auto_indent) {
        ImGui::SameLine(0, 16);
        ImGui::TextDisabled("Auto-indent");
    }
    ImGui::SameLine(0, 16);
    ImGui::TextDisabled("%s", e.Crlf() ? "CRLF" : "LF");
    if (e.Modified()) {
        ImGui::SameLine(0, 16);
        ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram), "alterado");
    }
    if (!e.Message().empty()) {
        ImGui::SameLine(0, 24);
        ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_PlotLines), "%s", Tr(e.Message()).c_str());
    }
}

// ---------------------------------------------------------------------------------------------------------------
// o texto
// ---------------------------------------------------------------------------------------------------------------

int EditorPanel::MaxLen(int doc, const Editor &ed) {
    if (static_cast<size_t>(doc) >= len_cache_.size()) len_cache_.resize(static_cast<size_t>(doc) + 1);
    LenCache &c = len_cache_[static_cast<size_t>(doc)];
    if (c.rev != ed.Revision() || c.lines != ed.LineCount()) {
        int mx = 0;
        const int tab = std::max(1, ed.settings().tab_width);
        for (int i = 0; i < ed.LineCount(); ++i) {
            const std::string &s = ed.Line(i);
            int len = static_cast<int>(s.size());
            if (s.find('\t') != std::string::npos) len = ed.DisplayCol({i, static_cast<int>(s.size())});
            mx = std::max(mx, len);
        }
        (void)tab;
        c.rev = ed.Revision();
        c.lines = ed.LineCount();
        c.maxlen = mx;
    }
    return c.maxlen;
}

void EditorPanel::ContextMenu(Editor &ed) {
    if (!ImGui::BeginPopupContextWindow("##edctx")) return;
    const bool ro = ed.ReadOnly();
    if (ImGui::MenuItem("Recortar", "Shift+Del", false, !ro && ed.HasBlock())) { ed.Execute(Cmd::Cut); SyncClipboardOut(); }
    if (ImGui::MenuItem("Copiar", "Ctrl+Ins", false, ed.HasBlock())) { ed.Execute(Cmd::Copy); SyncClipboardOut(); }
    if (ImGui::MenuItem("Colar", "Shift+Ins", false, !ro)) { SyncClipboardIn(); ed.Execute(Cmd::Paste); }
    ImGui::Separator();
    if (ImGui::MenuItem("Desfazer", "Alt+Backspace", false, !ro)) ed.Execute(Cmd::Undo);
    if (ImGui::MenuItem("Refazer", "Alt+Shift+Backspace", false, !ro)) ed.Execute(Cmd::Redo);
    ImGui::Separator();
    if (ImGui::MenuItem("Selecionar tudo")) {
        const int last = ed.LineCount() - 1;
        ed.SelectBlock({0, 0}, {last, static_cast<int>(ed.Line(last).size())});
    }
    ImGui::EndPopup();
}

void EditorPanel::DrawPane(int pane, ImVec2 size) {
    const int doc = app_.PaneDoc(pane);
    Editor &ed = app_.Win(doc);
    PaneState &st = panes_[pane];
    ImGuiIO &io = ImGui::GetIO();
    const bool is_active = doc == app_.Active();

    if (mono_) ImGui::PushFont(mono_, mono_size_);
    ImFont *font = ImGui::GetFont();
    const float fs = ImGui::GetFontSize();
    const float cw = std::max(1.0f, ImGui::CalcTextSize("M").x);
    const float lh = std::ceil(fs * 1.35f);
    const int nlines = ed.LineCount();
    const int digits = std::max(3, static_cast<int>(std::to_string(nlines).size()));
    const float gutter = show_numbers_ ? (static_cast<float>(digits) + 2.0f) * cw : 8.0f;
    const int maxlen = MaxLen(doc, ed);

    const ImU32 bg = ImGui::GetColorU32(ImGuiCol_FrameBg);
    const ImU32 fg = ImGui::GetColorU32(ImGuiCol_Text);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    // o modelo rolou a vista (cursor, PgDn, busca...): a rolagem do ImGui acompanha
    if (ed.TopLine() != st.last_top || ed.LeftCol() != st.last_left) {
        ImGui::SetNextWindowScroll(ImVec2(static_cast<float>(ed.LeftCol()) * cw, static_cast<float>(ed.TopLine()) * lh));
    }
    ImGui::SetNextWindowContentSize(ImVec2(gutter + (static_cast<float>(maxlen) + 8.0f) * cw, static_cast<float>(nlines) * lh + size.y * 0.4f));
    const std::string child_id = "##edpane" + std::to_string(pane);
    if (ImGui::BeginChild(child_id.c_str(), size, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoNavInputs)) {
        const ImVec2 origin = ImGui::GetCursorScreenPos(); // ja' desconta a rolagem
        const ImVec2 wpos = ImGui::GetWindowPos();
        const ImVec2 wsize = ImGui::GetWindowSize();
        const float sb = ImGui::GetStyle().ScrollbarSize;
        const int rows = std::max(1, static_cast<int>((wsize.y - sb) / lh));
        const int cols = std::max(1, static_cast<int>((wsize.x - gutter - sb) / cw));
        const int first = std::max(0, static_cast<int>(ImGui::GetScrollY() / lh));
        ed.SetViewSize(cols, rows, false);
        ed.SetTopLine(first);
        ed.SetLeftCol(static_cast<int>(ImGui::GetScrollX() / cw));
        st.last_top = ed.TopLine();
        st.last_left = ed.LeftCol();

        ImDrawList *dl = ImGui::GetWindowDrawList();
        const int tab = std::max(1, ed.settings().tab_width);
        const int last = std::min(nlines, first + rows + 2);
        Pos ba, bb;
        const bool has_block = ed.BlockRange(ba, bb);
        const Pos cur = ed.Cursor();
        const ImU32 sel_col = ImGui::GetColorU32(ImGuiCol_TextSelectedBg);
        const ImU32 line_col = WithAlpha(ImGui::GetColorU32(ImGuiCol_Header), 0.35f);
        const int left_col = ed.LeftCol();

        for (int l = first; l < last; ++l) {
            const float y = origin.y + static_cast<float>(l) * lh;
            const std::string &s = ed.Line(l);
            if (l == cur.line && is_active) dl->AddRectFilled(ImVec2(wpos.x, y), ImVec2(wpos.x + wsize.x, y + lh), line_col);
            if (has_block && l >= ba.line && l <= bb.line) {
                const int from = l == ba.line ? ba.col : 0;
                const int to = l == bb.line ? bb.col : static_cast<int>(s.size());
                const float x1 = origin.x + gutter + static_cast<float>(ed.DisplayCol({l, from})) * cw;
                float x2 = origin.x + gutter + static_cast<float>(ed.DisplayCol({l, to})) * cw;
                if (l < bb.line) x2 += cw * 0.6f; // a quebra de linha tambem esta' no bloco
                dl->AddRectFilled(ImVec2(x1, y), ImVec2(x2, y + lh), sel_col);
            }
            const std::string shown = DisplayLine(s, tab);
            const int from_col = std::max(0, left_col - 1);
            if (static_cast<int>(shown.size()) > from_col) {
                // trechos de varios bytes (acentos) nao sao cortados: o recorte so' acontece em ASCII
                size_t b = static_cast<size_t>(from_col);
                while (b < shown.size() && (static_cast<unsigned char>(shown[b]) & 0xC0) == 0x80) ++b;
                std::string seg = shown.substr(b, static_cast<size_t>(cols) * 2 + 8);
                dl->AddText(font, fs, ImVec2(origin.x + gutter + static_cast<float>(b) * cw, y + (lh - fs) / 2), fg, seg.c_str(), seg.c_str() + seg.size());
            }
        }
        // parentese que casa com o que esta' a esquerda do cursor
        if (is_active && ed.settings().show_bracket && cur.col > 0) {
            const std::string &cl = ed.Line(cur.line);
            const char ch = cl[static_cast<size_t>(cur.col - 1)];
            Pos m;
            if ((ch == ')' || ch == ']' || ch == '}' || ch == '>') && ed.MatchOf({cur.line, cur.col - 1}, m)) {
                for (const Pos p : {Pos{cur.line, cur.col - 1}, m}) {
                    const float x = origin.x + gutter + static_cast<float>(ed.DisplayCol(p)) * cw;
                    const float y = origin.y + static_cast<float>(p.line) * lh;
                    dl->AddRect(ImVec2(x, y), ImVec2(x + cw, y + lh), ImGui::GetColorU32(ImGuiCol_PlotHistogram), 2.0f);
                }
            }
        }
        // cursor
        if (cur.line >= first && cur.line < last) {
            const float x = origin.x + gutter + static_cast<float>(ed.DisplayCol(cur)) * cw;
            const float y = origin.y + static_cast<float>(cur.line) * lh;
            const bool focus = is_active && wants_kb_ && app_.MenuIndex() < 0 && !app_.HasDialog();
            const bool on = std::fmod(ImGui::GetTime() - blink_reset_, 1.0) < 0.6;
            if (focus && on) {
                if (ed.InsertMode()) dl->AddRectFilled(ImVec2(x, y + 1), ImVec2(x + 2, y + lh - 1), fg);
                else dl->AddRectFilled(ImVec2(x, y + 1), ImVec2(x + cw, y + lh - 1), WithAlpha(fg, 0.55f));
            } else if (is_active && !focus) {
                dl->AddRect(ImVec2(x, y + 1), ImVec2(x + 2, y + lh - 1), WithAlpha(fg, 0.5f));
            }
        }
        // numeros de linha (fixos na esquerda)
        if (show_numbers_) {
            const ImU32 gbg = ImGui::GetColorU32(ImGuiCol_MenuBarBg);
            dl->AddRectFilled(ImVec2(wpos.x, wpos.y), ImVec2(wpos.x + gutter, wpos.y + wsize.y), gbg);
            for (int l = first; l < last; ++l) {
                char num[16];
                std::snprintf(num, sizeof num, "%d", l + 1);
                const float w = ImGui::CalcTextSize(num).x;
                const float y = origin.y + static_cast<float>(l) * lh + (lh - fs) / 2;
                dl->AddText(font, fs, ImVec2(wpos.x + gutter - w - cw, y), l == cur.line && is_active ? fg : ImGui::GetColorU32(ImGuiCol_TextDisabled), num);
            }
        }

        // mouse
        const ImVec2 mp = io.MousePos;
        const bool inside = ImGui::IsWindowHovered() && mp.x < wpos.x + wsize.x - sb && mp.y < wpos.y + wsize.y - sb;
        auto to_pos = [&](ImVec2 m) {
            const int line = static_cast<int>(std::floor((m.y - origin.y) / lh));
            const int dcol = static_cast<int>(std::floor((m.x - origin.x - gutter) / cw + 0.5f));
            return ed.PosAt(line, dcol);
        };
        if (inside && ImGui::IsMouseClicked(0)) {
            app_.SetActive(doc);
            blink_reset_ = ImGui::GetTime();
            wants_kb_ = true;
            const Pos p = to_pos(mp);
            const int clicks = io.MouseClickedCount[0];
            if (clicks == 2) {
                ed.SelectWord(p);
            } else if (clicks >= 3) {
                ed.SelectLine(p.line);
            } else {
                ed.SetCursor(p);
                ed.Execute(Cmd::BlockRemoveMarkers);
                st.drag = true;
                st.anchor = p;
            }
        }
        if (st.drag) {
            if (ImGui::IsMouseDown(0)) {
                const Pos p = to_pos(mp);
                ed.SelectBlock(st.anchor, p);
                ed.SetCursor(p);
                if (mp.y < wpos.y) ed.ScrollView(-1);
                else if (mp.y > wpos.y + wsize.y) ed.ScrollView(1);
            } else {
                st.drag = false;
            }
        }
        if (inside || ImGui::IsWindowHovered()) {
            if (inside) ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
        }
        ContextMenu(ed);
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    if (mono_) ImGui::PopFont();
}

void EditorPanel::DrawPanes(float height) {
    if (!app_.Split()) {
        DrawPane(0, ImVec2(0, height));
        return;
    }
    const float w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2;
    for (int p = 0; p < 2; ++p) {
        if (p) ImGui::SameLine();
        ImGui::BeginGroup();
        const Editor &e = app_.Win(app_.PaneDoc(p));
        const bool act = p == app_.ActivePane();
        std::string name = e.Title();
        if (e.Modified()) name += " *";
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(act ? ImGuiCol_Text : ImGuiCol_TextDisabled));
        ImGui::TextUnformatted(name.c_str());
        ImGui::PopStyleColor();
        if (ImGui::IsItemClicked(0)) app_.SetActive(app_.PaneDoc(p));
        DrawPane(p, ImVec2(w, height - ImGui::GetTextLineHeightWithSpacing()));
        ImGui::EndGroup();
    }
}

// ---------------------------------------------------------------------------------------------------------------
// dialogos: os widgets do `DialogBox` desenhados como widgets graficos
// ---------------------------------------------------------------------------------------------------------------

void EditorPanel::DrawDialogWidgets(DialogBox &d, float ux, float uy) {
    ImDrawList *dl = ImGui::GetWindowDrawList();
    const float pad_y = ImGui::GetStyle().FramePadding.y;
    const ImU32 accent = ImGui::GetColorU32(ImGuiCol_ButtonActive);
    auto field_of = [&](size_t wi) {
        const Widget &w = d.ws[wi];
        for (size_t f = 0; f < d.fields.size(); ++f) {
            const Widget &fw = d.ws[static_cast<size_t>(d.fields[f])];
            if (d.fields[f] == static_cast<int>(wi) || (w.type == Widget::Radio && fw.type == Widget::Radio && fw.group == w.group)) return static_cast<int>(f);
        }
        return -1;
    };
    for (size_t i = 0; i < d.ws.size(); ++i) {
        Widget &w = d.ws[i];
        const int fi = field_of(i);
        const bool focused = fi >= 0 && fi == d.focus;
        ImGui::PushID(static_cast<int>(i));
        const ImVec2 base(10 + static_cast<float>(w.x) * ux, 6 + static_cast<float>(w.y) * uy);
        switch (w.type) {
        case Widget::Label: {
            ImGui::SetCursorPos(ImVec2(base.x, base.y + pad_y));
            ImGui::TextUnformatted(Tr(w.text).c_str());
            break;
        }
        case Widget::Input: {
            ImGui::SetCursorPos(base);
            const ImVec2 p = ImGui::GetCursorScreenPos();
            const ImVec2 sz(static_cast<float>(w.w) * ux + 12, ImGui::GetFrameHeight());
            dl->AddRectFilled(p, ImVec2(p.x + sz.x, p.y + sz.y), ImGui::GetColorU32(ImGuiCol_FrameBg), 3.0f);
            dl->AddRect(p, ImVec2(p.x + sz.x, p.y + sz.y), focused ? accent : ImGui::GetColorU32(ImGuiCol_Border), 3.0f, 0, focused ? 2.0f : 1.0f);
            const float cx = ImGui::CalcTextSize(w.text.substr(0, w.cur).c_str()).x;
            const float shift = std::max(0.0f, cx - (sz.x - 16));
            dl->PushClipRect(p, ImVec2(p.x + sz.x, p.y + sz.y), true);
            if (w.fresh && focused && !w.text.empty()) {
                dl->AddRectFilled(ImVec2(p.x + 6, p.y + 2), ImVec2(p.x + 6 + ImGui::CalcTextSize(w.text.c_str()).x, p.y + sz.y - 2), ImGui::GetColorU32(ImGuiCol_TextSelectedBg));
            }
            dl->AddText(ImVec2(p.x + 6 - shift, p.y + pad_y), ImGui::GetColorU32(ImGuiCol_Text), w.text.c_str());
            if (focused && std::fmod(ImGui::GetTime(), 1.0) < 0.6) {
                dl->AddRectFilled(ImVec2(p.x + 6 + cx - shift, p.y + 3), ImVec2(p.x + 7 + cx - shift, p.y + sz.y - 3), ImGui::GetColorU32(ImGuiCol_Text));
            }
            dl->PopClipRect();
            if (ImGui::InvisibleButton("##in", sz)) {
                d.focus = fi;
                w.fresh = false;
                const float mx = ImGui::GetIO().MousePos.x - p.x - 6 + shift;
                size_t c = 0;
                while (c < w.text.size() && ImGui::CalcTextSize(w.text.substr(0, c + 1).c_str()).x < mx) ++c;
                w.cur = c;
            }
            break;
        }
        case Widget::Radio: {
            ImGui::SetCursorPos(base);
            const bool on = d.group_sel[static_cast<size_t>(w.group)] == w.index;
            if (ImGui::RadioButton(Tr(w.text).c_str(), on)) {
                d.group_sel[static_cast<size_t>(w.group)] = w.index;
                d.focus = fi;
            }
            if (focused && on) dl->AddRect(ImVec2(ImGui::GetItemRectMin().x - 2, ImGui::GetItemRectMin().y - 1), ImVec2(ImGui::GetItemRectMax().x + 2, ImGui::GetItemRectMax().y + 1), accent, 3.0f);
            break;
        }
        case Widget::Check: {
            ImGui::SetCursorPos(base);
            bool v = w.checked;
            if (ImGui::Checkbox(Tr(w.text).c_str(), &v)) {
                w.checked = v;
                d.focus = fi;
            }
            if (focused) dl->AddRect(ImVec2(ImGui::GetItemRectMin().x - 2, ImGui::GetItemRectMin().y - 1), ImVec2(ImGui::GetItemRectMax().x + 2, ImGui::GetItemRectMax().y + 1), accent, 3.0f);
            break;
        }
        case Widget::Button: {
            ImGui::SetCursorPos(base);
            const std::string label = Tr(w.text);
            const float bw = std::max(80.0f, ImGui::CalcTextSize(label.c_str()).x + 28);
            if (w.is_default) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            const bool pressed = ImGui::Button(label.c_str(), ImVec2(bw, 0));
            if (w.is_default) ImGui::PopStyleColor();
            if (focused) dl->AddRect(ImVec2(ImGui::GetItemRectMin().x - 2, ImGui::GetItemRectMin().y - 2), ImVec2(ImGui::GetItemRectMax().x + 2, ImGui::GetItemRectMax().y + 2), accent, 4.0f, 0, 2.0f);
            if (pressed) {
                app_.PressDialog(w.id);
                ImGui::PopID();
                return; // o dialogo pode ter sido trocado
            }
            break;
        }
        case Widget::List: {
            ImGui::SetCursorPos(base);
            if (!w.text.empty()) {
                ImGui::TextDisabled("%s", Tr(w.text).c_str());
                ImGui::SetCursorPos(ImVec2(base.x, base.y + ImGui::GetTextLineHeightWithSpacing()));
            }
            const float h = static_cast<float>(w.h) * uy - (w.text.empty() ? 0 : ImGui::GetTextLineHeightWithSpacing());
            if (ImGui::BeginChild("##list", ImVec2(static_cast<float>(w.w) * ux, h), ImGuiChildFlags_Borders)) {
                for (int k = 0; k < static_cast<int>(w.items.size()); ++k) {
                    const bool sel = k == w.sel;
                    if (ImGui::Selectable(w.items[static_cast<size_t>(k)].c_str(), sel, ImGuiSelectableFlags_AllowDoubleClick)) {
                        w.sel = k;
                        d.focus = fi;
                        if (ImGui::IsMouseDoubleClicked(0)) {
                            ImGui::EndChild();
                            ImGui::PopID();
                            app_.DialogEnter();
                            return;
                        }
                    }
                    if (sel && k != w.top) {
                        // a seta do teclado moveu a selecao: mantem-na visivel
                        ImGui::SetScrollHereY(0.5f);
                        w.top = k;
                    }
                }
            }
            ImGui::EndChild();
            if (focused) dl->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), accent, 3.0f, 0, 2.0f);
            break;
        }
        }
        ImGui::PopID();
    }
}

void EditorPanel::DrawDialog() {
    // A caixa de dialogo tambem e' uma janela COMUM (sem popup modal): e' o que evitava o travamento ao reabrir o editor depois de
    // fecha-lo com um dialogo aberto. O "modal" e' feito aqui: enquanto ha' dialogo o teclado e' dele e a janela fica na frente.
    DialogBox *d = app_.CurrentMutable();
    if (!d) {
        dlg_live_ = false;
        return;
    }
    const float ux = std::max(7.0f, ImGui::GetFontSize() * 0.55f);
    const float uy = ImGui::GetFrameHeightWithSpacing() + 2;
    last_dlg_title_ = d->title.empty() ? std::string("Editor") : Tr(d->title);
    // a altura vem do conteudo (o ultimo widget), nao da caixa de 25 linhas do terminal
    int rows = 1;
    for (const Widget &w : d->ws) rows = std::max(rows, w.y + (w.type == Widget::List ? w.h : 1) + 1);
    const ImVec2 sz(static_cast<float>(d->w) * ux + 24, static_cast<float>(rows) * uy + 46);
    ImGui::SetNextWindowSize(sz, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowFocus(); // sempre na frente do editor enquanto estiver aberto
    dlg_live_ = true;
    const std::string title = last_dlg_title_ + "###eddlg";
    if (ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove)) {
        if (!d->footer.empty()) {
            ImGui::SetCursorPos(ImVec2(static_cast<float>(d->w) * ux - 40, ImGui::GetWindowHeight() - 26));
            ImGui::TextDisabled("%s", "mais...");
        }
        DrawDialogWidgets(*d, ux, uy);
    }
    ImGui::End();
}

// ---------------------------------------------------------------------------------------------------------------
// o conjunto
// ---------------------------------------------------------------------------------------------------------------

void EditorPanel::DrawContents() {
    app_.EnsureUntitled();
    wants_kb_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) || NeedsAttention();
    if (wants_kb_) HandleKeyboard();

    DrawMenuBar();
    DrawTabs();
    // reserva a barra de estado: um separador, uma linha de texto e o respiro inferior da janela
    const float status_h = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y * 2 + ImGui::GetStyle().WindowPadding.y + 4;
    const float avail = ImGui::GetContentRegionAvail().y;
    DrawPanes(std::max(40.0f, avail - status_h));
    DrawStatus();
    DrawMenuPopup();
    DrawDialog();
}

void EditorPanel::DrawWindow(bool *p_open) {
    app_.SetEmbedded(true);
    ImGui::SetNextWindowSize(ImVec2(860, 580), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(80, 60), ImGuiCond_FirstUseEver);
    if (request_focus_) {
        ImGui::SetNextWindowFocus();
        request_focus_ = false;
    }
    bool open = true;
    const Editor &e = app_.Ed();
    std::string title = "Editor - " + e.Title();
    if (e.Modified()) title += " *";
    title += "###fwedit";
    if (ImGui::Begin(title.c_str(), &open, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse)) DrawContents();
    else wants_kb_ = NeedsAttention();
    ImGui::End();
    // O X da janela e "Arquivo > Sair" so' ESCONDEM o editor: os arquivos e o texto nao salvo ficam abertos para a proxima vez
    // (quem trabalha no MSX e anota ideias no editor so' salva no fim). Um dialogo ou menu aberto e' cancelado.
    if (!open || app_.TakeHideRequest() || app_.QuitRequested()) {
        app_.CancelDialog();
        app_.CloseMenuNow();
        app_.ClearQuit();
        *p_open = false;
        wants_kb_ = false;
    }
}

void EditorPanel::DrawFullscreen() {
    const ImGuiViewport *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (ImGui::Begin("##fwedit_full", nullptr, flags)) {
        if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !NeedsAttention()) ImGui::SetWindowFocus();
        DrawContents();
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
}

} // namespace ed::gui
