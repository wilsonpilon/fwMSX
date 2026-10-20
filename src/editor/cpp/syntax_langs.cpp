// fwMSX -- editor de textos: as linguagens do destaque de sintaxe (fase E4). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver syntax.h.
//
// Cada linguagem e' DADOS (listas de palavras) mais um varredor pequeno de uma linha. Compartilhado: a tabela de palavras (hash em NASM),
// a medida de identificador/numero/string (C) e a classificacao do numero (Fortran). As listas de MSX-BASIC, MSXBAS2ROM e NestorBASIC vem
// de syntax_words_gen.inc (gerado do man por tools/editor/gen_syntax_words.py); a BIOS e' a mesma tabela do montador (fwasm).
#include "syntax.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <memory>
#include <unordered_map>

extern "C" {
#include "../c/ed_syntax.h"
}
#include "../asm/ed_hash.h"

// a tabela da BIOS do fwasm (arrays estaticos kBiosLabels e kBiosVars)
#include "../../asm/cpp/bios_tables.inc"
#include "syntax_words_gen.inc"

namespace ed {

namespace {

// ---------------------------------------------------------------------------------------------------------------
// ferramentas comuns
// ---------------------------------------------------------------------------------------------------------------

class WordTable {
public:
    // `words` separadas por espaco (minusculas); `cls` e' a classe de todas.
    void Add(const std::string &words, uint8_t cls) {
        size_t i = 0;
        while (i < words.size()) {
            while (i < words.size() && words[i] == ' ') ++i;
            size_t j = i;
            while (j < words.size() && words[j] != ' ') ++j;
            if (j > i) AddOne(words.substr(i, j - i), cls);
            i = j;
        }
    }
    void AddOne(std::string w, uint8_t cls) {
        for (char &c : w) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        const unsigned h = ed_hash_ci(reinterpret_cast<const unsigned char *>(w.data()), static_cast<int>(w.size()));
        map_.emplace(h, Entry{w, cls});
        maxlen_ = std::max(maxlen_, w.size());
    }
    uint8_t Find(const char *s, int n) const {
        if (n <= 0 || static_cast<size_t>(n) > maxlen_) return kSynNone;
        const unsigned h = ed_hash_ci(reinterpret_cast<const unsigned char *>(s), n);
        const auto range = map_.equal_range(h);
        for (auto it = range.first; it != range.second; ++it) {
            const std::string &w = it->second.word;
            if (w.size() != static_cast<size_t>(n)) continue;
            bool same = true;
            for (int k = 0; k < n && same; ++k) same = std::tolower(static_cast<unsigned char>(s[k])) == static_cast<unsigned char>(w[static_cast<size_t>(k)]);
            if (same) return it->second.cls;
        }
        return kSynNone;
    }
    size_t MaxLen() const { return maxlen_; }

private:
    struct Entry {
        std::string word;
        uint8_t cls;
    };
    std::unordered_multimap<unsigned, Entry> map_;
    size_t maxlen_ = 0;
};

struct Out {
    std::vector<Span> &v;
    void Add(int from, int to, uint8_t cls) {
        if (to <= from || cls == kSynNone) return;
        if (!v.empty() && v.back().cls == cls && v.back().to == from) {
            v.back().to = to;
        } else {
            v.push_back({from, to, cls});
        }
    }
};

bool IsSpace(char c) {
    return c == ' ' || c == '\t';
}

int SkipSpaces(const char *s, int n, int i) {
    while (i < n && IsSpace(s[i])) ++i;
    return i;
}

// Numero valido (C mede, Fortran decide) a partir de `pos`: devolve o fim, ou `pos` se nao e' numero.
int NumberAt(const char *s, int n, int pos) {
    const int e = ed_hl_number_end(s, n, pos);
    if (e <= pos) return pos;
    return ed_number_kind(s + pos, e - pos) > 0 ? e : pos;
}

bool HasPrefixCi(const std::string &line, size_t at, const char *word) {
    const size_t n = std::strlen(word);
    if (line.size() < at + n) return false;
    for (size_t k = 0; k < n; ++k) {
        if (std::tolower(static_cast<unsigned char>(line[at + k])) != word[k]) return false;
    }
    return true;
}

std::string Lower(const std::string &s) {
    std::string r = s;
    for (char &c : r) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return r;
}

// ---------------------------------------------------------------------------------------------------------------
// texto puro
// ---------------------------------------------------------------------------------------------------------------

class PlainLang : public Lang {
public:
    PlainLang() : Lang("plain", "Texto", {"txt", "text", "doc", "log", "cfg", "ini"}) {}
    void Highlight(const std::string &, HlState &, std::vector<Span> &out) const override { out.clear(); }
};

// ---------------------------------------------------------------------------------------------------------------
// MSX-BASIC e seus dialetos
// ---------------------------------------------------------------------------------------------------------------

enum class BasicKind { Msx, Dignified, Bas2Rom, Nestor, Turbo };

const char *kBasicFunctions =
    "abs asc atn base bin$ cdbl chr$ cint cos csng csrlin cvd cvi cvs dskf eof exp fix fpos fre hex$ inkey$ inp input$ instr int left$ len loc lof log "
    "lpos mid$ mkd$ mki$ mks$ oct$ pad pdl peek pos right$ rnd sgn sin space$ sqr stick str$ strig string$ tan val varptr vpeek usr fn";
const char *kBasicOperators = "and or xor eqv imp mod not";

class BasicLang : public Lang {
public:
    BasicLang(const char *id, const char *name, std::vector<std::string> exts, BasicKind kind) : Lang(id, name, std::move(exts)), kind_(kind) {
        words_.Add(kGenBasicWords, kSynKeyword);
        words_.Add(kBasicFunctions, kSynKeyword2);
        words_.Add(kBasicOperators, kSynOperator);
        call_.Add(kGenCallExt, kSynDirective);
        if (kind == BasicKind::Dignified) {
            words_.Add("define declare func ret include keep exit endif loop true false", kSynDirective);
        }
        if (kind == BasicKind::Bas2Rom) {
            words_.Add("file text include", kSynDirective);
            cmd_.Add(kGenBas2RomCmds, kSynDirective);
        }
        if (kind == BasicKind::Nestor) {
            call_.Add(kGenNestorFuncs, kSynDirective);
            words_.Add(kGenNestorFuncs, kSynDirective);
        }
        if (kind == BasicKind::Turbo) {
            // Basic Kun / Turbo Basic: acrescenta os blocos estruturados (lista inicial; o usuario completa)
            words_.Add("while wend repeat until do loop exit turbo", kSynKeyword);
        }
    }
    bool MultiLine() const override { return kind_ == BasicKind::Dignified; }
    void Highlight(const std::string &line, HlState &st, std::vector<Span> &out) const override;

private:
    BasicKind kind_;
    WordTable words_;
    WordTable call_;
    WordTable cmd_;
};

void BasicLang::Highlight(const std::string &line, HlState &st, std::vector<Span> &out) const {
    out.clear();
    Out o{out};
    const char *s = line.data();
    const int n = static_cast<int>(line.size());
    int i = SkipSpaces(s, n, 0);
    const bool dig = kind_ == BasicKind::Dignified;

    if (dig) {
        // comentarios exclusivos (## e ###), blocos ('' e ###) e remtags (##BB:)
        const std::string t = line.substr(static_cast<size_t>(i));
        std::string tt = t;
        while (!tt.empty() && IsSpace(tt.back())) tt.pop_back();
        if (st.mode != 0) {
            o.Add(0, n, kSynComment);
            if ((st.mode == 1 && tt == "''") || (st.mode == 2 && tt == "###")) st.mode = 0;
            return;
        }
        if (tt == "''" || tt == "###") {
            o.Add(0, n, kSynComment);
            st.mode = tt == "''" ? 1 : 2;
            return;
        }
        if (t.rfind("##", 0) == 0) {
            const bool remtag = t.rfind("##BB:", 0) == 0 || t.rfind("##bb:", 0) == 0;
            o.Add(i, n, remtag ? kSynDirective : kSynComment);
            return;
        }
        // toggle de linha: #nome no inicio
        if (i < n && s[i] == '#' && i + 1 < n && ed_hl_ident_start(static_cast<unsigned char>(s[i + 1]))) {
            const int e = ed_hl_ident_end(s, n, i + 1, 0);
            o.Add(i, e, kSynDirective);
            i = e;
        }
    } else if (i < n && s[i] >= '0' && s[i] <= '9') {
        int e = i;
        while (e < n && s[e] >= '0' && s[e] <= '9') ++e;
        o.Add(i, e, kSynLineNo);
        i = e;
    }

    bool stmt_start = true; // inicio de instrucao: depois de ':', THEN, ELSE
    bool ref_mode = false;  // depois de GOTO/GOSUB/THEN/...: os numeros sao numeros de linha
    bool call_next = false;
    bool cmd_next = false;
    while (i < n) {
        const char c = s[i];
        if (IsSpace(c)) {
            ++i;
            continue;
        }
        if (c == '"') {
            const int e = ed_hl_string_end(s, n, i, '"');
            o.Add(i, e, kSynString);
            i = e;
            stmt_start = ref_mode = false;
            continue;
        }
        if (c == '\'') {
            o.Add(i, n, kSynComment);
            return;
        }
        if (c == ':') {
            stmt_start = true;
            ref_mode = false;
            ++i;
            continue;
        }
        if (c == ',' || c == '-') { // ON x GOTO 10,20,30  /  LIST 10-20: continuam a lista de linhas
            ++i;
            continue;
        }
        if (c == '?') {
            o.Add(i, i + 1, kSynKeyword); // ? = PRINT
            ++i;
            stmt_start = false;
            continue;
        }
        if (dig) {
            if (c == '{') { // {rotulo}
                const int close = static_cast<int>(line.find('}', static_cast<size_t>(i)));
                if (close != static_cast<int>(std::string::npos)) {
                    o.Add(i, close + 1, kSynLabel);
                    i = close + 1;
                    stmt_start = false;
                    continue;
                }
            }
            if (c == '}') { // fim de loop label
                o.Add(i, i + 1, kSynLabel);
                ++i;
                continue;
            }
            if (c == '[') { // [alias] de define (os colchetes podem se aninhar: [pk][poke 100,[10]])
                int depth = 0;
                int close = -1;
                for (int k = i; k < n; ++k) {
                    if (s[k] == '[') ++depth;
                    else if (s[k] == ']' && --depth == 0) {
                        close = k;
                        break;
                    }
                }
                if (close >= 0) {
                    o.Add(i, close + 1, kSynDirective);
                    i = close + 1;
                    continue;
                }
            }
            if (c == '~' && i + 1 < n && ed_hl_ident_start(static_cast<unsigned char>(s[i + 1]))) {
                const int e = ed_hl_ident_end(s, n, i + 1, ED_ID_SUFFIX);
                o.Add(i, e, kSynVariable);
                i = e;
                continue;
            }
            if (c == '.' && i + 1 < n && ed_hl_ident_start(static_cast<unsigned char>(s[i + 1]))) { // .funcao(...)
                const int e = ed_hl_ident_end(s, n, i + 1, ED_ID_SUFFIX);
                o.Add(i, e, kSynKeyword2);
                i = e;
                continue;
            }
        }
        if (c == '_' && stmt_start && i + 1 < n && ed_hl_ident_start(static_cast<unsigned char>(s[i + 1]))) { // _CMD = CALL CMD
            const int e = ed_hl_ident_end(s, n, i + 1, ED_ID_SUFFIX);
            o.Add(i, e, kSynDirective);
            i = e;
            stmt_start = false;
            continue;
        }
        if ((c >= '0' && c <= '9') || c == '&' || c == '$' || c == '#' || c == '%' || c == '.') {
            const int e = NumberAt(s, n, i);
            if (e > i) {
                o.Add(i, e, ref_mode ? kSynLabel : kSynNumber);
                i = e;
                stmt_start = false;
                continue;
            }
            ++i;
            continue;
        }
        if (ed_hl_ident_start(static_cast<unsigned char>(c))) {
            const int e = ed_hl_ident_end(s, n, i, ED_ID_SUFFIX);
            // listagens antigas sem espacos (FORI=1TO10, PRINTA$): so' se a palavra esta' toda em MAIUSCULAS e nao e' uma palavra conhecida
            bool upper_run = true;
            for (int k = i; k < e; ++k) {
                if (s[k] >= 'a' && s[k] <= 'z') upper_run = false;
            }
            uint8_t cls = words_.Find(s + i, e - i);
            if (call_next) {
                o.Add(i, e, kSynDirective);
                call_next = false;
                i = e;
                continue;
            }
            if (cmd_next) {
                cmd_next = false;
                const uint8_t cc = cmd_.Find(s + i, e - i);
                if (cc) {
                    o.Add(i, e, cc);
                    i = e;
                    continue;
                }
            }
            if (cls == kSynNone && kind_ == BasicKind::Msx && upper_run && e - i > 2) {
                // divide a corrida pelo prefixo conhecido mais longo, como o tokenizador do MSX-BASIC
                int p = i;
                bool after_kw = false;
                while (p < e) {
                    if (after_kw && s[p] >= '0' && s[p] <= '9') { // TO10, GOTO100: o numero que segue a palavra
                        const int ne = NumberAt(s, e, p);
                        if (ne > p) {
                            o.Add(p, ne, ref_mode ? kSynLabel : kSynNumber);
                            p = ne;
                            after_kw = false;
                            continue;
                        }
                    }
                    int found = 0;
                    uint8_t fc = kSynNone;
                    const int maxl = static_cast<int>(std::min<size_t>(words_.MaxLen(), static_cast<size_t>(e - p)));
                    for (int l = maxl; l >= 2; --l) {
                        fc = words_.Find(s + p, l);
                        if (fc) {
                            found = l;
                            break;
                        }
                    }
                    if (found) {
                        o.Add(p, p + found, fc);
                        p += found;
                        after_kw = true;
                    } else {
                        ++p;
                        after_kw = false;
                    }
                }
                i = e;
                stmt_start = false;
                continue;
            }
            const std::string w = Lower(line.substr(static_cast<size_t>(i), static_cast<size_t>(e - i)));
            if (w == "rem") {
                o.Add(i, n, kSynComment);
                return;
            }
            if (dig && e < n && s[e] == '{') { // nome{ abre um loop label
                o.Add(i, e + 1, kSynLabel);
                i = e + 1;
                stmt_start = false;
                continue;
            }
            o.Add(i, e, cls);
            if (w == "call") call_next = true;
            if (w == "cmd") cmd_next = true;
            stmt_start = (w == "then" || w == "else");
            ref_mode = w == "goto" || w == "gosub" || w == "then" || w == "else" || w == "restore" || w == "resume" || w == "return" || w == "run" ||
                       w == "list" || w == "llist" || w == "delete" || w == "renum";
            i = e;
            continue;
        }
        ++i;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// Assembly Z80 (asMSX / fwasm)
// ---------------------------------------------------------------------------------------------------------------

class AsmLang : public Lang {
public:
    AsmLang() : Lang("z80asm", "Assembly Z80", {"asm", "z80", "s", "inc", "mac", "asmsx"}) {
        words_.Add("adc add and bit call ccf cp cpd cpdr cpi cpir cpl daa dec di djnz ei ex exx halt im in inc ind indr ini inir jp jr ld ldd lddr ldi ldir "
                   "neg nop or otdr otir out outd outi pop push res ret reti retn rl rla rlc rlca rld rr rra rrc rrca rrd rst sbc scf set sla sra srl sub xor "
                   "sll sli mulub muluw",
                   kSynKeyword);
        words_.Add("a b c d e h l f i r af af' bc de hl sp ix iy ixh ixl iyh iyl nz z nc po pe p m", kSynRegister);
        words_.Add("org page db defb dt deft dw defw ds defs byte word zilog bios biosvars phase dephase incbin print printhex printfix printtext printdec "
                   "printstring filename rom megarom basic msxdos start search subpage select callbios calldos size debug break breakpoint cas cassette wav "
                   "sinclair equ rept endr macro endm include if else endif ifdef ifndef align skip fill end at tsx",
                   kSynDirective);
        for (const auto &e : kBiosLabels) words_.AddOne(e.name, kSynBios);
        for (const auto &e : kBiosVars) words_.AddOne(e.name, kSynBios);
        // konami konamiscc ascii8 ascii16 (tipos de MegaROM) e as funcoes de expressao
        words_.Add("konami konamiscc ascii8 ascii16", kSynKeyword2);
        words_.Add("sin cos tan acos asin atan sqr sqrt exp pow log ln abs fix fixmul fixdiv int pi random", kSynKeyword2);
    }
    bool MultiLine() const override { return true; } // /* ... */
    void Highlight(const std::string &line, HlState &st, std::vector<Span> &out) const override;

private:
    WordTable words_;
};

void AsmLang::Highlight(const std::string &line, HlState &st, std::vector<Span> &out) const {
    out.clear();
    Out o{out};
    const char *s = line.data();
    const int n = static_cast<int>(line.size());
    int i = 0;
    if (st.mode == 1) { // dentro de /* */
        const size_t close = line.find("*/");
        if (close == std::string::npos) {
            o.Add(0, n, kSynComment);
            return;
        }
        o.Add(0, static_cast<int>(close) + 2, kSynComment);
        i = static_cast<int>(close) + 2;
        st.mode = 0;
    }
    // rotulo na coluna 0: nome com ':' ou um nome que nao e' instrucao nem diretiva
    if (i == 0 && n > 0 && (ed_hl_ident_start(static_cast<unsigned char>(s[0])) || s[0] == '.' || s[0] == '@')) {
        int b = 0;
        const bool dotted = s[0] == '.';
        if (dotted || s[0] == '@') b = 1;
        int e = ed_hl_ident_end(s, n, b, ED_ID_DOT | ED_ID_AT);
        if (e > b) {
            const uint8_t cls = words_.Find(s + b, e - b);
            const bool colon = e < n && s[e] == ':';
            const bool directive_dot = dotted && cls == kSynDirective;
            if (colon || (cls == kSynNone && !directive_dot) || (dotted && !directive_dot)) {
                o.Add(0, e, kSynLabel);
                i = e;
            }
        }
    }
    bool after_ident = false; // para distinguir % de modulo e % de binario
    while (i < n) {
        const char c = s[i];
        if (IsSpace(c)) {
            ++i;
            continue;
        }
        if (c == ';' || (c == '/' && i + 1 < n && s[i + 1] == '/')) {
            o.Add(i, n, kSynComment);
            return;
        }
        if (c == '/' && i + 1 < n && s[i + 1] == '*') {
            const size_t close = line.find("*/", static_cast<size_t>(i) + 2);
            if (close == std::string::npos) {
                o.Add(i, n, kSynComment);
                st.mode = 1;
                return;
            }
            o.Add(i, static_cast<int>(close) + 2, kSynComment);
            i = static_cast<int>(close) + 2;
            continue;
        }
        if (c == '"' || c == '\'') {
            const int e = ed_hl_string_end(s, n, i, c);
            o.Add(i, e, kSynString);
            i = e;
            after_ident = true;
            continue;
        }
        if ((c >= '0' && c <= '9') || c == '$' || c == '#' || (c == '%' && !after_ident) || c == '&') {
            const int e = NumberAt(s, n, i);
            if (e > i) {
                o.Add(i, e, kSynNumber);
                i = e;
                after_ident = true;
                continue;
            }
            ++i;
            after_ident = false;
            continue;
        }
        const bool dot = c == '.' && i + 1 < n && ed_hl_ident_start(static_cast<unsigned char>(s[i + 1]));
        if (dot || ed_hl_ident_start(static_cast<unsigned char>(c)) || c == '@') {
            const int b = (dot || c == '@') ? i + 1 : i;
            const int e = ed_hl_ident_end(s, n, b, ED_ID_DOT | ED_ID_AT | ED_ID_QUOTE);
            if (e > b) {
                uint8_t cls = words_.Find(s + b, e - b);
                if (dot && cls != kSynDirective) cls = kSynLabel; // .local
                if (c == '@' ) cls = kSynLabel;
                o.Add(i, e, cls);
                i = e;
                after_ident = true;
                continue;
            }
        }
        after_ident = c == ')';
        ++i;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// Pascal (Turbo Pascal 4 / Kizuna WIRTH80)
// ---------------------------------------------------------------------------------------------------------------

class PascalLang : public Lang {
public:
    PascalLang() : Lang("pascal", "Pascal", {"pas", "pp", "p", "dpr"}) {
        words_.Add("and array as asm begin case const constructor destructor div do downto else end except exports file finally for function goto if "
                   "implementation in inherited inline interface label library mod nil not object of on or packed private procedure program property protected "
                   "public published raise record repeat set shl shr then threadvar to try type unit until uses var while with xor forward external bank exit",
                   kSynKeyword);
        words_.Add("integer byte word boolean char real longint shortint cardinal string text pointer double single extended comp int64 smallint",
                   kSynKeyword2);
        words_.Add("true false maxint", kSynNumber);
    }
    bool MultiLine() const override { return true; }
    void Highlight(const std::string &line, HlState &st, std::vector<Span> &out) const override;

private:
    WordTable words_;
};

void PascalLang::Highlight(const std::string &line, HlState &st, std::vector<Span> &out) const {
    out.clear();
    Out o{out};
    const char *s = line.data();
    const int n = static_cast<int>(line.size());
    int i = 0;
    // mode 1 = dentro de { }, 2 = dentro de (* *), 3 = dentro de {$ ... }
    auto close_comment = [&]() {
        const char *tok = st.mode == 2 ? "*)" : "}";
        const size_t close = line.find(tok, static_cast<size_t>(i));
        const uint8_t cls = st.mode == 3 ? kSynDirective : kSynComment;
        if (close == std::string::npos) {
            o.Add(i, n, cls);
            i = n;
            return;
        }
        const int e = static_cast<int>(close) + static_cast<int>(std::strlen(tok));
        o.Add(i, e, cls);
        i = e;
        st.mode = 0;
    };
    if (st.mode != 0) close_comment();
    while (i < n) {
        const char c = s[i];
        if (IsSpace(c)) {
            ++i;
            continue;
        }
        if (c == '{') {
            st.mode = (i + 1 < n && s[i + 1] == '$') ? 3 : 1;
            close_comment();
            continue;
        }
        if (c == '(' && i + 1 < n && s[i + 1] == '*') {
            st.mode = 2;
            const int start = i;
            i += 2;
            const size_t close = line.find("*)", static_cast<size_t>(i));
            if (close == std::string::npos) {
                o.Add(start, n, kSynComment);
                i = n;
            } else {
                o.Add(start, static_cast<int>(close) + 2, kSynComment);
                i = static_cast<int>(close) + 2;
                st.mode = 0;
            }
            continue;
        }
        if (c == '/' && i + 1 < n && s[i + 1] == '/') {
            o.Add(i, n, kSynComment);
            return;
        }
        if (c == '\'') {
            const int e = ed_hl_string_end(s, n, i, '\'');
            o.Add(i, e, kSynString);
            i = e;
            continue;
        }
        if ((c >= '0' && c <= '9') || c == '$' || (c == '#' && i + 1 < n && s[i + 1] >= '0' && s[i + 1] <= '9')) {
            if (c == '#') { // #13 (caractere por codigo)
                int e = i + 1;
                while (e < n && s[e] >= '0' && s[e] <= '9') ++e;
                o.Add(i, e, kSynString);
                i = e;
                continue;
            }
            const int e = NumberAt(s, n, i);
            if (e > i) {
                o.Add(i, e, kSynNumber);
                i = e;
                continue;
            }
            ++i;
            continue;
        }
        if (ed_hl_ident_start(static_cast<unsigned char>(c))) {
            const int e = ed_hl_ident_end(s, n, i, 0);
            o.Add(i, e, words_.Find(s + i, e - i));
            i = e;
            continue;
        }
        ++i;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// .BAT / .BTM (MSX-DOS 2 e 4DOS)
// ---------------------------------------------------------------------------------------------------------------

class BatLang : public Lang {
public:
    BatLang() : Lang("bat", "Batch (.BAT/.BTM)", {"bat", "btm", "cmd"}) {
        words_.Add("if else not exist defined errorlevel goto call for in do set echo pause shift cd chdir md mkdir rd rmdir del erase copy xcopy move ren rename "
                   "type cls dir path prompt ver vol verify date time exit setlocal endlocal title start more find sort choice mode format chkdsk label "
                   "iff then elseif endiff gosub return switch case endswitch default enddo quit cancel beep input inkey screen keystack on off lh loadhigh "
                   "attrib basic fixdisk romdisk ramdisk mapper cdir ",
                   kSynKeyword);
    }
    void Highlight(const std::string &line, HlState &st, std::vector<Span> &out) const override;

private:
    WordTable words_;
};

void BatLang::Highlight(const std::string &line, HlState &, std::vector<Span> &out) const {
    out.clear();
    Out o{out};
    const char *s = line.data();
    const int n = static_cast<int>(line.size());
    int i = SkipSpaces(s, n, 0);
    if (i < n && s[i] == '@') {
        o.Add(i, i + 1, kSynOperator);
        i = SkipSpaces(s, n, i + 1);
    }
    if (i + 1 < n && s[i] == ':' && s[i + 1] == ':') { // :: comentario
        o.Add(i, n, kSynComment);
        return;
    }
    if (i < n && s[i] == ':') { // :rotulo
        int e = i + 1;
        while (e < n && !IsSpace(s[e])) ++e;
        o.Add(i, e, kSynLabel);
        i = e;
    }
    if (HasPrefixCi(line, static_cast<size_t>(i), "rem") && (i + 3 >= n || IsSpace(s[i + 3]))) {
        o.Add(i, n, kSynComment);
        return;
    }
    while (i < n) {
        const char c = s[i];
        if (IsSpace(c)) {
            ++i;
            continue;
        }
        if (c == '"') {
            const int e = ed_hl_string_end(s, n, i, '"');
            o.Add(i, e, kSynString);
            i = e;
            continue;
        }
        if (c == '%') { // %VAR%  %1  %%i  %*
            int e = i + 1;
            if (e < n && s[e] == '%') ++e;
            if (e < n && (s[e] == '*' || (s[e] >= '0' && s[e] <= '9'))) {
                ++e;
            } else {
                const int start = e;
                while (e < n && s[e] != '%' && !IsSpace(s[e])) ++e;
                if (e < n && s[e] == '%' && e > start) ++e;
            }
            o.Add(i, e, kSynVariable);
            i = e;
            continue;
        }
        if (c >= '0' && c <= '9') {
            int e = i;
            while (e < n && s[e] >= '0' && s[e] <= '9') ++e;
            o.Add(i, e, kSynNumber);
            i = e;
            continue;
        }
        if (ed_hl_ident_start(static_cast<unsigned char>(c))) {
            const int e = ed_hl_ident_end(s, n, i, 0);
            o.Add(i, e, words_.Find(s + i, e - i));
            i = e;
            continue;
        }
        if (c == '>' || c == '<' || c == '|' || c == '&') {
            o.Add(i, i + 1, kSynOperator);
        }
        ++i;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// Markdown
// ---------------------------------------------------------------------------------------------------------------

class MarkdownLang : public Lang {
public:
    MarkdownLang() : Lang("markdown", "Markdown", {"md", "markdown"}) {}
    bool MultiLine() const override { return true; }
    void Highlight(const std::string &line, HlState &st, std::vector<Span> &out) const override;
};

void MarkdownLang::Highlight(const std::string &line, HlState &st, std::vector<Span> &out) const {
    out.clear();
    Out o{out};
    const char *s = line.data();
    const int n = static_cast<int>(line.size());
    int i = SkipSpaces(s, n, 0);
    const bool fence = line.compare(static_cast<size_t>(i), 3, "```") == 0 || line.compare(static_cast<size_t>(i), 3, "~~~") == 0;
    if (st.mode == 1) { // dentro de um bloco de codigo
        o.Add(0, n, kSynString);
        if (fence) st.mode = 0;
        return;
    }
    if (fence) {
        o.Add(0, n, kSynDirective);
        st.mode = 1;
        return;
    }
    if (i < n && s[i] == '#') { // titulo
        int e = i;
        while (e < n && s[e] == '#') ++e;
        if (e - i <= 6 && (e >= n || IsSpace(s[e]))) {
            o.Add(0, n, kSynHeading);
            return;
        }
    }
    if (i < n && s[i] == '>') {
        o.Add(0, n, kSynComment);
        return;
    }
    // linha horizontal
    {
        std::string t;
        for (int k = i; k < n; ++k) {
            if (!IsSpace(s[k])) t.push_back(s[k]);
        }
        if (t.size() >= 3 && (t.find_first_not_of('-') == std::string::npos || t.find_first_not_of('*') == std::string::npos ||
                              t.find_first_not_of('_') == std::string::npos)) {
            o.Add(0, n, kSynOperator);
            return;
        }
    }
    // marcador de lista
    if (i < n && (s[i] == '-' || s[i] == '*' || s[i] == '+') && i + 1 < n && IsSpace(s[i + 1])) {
        o.Add(i, i + 1, kSynOperator);
        i += 1;
    } else {
        int e = i;
        while (e < n && s[e] >= '0' && s[e] <= '9') ++e;
        if (e > i && e < n && s[e] == '.' && e + 1 < n && IsSpace(s[e + 1])) {
            o.Add(i, e + 1, kSynOperator);
            i = e + 1;
        }
    }
    while (i < n) {
        const char c = s[i];
        if (c == '`') {
            const size_t close = line.find('`', static_cast<size_t>(i) + 1);
            const int e = close == std::string::npos ? n : static_cast<int>(close) + 1;
            o.Add(i, e, kSynString);
            i = e;
            continue;
        }
        if ((c == '*' || c == '_') && i + 1 < n && s[i + 1] == c) { // **negrito**
            const std::string mark(2, c);
            const size_t close = line.find(mark, static_cast<size_t>(i) + 2);
            if (close != std::string::npos) {
                o.Add(i, static_cast<int>(close) + 2, kSynKeyword2);
                i = static_cast<int>(close) + 2;
                continue;
            }
        }
        if (c == '[') { // [texto](url)
            const size_t close = line.find(']', static_cast<size_t>(i) + 1);
            if (close != std::string::npos) {
                o.Add(i, static_cast<int>(close) + 1, kSynLabel);
                i = static_cast<int>(close) + 1;
                if (i < n && s[i] == '(') {
                    const size_t pc = line.find(')', static_cast<size_t>(i));
                    if (pc != std::string::npos) {
                        o.Add(i, static_cast<int>(pc) + 1, kSynComment);
                        i = static_cast<int>(pc) + 1;
                    }
                }
                continue;
            }
        }
        ++i;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// registro
// ---------------------------------------------------------------------------------------------------------------

struct Registry {
    std::vector<std::unique_ptr<Lang>> owned;
    std::vector<const Lang *> all;
    Registry() {
        auto add = [&](Lang *l) {
            owned.emplace_back(l);
            all.push_back(l);
        };
        add(new PlainLang());
        add(new BasicLang("msxbasic", "MSX-BASIC", {"bas", "asc", "amx"}, BasicKind::Msx));
        add(new BasicLang("dignified", "Basic Dignified", {"dmx"}, BasicKind::Dignified));
        add(new BasicLang("msxbas2rom", "MSXBAS2ROM", {"mbas"}, BasicKind::Bas2Rom));
        add(new BasicLang("nestorbasic", "NestorBASIC", {"nbas"}, BasicKind::Nestor));
        add(new BasicLang("turbobasic", "Turbo Basic / Basic Kun", {"tbas", "bk"}, BasicKind::Turbo));
        add(new AsmLang());
        add(new PascalLang());
        add(new BatLang());
        add(new MarkdownLang());
    }
};

const Registry &Reg() {
    static const Registry r;
    return r;
}

} // namespace

const std::vector<const Lang *> &AllLangs() {
    return Reg().all;
}

const Lang *LangById(const std::string &id) {
    for (const Lang *l : Reg().all) {
        if (l->Id() == id) return l;
    }
    return nullptr;
}

const Lang *LangForFile(const std::string &path, const std::string &sample) {
    const size_t dot = path.find_last_of('.');
    const size_t sep = path.find_last_of("/\\");
    std::string ext;
    if (dot != std::string::npos && (sep == std::string::npos || dot > sep)) ext = Lower(path.substr(dot + 1));
    if (ext == "bas") {
        // o mesmo .bas pode ser MSX-BASIC, MSXBAS2ROM ou NestorBASIC: o inicio do texto decide
        const std::string t = Lower(sample.substr(0, 4096));
        bool bas2rom = false;
        size_t pos = 0;
        while (pos < t.size()) {
            size_t end = t.find('\n', pos);
            if (end == std::string::npos) end = t.size();
            std::string ln = t.substr(pos, end - pos);
            size_t a = 0;
            while (a < ln.size() && (ln[a] == ' ' || ln[a] == '\t')) ++a;
            ln = ln.substr(a);
            if (ln.rfind("file \"", 0) == 0 || ln.rfind("text \"", 0) == 0 || ln.rfind("include \"", 0) == 0) bas2rom = true;
            if (ln.find("cmd ") != std::string::npos) {
                const size_t c = ln.find("cmd ") + 4;
                size_t e = c;
                while (e < ln.size() && std::isalnum(static_cast<unsigned char>(ln[e]))) ++e;
                if (e > c && ln.substr(c, e - c).size() >= 3) {
                    const std::string w = ln.substr(c, e - c);
                    if (std::strstr(kGenBas2RomCmds, w.c_str()) != nullptr) bas2rom = true;
                }
            }
            pos = end + 1;
        }
        if (bas2rom) return LangById("msxbas2rom");
        if (t.find("nb_") != std::string::npos || t.find("nestorbasic") != std::string::npos) return LangById("nestorbasic");
        return LangById("msxbasic");
    }
    if (!ext.empty()) {
        for (const Lang *l : Reg().all) {
            for (const std::string &e : l->Extensions()) {
                if (e == ext) return l;
            }
        }
    }
    return LangById("plain");
}

const char *SynClassName(uint8_t cls) {
    static const char *names[] = {"none", "keyword", "keyword2", "directive", "comment", "string", "number", "label",
                                  "register", "operator", "lineno", "variable", "bios", "heading"};
    return cls < kSynClassCount ? names[cls] : "?";
}

} // namespace ed
