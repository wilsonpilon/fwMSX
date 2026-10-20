// fwMSX -- editor de textos: a grade de celulas (barra de menu, texto, barra de rolagem e barra de estado) no
// desenho do Norton Editor 2. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/editor-spec.md, secao 1b.4.
#include "editor.h"

#include <algorithm>
#include <string>

namespace ed {

void Grid::Resize(int c, int r) {
    cols = c;
    rows = r;
    cells.assign(static_cast<size_t>(c) * r, Cell{});
}

void Grid::Put(int x, int y, const std::string &text, Style style) {
    if (y < 0 || y >= rows) return;
    for (size_t i = 0; i < text.size(); ++i) {
        const int xx = x + static_cast<int>(i);
        if (xx < 0 || xx >= cols) continue;
        Cell &c = at(xx, y);
        c.ch = static_cast<unsigned char>(text[i]);
        c.style = style;
    }
}

void Grid::PaintRow(int y, Style style) {
    if (y < 0 || y >= rows) return;
    for (int x = 0; x < cols; ++x) at(x, y).style = style;
}

void Editor::SetSize(int cols, int rows) {
    cols_ = std::max(20, cols);
    rows_ = std::max(4, rows);
    text_rows_ = rows_ - 2;
    Reveal();
}

void Editor::SetViewSize(int cols, int text_rows, bool reveal) {
    cols_ = std::max(20, cols);
    text_rows_ = std::max(1, text_rows);
    if (reveal) Reveal();
}

bool Editor::CursorScreen(int &x, int &y) const {
    x = DisplayCol(cur_) - left_;
    y = cur_.line - top_ + 1;
    return x >= 0 && x < cols_ - 1 && y >= 1 && y <= TextRows();
}

namespace {

// Colunas das etiquetas da barra de estado, como no NE 2 (80 colunas).
constexpr int kColLine = 4;
constexpr int kColCol = 15;
constexpr int kColName = 36;
constexpr int kColMode = 58;
constexpr int kColWw = 66;
constexpr int kColAuto = 73;

void PutWide(Grid &g, int x, int y, uint16_t ch, Style st) {
    if (x < 0 || x >= g.cols || y < 0 || y >= g.rows) return;
    g.at(x, y).ch = ch;
    g.at(x, y).style = st;
}

} // namespace

void Editor::Render(Grid &g) const {
    g.Resize(cols_, rows_);

    // --- barra de menu ---
    g.PaintRow(0, kBar);
    for (const Menu &m : Menus()) {
        const bool open = menu_prefix_ == m.hot;
        const int x = m.col * cols_ / kDefaultCols;
        g.Put(x, 0, m.title, open ? kBarSel : kBar);
    }
    g.Put(cols_ - 10, 0, "F1=Help", kBar);

    RenderBody(g, 1, true);
    RenderStatus(g, rows_ - 1);
}

void Editor::RenderBody(Grid &g, int y0, bool scrollbar) const {
    const int h = TextRows();
    Pos ba, bb;
    const bool has_block = BlockRange(ba, bb);
    const int width = scrollbar ? cols_ - 1 : cols_;
    Pos bracket{-1, -1};
    if (set_.show_bracket && cur_.col > 0) {
        const std::string &cl = lines_[static_cast<size_t>(cur_.line)];
        const char c = cl[static_cast<size_t>(cur_.col - 1)];
        if (c == ')' || c == ']' || c == '}' || c == '>') MatchScan({cur_.line, cur_.col - 1}, bracket);
    }
    for (int row = 0; row < h; ++row) {
        const int l = top_ + row;
        if (l >= LineCount()) continue;
        const std::string &s = lines_[static_cast<size_t>(l)];
        const int tw = std::max(1, set_.tab_width);
        int dc = 0;
        for (int i = 0; i <= static_cast<int>(s.size()); ++i) {
            const bool eol = i == static_cast<int>(s.size());
            const int w = eol ? 1 : (s[static_cast<size_t>(i)] == '	' ? tw - dc % tw : 1);
            const Pos here{l, i};
            const bool in_block = has_block && ba <= here && here < bb;
            const bool marker = (m1_set_ && here == m1_) || (m2_set_ && here == m2_);
            for (int k = 0; k < w; ++k) {
                const int x = dc + k - left_;
                if (x < 0 || x >= width) continue;
                Cell &c = g.at(x, y0 + row);
                if (!eol && s[static_cast<size_t>(i)] != '	') c.ch = static_cast<unsigned char>(s[static_cast<size_t>(i)]);
                c.style = marker ? kBlockMark : (in_block ? kBlockSel : kText);
                if (here == bracket) c.style = kBracket;
                if (marker && c.ch == ' ') c.ch = 0x25AA; // quadradinho onde o marcador esta'
            }
            dc += w;
            if (dc - left_ >= width) break;
        }
    }

    if (!scrollbar) return;
    // --- barra de rolagem vertical ---
    PutWide(g, cols_ - 1, y0, 0x2191, kScrollArrow);
    PutWide(g, cols_ - 1, y0 + h - 1, 0x2193, kScrollArrow);
    const int track = std::max(0, h - 2);
    for (int i = 0; i < track; ++i) PutWide(g, cols_ - 1, y0 + 1 + i, 0x2591, kScrollTrack);
    if (track > 0) {
        const int thumb = LineCount() <= 1 ? 0 : std::min(track - 1, cur_.line * track / LineCount());
        PutWide(g, cols_ - 1, y0 + 1 + thumb, 0x2588, kScrollThumb);
    }
}

void Editor::RenderStatus(Grid &g, int sy) const {
    g.PaintRow(sy, kBar);
    PutWide(g, 0, sy, 0x2190, kBar);
    if (modified_) g.Put(2, sy, "*", kBar);
    g.Put(kColLine, sy, "Line=" + std::to_string(cur_.line + 1), kBar);
    g.Put(kColCol, sy, "Col=" + std::to_string(DisplayCol(cur_) + 1), kBar);
    std::string name = name_.empty() ? "Untitled" : name_;
    if (name.size() > 20) name = name.substr(name.size() - 20);
    g.Put(kColName, sy, name, kBar);
    g.Put(kColMode, sy, read_only_ ? "View" : (insert_ ? "Insert" : "Over"), kBar);
    g.Put(kColWw, sy, set_.word_wrap ? "WW=On" : "WW=Off", kBar);
    if (set_.auto_indent) g.Put(kColAuto, sy, "Auto", kBar);
    if (!message_.empty()) {
        // a mensagem passageira cobre o nome do arquivo (o NE tambem usa a barra de estado para avisos)
        g.Put(kColName - 4, sy, std::string(static_cast<size_t>(kColMode - kColName + 4), ' '), kBar);
        g.Put(kColName - 4, sy, message_.substr(0, static_cast<size_t>(kColMode - kColName + 3)), kBar);
    }
    PutWide(g, cols_ - 1, sy, 0x2192, kBar);
}

} // namespace ed
