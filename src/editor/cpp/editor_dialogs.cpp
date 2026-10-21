// fwMSX -- editor de textos: caixas de dialogo (widgets, teclado, desenho), dialogos do Norton Editor 2 e a ajuda F1.
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/editor-spec.md, secao 1b.3. Os textos de ajuda sao NOSSOS (o NE 2 e'
// da Symantec); so' a estrutura e as teclas seguem o original. Textos da interface em ingles, como os menus do NE.
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>

#include "../../common/version.h"
#include "editor_ui.h"

namespace ed {

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------------------------------------------
// configuracao em arquivo
// ---------------------------------------------------------------------------------------------------------------

bool SaveConfigFile(const std::string &path, const Settings &s, const UiSettings &u, std::string &err) {
    std::ofstream f(path, std::ios::trunc);
    if (!f) {
        err = "Cannot write " + path;
        return false;
    }
    f << "# fwEdit -- configuracao do editor (Options > Save configuration)\n";
    f << "tab_width=" << s.tab_width << "\n";
    f << "tab_key=" << static_cast<int>(s.tab_key) << "\n";
    f << "over_mode=" << static_cast<int>(s.over_mode) << "\n";
    f << "backup_char=" << static_cast<int>(static_cast<unsigned char>(s.backup_char)) << "\n";
    f << "delete_backup=" << s.delete_backup << "\n";
    f << "alt_selects_menu=" << s.alt_selects_menu << "\n";
    f << "auto_indent=" << s.auto_indent << "\n";
    f << "word_wrap=" << s.word_wrap << "\n";
    f << "line_length=" << s.line_length << "\n";
    f << "first_margin=" << s.first_margin << "\n";
    f << "left_margin=" << s.left_margin << "\n";
    f << "page_length=" << s.page_length << "\n";
    f << "top_margin=" << s.top_margin << "\n";
    f << "autosave_minutes=" << s.autosave_minutes << "\n";
    f << "syntax=" << s.syntax << "\n";
    f << "line_numbers=" << s.line_numbers << "\n";
    f << "auto_complete=" << s.auto_complete << "\n";
    f << "autosave_files=" << s.autosave_files << "\n";
    f << "scheme=" << u.scheme << "\n";
    f << "lines=" << u.lines << "\n";
    f << "scrollbar=" << u.scrollbar << "\n";
    f << "blinking_bracket=" << u.blinking_bracket << "\n";
    f << "zoom_effects=" << u.zoom_effects << "\n";
    f << "large_cursor=" << u.large_cursor << "\n";
    f << "mouse_button=" << u.mouse_button << "\n";
    f << "scroll_delay=" << u.scroll_delay << "\n";
    f << "adj_mouse_speed=" << u.adj_mouse_speed << "\n";
    f << "format_begin=" << u.format_begin << "\n";
    f << "print_left_margin=" << u.print_left_margin << "\n";
    f << "formfeed_between=" << u.formfeed_between << "\n";
    f << "formfeed_after=" << u.formfeed_after << "\n";
    return static_cast<bool>(f);
}

bool LoadConfigFile(const std::string &path, Settings &s, UiSettings &u) {
    std::ifstream f(path);
    if (!f) return false;
    std::string line;
    while (std::getline(f, line)) {
        const size_t eq = line.find('=');
        if (eq == std::string::npos || line[0] == '#') continue;
        const std::string k = line.substr(0, eq);
        const int v = std::atoi(line.c_str() + eq + 1);
        if (k == "tab_width") s.tab_width = std::max(1, std::min(v, 32));
        else if (k == "tab_key") s.tab_key = static_cast<TabKey>(std::max(0, std::min(v, 2)));
        else if (k == "over_mode") s.over_mode = static_cast<OverMode>(std::max(0, std::min(v, 2)));
        else if (k == "backup_char") s.backup_char = static_cast<char>(v);
        else if (k == "delete_backup") s.delete_backup = v != 0;
        else if (k == "alt_selects_menu") s.alt_selects_menu = v != 0;
        else if (k == "auto_indent") s.auto_indent = v != 0;
        else if (k == "word_wrap") s.word_wrap = v != 0;
        else if (k == "line_length") s.line_length = std::max(10, v);
        else if (k == "first_margin") s.first_margin = std::max(0, v);
        else if (k == "left_margin") s.left_margin = std::max(0, v);
        else if (k == "page_length") s.page_length = std::max(1, v);
        else if (k == "top_margin") s.top_margin = std::max(0, v);
        else if (k == "syntax") s.syntax = v != 0;
        else if (k == "line_numbers") s.line_numbers = v != 0;
        else if (k == "auto_complete") s.auto_complete = v != 0;
        else if (k == "autosave_minutes") s.autosave_minutes = std::max(0, std::min(v, 1440));
        else if (k == "autosave_files") s.autosave_files = v != 0;
        else if (k == "scheme") u.scheme = std::max(0, std::min(v, 4));
        else if (k == "lines") u.lines = v;
        else if (k == "scrollbar") u.scrollbar = v != 0;
        else if (k == "blinking_bracket") u.blinking_bracket = v != 0;
        else if (k == "zoom_effects") u.zoom_effects = v != 0;
        else if (k == "large_cursor") u.large_cursor = v != 0;
        else if (k == "mouse_button") u.mouse_button = v != 0;
        else if (k == "scroll_delay") u.scroll_delay = std::max(0, std::min(v, 4));
        else if (k == "adj_mouse_speed") u.adj_mouse_speed = v != 0;
        else if (k == "format_begin") u.format_begin = v != 0;
        else if (k == "print_left_margin") u.print_left_margin = std::max(0, v);
        else if (k == "formfeed_between") u.formfeed_between = v != 0;
        else if (k == "formfeed_after") u.formfeed_after = std::max(0, std::min(v, 2));
    }
    return true;
}

// ---------------------------------------------------------------------------------------------------------------
// DialogBox
// ---------------------------------------------------------------------------------------------------------------

Widget *DialogBox::Find(int id) {
    for (Widget &w : ws) {
        if (w.id == id && w.type != Widget::Label && w.type != Widget::Button && w.type != Widget::Radio) return &w;
    }
    return nullptr;
}

const Widget *DialogBox::Find(int id) const {
    for (const Widget &w : ws) {
        if (w.id == id && w.type != Widget::Label && w.type != Widget::Button && w.type != Widget::Radio) return &w;
    }
    return nullptr;
}

std::string DialogBox::Text(int id) const {
    const Widget *w = Find(id);
    return w ? w->text : std::string();
}

bool DialogBox::Checked(int id) const {
    const Widget *w = Find(id);
    return w && w->checked;
}

void DialogBox::BuildFields() {
    fields.clear();
    int max_group = -1;
    std::vector<bool> seen;
    for (size_t i = 0; i < ws.size(); ++i) {
        const Widget &w = ws[i];
        if (w.type == Widget::Label) continue;
        if (w.type == Widget::Radio) {
            max_group = std::max(max_group, w.group);
            if (static_cast<int>(seen.size()) <= w.group) seen.resize(static_cast<size_t>(w.group) + 1, false);
            if (seen[static_cast<size_t>(w.group)]) continue;
            seen[static_cast<size_t>(w.group)] = true;
        }
        fields.push_back(static_cast<int>(i));
    }
    if (static_cast<int>(group_sel.size()) <= max_group) group_sel.resize(static_cast<size_t>(max_group) + 1, 0);
    focus = 0;
    for (size_t i = 0; i < fields.size(); ++i) {
        if (ws[static_cast<size_t>(fields[i])].type != Widget::Button) {
            focus = static_cast<int>(i);
            break;
        }
    }
}

namespace {

int DefaultButton(const DialogBox &d) {
    for (const Widget &w : d.ws) {
        if (w.type == Widget::Button && w.is_default) return w.id;
    }
    return 0;
}

bool IsPrintable(const std::string &k) {
    return !k.empty() && (k.size() > 1 ? static_cast<unsigned char>(k[0]) >= 0x80 : (static_cast<unsigned char>(k[0]) >= 32 && static_cast<unsigned char>(k[0]) < 127));
}

} // namespace

int DialogBox::Key(const std::string &canon) {
    if (canon == "Esc") return kBtnCancel;
    if (fields.empty()) return canon == "Enter" ? kBtnOk : 0;
    const int n = static_cast<int>(fields.size());
    if (canon == "Tab") {
        focus = (focus + 1) % n;
        return 0;
    }
    if (canon == "S-Tab") {
        focus = (focus + n - 1) % n;
        return 0;
    }
    Widget &w = ws[static_cast<size_t>(fields[static_cast<size_t>(focus)])];
    if (canon == "Enter") {
        if (w.type == Widget::Button) return w.id;
        return DefaultButton(*this);
    }
    switch (w.type) {
    case Widget::Input: {
        if (canon == "Left") {
            if (w.cur > 0) --w.cur;
            w.fresh = false;
        } else if (canon == "Right") {
            if (w.cur < w.text.size()) ++w.cur;
            w.fresh = false;
        } else if (canon == "Home") {
            w.cur = 0;
            w.fresh = false;
        } else if (canon == "End") {
            w.cur = w.text.size();
            w.fresh = false;
        } else if (canon == "BS") {
            if (w.fresh) {
                w.text.clear();
                w.cur = 0;
            } else if (w.cur > 0) {
                w.text.erase(--w.cur, 1);
            }
            w.fresh = false;
        } else if (canon == "Del") {
            if (w.fresh) {
                w.text.clear();
                w.cur = 0;
            } else if (w.cur < w.text.size()) {
                w.text.erase(w.cur, 1);
            }
            w.fresh = false;
        } else if (canon == "C-u") {
            w.text.clear();
            w.cur = 0;
            w.fresh = false;
        } else if (IsPrintable(canon)) {
            if (w.numeric && !(canon.size() == 1 && std::isdigit(static_cast<unsigned char>(canon[0])))) return 0;
            if (w.fresh) {
                w.text.clear();
                w.cur = 0;
                w.fresh = false;
            }
            if (static_cast<int>(w.text.size()) < w.maxlen) w.text.insert(w.cur++, canon);
        }
        return 0;
    }
    case Widget::Radio: {
        const int g = w.group;
        int count = 0;
        for (const Widget &r : ws) {
            if (r.type == Widget::Radio && r.group == g) ++count;
        }
        int &sel = group_sel[static_cast<size_t>(g)];
        if (canon == "Up" || canon == "Left") sel = (sel + count - 1) % count;
        else if (canon == "Down" || canon == "Right") sel = (sel + 1) % count;
        return 0;
    }
    case Widget::Check:
        if (canon == " ") w.checked = !w.checked;
        return 0;
    case Widget::List: {
        const int cnt = static_cast<int>(w.items.size());
        const int vis = std::max(1, w.h - 2);
        if (canon == "Up") w.sel = std::max(0, w.sel - 1);
        else if (canon == "Down") w.sel = std::min(cnt - 1, w.sel + 1);
        else if (canon == "PgUp") w.sel = std::max(0, w.sel - vis);
        else if (canon == "PgDn") w.sel = std::min(cnt - 1, w.sel + vis);
        else if (canon == "Home") w.sel = 0;
        else if (canon == "End") w.sel = std::max(0, cnt - 1);
        if (w.sel < w.top) w.top = w.sel;
        if (w.sel >= w.top + vis) w.top = w.sel - vis + 1;
        return 0;
    }
    case Widget::Button:
        if (canon == " ") return w.id;
        break;
    default:
        break;
    }
    // letra inicial do botao
    if (canon.size() == 1 && std::isalpha(static_cast<unsigned char>(canon[0]))) {
        for (const Widget &b : ws) {
            if (b.type == Widget::Button && !b.text.empty() &&
                std::tolower(static_cast<unsigned char>(b.text[0])) == std::tolower(static_cast<unsigned char>(canon[0]))) {
                return b.id;
            }
        }
    }
    return 0;
}

namespace {

int ButtonWidth(const Widget &w) {
    return static_cast<int>(w.text.size()) + 4;
}

} // namespace

int DialogBox::ClickAt(int x, int y) {
    const int px = x - 1;
    const int py = y - 1;
    for (size_t i = 0; i < ws.size(); ++i) {
        Widget &w = ws[i];
        int fi = -1;
        for (size_t f = 0; f < fields.size(); ++f) {
            if (fields[f] == static_cast<int>(i)) fi = static_cast<int>(f);
        }
        if (w.type == Widget::Button) {
            if (py == w.y && px >= w.x && px < w.x + ButtonWidth(w)) {
                if (fi >= 0) focus = fi;
                return w.id;
            }
        } else if (w.type == Widget::Input) {
            if (py == w.y && px >= w.x && px <= w.x + w.w + 1) {
                if (fi >= 0) focus = fi;
                w.fresh = false;
                w.cur = std::min(w.text.size(), static_cast<size_t>(std::max(0, px - w.x - 1)));
                return 0;
            }
        } else if (w.type == Widget::Check) {
            if (py == w.y && px >= w.x && px < w.x + 4 + static_cast<int>(w.text.size())) {
                if (fi >= 0) focus = fi;
                w.checked = !w.checked;
                return 0;
            }
        } else if (w.type == Widget::Radio) {
            if (py == w.y && px >= w.x && px < w.x + 4 + static_cast<int>(w.text.size())) {
                group_sel[static_cast<size_t>(w.group)] = w.index;
                for (size_t f = 0; f < fields.size(); ++f) {
                    const Widget &fw = ws[static_cast<size_t>(fields[f])];
                    if (fw.type == Widget::Radio && fw.group == w.group) focus = static_cast<int>(f);
                }
                return 0;
            }
        } else if (w.type == Widget::List) {
            if (py > w.y && py < w.y + w.h - 1 && px >= w.x && px < w.x + w.w) {
                const int idx = w.top + (py - w.y - 1);
                if (idx < static_cast<int>(w.items.size())) w.sel = idx;
                if (fi >= 0) focus = fi;
                return 0;
            }
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// construcao dos dialogos
// ---------------------------------------------------------------------------------------------------------------

namespace {

Widget Label(int x, int y, const std::string &t) {
    Widget w;
    w.type = Widget::Label;
    w.x = x;
    w.y = y;
    w.text = t;
    return w;
}

Widget InputW(int id, int x, int y, int width, const std::string &value, bool fresh = true, bool numeric = false) {
    Widget w;
    w.type = Widget::Input;
    w.id = id;
    w.x = x;
    w.y = y;
    w.w = width;
    w.text = value;
    w.cur = value.size();
    w.fresh = fresh;
    w.numeric = numeric;
    w.maxlen = std::max(width, 64);
    return w;
}

Widget CheckW(int id, int x, int y, const std::string &t, bool on) {
    Widget w;
    w.type = Widget::Check;
    w.id = id;
    w.x = x;
    w.y = y;
    w.text = t;
    w.checked = on;
    return w;
}

Widget RadioW(int group, int index, int x, int y, const std::string &t) {
    Widget w;
    w.type = Widget::Radio;
    w.group = group;
    w.index = index;
    w.x = x;
    w.y = y;
    w.text = t;
    return w;
}

Widget ButtonW(int id, int x, int y, const std::string &t, bool def) {
    Widget w;
    w.type = Widget::Button;
    w.id = id;
    w.x = x;
    w.y = y;
    w.text = t;
    w.is_default = def;
    return w;
}

Widget ListW(int id, int x, int y, int width, int height, const std::string &title) {
    Widget w;
    w.type = Widget::List;
    w.id = id;
    w.x = x;
    w.y = y;
    w.w = width;
    w.h = height;
    w.text = title;
    return w;
}

// Dois botoes (OK / Cancel) centrados em y.
void OkCancel(DialogBox &d, int y) {
    const int total = 6 + 2 + 8;
    const int x = (d.w - 2 - total) / 2;
    d.ws.push_back(ButtonW(kBtnOk, x, y, "OK", true));
    d.ws.push_back(ButtonW(kBtnCancel, x + 10, y, "Cancel", false));
}

bool GlobMatch(const std::string &pat, const std::string &name, size_t pi = 0, size_t ni = 0) {
    while (pi < pat.size()) {
        if (pat[pi] == '*') {
            for (size_t k = ni; k <= name.size(); ++k) {
                if (GlobMatch(pat, name, pi + 1, k)) return true;
            }
            return false;
        }
        if (ni >= name.size()) return false;
        if (pat[pi] != '?' && std::tolower(static_cast<unsigned char>(pat[pi])) != std::tolower(static_cast<unsigned char>(name[ni]))) return false;
        ++pi;
        ++ni;
    }
    return ni == name.size();
}

std::string Describe(Cmd c) {
    switch (c) {
    case Cmd::New: return "Starts an empty untitled file.";
    case Cmd::Open: return "Loads an existing file.";
    case Cmd::Save: return "Saves the file being edited.";
    case Cmd::SaveAs: return "Saves the file under a new name.";
    case Cmd::Merge: return "Inserts another file at the cursor.";
    case Cmd::Window2: return "Opens a second window; with two open, moves to the other one.";
    case Cmd::CloseWindow: return "Closes the second window.";
    case Cmd::LoadMore: return "Loads more of a file that did not fit in memory.";
    case Cmd::WriteToCursor: return "Writes the text above the cursor to disk to make room.";
    case Cmd::TopOfFile: return "Reloads the top of the file after Write to cursor.";
    case Cmd::DosShell: return "Runs a shell and returns to the editor.";
    case Cmd::Exit: return "Leaves the editor (asks about unsaved files).";
    case Cmd::BlockCopy: return "Copies the marked block to the cursor.";
    case Cmd::BlockMove: return "Moves the marked block to the cursor.";
    case Cmd::BlockDelete: return "Deletes the marked block.";
    case Cmd::BlockSetMarker: return "Sets a block marker at the cursor (dragging the mouse also marks).";
    case Cmd::BlockRemoveMarkers: return "Removes the block markers.";
    case Cmd::BlockFindMarker: return "Moves the cursor to the next block marker.";
    case Cmd::BlockIndent: return "Indents each line of the block by an amount you give.";
    case Cmd::BlockUnindent: return "Removes leading spaces from each line of the block.";
    case Cmd::BlockOutput: return "Writes the block to a file.";
    case Cmd::BlockDeleteNoAsk: return "Deletes the block without asking.";
    case Cmd::BlockLine: return "Marks the current line as a block.";
    case Cmd::BlockToEol: return "Marks a block from the cursor to the end of the line.";
    case Cmd::ParagraphFormat: return "Formats the paragraph.";
    case Cmd::DocumentFormat: return "Formats all paragraphs.";
    case Cmd::CenterLine: return "Centers the line between the left margin and the line length.";
    case Cmd::TextSetup: return "Sets the line length and margins used to format and wrap.";
    case Cmd::WordWrap: return "Turns word wrap on or off (WW=On in the status line).";
    case Cmd::InsertPageBreaks: return "Inserts page breaks for printing.";
    case Cmd::RemovePageBreaks: return "Removes the page break lines.";
    case Cmd::ExpandTabs: return "Replaces tabs with spaces.";
    case Cmd::PutInTabs: return "Replaces leading spaces with tabs.";
    case Cmd::AutoIndent: return "Turns auto indent on or off (Auto in the status line).";
    case Cmd::Find: return "Searches for text.";
    case Cmd::Replace: return "Searches for text and replaces it.";
    case Cmd::ContinueFwd: return "Repeats the last search forward.";
    case Cmd::ContinueRev: return "Repeats the last search backward.";
    case Cmd::LineNumber: return "Moves the cursor to a line number.";
    case Cmd::MatchBracket: return "Moves to the bracket that matches the one under the cursor.";
    case Cmd::WindowDiff: return "Compares the two windows from the cursors.";
    case Cmd::TextInBlock: return "Searches for other copies of the text inside the block.";
    case Cmd::Print: return "Prints the whole file.";
    case Cmd::PrintBlock: return "Prints the marked block.";
    case Cmd::EjectPage: return "Advances the printer to the next page.";
    case Cmd::PrintSetup: return "Sets margins and page length for printing.";
    case Cmd::Information: return "Shows file name, counts, date and time and the character under the cursor.";
    case Cmd::Outline: return "Shows only the lines that start in column 1.";
    case Cmd::About: return "Shows information about the editor.";
    case Cmd::DisplaySetup: return "Colors, screen lines, scroll bar, tab width and cursor.";
    case Cmd::MouseSetup: return "Mouse button, scrolling speed.";
    case Cmd::EditorSetup: return "Insert/overstrike, Tab key, backup files, Alt key.";
    case Cmd::SaveConfig: return "Saves the current settings.";
    default: return "";
    }
}

std::string KeyLabel(const std::string &k) {
    std::string out;
    size_t i = 0;
    while (k.size() - i > 2 && k[i + 1] == '-') {
        out += k[i] == 'C' ? "Ctrl-" : (k[i] == 'A' ? "Alt-" : "Shift-");
        i += 2;
    }
    std::string base = k.substr(i);
    if (base.size() == 1) base[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(base[0])));
    return out + base;
}

std::string CmdWords(Cmd c) {
    switch (c) {
    case Cmd::Left: return "Moves the cursor left";
    case Cmd::Right: return "Moves the cursor right";
    case Cmd::Up: return "Moves the cursor up";
    case Cmd::Down: return "Moves the cursor down";
    case Cmd::WordLeft: return "Moves left one word";
    case Cmd::WordRight: return "Moves right one word";
    case Cmd::Home: return "Beginning of the line";
    case Cmd::End: return "End of the line";
    case Cmd::PgUp: return "Up one page";
    case Cmd::PgDn: return "Down one page";
    case Cmd::ScreenTop: return "Top of the screen";
    case Cmd::ScreenBottom: return "Bottom of the screen";
    case Cmd::FileTop: return "Beginning of the file";
    case Cmd::FileEnd: return "End of the file";
    case Cmd::ScrollUp: return "Scrolls the text up";
    case Cmd::ScrollDown: return "Scrolls the text down";
    case Cmd::DelLeft: return "Deletes the character on the left";
    case Cmd::DelRight: return "Deletes the character on the right";
    case Cmd::DelWordLeft: return "Deletes the word on the left";
    case Cmd::DelWordRight: return "Deletes the word on the right";
    case Cmd::DelToBol: return "Deletes to the beginning of the line";
    case Cmd::DelToEol: return "Deletes to the end of the line";
    case Cmd::KillLine: return "Deletes the whole line";
    case Cmd::Undelete: return "Restores what was just deleted";
    case Cmd::FlipCaseBack: return "Flips case to the line start";
    case Cmd::FlipCaseFwd: return "Flips case to the line end";
    case Cmd::Literal: return "Inserts the next key as typed";
    case Cmd::EnterIndent: return "New line with indentation";
    case Cmd::Unindent: return "Removes one level of indentation";
    case Cmd::ToggleInsert: return "Toggles Insert and Over";
    case Cmd::DividerUp: return "Moves the window divider up";
    case Cmd::DividerDown: return "Moves the window divider down";
    case Cmd::Find: return "Searches for text";
    case Cmd::Replace: return "Searches and replaces";
    case Cmd::ContinueFwd: return "Continues the search forward";
    case Cmd::ContinueRev: return "Continues the search backward";
    case Cmd::MatchBracket: return "Finds the matching bracket";
    case Cmd::Help: return "Help";
    case Cmd::Window2: return "2nd window";
    case Cmd::MenuOpen: return "Opens the menu bar";
    case Cmd::Undo: return "Undo";
    case Cmd::Redo: return "Redo";
    case Cmd::Copy: return "Copies the block to the clipboard";
    case Cmd::Cut: return "Cuts the block to the clipboard";
    case Cmd::Paste: return "Pastes the clipboard";
    case Cmd::SelLeft: case Cmd::SelRight: case Cmd::SelUp: case Cmd::SelDown: case Cmd::SelHome: case Cmd::SelEnd:
    case Cmd::SelPgUp: case Cmd::SelPgDn: case Cmd::SelWordLeft: case Cmd::SelWordRight:
        return "Extends the selection";
    default: return Info(c).name;
    }
}

std::vector<std::string> HelpLines(int topic) {
    std::vector<std::string> L;
    if (topic == 0) {
        L = {"fwEdit is a text editor in the style of the Norton",
             "Editor 2, made for MSX programming. Use it to write",
             "BASIC, Assembly and Pascal sources, batch files and",
             "plain text.",
             "",
             "Dialog boxes are controlled by buttons. A button is",
             "pushed with the mouse or with its first letter. The",
             "Cancel button is always pushed with Esc; the default",
             "button, shown in [ ], is pushed with Enter.",
             "",
             "Some dialogs have several fields: Tab and Shift-Tab",
             "move between them, and the space bar and the arrows",
             "change them.",
             "",
             "The top line is the menu bar. Hold Alt and press the",
             "first letter of a menu (or press F10), then the",
             "highlighted letter of the item. To leave the editor:",
             "Alt-F X.",
             "",
             "In view mode only the commands that do not change",
             "anything are available."};
    } else if (topic == 1) {
        L.push_back("Standard keys (Norton Editor 2):");
        for (const KeyBinding &b : Bindings()) {
            if (b.alt) continue;
            std::string key = KeyLabel(b.key);
            key.resize(std::max<size_t>(key.size(), 14), ' ');
            L.push_back(key + " " + CmdWords(b.cmd));
        }
        L.push_back("");
        L.push_back("Alternate keys:");
        for (const KeyBinding &b : Bindings()) {
            if (!b.alt) continue;
            std::string key = KeyLabel(b.key);
            key.resize(std::max<size_t>(key.size(), 14), ' ');
            L.push_back(key + " " + CmdWords(b.cmd));
        }
    } else if (topic >= 2 && topic <= 8) {
        const Menu &m = Menus()[static_cast<size_t>(topic - 2)];
        for (const MenuItem &it : m.items) {
            std::string name = it.label ? it.label : Info(it.cmd).name;
            std::string mn = std::string("Alt-") + static_cast<char>(std::toupper(m.hot)) + " " +
                             (it.hot == 0x7F ? std::string("Del") : std::string(1, static_cast<char>(std::toupper(it.hot ? it.hot : ' '))));
            if (!it.hot) mn = it.shortcut;
            name.resize(std::max<size_t>(name.size(), 22), ' ');
            L.push_back(name + " " + mn);
            const std::string d = Describe(it.cmd);
            if (!d.empty()) L.push_back("  " + d);
            L.push_back("");
        }
    }
    return L;
}

} // namespace

void EditorApp::BuildConfirm(const std::vector<std::string> &lines, const std::vector<int> &buttons, int default_button, Dialog kind) {
    dlg_ = DialogBox();
    dlg_.kind = kind;
    size_t width = 0;
    for (const std::string &l : lines) width = std::max(width, l.size());
    dlg_.w = std::max<int>(static_cast<int>(width) + 6, 36);
    dlg_.h = static_cast<int>(lines.size()) + 5;
    for (size_t i = 0; i < lines.size(); ++i) {
        const int x = (dlg_.w - 2 - static_cast<int>(lines[i].size())) / 2;
        dlg_.ws.push_back(Label(x, 1 + static_cast<int>(i), lines[i]));
    }
    int total = 0;
    for (const int b : buttons) {
        total += (b == kBtnSave ? 4 : b == kBtnDontSave ? 10 : b == kBtnCancel ? 6 : b == kBtnYes ? 3 : b == kBtnNo ? 2 : b == kBtnAll ? 3 : b == kBtnSkip ? 4 : b == kBtnTop ? 3 : b == kBtnCursor ? 6 : 2) + 4 + 2;
    }
    int x = std::max(1, (dlg_.w - 2 - total) / 2);
    for (const int b : buttons) {
        const char *label = b == kBtnSave ? "Save" : b == kBtnDontSave ? "Don't Save" : b == kBtnCancel ? "Cancel" : b == kBtnYes ? "Yes"
                          : b == kBtnNo ? "No" : b == kBtnAll ? "All" : b == kBtnSkip ? "Skip" : b == kBtnTop ? "Top" : b == kBtnCursor ? "Cursor" : "OK";
        dlg_.ws.push_back(ButtonW(b, x, 2 + static_cast<int>(lines.size()), label, b == default_button));
        x += static_cast<int>(std::string(label).size()) + 4 + 2;
    }
    dlg_.BuildFields();
    for (size_t i = 0; i < dlg_.fields.size(); ++i) {
        if (dlg_.ws[static_cast<size_t>(dlg_.fields[i])].is_default) dlg_.focus = static_cast<int>(i);
    }
}

void EditorApp::BuildPageBreakPrompt(Dialog kind, const std::string &text) {
    BuildConfirm({text}, {kBtnTop, kBtnCursor, kBtnCancel}, kBtnTop, kind);
}

void EditorApp::BuildSimpleInput(Dialog kind, const std::string &title, const std::string &label, const std::string &value, int width) {
    dlg_ = DialogBox();
    dlg_.kind = kind;
    dlg_.title = title;
    dlg_.w = std::max<int>(static_cast<int>(label.size()) + width + 8, 32);
    dlg_.h = 6;
    dlg_.ws.push_back(Label(2, 1, label));
    dlg_.ws.push_back(InputW(1, 3 + static_cast<int>(label.size()), 1, width, value, true, kind == Dialog::LineNumber || kind == Dialog::BlockIndent || kind == Dialog::BlockUnindent));
    OkCancel(dlg_, 3);
    dlg_.BuildFields();
}

void EditorApp::BuildFind(bool replace) {
    dlg_ = DialogBox();
    dlg_.kind = replace ? Dialog::Replace : Dialog::Find;
    dlg_.title = replace ? "Search & Replace" : "Search for text";
    dlg_.w = 62;
    dlg_.h = replace ? 11 : 10;
    int y = 1;
    dlg_.ws.push_back(Label(2, y, "Find:"));
    dlg_.ws.push_back(InputW(1, replace ? 11 : 8, y, 46, find_text_.empty() ? std::string() : find_text_));
    ++y;
    if (replace) {
        dlg_.ws.push_back(Label(2, y, "Replace:"));
        dlg_.ws.push_back(InputW(2, 11, y, 46, repl_text_));
        ++y;
    }
    ++y;
    dlg_.ws.push_back(CheckW(10, 11, y, "Upper/lower case must match", find_case_));
    dlg_.ws.push_back(CheckW(11, 11, y + 1, "Search backwards", find_back_));
    y += 3;
    dlg_.ws.push_back(ButtonW(kBtnOk, 10, y, "OK", true));
    dlg_.ws.push_back(ButtonW(kBtnHelp, 22, y, "F1=Help", false));
    dlg_.ws.push_back(ButtonW(kBtnCancel, 38, y, "Cancel", false));
    dlg_.BuildFields();
}

void EditorApp::BuildTextSetup() {
    dlg_ = DialogBox();
    dlg_.kind = Dialog::TextSetup;
    dlg_.title = "Text-layout setup";
    dlg_.w = 40;
    dlg_.h = 13;
    const Settings &s = Ed().settings();
    dlg_.ws.push_back(Label(2, 1, "Line Length:"));
    dlg_.ws.push_back(InputW(1, 19, 1, 4, std::to_string(s.line_length), true, true));
    dlg_.ws.push_back(Label(2, 2, "1st line margin:"));
    dlg_.ws.push_back(InputW(2, 19, 2, 4, std::to_string(s.first_margin), true, true));
    dlg_.ws.push_back(Label(2, 3, "Left Margin:"));
    dlg_.ws.push_back(InputW(3, 19, 3, 4, std::to_string(s.left_margin), true, true));
    dlg_.ws.push_back(Label(2, 5, "Begin formatting at:"));
    dlg_.ws.push_back(RadioW(0, 0, 4, 6, "Cursor"));
    dlg_.ws.push_back(RadioW(0, 1, 4, 7, "Top of paragraph"));
    OkCancel(dlg_, 9);
    dlg_.BuildFields();
    dlg_.group_sel[0] = ui_.format_begin;
}

void EditorApp::BuildPrintSetup() {
    dlg_ = DialogBox();
    dlg_.kind = Dialog::PrintSetup;
    dlg_.title = "Print setup";
    dlg_.w = 44;
    dlg_.h = 15;
    const Settings &s = Ed().settings();
    dlg_.ws.push_back(Label(2, 1, "Top margin:"));
    dlg_.ws.push_back(InputW(1, 17, 1, 4, std::to_string(s.top_margin), true, true));
    dlg_.ws.push_back(Label(2, 2, "Left margin:"));
    dlg_.ws.push_back(InputW(2, 17, 2, 4, std::to_string(ui_.print_left_margin), true, true));
    dlg_.ws.push_back(Label(2, 3, "Page length:"));
    dlg_.ws.push_back(InputW(3, 17, 3, 4, std::to_string(s.page_length), true, true));
    dlg_.ws.push_back(CheckW(10, 2, 5, "Formfeed between pages", ui_.formfeed_between));
    dlg_.ws.push_back(Label(2, 7, "Formfeed after printing:"));
    dlg_.ws.push_back(RadioW(0, 0, 4, 8, "No"));
    dlg_.ws.push_back(RadioW(0, 1, 4, 9, "One page"));
    dlg_.ws.push_back(RadioW(0, 2, 4, 10, "Two pages"));
    OkCancel(dlg_, 12);
    dlg_.BuildFields();
    dlg_.group_sel[0] = ui_.formfeed_after;
}

void EditorApp::BuildInfo() {
    const DocInfo d = Ed().Information();
    std::time_t t = std::time(nullptr);
    std::tm tmv{};
#ifdef _WIN32
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    char buf[64];
    std::snprintf(buf, sizeof buf, "%d:%02d%s    date: %02d-%02d-%04d", tmv.tm_hour % 12 == 0 ? 12 : tmv.tm_hour % 12, tmv.tm_min,
                  tmv.tm_hour < 12 ? "am" : "pm", tmv.tm_mon + 1, tmv.tm_mday, tmv.tm_year + 1900);
    std::error_code ec;
    const auto space = fs::space(cwd_.empty() ? fs::current_path() : fs::path(cwd_), ec);
    std::string chr = "(end of line)";
    if (d.char_at_cursor >= 0) {
        char c[64];
        const int ch = d.char_at_cursor;
        std::snprintf(c, sizeof c, "'%c' = %d = 0x%02X", ch >= 32 && ch < 127 ? ch : '.', ch, ch);
        chr = c;
    }
    dlg_ = DialogBox();
    dlg_.kind = Dialog::Information;
    dlg_.title = "Information display";
    dlg_.w = 62;
    dlg_.h = 18;
    dlg_.ws.push_back(Label(4, 1, "File: " + d.file));
    dlg_.ws.push_back(Label(4, 2, "Dir:  " + (cwd_.empty() ? fs::current_path().string() : cwd_)));
    dlg_.ws.push_back(Label(4, 4, std::string("Current time: ") + buf));
    dlg_.ws.push_back(Label(4, 6, "Characters in edit buffer: " + std::to_string(d.chars)));
    dlg_.ws.push_back(Label(4, 7, "Words in edit buffer:      " + std::to_string(d.words)));
    dlg_.ws.push_back(Label(4, 8, "Lines in edit buffer:      " + std::to_string(d.lines)));
    dlg_.ws.push_back(Label(4, 10, "Unread characters in input file: 0"));
    dlg_.ws.push_back(Label(4, 11, "Unused space on output drive: " + (ec ? std::string("?") : std::to_string(static_cast<unsigned long long>(space.available)))));
    dlg_.ws.push_back(Label(4, 13, "Character at cursor: " + chr));
    dlg_.ws.push_back(ButtonW(kBtnOk, 25, 15, "OK", true));
    dlg_.BuildFields();
}

void EditorApp::BuildAbout() {
    dlg_ = DialogBox();
    dlg_.kind = Dialog::About;
    dlg_.w = 46;
    dlg_.h = 11;
    const std::string ver = std::string("fwMSX ") + std::to_string(FWMSX_VERSION_MAJOR) + "." + std::to_string(FWMSX_VERSION_MINOR) + "." + std::to_string(FWMSX_VERSION_PATCH);
    const char *lines[] = {"fwEdit", "the text editor of fwMSX", "", "Version 1 (phase E2)", "Inspired by the Norton Editor 2"};
    int y = 1;
    for (const char *l : lines) {
        dlg_.ws.push_back(Label((dlg_.w - 2 - static_cast<int>(std::string(l).size())) / 2, y++, l));
    }
    dlg_.ws.push_back(Label((dlg_.w - 2 - static_cast<int>(ver.size())) / 2, y + 1, ver));
    dlg_.ws.push_back(ButtonW(kBtnOk, 17, 8, "OK", true));
    dlg_.BuildFields();
}

void EditorApp::BuildDisplay() {
    dlg_ = DialogBox();
    dlg_.kind = Dialog::DisplaySetup;
    dlg_.title = "Display setup";
    dlg_.w = 64;
    dlg_.h = 18;
    dlg_.ws.push_back(Label(2, 1, "Color:"));
    const char *colors[] = {"Color 1", "Color 2", "VGA Monochrome", "Monochrome", "LCD"};
    for (int i = 0; i < 5; ++i) dlg_.ws.push_back(RadioW(0, i, 4, 2 + i, colors[i]));
    dlg_.ws.push_back(Label(36, 1, "Display lines:"));
    const char *lines[] = {"25", "33/35", "40/43", "50", "Don't change"};
    for (int i = 0; i < 5; ++i) dlg_.ws.push_back(RadioW(1, i, 38, 2 + i, lines[i]));
    dlg_.ws.push_back(CheckW(10, 2, 8, "Display scrollbar", ui_.scrollbar));
    dlg_.ws.push_back(CheckW(11, 2, 9, "Blinking bracket", ui_.blinking_bracket));
    dlg_.ws.push_back(CheckW(12, 2, 10, "Zoom window effects", ui_.zoom_effects));
    dlg_.ws.push_back(CheckW(13, 2, 11, "Large cursor", ui_.large_cursor));
    dlg_.ws.push_back(CheckW(15, 2, 12, "Syntax highlighting", Ed().settings().syntax));
    dlg_.ws.push_back(CheckW(16, 2, 13, "Line numbers", Ed().settings().line_numbers));
    dlg_.ws.push_back(Label(36, 9, "Tab spacing:"));
    dlg_.ws.push_back(InputW(14, 49, 9, 3, std::to_string(Ed().settings().tab_width), true, true));
    OkCancel(dlg_, 14);
    dlg_.BuildFields();
    // mapeia o esquema atual para a ordem da lista (0 LCD, 1 Color 1, 2 Color 2, 3 VGA mono, 4 mono)
    static const int scheme_to_radio[] = {4, 0, 1, 2, 3};
    dlg_.group_sel[0] = scheme_to_radio[std::max(0, std::min(ui_.scheme, 4))];
    dlg_.group_sel[1] = ui_.lines == 25 ? 0 : ui_.lines == 35 ? 1 : ui_.lines == 43 ? 2 : ui_.lines == 50 ? 3 : 4;
}

void EditorApp::BuildLanguage() {
    dlg_ = DialogBox();
    dlg_.kind = Dialog::Language;
    dlg_.title = "Language";
    dlg_.w = 44;
    dlg_.h = 17;
    Widget list = ListW(2, 2, 1, 38, 12, "");
    list.items.push_back("(auto)");
    int sel = 0;
    const std::vector<const Lang *> &all = AllLangs();
    for (size_t i = 0; i < all.size(); ++i) {
        list.items.push_back(all[i]->Name());
        if (Ed().LanguageManual() && Ed().Language() == all[i]) sel = static_cast<int>(i) + 1;
    }
    list.sel = sel;
    list.top = std::max(0, sel - 5);
    dlg_.ws.push_back(list);
    OkCancel(dlg_, 14);
    dlg_.BuildFields();
}

// A lista das mensagens do montador (Build > Messages...): Enter vai para a linha da mensagem escolhida.
void EditorApp::BuildMessagesDialog() {
    dlg_ = DialogBox();
    dlg_.kind = Dialog::Errors;
    dlg_.title = "Build messages";
    dlg_.w = std::min(cols_ - 2, 76);
    dlg_.h = 17;
    Widget list = ListW(2, 2, 1, dlg_.w - 6, 12, "");
    for (const BuildMsg &m : build_msgs_) {
        std::string where = m.line > 0 ? (m.file.empty() ? std::string() : m.file + ":") + std::to_string(m.line) : std::string("-");
        list.items.push_back((m.warning ? "W " : "E ") + where + "  " + m.text);
    }
    list.sel = std::max(0, build_cur_);
    list.top = std::max(0, list.sel - 5);
    dlg_.ws.push_back(list);
    OkCancel(dlg_, 14);
    dlg_.BuildFields();
}

// O texto de uma pagina do manual (o `man`), so' para ler: a lista rola com as setas e PgUp/PgDn.
void EditorApp::BuildReference(const std::string &title, const std::string &text) {
    dlg_ = DialogBox();
    dlg_.kind = Dialog::Reference;
    dlg_.title = "Manual: " + title;
    dlg_.w = std::min(cols_ - 2, 78);
    dlg_.h = std::max(10, std::min(rows_ - 2, 24));
    Widget list = ListW(2, 2, 1, dlg_.w - 6, dlg_.h - 5, "");
    size_t pos = 0;
    while (pos <= text.size()) {
        size_t nl = text.find('\n', pos);
        if (nl == std::string::npos) nl = text.size();
        std::string line = text.substr(pos, nl - pos);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        list.items.push_back(line);
        pos = nl + 1;
    }
    dlg_.ws.push_back(list);
    dlg_.ws.push_back(ButtonW(kBtnOk, (dlg_.w - 2 - 6) / 2, dlg_.h - 3, "OK", true));
    dlg_.BuildFields();
}

// A configuracao do Basic Dignified (a mesma tela do msxIDE, so' com o que vale para o MSX e o fwMSX), em duas paginas. Os campos tem os nomes
// do badig.ini / msxbatoken.ini; OK grava o badig.cfg ao lado do executavel.
void EditorApp::BuildBadigSetup(int page) {
    badig_page_ = page;
    const badig::Options &o = badig_edit_;
    dlg_ = DialogBox();
    dlg_.kind = Dialog::BadigSetup;
    dlg_.title = page == 0 ? "Basic Dignified setup (1/2)" : "Basic Dignified setup (2/2)";
    dlg_.w = 68;
    dlg_.h = 20;
    if (page == 0) {
        dlg_.ws.push_back(Label(2, 1, "Line start:"));
        dlg_.ws.push_back(InputW(20, 14, 1, 6, std::to_string(o.line_start), true, true));
        dlg_.ws.push_back(Label(24, 1, "Step:"));
        dlg_.ws.push_back(InputW(21, 30, 1, 5, std::to_string(o.line_step), true, true));
        dlg_.ws.push_back(Label(40, 1, "Tab length:"));
        dlg_.ws.push_back(InputW(22, 52, 1, 3, std::to_string(o.tab_length), true, true));
        dlg_.ws.push_back(CheckW(23, 2, 3, "REM header (Converted with Basic Dignified)", o.rem_header));
        dlg_.ws.push_back(CheckW(24, 2, 4, "Strip all spaces", o.strip_spaces));
        dlg_.ws.push_back(CheckW(25, 2, 5, "Capitalize instructions", o.capitalise_all));
        dlg_.ws.push_back(CheckW(26, 2, 6, "Translate Unicode to MSX characters (UTF-8 source)", o.translate));
        dlg_.ws.push_back(Label(2, 8, "PRINT:"));
        dlg_.ws.push_back(RadioW(0, 0, 14, 8, "keep"));
        dlg_.ws.push_back(RadioW(0, 1, 24, 8, "always ?"));
        dlg_.ws.push_back(RadioW(0, 2, 40, 8, "always PRINT"));
        dlg_.ws.push_back(Label(2, 9, "THEN / GOTO:"));
        dlg_.ws.push_back(RadioW(1, 0, 14, 9, "keep"));
        dlg_.ws.push_back(RadioW(1, 1, 24, 9, "drop THEN before GOTO"));
        dlg_.ws.push_back(RadioW(1, 2, 14, 10, "drop GOTO after THEN/ELSE"));
        dlg_.ws.push_back(CheckW(27, 2, 12, "Tokenize: write the .bmx program (MSX tokenized BASIC)", o.tokenize));
        dlg_.ws.push_back(CheckW(28, 2, 13, "Delete the .amx after tokenizing", o.tk_del_ascii));
        dlg_.ws.push_back(Label(2, 14, "List file (.lmx), bytes per line (0 = none, max 32):"));
        dlg_.ws.push_back(InputW(29, 56, 14, 3, std::to_string(o.tk_list), true, true));
        dlg_.ws.push_back(ButtonW(kBtnNext, 8, 16, "Next >", false));
        dlg_.ws.push_back(ButtonW(kBtnOk, 24, 16, "OK", true));
        dlg_.ws.push_back(ButtonW(kBtnCancel, 34, 16, "Cancel", false));
        dlg_.BuildFields();
        dlg_.group_sel[0] = o.convert_print == "?" ? 1 : o.convert_print == "p" ? 2 : 0;
        dlg_.group_sel[1] = o.strip_then_goto == "t" ? 1 : o.strip_then_goto == "g" ? 2 : 0;
    } else {
        dlg_.ws.push_back(Label(2, 1, "Reports (saved next to the .amx, or printed):"));
        dlg_.ws.push_back(CheckW(30, 2, 3, "Label and function flow as a REM at each line end (-lbr)", o.label_report));
        dlg_.ws.push_back(CheckW(31, 2, 4, "Line correspondence: BASIC line - Dignified line (-lnr)", o.line_report));
        dlg_.ws.push_back(CheckW(32, 2, 5, "Long variable names and their short names (-var)", o.var_report));
        dlg_.ws.push_back(CheckW(33, 2, 6, "Lexer tokens (-lex)", o.lexer_report));
        dlg_.ws.push_back(CheckW(34, 2, 7, "Parser tokens (-par)", o.parser_report));
        dlg_.ws.push_back(CheckW(35, 2, 8, "Show the reports in the console instead of saving (-prr)", o.print_report));
        dlg_.ws.push_back(Label(2, 10, "Messages level (0 = silent .. 5 = every pass):"));
        dlg_.ws.push_back(InputW(36, 50, 10, 3, std::to_string(o.verbose), true, true));
        dlg_.ws.push_back(Label(2, 11, "Tokenizer messages level (0 .. 5):"));
        dlg_.ws.push_back(InputW(37, 50, 11, 3, std::to_string(o.tk_verbose), true, true));
        dlg_.ws.push_back(Label(2, 13, "F5 (Run on MSX) builds a disk with the .bmx and an AUTOEXEC.BAS, plus:"));
        dlg_.ws.push_back(Label(2, 14, "Files of the source folder (names or masks):"));
        dlg_.ws.push_back(InputW(38, 46, 14, 18, o.disk_files, false, false));
        dlg_.ws.push_back(CheckW(39, 2, 15, "NestorBASIC: put NBASIC.BIN on the disk (auto with nbasic.dmx)", o.nbasic));
        dlg_.ws.push_back(ButtonW(kBtnPrev, 8, 17, "< Prev", false));
        dlg_.ws.push_back(ButtonW(kBtnOk, 24, 17, "OK", true));
        dlg_.ws.push_back(ButtonW(kBtnCancel, 34, 17, "Cancel", false));
        dlg_.BuildFields();
    }
}

void EditorApp::ReadBadigPage(const DialogBox &d, int page) {
    auto num = [&](int id, int def, int lo, int hi) {
        const std::string t = d.Text(id);
        if (t.empty() || t.find_first_not_of("0123456789") != std::string::npos) return def;
        return std::max(lo, std::min(hi, std::atoi(t.c_str())));
    };
    badig::Options &o = badig_edit_;
    if (page == 0) {
        o.line_start = num(20, o.line_start, 1, 65535);
        o.line_step = num(21, o.line_step, 1, 9999);
        o.tab_length = num(22, o.tab_length, 1, 16);
        o.rem_header = d.Checked(23);
        o.strip_spaces = d.Checked(24);
        o.capitalise_all = d.Checked(25);
        o.translate = d.Checked(26);
        o.convert_print = d.Group(0) == 1 ? "?" : d.Group(0) == 2 ? "p" : "";
        o.strip_then_goto = d.Group(1) == 1 ? "t" : d.Group(1) == 2 ? "g" : "";
        o.tokenize = d.Checked(27);
        o.tk_del_ascii = d.Checked(28);
        o.tk_list = num(29, o.tk_list, 0, 32);
    } else {
        o.label_report = d.Checked(30);
        o.line_report = d.Checked(31);
        o.var_report = d.Checked(32);
        o.lexer_report = d.Checked(33);
        o.parser_report = d.Checked(34);
        o.print_report = d.Checked(35);
        o.verbose = num(36, o.verbose, 0, 5);
        o.tk_verbose = num(37, o.tk_verbose, 0, 5);
        o.disk_files = d.Text(38);
        o.nbasic = d.Checked(39);
    }
}

void EditorApp::BuildMouse() {
    dlg_ = DialogBox();
    dlg_.kind = Dialog::MouseSetup;
    dlg_.title = "Mouse setup";
    dlg_.w = 40;
    dlg_.h = 17;
    dlg_.ws.push_back(Label(2, 1, "Mouse button:"));
    dlg_.ws.push_back(RadioW(0, 0, 4, 2, "Use left button"));
    dlg_.ws.push_back(RadioW(0, 1, 4, 3, "Use right button"));
    dlg_.ws.push_back(Label(2, 5, "Mouse scrolling delay:"));
    const char *d[] = {"No delay", "12 milliseconds", "18 milliseconds", "26 milliseconds", "Sync to clock"};
    for (int i = 0; i < 5; ++i) dlg_.ws.push_back(RadioW(1, i, 4, 6 + i, d[i]));
    dlg_.ws.push_back(CheckW(10, 4, 12, "Adj mouse cursor speed", ui_.adj_mouse_speed));
    OkCancel(dlg_, 14);
    dlg_.BuildFields();
    dlg_.group_sel[0] = ui_.mouse_button;
    dlg_.group_sel[1] = ui_.scroll_delay;
}

void EditorApp::BuildEditorSetup() {
    dlg_ = DialogBox();
    dlg_.kind = Dialog::EditorSetup;
    dlg_.title = "Editor setup";
    dlg_.w = 62;
    dlg_.h = 18;
    const Settings &s = Ed().settings();
    dlg_.ws.push_back(Label(2, 1, "Insert / overstrike mode:"));
    dlg_.ws.push_back(RadioW(0, 0, 4, 2, "Start in insert"));
    dlg_.ws.push_back(RadioW(0, 1, 4, 3, "Start in overstrike"));
    dlg_.ws.push_back(RadioW(0, 2, 4, 4, "Never overstrike"));
    dlg_.ws.push_back(Label(2, 6, "Tab key:"));
    dlg_.ws.push_back(RadioW(1, 0, 4, 7, "Inserts a tab"));
    dlg_.ws.push_back(RadioW(1, 1, 4, 8, "Inserts spaces"));
    dlg_.ws.push_back(RadioW(1, 2, 4, 9, "Moves to tab stop"));
    dlg_.ws.push_back(Label(34, 1, "Backup file character:"));
    dlg_.ws.push_back(RadioW(2, 0, 36, 2, "{"));
    dlg_.ws.push_back(RadioW(2, 1, 36, 3, "~"));
    dlg_.ws.push_back(RadioW(2, 2, 36, 4, "@"));
    dlg_.ws.push_back(CheckW(10, 34, 6, "Delete backup files", s.delete_backup));
    dlg_.ws.push_back(CheckW(11, 34, 8, "Alt alone selects menu", s.alt_selects_menu));
    dlg_.ws.push_back(Label(2, 11, "Auto-save every (minutes, 0 = off):"));
    dlg_.ws.push_back(InputW(12, 40, 11, 4, std::to_string(s.autosave_minutes), true, true));
    dlg_.ws.push_back(CheckW(13, 2, 12, "Also overwrite files that have a name", s.autosave_files));
    dlg_.ws.push_back(CheckW(15, 34, 10, "Auto completion", s.auto_complete));
    OkCancel(dlg_, 14);
    dlg_.BuildFields();
    dlg_.group_sel[0] = static_cast<int>(s.over_mode);
    dlg_.group_sel[1] = static_cast<int>(s.tab_key);
    dlg_.group_sel[2] = s.backup_char == '~' ? 1 : s.backup_char == '@' ? 2 : 0;
}

void EditorApp::BuildSaveConfig() {
    dlg_ = DialogBox();
    dlg_.kind = Dialog::SaveConfig;
    dlg_.title = "Save configuration";
    dlg_.w = 44;
    dlg_.h = 10;
    dlg_.ws.push_back(Label(2, 1, "Update:"));
    dlg_.ws.push_back(RadioW(0, 0, 4, 2, "current config file"));
    dlg_.ws.push_back(RadioW(0, 1, 4, 3, "config for this directory"));
    OkCancel(dlg_, 6);
    dlg_.BuildFields();
}

void EditorApp::BuildHelp(int topic, int page) {
    static const char *titles[] = {"General help", "Help on keys", "File menu help", "Block menu help", "Text-layout menu help",
                                   "Search menu help", "Print menu help", "View menu help", "Option menu help"};
    topic = std::max(0, std::min(topic, 8));
    const std::vector<std::string> lines = HelpLines(topic);
    const int per_page = 17;
    const int pages = std::max(1, (static_cast<int>(lines.size()) + per_page - 1) / per_page);
    page = std::max(0, std::min(page, pages - 1));
    dlg_ = DialogBox();
    dlg_.kind = Dialog::Help;
    dlg_.title = titles[topic];
    dlg_.w = 62;
    dlg_.h = per_page + 5;
    dlg_.help_topic = topic;
    dlg_.page = page;
    for (int i = 0; i < per_page; ++i) {
        const size_t idx = static_cast<size_t>(page * per_page + i);
        if (idx < lines.size()) dlg_.ws.push_back(Label(2, 1 + i, lines[idx]));
    }
    const int by = per_page + 2;
    if (page + 1 < pages) dlg_.ws.push_back(ButtonW(kBtnNext, 6, by, "Next", true));
    if (page > 0) dlg_.ws.push_back(ButtonW(kBtnPrev, 22, by, "Previous", page + 1 >= pages));
    dlg_.ws.push_back(ButtonW(kBtnCancel, 42, by, "Cancel", page + 1 >= pages && page == 0));
    if (pages > 1) dlg_.footer = "More";
    dlg_.BuildFields();
    for (size_t i = 0; i < dlg_.fields.size(); ++i) {
        if (dlg_.ws[static_cast<size_t>(dlg_.fields[i])].is_default) dlg_.focus = static_cast<int>(i);
    }
}

// ---------------------------------------------------------------------------------------------------------------
// dialogo de arquivo
// ---------------------------------------------------------------------------------------------------------------

void EditorApp::BuildFileDialog(Dialog kind, const std::string &title) {
    dlg_ = DialogBox();
    dlg_.kind = kind;
    dlg_.title = title;
    dlg_.w = 62;
    dlg_.h = 18;
    dlg_.dir = cwd_.empty() ? fs::current_path().string() : cwd_;
    std::string mask = "*";
    const std::string &name = Ed().Name();
    if (!name.empty()) {
        const std::string ext = fs::path(name).extension().string();
        if (!ext.empty()) mask = "*" + ext;
    }
    dlg_.mask = mask;
    dlg_.ws.push_back(Label(2, 1, "File name:"));
    dlg_.ws.push_back(InputW(1, 13, 1, 42, mask));
    dlg_.ws.push_back(ListW(2, 2, 3, 38, 11, dlg_.dir));
    dlg_.ws.push_back(ListW(3, 41, 3, 18, 11, "Dirs:"));
    dlg_.ws.push_back(ButtonW(kBtnOk, 18, 15, "OK", true));
    dlg_.ws.push_back(ButtonW(kBtnCancel, 30, 15, "Cancel", false));
    dlg_.BuildFields();
    RefreshFileList();
}

void EditorApp::RefreshFileList() {
    Widget *files = dlg_.Find(2);
    Widget *dirs = dlg_.Find(3);
    if (!files || !dirs) return;
    files->items.clear();
    dirs->items.clear();
    files->text = dlg_.dir;
    std::error_code ec;
    std::vector<std::string> fl, dl;
    for (const fs::directory_entry &e : fs::directory_iterator(dlg_.dir, fs::directory_options::skip_permission_denied, ec)) {
        std::error_code ec2;
        const std::string n = e.path().filename().string();
        if (e.is_directory(ec2)) {
            dl.push_back("[" + n + "]");
        } else if (GlobMatch(dlg_.mask, n)) {
            fl.push_back(n);
        }
    }
    auto lower = [](std::string s) {
        for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    };
    std::sort(fl.begin(), fl.end(), [&](const std::string &a, const std::string &b) { return lower(a) < lower(b); });
    std::sort(dl.begin(), dl.end(), [&](const std::string &a, const std::string &b) { return lower(a) < lower(b); });
    files->items = fl;
    if (fs::path(dlg_.dir).has_parent_path() && fs::path(dlg_.dir).parent_path() != fs::path(dlg_.dir)) dirs->items.push_back("..");
    for (const std::string &d : dl) dirs->items.push_back(d);
#ifdef _WIN32
    for (char drive = 'A'; drive <= 'Z'; ++drive) {
        std::error_code ec3;
        if (fs::exists(std::string(1, drive) + ":\\", ec3)) dirs->items.push_back(std::string("[-") + drive + "-]");
    }
#endif
    files->sel = files->top = 0;
    dirs->sel = dirs->top = 0;
}

namespace {

std::string AbsPath(const std::string &dir, const std::string &name) {
    fs::path p(name);
    if (p.is_absolute()) return p.string();
    return (fs::path(dir) / p).string();
}

} // namespace

void EditorApp::FileDialogEnter() {
    Widget &name = *dlg_.Find(1);
    const int focused = dlg_.fields[static_cast<size_t>(dlg_.focus)];
    Widget &fw = dlg_.ws[static_cast<size_t>(focused)];
    // diretorio escolhido na lista
    if (fw.type == Widget::List && fw.id == 3 && !fw.items.empty()) {
        const std::string &it = fw.items[static_cast<size_t>(fw.sel)];
        std::error_code ec;
        fs::path np;
        if (it == "..") {
            np = fs::path(dlg_.dir).parent_path();
        } else if (it.size() > 4 && it[0] == '[' && it[1] == '-') {
            np = fs::path(std::string(1, it[2]) + ":\\");
        } else {
            np = fs::path(dlg_.dir) / it.substr(1, it.size() - 2);
        }
        if (fs::is_directory(np, ec)) {
            dlg_.dir = np.string();
            RefreshFileList();
        }
        return;
    }
    if (fw.type == Widget::List && fw.id == 2 && !fw.items.empty()) {
        name.text = fw.items[static_cast<size_t>(fw.sel)];
        name.cur = name.text.size();
    }
    const std::string text = name.text;
    if (text.empty()) return;
    if (text.find_first_of("*?") != std::string::npos) {
        // so' a mascara (pode trazer um diretorio na frente)
        const fs::path p(text);
        if (p.has_parent_path()) {
            std::error_code ec;
            const fs::path d = fs::path(AbsPath(dlg_.dir, p.parent_path().string()));
            if (fs::is_directory(d, ec)) dlg_.dir = d.string();
        }
        dlg_.mask = p.filename().string();
        RefreshFileList();
        return;
    }
    std::error_code ec;
    const std::string full = AbsPath(dlg_.dir, text);
    if (fs::is_directory(full, ec)) {
        dlg_.dir = full;
        RefreshFileList();
        name.text = dlg_.mask;
        name.cur = name.text.size();
        return;
    }
    // arquivo escolhido: quem abriu o dialogo decide o que fazer
    pending_path_ = full;
    DialogDone(kBtnOk + 100);
}

// ---------------------------------------------------------------------------------------------------------------
// desenho
// ---------------------------------------------------------------------------------------------------------------

namespace {

void PutCh(Grid &g, int x, int y, uint16_t ch, Style st) {
    if (x < 0 || y < 0 || x >= g.cols || y >= g.rows) return;
    g.at(x, y).ch = ch;
    g.at(x, y).style = st;
}

void PutStr(Grid &g, int x, int y, const std::string &s, Style st) {
    for (size_t i = 0; i < s.size(); ++i) PutCh(g, x + static_cast<int>(i), y, static_cast<unsigned char>(s[i]), st);
}

void Box(Grid &g, int x, int y, int w, int h, bool dbl, const std::string &title) {
    const uint16_t tl = dbl ? 0x2554 : 0x250C, tr = dbl ? 0x2557 : 0x2510, bl = dbl ? 0x255A : 0x2514, br = dbl ? 0x255D : 0x2518;
    const uint16_t hz = dbl ? 0x2550 : 0x2500, vt = dbl ? 0x2551 : 0x2502;
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) PutCh(g, x + i, y + j, ' ', kPanel);
    }
    for (int i = 1; i < w - 1; ++i) {
        PutCh(g, x + i, y, hz, kPanel);
        PutCh(g, x + i, y + h - 1, hz, kPanel);
    }
    for (int j = 1; j < h - 1; ++j) {
        PutCh(g, x, y + j, vt, kPanel);
        PutCh(g, x + w - 1, y + j, vt, kPanel);
    }
    PutCh(g, x, y, tl, kPanel);
    PutCh(g, x + w - 1, y, tr, kPanel);
    PutCh(g, x, y + h - 1, bl, kPanel);
    PutCh(g, x + w - 1, y + h - 1, br, kPanel);
    if (!title.empty()) {
        const std::string t = std::string(1, ' ') + title + " ";
        PutStr(g, x + (w - static_cast<int>(t.size())) / 2, y, t, kPanel);
    }
}

void Shadow(Grid &g, int x, int y, int w, int h) {
    for (int j = 1; j <= h; ++j) {
        PutCh(g, x + w, y + j, 0x2591, kShadow);
        PutCh(g, x + w + 1, y + j, 0x2591, kShadow);
    }
    for (int i = 2; i < w + 2; ++i) PutCh(g, x + i, y + h, 0x2591, kShadow);
}

} // namespace

void EditorApp::RenderDialog(Grid &g) const {
    const DialogBox &d = dlg_;
    const int bx = d.box_x >= 0 ? d.box_x : std::max(0, (cols_ - d.w) / 2);
    const int by = d.box_y >= 0 ? d.box_y : std::max(1, (rows_ - d.h) / 2);
    Box(g, bx, by, d.w, d.h, true, d.title);
    Shadow(g, bx, by, d.w, d.h);
    if (!d.footer.empty()) PutStr(g, bx + (d.w - static_cast<int>(d.footer.size()) - 2) / 2, by + d.h - 1, " " + d.footer + " ", kPanel);
    const int ox = bx + 1;
    const int oy = by + 1;
    const int focused = d.fields.empty() ? -1 : d.fields[static_cast<size_t>(d.focus)];
    for (size_t i = 0; i < d.ws.size(); ++i) {
        const Widget &w = d.ws[i];
        const bool has_focus = static_cast<int>(i) == focused || (w.type == Widget::Radio && focused >= 0 && d.ws[static_cast<size_t>(focused)].type == Widget::Radio &&
                                                                   d.ws[static_cast<size_t>(focused)].group == w.group);
        switch (w.type) {
        case Widget::Label:
            PutStr(g, ox + w.x, oy + w.y, w.text, kPanel);
            break;
        case Widget::Input: {
            PutCh(g, ox + w.x, oy + w.y, '[', kPanel);
            const size_t start = w.cur >= static_cast<size_t>(w.w) ? w.cur - static_cast<size_t>(w.w) + 1 : 0;
            for (int k = 0; k < w.w; ++k) {
                const size_t idx = start + static_cast<size_t>(k);
                const uint16_t ch = idx < w.text.size() ? static_cast<unsigned char>(w.text[idx]) : ' ';
                PutCh(g, ox + w.x + 1 + k, oy + w.y, ch, has_focus && w.fresh ? kPanelSel : kPanelField);
            }
            PutCh(g, ox + w.x + w.w + 1, oy + w.y, ']', kPanel);
            break;
        }
        case Widget::Radio: {
            const bool on = d.group_sel[static_cast<size_t>(w.group)] == w.index;
            PutStr(g, ox + w.x, oy + w.y, "( ) " + w.text, kPanel);
            if (on) PutCh(g, ox + w.x + 1, oy + w.y, 0x2022, kPanel);
            if (has_focus && on) PutStr(g, ox + w.x, oy + w.y, "(" + std::string(1, ' ') + ")", kPanelSel), PutCh(g, ox + w.x + 1, oy + w.y, 0x2022, kPanelSel);
            break;
        }
        case Widget::Check:
            PutStr(g, ox + w.x, oy + w.y, std::string(w.checked ? "[x] " : "[ ] ") + w.text, kPanel);
            if (has_focus) PutStr(g, ox + w.x, oy + w.y, w.checked ? "[x]" : "[ ]", kPanelSel);
            break;
        case Widget::Button: {
            const std::string face = (w.is_default ? "[ " : "  ") + w.text + (w.is_default ? " ]" : "  ");
            PutStr(g, ox + w.x, oy + w.y, face, has_focus ? kPanelHot : kPanelSel);
            break;
        }
        case Widget::List: {
            Box(g, ox + w.x, oy + w.y, w.w, w.h, false, "");
            if (!w.text.empty()) PutStr(g, ox + w.x + 2, oy + w.y, " " + w.text.substr(w.text.size() > static_cast<size_t>(w.w - 6) ? w.text.size() - static_cast<size_t>(w.w - 6) : 0) + " ", kPanel);
            const int vis = w.h - 2;
            for (int k = 0; k < vis; ++k) {
                const size_t idx = static_cast<size_t>(w.top + k);
                if (idx >= w.items.size()) break;
                const bool sel = static_cast<int>(idx) == w.sel;
                std::string it = w.items[idx].substr(0, static_cast<size_t>(w.w - 2));
                it.resize(static_cast<size_t>(w.w - 2), ' ');
                PutStr(g, ox + w.x + 1, oy + w.y + 1 + k, it, sel ? (has_focus ? kPanelSel : kPanelHot) : kPanel);
            }
            break;
        }
        }
    }
}

bool EditorApp::CursorScreen(int &x, int &y) const {
    if (HasDialog()) {
        const DialogBox &d = dlg_;
        if (d.fields.empty()) return false;
        const Widget &w = d.ws[static_cast<size_t>(d.fields[static_cast<size_t>(d.focus)])];
        if (w.type != Widget::Input) return false;
        const int bx = d.box_x >= 0 ? d.box_x : std::max(0, (cols_ - d.w) / 2);
        const int by = d.box_y >= 0 ? d.box_y : std::max(1, (rows_ - d.h) / 2);
        const size_t start = w.cur >= static_cast<size_t>(w.w) ? w.cur - static_cast<size_t>(w.w) + 1 : 0;
        x = bx + 1 + w.x + 1 + static_cast<int>(w.cur - start);
        y = by + 1 + w.y;
        return true;
    }
    if (bar_active_) return false;
    int cx = 0, cy = 0;
    if (!Ed().CursorScreen(cx, cy)) return false;
    x = cx;
    y = Rect(active_).y0 + (cy - 1);
    return true;
}

} // namespace ed
