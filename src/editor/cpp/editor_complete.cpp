// fwMSX -- editor de textos: completar palavras, snippets e a margem de numeros de linha da grade (fase E5).
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver completion.h e doc/editor-spec.md, secao 4.
#include "editor.h"

#include <algorithm>

extern "C" {
#include "../c/ed_syntax.h"
}

namespace ed {

int Editor::Gutter() const {
    if (!set_.line_numbers || !grid_gutter_) return 0;
    int digits = 1;
    for (int n = LineCount(); n >= 10; n /= 10) ++digits;
    return std::max(3, digits) + 1; // os digitos e uma coluna de respiro
}

// ---------------------------------------------------------------------------------------------------------------
// completar
// ---------------------------------------------------------------------------------------------------------------

void Editor::RefreshCompletion() {
    const std::string &s = lines_[static_cast<size_t>(cur_.line)];
    const int st = ed_word_start(s.data(), static_cast<int>(s.size()), cur_.col);
    if (cur_.col - st < 1) {
        comp_open_ = false;
        return;
    }
    const std::string prefix = s.substr(static_cast<size_t>(st), static_cast<size_t>(cur_.col - st));
    comp_items_ = CompletionCandidates(Language(), prefix, comp_words_);
    // a palavra inteira que esta' sob o cursor (o cursor no meio dela) nao e' candidata de si mesma
    const int word_end = ed_hl_ident_end(s.data(), static_cast<int>(s.size()), st, 0);
    if (word_end > cur_.col) {
        const std::string whole = s.substr(static_cast<size_t>(st), static_cast<size_t>(word_end - st));
        comp_items_.erase(std::remove_if(comp_items_.begin(), comp_items_.end(), [&](const Candidate &c) { return !c.snippet && c.text == whole; }), comp_items_.end());
    }
    comp_col_ = st;
    comp_sel_ = 0;
    comp_open_ = !comp_items_.empty();
}

void Editor::CompleteCmd() {
    comp_words_.clear();
    CollectBufferWords(lines_, comp_words_);
    RefreshCompletion();
    if (!comp_open_) {
        message_ = "No completions";
        return;
    }
    if (comp_items_.size() == 1) CompletionAccept(); // so' uma saida: completa direto
}

void Editor::AfterTyping(const std::string &text) {
    const bool word_char = text.size() == 1 && ed_hl_ident_char(static_cast<unsigned char>(text[0])) != 0;
    if (!word_char) {
        comp_open_ = false;
        return;
    }
    if (comp_open_) {
        RefreshCompletion();
        return;
    }
    if (!set_.auto_complete) return;
    const std::string &s = lines_[static_cast<size_t>(cur_.line)];
    if (cur_.col - ed_word_start(s.data(), static_cast<int>(s.size()), cur_.col) < 2) return; // pelo menos 2 letras para abrir sozinha
    comp_words_.clear();
    CollectBufferWords(lines_, comp_words_);
    RefreshCompletion();
}

void Editor::CompletionSelect(int i) {
    if (comp_items_.empty()) return;
    comp_sel_ = std::max(0, std::min(i, static_cast<int>(comp_items_.size()) - 1));
}

bool Editor::CompletionKey(const std::string &canon) {
    const int n = static_cast<int>(comp_items_.size());
    if (canon == "Up") {
        comp_sel_ = (comp_sel_ + n - 1) % n;
    } else if (canon == "Down") {
        comp_sel_ = (comp_sel_ + 1) % n;
    } else if (canon == "PgUp") {
        CompletionSelect(comp_sel_ - 8);
    } else if (canon == "PgDn") {
        CompletionSelect(comp_sel_ + 8);
    } else if (canon == "Enter" || canon == "Tab") {
        CompletionAccept();
    } else if (canon == "Esc") {
        comp_open_ = false;
    } else {
        return false;
    }
    return true;
}

bool Editor::CompletionAccept() {
    if (!comp_open_ || comp_sel_ < 0 || comp_sel_ >= static_cast<int>(comp_items_.size())) {
        comp_open_ = false;
        return false;
    }
    const Candidate c = comp_items_[static_cast<size_t>(comp_sel_)];
    const Pos from{cur_.line, comp_col_};
    comp_open_ = false;
    if (read_only_ || !(from < cur_)) return false;
    last_typed_ = false;
    BeginGroup(false);
    if (c.snippet) {
        SnippetDef d;
        if (FindSnippet(Language(), c.text, d)) {
            ExpandSnippetAt(from, cur_, d.body);
            return true;
        }
    }
    DoDelete(from, cur_);
    cur_ = DoInsert(from, c.text);
    goal_dcol_ = DisplayCol(cur_);
    Reveal();
    return true;
}

// ---------------------------------------------------------------------------------------------------------------
// snippets
// ---------------------------------------------------------------------------------------------------------------

void Editor::ExpandSnippetAt(Pos from, Pos to, const std::string &body) {
    // a identacao da linha do gatilho vale para todas as linhas do snippet (so' ate' a coluna do gatilho)
    const std::string indent = LeadingIndent(from.line).substr(0, static_cast<size_t>(from.col));
    const Expansion ex = ExpandSnippet(body, indent);
    if (from < to) DoDelete(from, to);
    cur_ = from;
    DoInsert(from, ex.text);
    snip_stops_.clear();
    for (const auto &st : ex.stops) {
        snip_stops_.push_back({from.line + st.first, st.first == 0 ? from.col + st.second : st.second});
    }
    const Pos first = snip_stops_.front();
    snip_stops_.erase(snip_stops_.begin());
    MoveTo(first);
}

bool Editor::SnippetTab() {
    if (!snip_stops_.empty()) { // dentro de um snippet: Tab pula para o proximo ponto de parada
        Pos p = snip_stops_.front();
        snip_stops_.erase(snip_stops_.begin());
        p.line = std::max(0, std::min(p.line, LineCount() - 1));
        p.col = std::max(0, std::min(p.col, static_cast<int>(lines_[static_cast<size_t>(p.line)].size())));
        MoveTo(p);
        return true;
    }
    const std::string &s = lines_[static_cast<size_t>(cur_.line)];
    const int len = static_cast<int>(s.size());
    const int st = ed_word_start(s.data(), len, cur_.col);
    if (cur_.col - st < 1) return false;
    if (cur_.col < len && ed_hl_ident_char(static_cast<unsigned char>(s[static_cast<size_t>(cur_.col)]))) return false; // no meio de uma palavra
    SnippetDef d;
    if (!FindSnippet(Language(), s.substr(static_cast<size_t>(st), static_cast<size_t>(cur_.col - st)), d)) return false;
    ExpandSnippetAt({cur_.line, st}, cur_, d.body);
    return true;
}

// ---------------------------------------------------------------------------------------------------------------
// a lista na grade (TUI)
// ---------------------------------------------------------------------------------------------------------------

void Editor::DrawCompletion(Grid &g, int y0, int gutter, int width) const {
    if (!comp_open_ || comp_items_.empty() || comp_col_ > cur_.col) return;
    const int h = TextRows();
    const int n = static_cast<int>(comp_items_.size());
    const int show = std::min(n, 8);
    size_t maxw = 4;
    for (const Candidate &c : comp_items_) maxw = std::max(maxw, c.text.size() + (c.snippet ? 2 : 0));
    const int w = std::min(static_cast<int>(maxw) + 2, std::max(8, width));
    const int row = cur_.line - top_;
    if (row < 0 || row >= h) return;
    // abaixo da linha do cursor; se nao couber, acima
    int ry = row + 1;
    if (ry + show > h && row - show >= 0) ry = row - show;
    int rx = gutter + DisplayCol({cur_.line, comp_col_}) - left_;
    rx = std::max(gutter, std::min(rx, gutter + width - w));
    int top = 0;
    if (comp_sel_ >= show) top = comp_sel_ - show + 1;
    for (int i = 0; i < show; ++i) {
        const int yy = y0 + ry + i;
        if (ry + i < 0 || ry + i >= h) continue;
        const int idx = top + i;
        const Candidate &c = comp_items_[static_cast<size_t>(idx)];
        const Style st = idx == comp_sel_ ? kPanelSel : kPanel;
        std::string text = " " + c.text + (c.snippet ? " *" : "");
        text.resize(static_cast<size_t>(w), ' ');
        g.Put(rx, yy, text, st);
    }
}

} // namespace ed
