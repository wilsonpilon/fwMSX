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

// `count` bytes de `path` a partir de `offset` (menos, se o arquivo acabar antes).
bool ReadRange(const std::string &path, long long offset, long long count, std::string &out, std::string &err) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        err = "Cannot open " + path;
        return false;
    }
    f.seekg(static_cast<std::streamoff>(offset));
    out.resize(static_cast<size_t>(std::max(0LL, count)));
    f.read(&out[0], static_cast<std::streamsize>(out.size()));
    out.resize(static_cast<size_t>(f.gcount()));
    return true;
}

// Copia o arquivo `path` (a partir de `offset`) para o fim de `out`.
bool AppendFile(std::ofstream &out, const std::string &path, long long offset) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    in.seekg(static_cast<std::streamoff>(offset));
    char buf[65536];
    while (in) {
        in.read(buf, sizeof(buf));
        if (in.gcount() > 0) out.write(buf, in.gcount());
    }
    return static_cast<bool>(out);
}

} // namespace

void Editor::SpillFile::Drop() {
    if (path.empty()) return;
    std::error_code ec;
    fs::remove(path, ec);
    path.clear();
}

void Editor::NewFile() {
    spill_.Drop();
    has_tail_ = false;
    tail_src_.clear();
    ++rev_;
    snip_stops_.clear();
    comp_open_ = false;
    lines_.assign(1, std::string());
    cur_ = {};
    top_ = left_ = goal_dcol_ = 0;
    modified_ = false;
    crlf_ = true;
    eof_ctrl_z_ = false;
    bom_ = false;
    name_.clear();
    untitled_.clear();
    lang_ = nullptr;
    lang_manual_ = false;
    hl_states_.clear();
    autosaved_ = false;
    m1_set_ = m2_set_ = sel_active_ = false;
    undo_.clear();
    redo_.clear();
    undel_valid_ = false;
    insert_ = set_.over_mode != OverMode::StartOverstrike;
    message_.clear();
}

void Editor::SetText(const std::string &text) {
    ++rev_;
    snip_stops_.clear(); // os pontos de parada e a lista eram do texto antigo
    comp_open_ = false;
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
    std::error_code sec;
    const long long total = static_cast<long long>(fs::file_size(path, sec));
    const bool partial = !sec && set_.load_limit > 0 && total > set_.load_limit;
    if (partial) {
        if (!ReadRange(path, 0, set_.load_limit, data, err)) return false;
    } else if (!ReadAll(path, data, err)) {
        return false;
    }
    bool bom = false;
    if (data.size() >= 3 && static_cast<unsigned char>(data[0]) == 0xEF && static_cast<unsigned char>(data[1]) == 0xBB &&
        static_cast<unsigned char>(data[2]) == 0xBF) {
        bom = true;
        data.erase(0, 3);
    }
    bool ctrl_z = false;
    long long tail_off = 0;
    if (partial) {
        // corta na ultima quebra de linha lida: o fim de linha que fecha a ultima linha do buffer e' o 1o byte do que fica por ler (sem nenhuma, o bloco todo)
        const size_t nl = data.rfind('\n');
        size_t cut = nl == std::string::npos ? data.size() : nl;
        if (nl != std::string::npos && nl > 0 && data[nl - 1] == '\r') cut = nl - 1;
        tail_off = (bom ? 3 : 0) + static_cast<long long>(cut);
        data.erase(cut);
    } else {
        const size_t z = data.find('\x1A');
        if (z != std::string::npos) {
            ctrl_z = true;
            data.erase(z);
        }
    }
    const bool crlf = data.empty() ? true : data.find("\r\n") != std::string::npos;
    NewFile();
    has_tail_ = partial;
    if (partial) {
        tail_src_ = path;
        tail_off_ = tail_off;
        tail_size_ = total;
    }
    lines_ = SplitLines(data, crlf);
    snip_stops_.clear();
    comp_open_ = false;
    crlf_ = crlf;
    eof_ctrl_z_ = ctrl_z;
    bom_ = bom;
    name_ = path;
    modified_ = false;
    lang_manual_ = false;
    DetectLanguage(); // a linguagem vem da extensao e do inicio do texto
    return true;
}

std::string Editor::Title() const {
    if (!name_.empty()) return fs::path(name_).filename().string();
    return untitled_.empty() ? std::string("Untitled") : untitled_;
}

bool Editor::Save(std::string &err, bool make_backup) {
    if (name_.empty()) {
        err = "File has no name";
        return false;
    }
    std::error_code ec;
    // Copia de seguranca: o primeiro caractere da extensao vira o caractere de backup (READ.ME -> READ.{E), como no DOS.
    if (make_backup && !set_.delete_backup && fs::exists(name_, ec)) {
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
    // Com texto fora do buffer (arquivo em partes) o Save monta o arquivo novo ao lado e so' troca no fim: o original e' a fonte do que ainda nao foi lido
    const bool composed = Partial();
    const std::string target = composed ? name_ + ".fwtmp" : name_;
    long long before_tail = 0;
    {
        std::ofstream f(target, std::ios::binary | std::ios::trunc);
        if (!f) {
            err = "Cannot write " + target;
            return false;
        }
        if (bom_) f.write("\xEF\xBB\xBF", 3);
        if (!spill_.path.empty() && !AppendFile(f, spill_.path, 0)) {
            err = "Cannot read the temporary file " + spill_.path;
            return false;
        }
        const char *eol = crlf_ ? "\r\n" : "\n";
        for (size_t i = 0; i < lines_.size(); ++i) {
            if (i) f.write(eol, static_cast<std::streamsize>(std::char_traits<char>::length(eol)));
            f.write(lines_[i].data(), static_cast<std::streamsize>(lines_[i].size()));
        }
        before_tail = static_cast<long long>(f.tellp());
        if (has_tail_ && !AppendFile(f, tail_src_, tail_off_)) {
            err = "Cannot read the rest of " + tail_src_;
            return false;
        }
        if (eof_ctrl_z_) f.put('\x1A');
        if (!f) {
            err = "Write error on " + target;
            return false;
        }
    }
    if (composed) {
        fs::rename(target, name_, ec);
        if (ec) {
            err = "Cannot replace " + name_ + ": " + ec.message();
            return false;
        }
        if (has_tail_) { // o resto agora esta' no arquivo novo, depois do que foi gravado
            tail_src_ = name_;
            tail_off_ = before_tail;
            tail_size_ = static_cast<long long>(fs::file_size(name_, ec));
        }
    }
    modified_ = false;
    ++save_count_;
    if (make_backup) autosaved_ = false; // um Save manual refaz a copia na proxima vez que o autosave gravar
    return true;
}

// File > Load more: acrescenta ao buffer a proxima parte do arquivo (veja Load). So' vale se o arquivo nao mudou desde que foi aberto.
void Editor::LoadMoreCmd() {
    if (!has_tail_) {
        message_ = "Nothing more to load";
        return;
    }
    std::error_code ec;
    const long long size = static_cast<long long>(fs::file_size(tail_src_, ec));
    if (ec || size != tail_size_) {
        message_ = "The file changed on disk: cannot load more";
        return;
    }
    const long long remain = tail_size_ - tail_off_;
    const long long want = std::min(remain, set_.load_limit > 0 ? static_cast<long long>(set_.load_limit) : remain);
    std::string chunk, err;
    if (!ReadRange(tail_src_, tail_off_, want, chunk, err)) {
        message_ = err;
        return;
    }
    const bool last = want >= remain;
    size_t cut = chunk.size();
    bool ctrl_z = false;
    if (last) {
        const size_t z = chunk.find('\x1A');
        if (z != std::string::npos) {
            ctrl_z = true;
            chunk.erase(z);
        }
        cut = chunk.size();
    } else {
        const size_t nl = chunk.rfind('\n');
        if (nl != std::string::npos && nl > 1) { // nl <= 1: so' o fim de linha que ja' abre o bloco (uma linha gigante): leva o bloco todo
            cut = nl;
            if (chunk[nl - 1] == '\r') cut = nl - 1;
        }
    }
    const std::vector<std::string> ls = SplitLines(chunk.substr(0, cut), crlf_);
    lines_.back() += ls[0]; // o 1o pedaco continua a ultima linha (vazio quando o bloco comeca no fim de linha)
    lines_.insert(lines_.end(), ls.begin() + 1, ls.end());
    tail_off_ += static_cast<long long>(cut);
    if (last) {
        has_tail_ = false;
        eof_ctrl_z_ = ctrl_z;
    }
    ++rev_;
    hl_states_.clear();
    message_ = (last ? "Loaded the rest: " : "Loaded ") + std::to_string(ls.size() - 1) + " more lines";
}

// File > Write to cursor: as linhas acima da do cursor vao para um temporario e saem do buffer. O Save junta tudo de volta.
void Editor::WriteToCursorCmd() {
    if (!Writable()) return;
    if (cur_.line == 0) {
        message_ = "Nothing above the cursor";
        return;
    }
    if (spill_.path.empty()) {
        static int counter = 0;
        spill_.path = (fs::temp_directory_path() / ("fwedit_top_" + std::to_string(++counter) + "_" + std::to_string(reinterpret_cast<uintptr_t>(this) % 100000) + ".tmp")).string();
        std::ofstream(spill_.path, std::ios::binary | std::ios::trunc);
    }
    std::ofstream f(spill_.path, std::ios::binary | std::ios::app);
    const char *eol = crlf_ ? "\r\n" : "\n";
    for (int i = 0; i < cur_.line; ++i) {
        f.write(lines_[static_cast<size_t>(i)].data(), static_cast<std::streamsize>(lines_[static_cast<size_t>(i)].size()));
        f.write(eol, static_cast<std::streamsize>(std::char_traits<char>::length(eol)));
    }
    if (!f) {
        message_ = "Cannot write the temporary file";
        return;
    }
    const int n = cur_.line;
    lines_.erase(lines_.begin(), lines_.begin() + n);
    cur_.line = 0;
    top_ = 0;
    m1_set_ = m2_set_ = sel_active_ = false;
    undo_.clear();
    redo_.clear();
    undel_valid_ = false;
    snip_stops_.clear();
    comp_open_ = false;
    hl_states_.clear();
    ++rev_;
    MoveTo(cur_);
    message_ = "Wrote " + std::to_string(n) + " lines to disk";
}

// File > Top of file: traz de volta o que o Write to cursor tirou e vai para o inicio.
void Editor::TopOfFileCmd() {
    if (spill_.path.empty()) {
        message_ = "Nothing was written (use Write to cursor first)";
        return;
    }
    std::string data, err;
    if (!ReadAll(spill_.path, data, err)) {
        message_ = err;
        return;
    }
    std::vector<std::string> ls = SplitLines(data, crlf_);
    if (!ls.empty() && ls.back().empty()) ls.pop_back(); // o fim de linha da ultima linha gravada
    lines_.insert(lines_.begin(), ls.begin(), ls.end());
    spill_.Drop();
    cur_ = {0, 0};
    top_ = 0;
    m1_set_ = m2_set_ = sel_active_ = false;
    undo_.clear();
    redo_.clear();
    undel_valid_ = false;
    hl_states_.clear();
    ++rev_;
    MoveTo(cur_);
    message_ = "Top of the file restored";
}

bool Editor::SaveAs(const std::string &path, std::string &err) {
    const std::string old = name_;
    name_ = path;
    if (!Save(err)) {
        name_ = old;
        return false;
    }
    if (!lang_manual_) DetectLanguage(); // um "sem nome" salvo como x.asm passa a ser Assembly
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

namespace {

// Linha que separa paragrafos: vazia, so' de espacos ou a quebra de pagina (form feed).
bool Separator(const std::string &s) {
    if (s == "\f") return true;
    for (const char c : s)
        if (c != ' ' && c != '\t') return false;
    return true;
}

// Reflui um paragrafo: as palavras (separadas por espacos/tabs) vao enchendo linhas de ate' `width` colunas de texto. A margem esquerda `left`
// vale para todas as linhas e a 1a ganha `first` colunas a mais (o recuo de paragrafo), que contam dentro da largura, como no Center line.
void Reflow(const std::vector<std::string> &para, int width, int first, int left, std::vector<std::string> &out) {
    std::vector<std::string> words;
    for (const std::string &l : para) {
        size_t i = 0;
        while (i < l.size()) {
            while (i < l.size() && (l[i] == ' ' || l[i] == '\t')) ++i;
            size_t j = i;
            while (j < l.size() && l[j] != ' ' && l[j] != '\t') ++j;
            if (j > i) words.push_back(l.substr(i, j - i));
            i = j;
        }
    }
    std::string cur;
    bool first_line = true;
    auto flush = [&]() {
        const int indent = std::max(0, left + (first_line ? first : 0));
        out.push_back(std::string(static_cast<size_t>(indent), ' ') + cur);
        cur.clear();
        first_line = false;
    };
    for (const std::string &w : words) {
        const int room = std::max(1, width - (first_line ? std::max(0, first) : 0));
        if (!cur.empty() && static_cast<int>(cur.size() + 1 + w.size()) > room) flush();
        if (!cur.empty()) cur.push_back(' ');
        cur += w;
    }
    if (!cur.empty()) flush();
}

} // namespace

// As linhas [a, b) viram os paragrafos reflowidos; as linhas separadoras ficam como estao. false se nada mudou.
bool Editor::FormatRange(int a, int b) {
    std::vector<std::string> out;
    int l = a;
    while (l < b) {
        const std::string &s = lines_[static_cast<size_t>(l)];
        if (Separator(s)) {
            out.push_back(s);
            ++l;
            continue;
        }
        std::vector<std::string> para;
        while (l < b && !Separator(lines_[static_cast<size_t>(l)])) para.push_back(lines_[static_cast<size_t>(l++)]);
        Reflow(para, set_.line_length, set_.first_margin, set_.left_margin, out);
    }
    bool same = static_cast<int>(out.size()) == b - a;
    for (int i = 0; same && i < b - a; ++i) same = out[static_cast<size_t>(i)] == lines_[static_cast<size_t>(a + i)];
    if (same) return false;
    std::string joined;
    for (size_t i = 0; i < out.size(); ++i) {
        if (i) joined.push_back('\n');
        joined += out[i];
    }
    DoDelete({a, 0}, {b - 1, static_cast<int>(lines_[static_cast<size_t>(b - 1)].size())});
    DoInsert({a, 0}, joined);
    return true;
}

void Editor::FormatParagraphCmd() {
    if (!Writable()) return;
    if (Separator(lines_[static_cast<size_t>(cur_.line)])) {
        message_ = "No paragraph here";
        return;
    }
    int a = cur_.line, b = cur_.line + 1;
    while (a > 0 && !Separator(lines_[static_cast<size_t>(a - 1)])) --a;
    while (b < LineCount() && !Separator(lines_[static_cast<size_t>(b)])) ++b;
    const int old_count = b - a;
    BeginGroup(false);
    // quantas linhas o paragrafo ocupa depois: o cursor segue para o proximo paragrafo, e Alt-E P repetido formata o texto todo, um a um
    const int before = LineCount();
    FormatRange(a, b);
    const int end = a + old_count + (LineCount() - before);
    int next = end;
    while (next < LineCount() && Separator(lines_[static_cast<size_t>(next)])) ++next;
    if (next >= LineCount()) next = std::max(0, LineCount() - 1);
    MoveTo({next, 0});
}

void Editor::ApplyFormatDocument(bool from_top) {
    if (!Writable()) return;
    int a = 0;
    if (!from_top) {
        a = cur_.line;
        while (a > 0 && !Separator(lines_[static_cast<size_t>(a)]) && !Separator(lines_[static_cast<size_t>(a - 1)])) --a;
    }
    BeginGroup(false);
    const Pos keep = cur_;
    FormatRange(a, LineCount());
    cur_ = {std::min(keep.line, LineCount() - 1), 0};
    MoveTo(cur_);
}

// O Outline do NE mostrava so' as linhas que comecam na coluna 1. Hoje depende da linguagem: Markdown = os titulos (#), recuados pelo nivel;
// MSX-BASIC (todas as linhas tem numero) = os comentarios REM, que fazem o papel de titulo; Assembly = rotulos, constantes e diretivas
// (as linhas na coluna 1 que nao sao comentario); o resto = qualquer linha que comeca na coluna 1.
std::vector<OutlineEntry> Editor::Outline() const {
    std::vector<OutlineEntry> out;
    const std::string id = Language() ? Language()->Id() : std::string();
    const bool markdown = id == "markdown";
    const bool numbered = id.find("bas") != std::string::npos && id != "dignified";
    bool fence = false; // dentro de um bloco de codigo ``` do Markdown um # nao e' titulo
    for (int l = 0; l < LineCount(); ++l) {
        const std::string &s = lines_[static_cast<size_t>(l)];
        if (s.empty()) continue;
        if (markdown) {
            if (s.rfind("```", 0) == 0) fence = !fence;
            if (fence || s[0] != '#') continue;
            size_t lv = 0;
            while (lv < s.size() && s[lv] == '#') ++lv;
            size_t b = lv;
            while (b < s.size() && s[b] == ' ') ++b;
            if (lv > 6 || b == s.size()) continue;
            out.push_back({l, std::string((lv - 1) * 2, ' ') + s.substr(b)});
        } else if (numbered) {
            size_t i = 0;
            while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) ++i;
            if (i == 0) continue;
            size_t b = i;
            while (b < s.size() && s[b] == ' ') ++b;
            const bool rem = s.compare(b, 3, "REM") == 0 || s.compare(b, 3, "rem") == 0 || (b < s.size() && s[b] == '\'');
            if (rem) out.push_back({l, s});
        } else {
            if (s[0] == ' ' || s[0] == '\t') continue;
            if (id == "z80asm" && s[0] == ';') continue;
            out.push_back({l, s});
        }
    }
    return out;
}

} // namespace ed
