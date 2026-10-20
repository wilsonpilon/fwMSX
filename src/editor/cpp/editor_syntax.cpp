// fwMSX -- editor de textos: a linguagem de cada documento e o destaque por linha (fase E4). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver editor.h (Language, HighlightLine) e syntax.h.
#include "editor.h"

namespace ed {

const Lang *Editor::Language() const {
    return lang_ ? lang_ : LangById("plain");
}

void Editor::SetLanguage(const Lang *l, bool manual) {
    lang_ = (l && l->Id() == "plain") ? nullptr : l;
    lang_manual_ = manual && l != nullptr;
    hl_states_.clear();
    hl_rev_ = 0;
}

void Editor::DetectLanguage() {
    std::string sample;
    for (int i = 0; i < LineCount() && sample.size() < 4096; ++i) {
        sample += lines_[static_cast<size_t>(i)];
        sample.push_back('\n');
    }
    const Lang *l = LangForFile(name_, sample);
    lang_ = (l && l->Id() != "plain") ? l : nullptr;
    lang_manual_ = false;
    hl_states_.clear();
    hl_rev_ = 0;
}

void Editor::HighlightLine(int line, std::vector<Span> &out) const {
    out.clear();
    if (!set_.syntax || !lang_ || line < 0 || line >= LineCount()) return;
    HlState st;
    if (lang_->MultiLine()) {
        // o estado do inicio de cada linha e' guardado ate' onde ja' foi pedido e some quando o texto muda
        if (hl_rev_ != rev_) {
            hl_states_.clear();
            hl_rev_ = rev_;
        }
        if (hl_states_.empty()) hl_states_.push_back(HlState{});
        std::vector<Span> scratch;
        while (static_cast<int>(hl_states_.size()) <= line) {
            HlState next = hl_states_.back();
            lang_->Highlight(lines_[hl_states_.size() - 1], next, scratch);
            hl_states_.push_back(next);
        }
        st = hl_states_[static_cast<size_t>(line)];
    }
    lang_->Highlight(lines_[static_cast<size_t>(line)], st, out);
}

} // namespace ed
