// fwMSX -- editor de textos: arquivo (abrir, gravar, mesclar), informacoes e conversoes do menu Text-layout.
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/editor-spec.md, secao 4 (arquivos do MSX).
//
// Formato de arquivo: o texto do MSX-DOS termina com Ctrl-Z (0x1A) e usa CRLF; o editor guarda o que achou ao abrir
// (CRLF ou LF, Ctrl-Z no fim, BOM UTF-8) e grava do mesmo jeito. Arquivo novo: CRLF, sem Ctrl-Z.
#include "editor.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>

extern "C" {
#include "../c/ed_text.h"
}
#include "../asm/ed_indent.h"

extern "C" int ed_count_words(const char *buf, int n);

namespace ed {

namespace fs = std::filesystem;

namespace {

bool ReadAll(const std::string &path, std::string &out, std::string &err) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        err = "Cannot open " + path;
        return false;
    }
    out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    return true;
}

// Separa em linhas. `crlf` diz se o arquivo usava CR+LF; os CR do fim das linhas saem do texto interno.
std::vector<std::string> SplitLines(const std::string &data, bool crlf) {
    std::vector<std::string> lines;
    size_t start = 0;
    for (;;) {
        const size_t nl = data.find('\n', start);
        if (nl == std::string::npos) {
            lines.push_back(data.substr(start));
            break;
        }
        size_t end = nl;
        if (crlf && end > start && data[end - 1] == '\r') --end;
        lines.push_back(data.substr(start, end - start));
        start = nl + 1;
    }
    return lines;
}

} // namespace

void Editor::NewFile() {
    ++rev_;
    lines_.assign(1, std::string());
    cur_ = {};
    top_ = left_ = goal_dcol_ = 0;
    modified_ = false;
    crlf_ = true;
    eof_ctrl_z_ = false;
    bom_ = false;
    name_.clear();
    m1_set_ = m2_set_ = sel_active_ = false;
    undo_.clear();
    redo_.clear();
    undel_valid_ = false;
    insert_ = set_.over_mode != OverMode::StartOverstrike;
    message_.clear();
}

void Editor::SetText(const std::string &text) {
    ++rev_;
    lines_ = SplitLines(text, false);
    cur_ = {};
    top_ = left_ = goal_dcol_ = 0;
    modified_ = false;
    m1_set_ = m2_set_ = sel_active_ = false;
    undo_.clear();
    redo_.clear();
    undel_valid_ = false;
}

std::string Editor::Text() const {
    std::string out;
    for (size_t i = 0; i < lines_.size(); ++i) {
        if (i) out.push_back('\n');
        out += lines_[i];
    }
    return out;
}

bool Editor::Load(const std::string &path, std::string &err) {
    std::string data;
    if (!ReadAll(path, data, err)) return false;
    bool bom = false;
    if (data.size() >= 3 && static_cast<unsigned char>(data[0]) == 0xEF && static_cast<unsigned char>(data[1]) == 0xBB &&
        static_cast<unsigned char>(data[2]) == 0xBF) {
        bom = true;
        data.erase(0, 3);
    }
    bool ctrl_z = false;
    const size_t z = data.find('\x1A');
    if (z != std::string::npos) {
        ctrl_z = true;
        data.erase(z);
    }
    const bool crlf = data.empty() ? true : data.find("\r\n") != std::string::npos;
    NewFile();
    lines_ = SplitLines(data, crlf);
    crlf_ = crlf;
    eof_ctrl_z_ = ctrl_z;
    bom_ = bom;
    name_ = path;
    modified_ = false;
    return true;
}

bool Editor::Save(std::string &err) {
    if (name_.empty()) {
        err = "File has no name";
        return false;
    }
    std::error_code ec;
    // Copia de seguranca: o primeiro caractere da extensao vira o caractere de backup (READ.ME -> READ.{E), como no DOS.
    if (!set_.delete_backup && fs::exists(name_, ec)) {
        fs::path bak(name_);
        std::string ext = bak.extension().string();
        if (ext.size() >= 2) {
            ext[1] = set_.backup_char;
        } else {
            ext = std::string(".") + set_.backup_char;
        }
        bak.replace_extension(ext);
        fs::copy_file(name_, bak, fs::copy_options::overwrite_existing, ec);
    }
    std::ofstream f(name_, std::ios::binary | std::ios::trunc);
    if (!f) {
        err = "Cannot write " + name_;
        return false;
    }
    if (bom_) f.write("\xEF\xBB\xBF", 3);
    const char *eol = crlf_ ? "\r\n" : "\n";
    for (size_t i = 0; i < lines_.size(); ++i) {
        if (i) f.write(eol, static_cast<std::streamsize>(std::char_traits<char>::length(eol)));
        f.write(lines_[i].data(), static_cast<std::streamsize>(lines_[i].size()));
    }
    if (eof_ctrl_z_) f.put('\x1A');
    if (!f) {
        err = "Write error on " + name_;
        return false;
    }
    modified_ = false;
    return true;
}

bool Editor::SaveAs(const std::string &path, std::string &err) {
    const std::string old = name_;
    name_ = path;
    if (!Save(err)) {
        name_ = old;
        return false;
    }
    return true;
}

bool Editor::Merge(const std::string &path, std::string &err) {
    if (!Writable()) return false;
    std::string data;
    if (!ReadAll(path, data, err)) return false;
    const size_t z = data.find('\x1A');
    if (z != std::string::npos) data.erase(z);
    std::string joined;
    const std::vector<std::string> ls = SplitLines(data, data.find("\r\n") != std::string::npos);
    for (size_t i = 0; i < ls.size(); ++i) {
        if (i) joined.push_back('\n');
        joined += ls[i];
    }
    BeginGroup(false);
    DoInsert(cur_, joined);
    MoveTo(cur_);
    return true;
}

bool Editor::WriteBlock(const std::string &path, std::string &err) {
    const std::string text = BlockText();
    if (text.empty()) {
        err = "No block marked";
        return false;
    }
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f) {
        err = "Cannot write " + path;
        return false;
    }
    for (const char c : text) {
        if (c == '\n' && crlf_) f.put('\r');
        f.put(c);
    }
    return true;
}

DocInfo Editor::Information() const {
    DocInfo d;
    d.file = name_.empty() ? "Untitled" : name_;
    d.lines = static_cast<long>(lines_.size());
    long chars = 0;
    long words = 0;
    for (const std::string &l : lines_) {
        chars += static_cast<long>(l.size());
        words += ed_count_words(l.data(), static_cast<int>(l.size()));
    }
    chars += (d.lines - 1) * (crlf_ ? 2 : 1);
    d.chars = chars;
    d.words = words;
    const std::string &cl = lines_[static_cast<size_t>(cur_.line)];
    d.char_at_cursor = cur_.col < static_cast<int>(cl.size()) ? static_cast<unsigned char>(cl[static_cast<size_t>(cur_.col)]) : -1;
    return d;
}

// ---------------------------------------------------------------------------------------------------------------
// Text-layout: tabulacoes e quebras de pagina
// ---------------------------------------------------------------------------------------------------------------

void Editor::ApplyExpandTabs(bool from_top) {
    if (!Writable()) return;
    BeginGroup(false);
    const Pos keep = cur_;
    for (int l = from_top ? 0 : cur_.line; l < LineCount(); ++l) {
        const std::string s = lines_[static_cast<size_t>(l)];
        if (s.find('\t') == std::string::npos) continue;
        const std::string out = ExpandedLine(s);
        DoDelete({l, 0}, {l, static_cast<int>(s.size())});
        DoInsert({l, 0}, out);
    }
    cur_ = keep;
    MoveTo(cur_);
}

void Editor::ApplyPutInTabs(bool from_top) {
    if (!Writable()) return;
    BeginGroup(false);
    const int tw = std::max(1, set_.tab_width);
    const Pos keep = cur_;
    for (int l = from_top ? 0 : cur_.line; l < LineCount(); ++l) {
        const std::string s = lines_[static_cast<size_t>(l)];
        const int lead = ed_leading_ws(reinterpret_cast<const unsigned char *>(s.data()), static_cast<int>(s.size()));
        if (lead == 0) continue;
        int width = 0;
        for (int i = 0; i < lead; ++i) width += s[static_cast<size_t>(i)] == '\t' ? tw - width % tw : 1;
        const std::string ws = std::string(static_cast<size_t>(width / tw), '\t') + std::string(static_cast<size_t>(width % tw), ' ');
        if (ws == s.substr(0, static_cast<size_t>(lead))) continue;
        DoDelete({l, 0}, {l, lead});
        DoInsert({l, 0}, ws);
    }
    cur_ = keep;
    MoveTo(cur_);
}

// Quebra de pagina = uma linha com so' o caractere 0Ch (form feed), a cada `page_length` linhas de texto.
void Editor::ApplyInsertPageBreaks(bool from_top) {
    if (!Writable() || set_.page_length < 1) return;
    BeginGroup(false);
    const Pos keep = cur_;
    std::vector<int> at;
    int count = 0;
    for (int l = from_top ? 0 : cur_.line; l < LineCount(); ++l) {
        if (lines_[static_cast<size_t>(l)] == "\f") {
            count = 0;
            continue;
        }
        if (count == set_.page_length) {
            at.push_back(l);
            count = 0;
        }
        ++count;
    }
    for (auto it = at.rbegin(); it != at.rend(); ++it) DoInsert({*it, 0}, "\f\n");
    cur_ = keep;
    MoveTo(cur_);
}

void Editor::ApplyRemovePageBreaks(bool from_top) {
    if (!Writable()) return;
    BeginGroup(false);
    const Pos keep = cur_;
    for (int l = LineCount() - 1; l >= (from_top ? 0 : cur_.line); --l) {
        if (lines_[static_cast<size_t>(l)] != "\f") continue;
        if (l + 1 < LineCount()) {
            DoDelete({l, 0}, {l + 1, 0});
        } else if (l > 0) {
            DoDelete({l - 1, static_cast<int>(lines_[static_cast<size_t>(l - 1)].size())}, {l, 1});
        } else {
            DoDelete({0, 0}, {0, 1});
        }
    }
    cur_ = keep;
    MoveTo(cur_);
}

} // namespace ed
