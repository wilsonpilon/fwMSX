// fwMSX -- editor de textos: buffer, cursor, comandos, blocos, busca e desfazer. Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver doc/editor-spec.md. Arquivo e tela estao em editor_file.cpp e editor_render.cpp.
#include "editor.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cstring>
#include <unordered_map>

extern "C" {
#include "../c/ed_text.h"
}
#include "../asm/ed_indent.h"

namespace ed {

namespace {

struct KeySpec {
    bool ctrl = false;
    bool alt = false;
    bool shift = false;
    std::string base; // com a caixa original
};

KeySpec ParseKey(const std::string &name) {
    KeySpec k;
    size_t i = 0;
    while (name.size() - i > 2 && name[i + 1] == '-') {
        const char m = name[i];
        if (m == 'C') {
            k.ctrl = true;
        } else if (m == 'A') {
            k.alt = true;
        } else if (m == 'S') {
            k.shift = true;
        } else {
            break;
        }
        i += 2;
    }
    k.base = name.substr(i);
    return k;
}

Cmd LookupKey(const std::string &canon) {
    static const std::unordered_map<std::string, Cmd> table = [] {
        std::unordered_map<std::string, Cmd> m;
        for (const KeyBinding &b : Bindings()) m[CanonicalKey(b.key)] = b.cmd;
        return m;
    }();
    const auto it = table.find(canon);
    return it == table.end() ? Cmd::None : it->second;
}

bool IsDeleteCmd(Cmd c) {
    switch (c) {
    case Cmd::DelLeft: case Cmd::DelRight: case Cmd::DelWordLeft: case Cmd::DelWordRight: case Cmd::DelToBol:
    case Cmd::DelToEol: case Cmd::KillLine:
        return true;
    default:
        return false;
    }
}

bool IsPlainMove(Cmd c) {
    switch (c) {
    case Cmd::Left: case Cmd::Right: case Cmd::Up: case Cmd::Down: case Cmd::WordLeft: case Cmd::WordRight:
    case Cmd::Home: case Cmd::End: case Cmd::PgUp: case Cmd::PgDn: case Cmd::ScreenTop: case Cmd::ScreenBottom:
    case Cmd::FileTop: case Cmd::FileEnd: case Cmd::ScrollUp: case Cmd::ScrollDown:
        return true;
    default:
        return false;
    }
}

Pos EndOf(Pos at, const std::string &text) {
    Pos p = at;
    for (const char c : text) {
        if (c == '\n') {
            ++p.line;
            p.col = 0;
        } else {
            ++p.col;
        }
    }
    return p;
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------
// construcao e estado
// ---------------------------------------------------------------------------------------------------------------

Editor::Editor() {
    lines_.emplace_back();
}

void Editor::SetCursor(Pos p) {
    MoveTo(p);
}

Pos Editor::EndOfFile() const {
    const int last = LineCount() - 1;
    return {last, static_cast<int>(lines_[static_cast<size_t>(last)].size())};
}

int Editor::DisplayCol(Pos p) const {
    const std::string &s = lines_[static_cast<size_t>(p.line)];
    const int tw = std::max(1, set_.tab_width);
    int dc = 0;
    const int n = std::min<int>(p.col, static_cast<int>(s.size()));
    for (int i = 0; i < n; ++i) dc += s[static_cast<size_t>(i)] == '\t' ? tw - dc % tw : 1;
    return dc;
}

int Editor::ColFromDisplay(int line, int dcol) const {
    const std::string &s = lines_[static_cast<size_t>(line)];
    const int tw = std::max(1, set_.tab_width);
    int dc = 0;
    int i = 0;
    const int n = static_cast<int>(s.size());
    while (i < n) {
        const int w = s[static_cast<size_t>(i)] == '\t' ? tw - dc % tw : 1;
        if (dc + w > dcol) break;
        dc += w;
        ++i;
    }
    return i;
}

std::string Editor::ExpandedLine(const std::string &s) const {
    const int tw = std::max(1, set_.tab_width);
    std::string out;
    for (const char c : s) {
        if (c == '\t') {
            out.append(static_cast<size_t>(tw - static_cast<int>(out.size()) % tw), ' ');
        } else {
            out.push_back(c);
        }
    }
    return out;
}

std::string Editor::LeadingIndent(int line) const {
    const std::string &s = lines_[static_cast<size_t>(line)];
    const int n = ed_leading_ws(reinterpret_cast<const unsigned char *>(s.data()), static_cast<int>(s.size()));
    return s.substr(0, static_cast<size_t>(n));
}

bool Editor::BlockRange(Pos &a, Pos &b) const {
    if (!HasBlock()) return false;
    a = m1_ < m2_ ? m1_ : m2_;
    b = m1_ < m2_ ? m2_ : m1_;
    return a != b;
}

std::string Editor::BlockText() const {
    Pos a, b;
    if (!BlockRange(a, b)) return {};
    std::string out;
    for (int l = a.line; l <= b.line; ++l) {
        const std::string &s = lines_[static_cast<size_t>(l)];
        const int from = l == a.line ? a.col : 0;
        const int to = l == b.line ? b.col : static_cast<int>(s.size());
        out.append(s, static_cast<size_t>(from), static_cast<size_t>(to - from));
        if (l != b.line) out.push_back('\n');
    }
    return out;
}

bool Editor::ItemEnabled(const MenuItem &item) const {
    const CmdInfo &ci = Info(item.cmd);
    if (!ci.implemented) return false;
    if (read_only_ && ci.destructive) return false;
    return true;
}

// ---------------------------------------------------------------------------------------------------------------
// primitivas de edicao
// ---------------------------------------------------------------------------------------------------------------

Pos Editor::RawInsert(Pos p, const std::string &s) {
    std::string &line = lines_[static_cast<size_t>(p.line)];
    const size_t nl = s.find('\n');
    if (nl == std::string::npos) {
        line.insert(static_cast<size_t>(p.col), s);
        return {p.line, p.col + static_cast<int>(s.size())};
    }
    const std::string tail = line.substr(static_cast<size_t>(p.col));
    line.erase(static_cast<size_t>(p.col));
    size_t start = 0;
    line += s.substr(0, nl);
    start = nl + 1;
    int at = p.line;
    for (;;) {
        const size_t next = s.find('\n', start);
        ++at;
        if (next == std::string::npos) {
            const std::string last = s.substr(start);
            lines_.insert(lines_.begin() + at, last + tail);
            return {at, static_cast<int>(last.size())};
        }
        lines_.insert(lines_.begin() + at, s.substr(start, next - start));
        start = next + 1;
    }
}

std::string Editor::RawDelete(Pos a, Pos b) {
    std::string out;
    for (int l = a.line; l <= b.line; ++l) {
        const std::string &s = lines_[static_cast<size_t>(l)];
        const int from = l == a.line ? a.col : 0;
        const int to = l == b.line ? b.col : static_cast<int>(s.size());
        out.append(s, static_cast<size_t>(from), static_cast<size_t>(to - from));
        if (l != b.line) out.push_back('\n');
    }
    if (a.line == b.line) {
        lines_[static_cast<size_t>(a.line)].erase(static_cast<size_t>(a.col), static_cast<size_t>(b.col - a.col));
    } else {
        std::string joined = lines_[static_cast<size_t>(a.line)].substr(0, static_cast<size_t>(a.col));
        joined += lines_[static_cast<size_t>(b.line)].substr(static_cast<size_t>(b.col));
        lines_[static_cast<size_t>(a.line)] = joined;
        lines_.erase(lines_.begin() + a.line + 1, lines_.begin() + b.line + 1);
    }
    return out;
}

void Editor::AdjustForInsert(Pos p, Pos end, Pos &q) const {
    if (!(p < q)) return;
    if (q.line == p.line) {
        q = {end.line, end.col + (q.col - p.col)};
    } else {
        q.line += end.line - p.line;
    }
}

void Editor::AdjustForDelete(Pos a, Pos b, Pos &q) const {
    if (q <= a) return;
    if (!(q < b)) {
        if (q.line == b.line) {
            q = {a.line, a.col + (q.col - b.col)};
        } else {
            q.line -= b.line - a.line;
        }
    } else {
        q = a;
    }
}

Pos Editor::DoInsert(Pos p, const std::string &s) {
    if (s.empty()) return p;
    const Pos end = RawInsert(p, s);
    AdjustForInsert(p, end, m1_);
    AdjustForInsert(p, end, m2_);
    AdjustForInsert(p, end, sel_anchor_);
    for (Pos &q : snip_stops_) AdjustForInsert(p, end, q);
    if (!replaying_) {
        undo_.push_back({true, p, s, cur_, end, group_});
        redo_.clear();
    }
    Touch();
    return end;
}

std::string Editor::DoDelete(Pos a, Pos b) {
    if (!(a < b)) return {};
    const std::string text = RawDelete(a, b);
    AdjustForDelete(a, b, m1_);
    AdjustForDelete(a, b, m2_);
    AdjustForDelete(a, b, sel_anchor_);
    for (Pos &q : snip_stops_) AdjustForDelete(a, b, q);
    if (!replaying_) {
        undo_.push_back({false, a, text, cur_, a, group_});
        redo_.clear();
    }
    Touch();
    return text;
}

void Editor::BeginGroup(bool merge) {
    if (!merge) ++group_;
}

void Editor::DoUndo(bool redo) {
    snip_stops_.clear();
    comp_open_ = false;
    std::vector<Op> &from = redo ? redo_ : undo_;
    std::vector<Op> &to = redo ? undo_ : redo_;
    if (from.empty()) {
        message_ = redo ? "Nothing to redo" : "Nothing to undo";
        return;
    }
    const int g = from.back().group;
    std::vector<Op> batch;
    while (!from.empty() && from.back().group == g) {
        batch.push_back(from.back());
        from.pop_back();
    }
    // `batch` esta' do mais novo para o mais velho. Desfazer aplica nessa ordem; refazer (que le' de redo_,
    // onde as operacoes foram empilhadas ao desfazer) tambem: a pilha de refazer ja' guarda o mais velho no topo.
    replaying_ = true;
    for (const Op &op : batch) {
        const bool do_insert = redo ? op.insert : !op.insert;
        if (do_insert) {
            const Pos end = RawInsert(op.at, op.text);
            AdjustForInsert(op.at, end, m1_);
            AdjustForInsert(op.at, end, m2_);
        } else {
            const Pos end = EndOf(op.at, op.text);
            RawDelete(op.at, end);
            AdjustForDelete(op.at, end, m1_);
            AdjustForDelete(op.at, end, m2_);
        }
    }
    replaying_ = false;
    // o que foi desfeito vai para a outra pilha, com o mais velho no topo quando for refazer
    for (const Op &op : batch) to.push_back(op);
    const Op &anchor = batch.back(); // desfazer: a mais velha; refazer: a mais nova
    Pos c = redo ? anchor.cursor_after : anchor.cursor_before;
    c.line = std::min(c.line, LineCount() - 1);
    c.col = std::min<int>(c.col, static_cast<int>(lines_[static_cast<size_t>(c.line)].size()));
    cur_ = c;
    goal_dcol_ = DisplayCol(cur_);
    Reveal();
    Touch();
    last_typed_ = false;
}

// ---------------------------------------------------------------------------------------------------------------
// cursor
// ---------------------------------------------------------------------------------------------------------------

void Editor::MoveTo(Pos p, bool keep_goal) {
    p.line = std::max(0, std::min(p.line, LineCount() - 1));
    const int len = static_cast<int>(lines_[static_cast<size_t>(p.line)].size());
    p.col = std::max(0, std::min(p.col, len));
    cur_ = p;
    if (!keep_goal) goal_dcol_ = DisplayCol(cur_);
    Reveal();
}

void Editor::MoveVertical(int dy) {
    const int line = std::max(0, std::min(cur_.line + dy, LineCount() - 1));
    cur_.line = line;
    cur_.col = ColFromDisplay(line, goal_dcol_);
    Reveal();
}

void Editor::Reveal() {
    const int h = std::max(1, TextRows());
    if (cur_.line < top_) {
        top_ = cur_.line;
    } else if (cur_.line >= top_ + h) {
        top_ = cur_.line - h + 1;
    }
    top_ = std::max(0, top_);
    const int dc = DisplayCol(cur_);
    const int width = std::max(1, cols_ - 1 - Gutter());
    if (dc < left_) {
        left_ = dc;
    } else if (dc >= left_ + width) {
        left_ = dc - width + 1;
    }
    left_ = std::max(0, left_);
}

void Editor::ClearSelectionIfPlain() {
    if (sel_active_) {
        m1_set_ = m2_set_ = false;
        sel_active_ = false;
    }
}

void Editor::MoveSel(Cmd base) {
    if (!sel_active_) {
        sel_anchor_ = cur_;
        sel_active_ = true;
    }
    const Pos anchor = sel_anchor_;
    sel_active_ = false; // as chamadas abaixo passam por Execute: o movimento "plano" nao pode limpar o que estamos montando
    Execute(base);
    sel_anchor_ = anchor;
    m1_ = anchor < cur_ ? anchor : cur_;
    m2_ = anchor < cur_ ? cur_ : anchor;
    m1_set_ = m2_set_ = true;
    sel_active_ = true;
}

// ---------------------------------------------------------------------------------------------------------------
// entrada
// ---------------------------------------------------------------------------------------------------------------

bool Editor::InsertLiteral(const std::string &base, bool ctrl) {
    if (read_only_) return true;
    int byte = -1;
    if (base.size() == 1) {
        const int c = static_cast<unsigned char>(base[0]);
        byte = ctrl && std::isalpha(c) ? (std::toupper(c) - '@') : c;
    } else if (base == "Enter") {
        byte = 13;
    } else if (base == "Tab") {
        byte = 9;
    } else if (base == "Esc") {
        byte = 27;
    } else if (base == "BS") {
        byte = 8;
    }
    if (byte < 0 || byte == 10) return true;
    BeginGroup(false);
    last_typed_ = false;
    ClearSelectionIfPlain();
    cur_ = DoInsert(cur_, std::string(1, static_cast<char>(byte)));
    goal_dcol_ = DisplayCol(cur_);
    Reveal();
    return true;
}

bool Editor::Key(const std::string &name) {
    const std::string canon = CanonicalKey(name);
    if (comp_open_ && !canon.empty()) {
        if (CompletionKey(canon)) return true;
        // digitar e apagar continuam filtrando a lista; qualquer outra tecla fecha e age normalmente
        const bool filter = canon == "BS" || canon.size() == 1 || (canon.size() == 3 && canon.compare(0, 2, "S-") == 0);
        if (!filter) comp_open_ = false;
    }
    if (canon == "Esc") snip_stops_.clear();
    const bool r = KeyImpl(name);
    if (comp_open_ && canon == "BS") RefreshCompletion();
    return r;
}

bool Editor::KeyImpl(const std::string &name) {
    message_.clear();
    const std::string canon = CanonicalKey(name);
    if (canon.empty()) return false;
    const KeySpec ks = ParseKey(name);

    if (literal_pending_) {
        literal_pending_ = false;
        return InsertLiteral(ks.base, ks.ctrl);
    }
    if (menu_prefix_) {
        const char menu = menu_prefix_;
        menu_prefix_ = 0;
        if (canon == "Esc") return true;
        return DispatchMenuLetter(menu, canon);
    }
    if (!ks.ctrl && !ks.alt && ks.base.size() == 1) {
        std::string ch = ks.base;
        if (ks.shift && std::isalpha(static_cast<unsigned char>(ch[0]))) {
            ch[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(ch[0])));
        }
        return Type(ch);
    }

    const Cmd cmd = LookupKey(canon);
    if (cmd != Cmd::None) return Execute(cmd);

    if (ks.alt && !ks.ctrl && ks.base.size() == 1) {
        const char l = static_cast<char>(std::tolower(static_cast<unsigned char>(ks.base[0])));
        for (const Menu &m : Menus()) {
            if (m.hot == l) {
                menu_prefix_ = l;
                return true;
            }
        }
    }
    return false;
}

bool Editor::DispatchMenuLetter(char menu, const std::string &canon) {
    char letter = 0;
    if (canon == "Del") {
        letter = 0x7F;
    } else if (canon.size() == 1) {
        letter = canon[0];
    }
    if (!letter) return true;
    for (const Menu &m : Menus()) {
        if (m.hot != menu) continue;
        for (const MenuItem &it : m.items) {
            if (it.hot == letter) return Execute(it.cmd, it.arg);
        }
    }
    return true;
}

bool Editor::Type(const std::string &text) {
    message_.clear();
    if (text.empty()) return false;
    if (read_only_) return true; // modo leitura: a tecla nao faz nada
    undel_valid_ = false;
    found_here_ = false;
    BeginGroup(last_typed_ && cur_ == last_type_end_);
    ClearSelectionIfPlain();
    InsertChars(text);
    last_typed_ = true;
    last_type_end_ = cur_;
    AfterTyping(text);
    return true;
}

bool Editor::InsertChars(const std::string &s) {
    for (const char c : s) {
        if (!insert_ && cur_.col < static_cast<int>(lines_[static_cast<size_t>(cur_.line)].size())) {
            DoDelete(cur_, {cur_.line, cur_.col + 1});
        }
        const Pos start = cur_;
        cur_ = DoInsert(cur_, std::string(1, c));
        if (set_.word_wrap && c != ' ') {
            const std::string &line = lines_[static_cast<size_t>(start.line)];
            const int limit = set_.left_margin + set_.line_length;
            if (static_cast<int>(line.size()) > limit) {
                const size_t br = line.rfind(' ', static_cast<size_t>(start.col));
                if (br != std::string::npos && static_cast<int>(br) > set_.left_margin) {
                    const int after = static_cast<int>(br) + 1;
                    const std::string indent(static_cast<size_t>(set_.left_margin), ' ');
                    DoDelete({start.line, static_cast<int>(br)}, {start.line, after});
                    DoInsert({start.line, static_cast<int>(br)}, "\n" + indent);
                    cur_ = {start.line + 1, set_.left_margin + (start.col + 1 - after)};
                }
            }
        }
    }
    goal_dcol_ = DisplayCol(cur_);
    Reveal();
    return true;
}

void Editor::InsertNewline(bool indent) {
    std::string ind;
    if (indent) ind = LeadingIndent(cur_.line).substr(0, static_cast<size_t>(cur_.col));
    cur_ = DoInsert(cur_, "\n" + ind);
    goal_dcol_ = DisplayCol(cur_);
    Reveal();
}

void Editor::DeleteRangeCmd(Pos a, Pos b) {
    if (!(a < b)) return;
    const Pos before = cur_;
    const std::string text = DoDelete(a, b);
    cur_ = a;
    undel_valid_ = true;
    undel_text_ = text;
    undel_pos_ = a;
    undel_cursor_ = cur_;
    undel_before_ = before;
    goal_dcol_ = DisplayCol(cur_);
    Reveal();
}

int Editor::FlipCase(bool to_line_start) {
    const std::string &s = lines_[static_cast<size_t>(cur_.line)];
    const int from = to_line_start ? 0 : cur_.col;
    const int to = to_line_start ? cur_.col : static_cast<int>(s.size());
    if (from >= to) return 0;
    std::string seg = s.substr(static_cast<size_t>(from), static_cast<size_t>(to - from));
    ed_flip_case(seg.data(), 0, static_cast<int>(seg.size()));
    if (seg == s.substr(static_cast<size_t>(from), static_cast<size_t>(to - from))) return 0;
    const Pos keep = cur_;
    DoDelete({keep.line, from}, {keep.line, to});
    DoInsert({keep.line, from}, seg);
    cur_ = keep;
    return to - from;
}

// ---------------------------------------------------------------------------------------------------------------
// comandos
// ---------------------------------------------------------------------------------------------------------------

bool Editor::Execute(Cmd cmd, int arg) {
    const CmdInfo &ci = Info(cmd);
    if (!ci.implemented) {
        message_ = std::string(ci.name) + ": not available yet";
        return false;
    }
    if (read_only_ && ci.destructive) return true; // consumida, sem efeito: o destrutivo fica desligado na leitura
    if (cmd != Cmd::Literal) last_typed_ = false;
    if (!IsDeleteCmd(cmd) && cmd != Cmd::Undelete) undel_valid_ = false;
    if (cmd != Cmd::ContinueFwd && cmd != Cmd::ContinueRev && cmd != Cmd::TextInBlock) found_here_ = false;
    if (IsPlainMove(cmd)) ClearSelectionIfPlain();
    if (cmd != Cmd::Undo && cmd != Cmd::Redo) BeginGroup(false);

    // comandos que abrem dialogo
    if (ci.dialog != Dialog::None && cmd != Cmd::Exit && cmd != Cmd::Save) {
        pending_dialog_ = ci.dialog;
        if (cmd == Cmd::Help) help_topic_ = arg;
        return true;
    }

    const int h = std::max(1, TextRows());
    const std::string &cl = lines_[static_cast<size_t>(cur_.line)];
    const int len = static_cast<int>(cl.size());

    switch (cmd) {
    // --- cursor ---
    case Cmd::Left:
        if (cur_.col > 0) {
            MoveTo({cur_.line, cur_.col - 1});
        } else if (cur_.line > 0) {
            MoveTo({cur_.line - 1, static_cast<int>(lines_[static_cast<size_t>(cur_.line - 1)].size())});
        }
        return true;
    case Cmd::Right:
        if (cur_.col < len) {
            MoveTo({cur_.line, cur_.col + 1});
        } else if (cur_.line + 1 < LineCount()) {
            MoveTo({cur_.line + 1, 0});
        }
        return true;
    case Cmd::Up:
        MoveVertical(-1);
        return true;
    case Cmd::Down:
        MoveVertical(1);
        return true;
    case Cmd::WordLeft:
        if (cur_.col == 0 && cur_.line > 0) {
            MoveTo({cur_.line - 1, static_cast<int>(lines_[static_cast<size_t>(cur_.line - 1)].size())});
        } else {
            MoveTo({cur_.line, ed_word_left(cl.data(), len, cur_.col)});
        }
        return true;
    case Cmd::WordRight:
        if (cur_.col >= len && cur_.line + 1 < LineCount()) {
            MoveTo({cur_.line + 1, 0});
        } else {
            MoveTo({cur_.line, ed_word_right(cl.data(), len, cur_.col)});
        }
        return true;
    case Cmd::Home:
        MoveTo({cur_.line, 0});
        return true;
    case Cmd::End:
        MoveTo({cur_.line, len});
        return true;
    case Cmd::PgUp:
    case Cmd::PgDn: {
        const int n = std::max(1, h - 1) * (cmd == Cmd::PgUp ? -1 : 1);
        top_ = std::max(0, std::min(top_ + n, LineCount() - 1));
        MoveVertical(n);
        return true;
    }
    case Cmd::ScreenTop:
        cur_.line = top_;
        cur_.col = ColFromDisplay(cur_.line, goal_dcol_);
        return true;
    case Cmd::ScreenBottom:
        cur_.line = std::min(top_ + h - 1, LineCount() - 1);
        cur_.col = ColFromDisplay(cur_.line, goal_dcol_);
        return true;
    case Cmd::FileTop:
        MoveTo({0, 0});
        return true;
    case Cmd::FileEnd:
        MoveTo(EndOfFile());
        return true;
    case Cmd::ScrollUp:
    case Cmd::ScrollDown: {
        top_ = std::max(0, std::min(top_ + (cmd == Cmd::ScrollUp ? 1 : -1), LineCount() - 1));
        const int lo = top_;
        const int hi = std::min(top_ + h - 1, LineCount() - 1);
        const int line = std::max(lo, std::min(cur_.line, hi));
        if (line != cur_.line) {
            cur_.line = line;
            cur_.col = ColFromDisplay(line, goal_dcol_);
        }
        return true;
    }
    case Cmd::SelLeft: MoveSel(Cmd::Left); return true;
    case Cmd::SelRight: MoveSel(Cmd::Right); return true;
    case Cmd::SelUp: MoveSel(Cmd::Up); return true;
    case Cmd::SelDown: MoveSel(Cmd::Down); return true;
    case Cmd::SelHome: MoveSel(Cmd::Home); return true;
    case Cmd::SelEnd: MoveSel(Cmd::End); return true;
    case Cmd::SelPgUp: MoveSel(Cmd::PgUp); return true;
    case Cmd::SelPgDn: MoveSel(Cmd::PgDn); return true;
    case Cmd::SelWordLeft: MoveSel(Cmd::WordLeft); return true;
    case Cmd::SelWordRight: MoveSel(Cmd::WordRight); return true;

    // --- apagar ---
    case Cmd::DelLeft:
        if (cur_.col > 0) {
            DeleteRangeCmd({cur_.line, cur_.col - 1}, cur_);
        } else if (cur_.line > 0) {
            DeleteRangeCmd({cur_.line - 1, static_cast<int>(lines_[static_cast<size_t>(cur_.line - 1)].size())}, cur_);
        }
        return true;
    case Cmd::DelRight:
        if (cur_.col < len) {
            DeleteRangeCmd(cur_, {cur_.line, cur_.col + 1});
        } else if (cur_.line + 1 < LineCount()) {
            DeleteRangeCmd(cur_, {cur_.line + 1, 0});
        }
        return true;
    case Cmd::DelWordLeft:
        if (cur_.col == 0) return Execute(Cmd::DelLeft);
        DeleteRangeCmd({cur_.line, ed_word_left(cl.data(), len, cur_.col)}, cur_);
        return true;
    case Cmd::DelWordRight:
        if (cur_.col >= len) return Execute(Cmd::DelRight);
        DeleteRangeCmd(cur_, {cur_.line, ed_word_right(cl.data(), len, cur_.col)});
        return true;
    case Cmd::DelToBol:
        DeleteRangeCmd({cur_.line, 0}, cur_);
        return true;
    case Cmd::DelToEol:
        DeleteRangeCmd(cur_, {cur_.line, len});
        return true;
    case Cmd::KillLine: {
        if (LineCount() == 1) {
            DeleteRangeCmd({0, 0}, {0, len});
        } else if (cur_.line + 1 < LineCount()) {
            DeleteRangeCmd({cur_.line, 0}, {cur_.line + 1, 0});
        } else {
            DeleteRangeCmd({cur_.line - 1, static_cast<int>(lines_[static_cast<size_t>(cur_.line - 1)].size())},
                           {cur_.line, len});
            MoveTo({cur_.line, 0});
        }
        return true;
    }
    case Cmd::Undelete:
        if (undel_valid_ && cur_ == undel_cursor_) {
            DoInsert(undel_pos_, undel_text_);
            cur_ = undel_before_; // o texto voltou ao que era: o cursor tambem
            undel_valid_ = false;
            goal_dcol_ = DisplayCol(cur_);
            Reveal();
        }
        return true;
    case Cmd::FlipCaseBack:
        FlipCase(true);
        return true;
    case Cmd::FlipCaseFwd:
        FlipCase(false);
        return true;
    case Cmd::Literal:
        literal_pending_ = true;
        return true;
    case Cmd::Enter:
        InsertNewline(set_.auto_indent);
        return true;
    case Cmd::EnterIndent:
        InsertNewline(true);
        return true;
    case Cmd::Complete:
        CompleteCmd();
        return true;
    case Cmd::Tab: {
        if (SnippetTab()) return true;
        const int tw = std::max(1, set_.tab_width);
        if (set_.tab_key == TabKey::InsertTab) {
            InsertChars("\t");
        } else if (set_.tab_key == TabKey::InsertSpaces) {
            InsertChars(std::string(static_cast<size_t>(tw - DisplayCol(cur_) % tw), ' '));
        } else {
            const int next = (DisplayCol(cur_) / tw + 1) * tw;
            MoveTo({cur_.line, ColFromDisplay(cur_.line, next)});
        }
        return true;
    }
    case Cmd::Unindent: {
        const std::string &s = lines_[static_cast<size_t>(cur_.line)];
        int remove = 0;
        if (!s.empty() && s[0] == '\t') {
            remove = 1;
        } else {
            const int lead = ed_leading_ws(reinterpret_cast<const unsigned char *>(s.data()), len);
            while (remove < lead && remove < std::max(1, set_.tab_width) && s[static_cast<size_t>(remove)] == ' ') ++remove;
        }
        if (remove > 0) {
            const Pos keep = cur_;
            DoDelete({cur_.line, 0}, {cur_.line, remove});
            cur_ = {keep.line, std::max(0, keep.col - remove)};
            goal_dcol_ = DisplayCol(cur_);
            Reveal();
        }
        return true;
    }
    case Cmd::MenuOpen:
        menu_request_ = true;
        return true;
    case Cmd::ToggleInsert:
        if (set_.over_mode != OverMode::NeverOverstrike) insert_ = !insert_;
        return true;
    case Cmd::Undo:
        DoUndo(false);
        return true;
    case Cmd::Redo:
        DoUndo(true);
        return true;
    case Cmd::Copy:
        if (HasBlock()) clipboard_ = BlockText();
        return true;
    case Cmd::Cut:
        if (HasBlock()) {
            clipboard_ = BlockText();
            BlockDeleteCmd();
        }
        return true;
    case Cmd::Paste:
        if (!clipboard_.empty()) {
            ClearSelectionIfPlain();
            cur_ = DoInsert(cur_, clipboard_);
            goal_dcol_ = DisplayCol(cur_);
            Reveal();
        }
        return true;

    // --- File ---
    case Cmd::New:
        if (modified_) {
            pending_dialog_ = Dialog::ConfirmExit; // a interface pergunta e depois chama NewFile()
        } else {
            NewFile();
        }
        return true;
    case Cmd::Save: {
        if (name_.empty()) {
            pending_dialog_ = Dialog::SaveAs;
            return true;
        }
        std::string err;
        if (!Save(err)) message_ = err;
        return true;
    }
    case Cmd::Exit:
        if (modified_) {
            pending_dialog_ = Dialog::ConfirmExit;
        } else {
            exit_requested_ = true;
        }
        return true;

    // --- Block ---
    case Cmd::BlockSetMarker:
        BlockSet();
        return true;
    case Cmd::BlockRemoveMarkers:
        m1_set_ = m2_set_ = false;
        sel_active_ = false;
        return true;
    case Cmd::BlockFindMarker: {
        Pos best{INT_MAX, 0};
        Pos first{INT_MAX, 0};
        bool any = false;
        for (int i = 0; i < 2; ++i) {
            const bool set = i == 0 ? m1_set_ : m2_set_;
            const Pos p = i == 0 ? m1_ : m2_;
            if (!set) continue;
            any = true;
            if (p < first) first = p;
            if (cur_ < p && p < best) best = p;
        }
        if (any) MoveTo(best.line == INT_MAX ? first : best);
        return true;
    }
    case Cmd::BlockLine:
        m1_ = {cur_.line, 0};
        m2_ = cur_.line + 1 < LineCount() ? Pos{cur_.line + 1, 0} : Pos{cur_.line, len};
        m1_set_ = m2_set_ = true;
        sel_active_ = false;
        return true;
    case Cmd::BlockToEol:
        m1_ = cur_;
        m2_ = {cur_.line, len};
        m1_set_ = m2_set_ = true;
        sel_active_ = false;
        return true;
    case Cmd::BlockCopy:
        BlockCopyMove(false);
        return true;
    case Cmd::BlockMove:
        BlockCopyMove(true);
        return true;
    case Cmd::BlockDelete:
    case Cmd::BlockDeleteNoAsk:
        BlockDeleteCmd();
        return true;

    // --- Text-layout ---
    case Cmd::CenterLine:
        CenterLineCmd();
        return true;
    case Cmd::WordWrap:
        set_.word_wrap = !set_.word_wrap;
        return true;
    case Cmd::AutoIndent:
        set_.auto_indent = !set_.auto_indent;
        return true;

    // --- Search ---
    case Cmd::ContinueFwd:
        Search(true);
        return true;
    case Cmd::ContinueRev:
        Search(false);
        return true;
    case Cmd::MatchBracket:
        if (!MatchBracketCmd()) message_ = "No matching bracket";
        return true;
    case Cmd::TextInBlock:
        TextInBlockCmd();
        return true;
    default:
        return false;
    }
}

Dialog Editor::TakeDialog() {
    const Dialog d = pending_dialog_;
    pending_dialog_ = Dialog::None;
    return d;
}

// ---------------------------------------------------------------------------------------------------------------
// blocos
// ---------------------------------------------------------------------------------------------------------------

void Editor::BlockSet() {
    sel_active_ = false;
    if (!m1_set_) {
        m1_ = cur_;
        m1_set_ = true;
        message_ = "Block start set";
    } else if (!m2_set_) {
        m2_ = cur_;
        m2_set_ = true;
        message_ = "Block end set";
    } else {
        m1_ = cur_;
        m2_set_ = false;
        message_ = "Block start set";
    }
}

void Editor::BlockCopyMove(bool move) {
    Pos a, b;
    if (!BlockRange(a, b)) {
        message_ = "No block marked";
        return;
    }
    if (a < cur_ && cur_ < b) {
        message_ = "Cursor is inside the block";
        return;
    }
    const std::string text = BlockText();
    Pos at = cur_;
    if (move) {
        AdjustForDelete(a, b, at);
        DoDelete(a, b);
    }
    const Pos end = DoInsert(at, text);
    m1_ = at;
    m2_ = end;
    m1_set_ = m2_set_ = true;
    sel_active_ = false;
    cur_ = end;
    goal_dcol_ = DisplayCol(cur_);
    Reveal();
}

void Editor::BlockDeleteCmd() {
    Pos a, b;
    if (!BlockRange(a, b)) {
        message_ = "No block marked";
        return;
    }
    m1_set_ = m2_set_ = false;
    sel_active_ = false;
    DeleteRangeCmd(a, b);
}

void Editor::BlockIndentImpl(int amount, bool out) {
    Pos a, b;
    if (!BlockRange(a, b) || amount <= 0) return;
    int last = b.line;
    if (b.col == 0 && b.line > a.line) --last;
    for (int l = a.line; l <= last; ++l) {
        const std::string &s = lines_[static_cast<size_t>(l)];
        if (!out) {
            if (!s.empty()) DoInsert({l, 0}, std::string(static_cast<size_t>(amount), ' '));
            continue;
        }
        int removed = 0;
        int cols = 0;
        const int tw = std::max(1, set_.tab_width);
        while (removed < static_cast<int>(s.size()) && cols < amount) {
            const char c = s[static_cast<size_t>(removed)];
            if (c == ' ') {
                ++cols;
            } else if (c == '\t') {
                cols += tw;
            } else {
                break;
            }
            ++removed;
        }
        if (removed > 0) DoDelete({l, 0}, {l, removed});
    }
    MoveTo(cur_);
}

void Editor::Indent(int amount) {
    if (!Writable()) return;
    BeginGroup(false);
    BlockIndentImpl(amount, false);
}

void Editor::Unindent(int amount) {
    if (!Writable()) return;
    BeginGroup(false);
    BlockIndentImpl(amount, true);
}

void Editor::CenterLineCmd() {
    const std::string s = lines_[static_cast<size_t>(cur_.line)];
    size_t b = 0;
    size_t e = s.size();
    while (b < e && (s[b] == ' ' || s[b] == '\t')) ++b;
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t')) --e;
    const std::string body = s.substr(b, e - b);
    const int width = std::max(0, set_.line_length - static_cast<int>(body.size()));
    const std::string out = std::string(static_cast<size_t>(set_.left_margin + width / 2), ' ') + body;
    if (out == s) return;
    const Pos keep = cur_;
    DoDelete({keep.line, 0}, {keep.line, static_cast<int>(s.size())});
    DoInsert({keep.line, 0}, out);
    cur_ = {keep.line, static_cast<int>(out.size())};
    goal_dcol_ = DisplayCol(cur_);
    Reveal();
}

// ---------------------------------------------------------------------------------------------------------------
// busca
// ---------------------------------------------------------------------------------------------------------------

bool Editor::Search(bool fwd) {
    if (!have_find_ || last_find_.empty()) {
        message_ = "Nothing to search for";
        return false;
    }
    const int n = static_cast<int>(last_find_.size());
    if (fwd) {
        int start = cur_.col + (found_here_ ? 1 : 0);
        for (int l = cur_.line; l < LineCount(); ++l, start = 0) {
            const std::string &s = lines_[static_cast<size_t>(l)];
            const int i = ed_find(s.data(), static_cast<int>(s.size()), last_find_.data(), n, start, 1, last_case_ ? 1 : 0);
            if (i >= 0) {
                MoveTo({l, i});
                found_here_ = true;
                return true;
            }
        }
    } else {
        int start = cur_.col - 1;
        for (int l = cur_.line; l >= 0; --l) {
            const std::string &s = lines_[static_cast<size_t>(l)];
            const int i = ed_find(s.data(), static_cast<int>(s.size()), last_find_.data(), n, start, 0, last_case_ ? 1 : 0);
            if (i >= 0) {
                MoveTo({l, i});
                found_here_ = true;
                return true;
            }
            if (l > 0) start = static_cast<int>(lines_[static_cast<size_t>(l - 1)].size());
        }
    }
    found_here_ = false;
    message_ = "Text not found";
    return false;
}

bool Editor::Find(const std::string &text, bool match_case, bool backwards) {
    message_.clear();
    if (text.empty()) return false;
    last_find_ = text;
    last_case_ = match_case;
    last_back_ = backwards;
    have_find_ = true;
    found_here_ = false;
    return Search(!backwards);
}

bool Editor::TextInBlockCmd() {
    const std::string t = BlockText();
    if (t.empty() || t.find('\n') != std::string::npos) {
        message_ = "Mark a block inside one line";
        return false;
    }
    last_find_ = t;
    last_case_ = true;
    have_find_ = true;
    return Search(true);
}

bool Editor::ReplaceNext(const std::string &find, const std::string &repl, bool match_case, bool backwards) {
    if (!Writable()) return false;
    BeginGroup(false);
    if (!Find(find, match_case, backwards)) return false;
    const Pos at = cur_;
    DoDelete(at, {at.line, at.col + static_cast<int>(find.size())});
    const Pos end = DoInsert(at, repl);
    cur_ = backwards ? at : end;
    found_here_ = false;
    goal_dcol_ = DisplayCol(cur_);
    Reveal();
    return true;
}

bool Editor::ReplaceHere(int n, const std::string &repl) {
    if (!Writable() || n < 0) return false;
    const int len = static_cast<int>(lines_[static_cast<size_t>(cur_.line)].size());
    if (cur_.col + n > len) return false;
    BeginGroup(false);
    const Pos at = cur_;
    DoDelete(at, {at.line, at.col + n});
    cur_ = DoInsert(at, repl);
    found_here_ = false;
    goal_dcol_ = DisplayCol(cur_);
    Reveal();
    return true;
}

int Editor::ReplaceAll(const std::string &find, const std::string &repl, bool match_case) {
    if (!Writable() || find.empty()) return 0;
    BeginGroup(false);
    int count = 0;
    const int n = static_cast<int>(find.size());
    int start = cur_.col;
    for (int l = cur_.line; l < LineCount(); ++l, start = 0) {
        for (;;) {
            const std::string &s = lines_[static_cast<size_t>(l)];
            const int i = ed_find(s.data(), static_cast<int>(s.size()), find.data(), n, start, 1, match_case ? 1 : 0);
            if (i < 0) break;
            DoDelete({l, i}, {l, i + n});
            DoInsert({l, i}, repl);
            start = i + static_cast<int>(repl.size());
            ++count;
        }
    }
    last_find_ = find;
    last_case_ = match_case;
    have_find_ = true;
    found_here_ = false;
    MoveTo(cur_);
    return count;
}

bool Editor::GoToLine(int line1) {
    if (LineCount() == 0) return false;
    const int l = std::max(1, std::min(line1, LineCount()));
    MoveTo({l - 1, 0});
    return line1 >= 1 && line1 <= LineCount();
}

bool Editor::MatchScan(Pos from, Pos &out) const {
    if (from.line < 0 || from.line >= LineCount()) return false;
    const std::string &s = lines_[static_cast<size_t>(from.line)];
    if (from.col < 0 || from.col >= static_cast<int>(s.size())) return false;
    const char c = s[static_cast<size_t>(from.col)];
    static const char open[] = "([{<";
    static const char close[] = ")]}>";
    if (c == 0) return false;
    const char *o = std::strchr(open, c);
    const char *k = std::strchr(close, c);
    if (!o && !k) return false;
    const char mate = o ? close[o - open] : open[k - close];
    const int dir = o ? 1 : -1;
    int depth = 0;
    int l = from.line;
    int col = from.col;
    for (;;) {
        const std::string &t = lines_[static_cast<size_t>(l)];
        for (; col >= 0 && col < static_cast<int>(t.size()); col += dir) {
            const char x = t[static_cast<size_t>(col)];
            if (x == c) {
                ++depth;
            } else if (x == mate) {
                --depth;
                if (depth == 0) {
                    out = {l, col};
                    return true;
                }
            }
        }
        l += dir;
        if (l < 0 || l >= LineCount()) return false;
        col = dir > 0 ? 0 : static_cast<int>(lines_[static_cast<size_t>(l)].size()) - 1;
    }
}

bool Editor::MatchBracketCmd() {
    Pos m;
    if (!MatchScan(cur_, m)) return false;
    MoveTo(m);
    return true;
}

bool Editor::MatchOf(Pos from, Pos &out) const {
    return MatchScan(from, out);
}

// ---------------------------------------------------------------------------------------------------------------
// mouse
// ---------------------------------------------------------------------------------------------------------------

Pos Editor::ScreenToPos(int x, int row) const {
    const int line = std::max(0, std::min(top_ + row, LineCount() - 1));
    return {line, ColFromDisplay(line, std::max(0, x - Gutter()) + left_)};
}

Pos Editor::PosAt(int line, int dcol) const {
    const int l = std::max(0, std::min(line, LineCount() - 1));
    return {l, ColFromDisplay(l, std::max(0, dcol))};
}

void Editor::SelectWord(Pos p) {
    p.line = std::max(0, std::min(p.line, LineCount() - 1));
    const std::string &s = lines_[static_cast<size_t>(p.line)];
    const int len = static_cast<int>(s.size());
    int a = std::min(p.col, len);
    int b = a;
    while (a > 0 && ed_is_word_char(static_cast<unsigned char>(s[static_cast<size_t>(a - 1)]))) --a;
    while (b < len && ed_is_word_char(static_cast<unsigned char>(s[static_cast<size_t>(b)]))) ++b;
    if (a == b && b < len) ++b; // pontuacao: seleciona o proprio caractere
    SelectBlock({p.line, a}, {p.line, b});
    sel_active_ = false;
    cur_ = {p.line, b};
    goal_dcol_ = DisplayCol(cur_);
}

void Editor::SelectLine(int line) {
    line = std::max(0, std::min(line, LineCount() - 1));
    const Pos a{line, 0};
    const Pos b = line + 1 < LineCount() ? Pos{line + 1, 0} : Pos{line, static_cast<int>(lines_[static_cast<size_t>(line)].size())};
    SelectBlock(a, b);
    sel_active_ = false;
    cur_ = b;
    goal_dcol_ = DisplayCol(cur_);
}

void Editor::SelectBlock(Pos a, Pos b) {
    if (a == b) {
        m1_set_ = m2_set_ = false;
        return;
    }
    m1_ = a < b ? a : b;
    m2_ = a < b ? b : a;
    m1_set_ = m2_set_ = true;
    sel_active_ = true;
}

void Editor::ScrollView(int dy) {
    const int h = std::max(1, TextRows());
    top_ = std::max(0, std::min(top_ + dy, LineCount() - 1));
    const int line = std::max(top_, std::min(cur_.line, std::min(top_ + h - 1, LineCount() - 1)));
    if (line != cur_.line) {
        cur_.line = line;
        cur_.col = ColFromDisplay(line, goal_dcol_);
    }
}

} // namespace ed
