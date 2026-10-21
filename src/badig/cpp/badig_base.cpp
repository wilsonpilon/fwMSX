// fwMSX -- Basic Dignified: utilitarios com a semantica do Python, Position, Token, o registro de mensagens e a descricao da linguagem
// (as tabelas do badig_dignified.py e do badig_msx.py). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "badig_core.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <cstdint>
#include <unordered_map>

namespace badig {

namespace {

bool g_latin1 = true; // com -tr o fonte e' UTF-8 e os bytes 0x85/0xA0 podem ser parte de um caractere: nao sao espaco

bool IsAlpha(unsigned char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
bool IsDigit(unsigned char c) { return c >= '0' && c <= '9'; }

} // namespace

void SetLatin1Mode(bool latin1) { g_latin1 = latin1; }

// ---------------------------------------------------------------------------------------------------------------
// texto
// ---------------------------------------------------------------------------------------------------------------

bool IsPySpace(unsigned char c) {
    if (c == ' ' || (c >= 9 && c <= 13) || (c >= 0x1c && c <= 0x1f)) return true;
    return g_latin1 && (c == 0x85 || c == 0xa0);
}

std::string PyUpper(const std::string &s) {
    std::string r = s;
    for (char &c : r) {
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 32);
    }
    return r;
}

std::string PyLower(const std::string &s) {
    std::string r = s;
    for (char &c : r) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + 32);
    }
    return r;
}

std::string PyStrip(const std::string &s, const std::string &chars) {
    size_t a = 0, b = s.size();
    while (a < b && chars.find(s[a]) != std::string::npos) ++a;
    while (b > a && chars.find(s[b - 1]) != std::string::npos) --b;
    return s.substr(a, b - a);
}

std::string PyLStripWs(const std::string &s) {
    size_t a = 0;
    while (a < s.size() && IsPySpace(static_cast<unsigned char>(s[a]))) ++a;
    return s.substr(a);
}

std::string PyRStripWs(const std::string &s) {
    size_t b = s.size();
    while (b > 0 && IsPySpace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(0, b);
}

std::string PyStripWs(const std::string &s) { return PyRStripWs(PyLStripWs(s)); }

std::string PyExpandTabs(const std::string &s, int tab) {
    std::string r;
    int col = 0;
    for (const char c : s) {
        if (c == '\t') {
            if (tab > 0) {
                const int n = tab - (col % tab);
                r.append(static_cast<size_t>(n), ' ');
                col += n;
            }
        } else {
            r.push_back(c);
            ++col;
            if (c == '\n' || c == '\r') col = 0;
        }
    }
    return r;
}

std::string BaseName(const std::string &path) {
    const size_t p = path.find_last_of("/\\");
    return p == std::string::npos ? path : path.substr(p + 1);
}

std::string DirName(const std::string &path) {
    const size_t p = path.find_last_of("/\\");
    if (p == std::string::npos) return {};
    if (p == 0) return path.substr(0, 1);
    return path.substr(0, p);
}

std::string JoinPath(const std::string &dir, const std::string &name) {
    if (dir.empty()) return name;
    const char last = dir.back();
    if (last == '/' || last == '\\') return dir + name;
#ifdef _WIN32
    return dir + "\\" + name; // os.path.join do original
#else
    return dir + "/" + name;
#endif
}

bool IsAbsPath(const std::string &p) {
    if (p.empty()) return false;
    if (p[0] == '/' || p[0] == '\\') return true;
    return p.size() >= 3 && IsAlpha(static_cast<unsigned char>(p[0])) && p[1] == ':' && (p[2] == '/' || p[2] == '\\');
}

std::string PyRepr(const std::string &s, bool utf8_input) {
    // os pontos de codigo de `s`: bytes (latin1) ou UTF-8 decodificado
    std::vector<uint32_t> cps;
    if (utf8_input) {
        for (size_t i = 0; i < s.size();) {
            const unsigned char c = static_cast<unsigned char>(s[i]);
            int n = 1;
            uint32_t cp = c;
            if (c >= 0xF0 && (c & 0xF8) == 0xF0) n = 4, cp = c & 0x07;
            else if ((c & 0xF0) == 0xE0) n = 3, cp = c & 0x0F;
            else if ((c & 0xE0) == 0xC0) n = 2, cp = c & 0x1F;
            if (i + static_cast<size_t>(n) > s.size()) n = 1, cp = c;
            for (int k = 1; k < n; ++k) cp = (cp << 6) | (static_cast<unsigned char>(s[i + static_cast<size_t>(k)]) & 0x3F);
            cps.push_back(cp);
            i += static_cast<size_t>(n);
        }
    } else {
        for (const char ch : s) cps.push_back(static_cast<unsigned char>(ch));
    }

    const bool has_sq = s.find('\'') != std::string::npos;
    const bool has_dq = s.find('"') != std::string::npos;
    const uint32_t quote = (has_sq && !has_dq) ? '"' : '\'';
    std::string r;
    auto put_utf8 = [&](uint32_t cp) {
        if (cp < 0x80) r.push_back(static_cast<char>(cp));
        else if (cp < 0x800) {
            r.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            r.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp < 0x10000) {
            r.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            r.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            r.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            r.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            r.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            r.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            r.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    };
    for (const uint32_t cp : cps) {
        char b[16];
        if (cp == '\\') r += "\\\\";
        else if (cp == quote) {
            r.push_back('\\');
            r.push_back(static_cast<char>(cp));
        } else if (cp == '\t') r += "\\t";
        else if (cp == '\n') r += "\\n";
        else if (cp == '\r') r += "\\r";
        else if (cp < 0x20 || cp == 0x7f || (cp >= 0x80 && cp <= 0xa0) || cp == 0xad) {
            std::snprintf(b, sizeof b, "\\x%02x", cp);
            r += b;
        } else if (cp == 0x2028 || cp == 0x2029 || cp == 0xFEFF || (cp >= 0x200B && cp <= 0x200F) || (cp >= 0x202A && cp <= 0x202E) || (cp >= 0x2060 && cp <= 0x2064)) {
            std::snprintf(b, sizeof b, "\\u%04x", cp);
            r += b;
        } else {
            put_utf8(cp);
        }
    }
    return r;
}

// ---------------------------------------------------------------------------------------------------------------
// Position e Token
// ---------------------------------------------------------------------------------------------------------------

Position::Position(const CodePtr &dc, int lin_, int col_, int offset_) : d_code(dc), lin(lin_), col(col_ + 1), offset(offset_) {
    if (dc) {
        code_len = static_cast<int>(dc->size());
        if (lin_ >= 0 && lin_ < code_len) {
            text = (*dc)[static_cast<size_t>(lin_)].text;
            file = (*dc)[static_cast<size_t>(lin_)].file;
        }
    }
}

PosPtr Position::Copy(std::optional<int> lin_, std::optional<int> col_, std::optional<int> offset_) const {
    const int l = lin_.value_or(lin);
    const int c = col_.value_or(col);
    const int lead = static_cast<int>(text.size()) - static_cast<int>(PyLStripWs(text).size());
    const int o = offset_.value_or(lead);
    return std::make_shared<Position>(d_code, l, c, o);
}

Token::Token(const std::string &t, const std::string &v, PosPtr p) : tok(PyUpper(t)), val(v), pos(std::move(p)) {}

TokPtr Token::MakeInt(const std::string &t, long long v, PosPtr p) {
    auto k = std::make_shared<Token>(t, "", std::move(p));
    k->SetInt(v);
    return k;
}

std::string Token::UVal() const { return PyUpper(val); }
std::string Token::LVal() const { return PyLower(val); }

std::string Token::VarType() const {
    if (!val.empty()) {
        const char c = val.back();
        if (c == '%' || c == '!' || c == '#' || c == '$') return std::string(1, c);
    }
    return {};
}

std::string Token::VarName() const {
    std::string v = val;
    if (!v.empty() && (v.back() == '%' || v.back() == '!' || v.back() == '#' || v.back() == '$')) v.pop_back();
    return PyLower(v);
}

std::string Token::ValWithFile() const { return PyLower(VarName() + "@" + (pos ? pos->file : std::string())); }

TokPtr Token::Copy() const {
    auto k = std::make_shared<Token>();
    k->tok = tok;
    k->val = val;
    k->is_int = is_int;
    k->ival = ival;
    k->pos = pos ? pos->Copy() : nullptr;
    k->bad_cp = bad_cp;
    return k;
}

int DisplayCol(const Position &p) {
    if (g_latin1) return p.col;
    // -tr: o original le' o fonte como texto (um caractere = uma coluna); aqui as posicoes sao de bytes
    int cont = 0;
    const size_t upto = static_cast<size_t>(std::max(0, std::min(p.col - 1, static_cast<int>(p.text.size()))));
    for (size_t i = 0; i < upto; ++i) {
        if ((static_cast<unsigned char>(p.text[i]) & 0xC0) == 0x80) ++cont;
    }
    return p.col - cont;
}

// ---------------------------------------------------------------------------------------------------------------
// registro de mensagens
// ---------------------------------------------------------------------------------------------------------------

void Logger::Log(int lvl, const std::string &desc_in, const Token *tok, const char *bullet, bool show_file) {
    static const char *const kBullets[] = {"", "*** ", "  * ", "--- ", "  - ", "    ", "    ", "    ", "    "};
    bool error = false;
    std::string desc = desc_in;
    if (lvl == 1 && !bullet) {
        error = true;
        if (tok && tok->pos) {
            const int col = DisplayCol(*tok->pos) - tok->pos->offset - 1;
            desc += "\n    " + PyStripWs(tok->pos->text);
            desc += "\n    " + std::string(static_cast<size_t>(std::max(col, 0)), '-') + "^";
        }
        desc += "\n    Conversion stopped.";
    }

    std::string text;
    if (lvl <= 2 || error) {
        Diag d;
        d.level = lvl == 1 ? 1 : 2;
        if (tok && tok->pos) {
            d.file = tok->pos->file;
            d.line = tok->pos->lin;
            d.col = DisplayCol(*tok->pos);
        }
        d.text = desc_in;
        diags.push_back(d);
    }
    if (error && level == 0) {
        throw BadigStop();
    }
    if (level < lvl) return;

    const std::string b = bullet ? std::string(bullet) : std::string(kBullets[std::min(lvl, 8)]);
    std::string pos, file;
    if (tok && tok->pos) {
        pos = "(" + std::to_string(tok->pos->lin) + "," + std::to_string(DisplayCol(*tok->pos)) + "): ";
        if (error || show_file) file = BaseName(tok->pos->file) + ": ";
    }
    text = b + file + pos + desc;
    lines.push_back(text);
    if (lvl <= 2 && !diags.empty() && diags.back().text == desc_in) diags.back().formatted = text;
    if (error) throw BadigStop();
}

void Logger::Fatal(const std::string &desc, const Token *tok) {
    Log(1, desc, tok);
    throw BadigStop();
}

// ---------------------------------------------------------------------------------------------------------------
// arquivos
// ---------------------------------------------------------------------------------------------------------------

namespace {
SourceReader g_source_reader;
} // namespace

void SetSourceReader(SourceReader r) { g_source_reader = std::move(r); }

bool SourceReadable(const std::string &path) {
    std::string tmp;
    if (g_source_reader && g_source_reader(path, tmp)) return true;
    return static_cast<bool>(std::ifstream(path));
}

CodePtr LoadFile(const std::string &path, const std::string *text, int tab_length, Logger &log, const Token *tk) {
    auto code = std::make_shared<Code>();
    code->push_back({0, "PROGRAM", path});
    if (path.empty()) log.Fatal("File name not given.", tk);

    std::string data;
    if (text) {
        data = *text;
    } else if (g_source_reader && g_source_reader(path, data)) {
        // veio do projeto
    } else {
        std::ifstream f(path, std::ios::binary);
        if (!f) log.Fatal("File not found: " + path, tk);
        data.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    }
    // newlines universais do Python: \r\n e \r viram \n; cada linha guarda o seu terminador
    int n = 1;
    std::string cur;
    for (size_t i = 0; i < data.size(); ++i) {
        const char c = data[i];
        if (c == '\r') {
            if (i + 1 < data.size() && data[i + 1] == '\n') ++i;
            cur.push_back('\n');
            code->push_back({n++, PyExpandTabs(cur, tab_length), path});
            cur.clear();
        } else if (c == '\n') {
            cur.push_back('\n');
            code->push_back({n++, PyExpandTabs(cur, tab_length), path});
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) code->push_back({n++, PyExpandTabs(cur, tab_length), path});
    return code;
}

// ---------------------------------------------------------------------------------------------------------------
// OrderedMap
// ---------------------------------------------------------------------------------------------------------------

int OrderedMap::Find(const std::string &k) const {
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i].first == k) return static_cast<int>(i);
    }
    return -1;
}

void OrderedMap::Set(const std::string &k, const std::string &v) {
    const int i = Find(k);
    if (i >= 0) items[static_cast<size_t>(i)].second = v;
    else items.emplace_back(k, v);
}

// ---------------------------------------------------------------------------------------------------------------
// as palavras da linguagem
// ---------------------------------------------------------------------------------------------------------------

namespace {

const std::vector<std::string> kDInstrc = {"DEFINE", "DECLARE", "INCLUDE", "KEEP", "ENDIF", "FUNC", "RET", "EXIT"};
const std::vector<std::string> kDOperat = {"TRUE", "FALSE"};
const std::vector<std::string> kDSymbol = {"[", "]", "{", "}", "@", "~"};

const std::vector<std::string> kCInstrc = {
    "AS", "BASE", "BEEP", "BLOAD", "BSAVE", "CALL", "CIRCLE", "CLEAR", "CLOAD", "CLOSE", "CLS", "CMD", "COLOR", "CONT", "COPY", "CSAVE",
    "CSRLIN", "DEF", "DEFDBL", "DEFINT", "MAXFILES", "DEFSNG", "DEFSTR", "DIM", "DRAW", "DSKI", "END", "EQV", "ERASE", "ERR", "ERROR",
    "FIELD", "FILES", "FN", "FOR", "GET", "IF", "INPUT", "INTERVAL", "IMP", "IPL", "KILL", "LET", "LFILES", "LINE", "LOAD", "LOCATE",
    "LPRINT", "LSET", "MAX", "MERGE", "MOTOR", "NAME", "NEW", "NEXT", "OFF", "ON", "OPEN", "OUT", "OUTPUT", "PAINT", "POINT", "POKE",
    "PRESET", "PRINT", "PSET", "PUT", "READ", "RSET", "SAVE", "SCREEN", "SET", "SOUND", "STEP", "STOP", "SWAP", "TIME", "TO", "TROFF",
    "TRON", "USING", "VPOKE", "WAIT", "WIDTH", "\\?", "DEFUSR(\\d)?"};
const std::vector<std::string> kCFuncDl = {"ATTR\\$", "BIN\\$", "CHR\\$", "DSKO\\$", "HEX\\$", "INKEY\\$", "INPUT\\$", "LEFT\\$", "MID\\$",
                                           "MKD\\$", "MKI\\$", "MKS\\$", "OCT\\$", "RIGHT\\$", "SPACE\\$", "SPRITE\\$", "STR\\$", "STRING\\$"};
const std::vector<std::string> kCFuncNm = {"ABS", "ASC", "ATN", "CDBL", "CINT", "COS", "CSNG", "CVD", "CVI", "CVS", "DSKF", "EOF", "EXP", "FIX",
                                           "FPOS", "FRE", "INP", "INSTR", "INT", "KEY", "LEN", "LOC", "LOF", "LOG", "LPOS", "PAD", "PDL", "PEEK",
                                           "PLAY", "POS", "RND", "SGN", "SIN", "SPC", "SPRITE", "SQR", "STICK", "STRIG", "TAB", "TAN", "USR(\\d)?",
                                           "VAL", "VARPTR", "VDP", "VPEEK"};
const std::vector<std::string> kCJumpIn = {"RESTORE", "AUTO", "RENUM", "DELETE", "RESUME", "ERL", "ELSE", "RUN", "LIST", "LLIST", "GOTO", "RETURN", "THEN", "GOSUB"};
const std::vector<std::string> kCOperat = {"AND", "MOD", "NOT", "OR", "XOR"};
const std::vector<std::string> kCSymbol = {">", "=", "<", "+", "*", "/", "^", "\\", "-", ",", "(", ")", ";", "#"};
const std::vector<std::string> kCESymb = {"++", "--", "+=", "-=", "*=", "/=", "^="};

// remove o escape do regex de uma palavra da lista ("ATTR\$" -> "ATTR$", "\?" -> "?")
std::string Unescape(const std::string &w) {
    std::string r;
    for (size_t i = 0; i < w.size(); ++i) {
        if (w[i] == '\\' && i + 1 < w.size()) {
            r.push_back(w[i + 1]);
            ++i;
        } else {
            r.push_back(w[i]);
        }
    }
    return r;
}

struct Words {
    std::unordered_map<std::string, const char *> map;
    std::set<std::string> reserved;
    std::set<std::string> func_stop;

    void Add(const std::vector<std::string> &l, const char *grp) {
        for (const std::string &w : l) {
            if (w.find("(\\d)?") != std::string::npos) continue; // USR e DEFUSR tem tratamento proprio
            map.emplace(Unescape(w), grp);
        }
    }

    Words() {
        // a ordem dos grupos do original: instrucoes Dignified, operadores, simbolos Dignified, ##, _, ###, ., classicos
        Add(kDInstrc, "D_INSTRC");
        Add(kDOperat, "D_OPERAT");
        Add(kDSymbol, "D_SYMBOL");
        map.emplace("##", "D_LINREM");
        map.emplace("_", "D_INSTSP");
        map.emplace("###", "D_O_EREM");
        map.emplace(".", "D_PARTLS");
        Add(kCOperat, "C_OPERAT");
        Add(kCSymbol, "C_SYMBOL");
        Add(kCESymb, "C_E_SYMB");
        Add(kCFuncNm, "C_FUNCTS");
        Add(kCFuncDl, "C_FUNCTS");
        Add(kCJumpIn, "C_JUMPIN");
        Add(kCInstrc, "C_INSTRC");
        map.emplace("'", "C_LINREM");
        map.emplace("REM", "C_LINREM");
        map.emplace("DATA", "C_DATALN");
        map.emplace("''", "C_O_BREM");
        map.emplace("&", "C_PARTLS");
        map.emplace("&H", "C_PARTLS");
        map.emplace("&O", "C_PARTLS");
        map.emplace("&B", "C_PARTLS");
        map.emplace(":", "C_INSTSP");
        map.emplace("\"", "C_QUOTES");

        // c_reserved_kw: o original tira "\d" e depois qualquer "\" das palavras: "DEFUSR(\d)?" vira "DEFUSR()?" (que nunca casa com nada)
        auto reserve = [&](const std::vector<std::string> &l) {
            for (std::string w : l) {
                for (size_t p; (p = w.find("\\d")) != std::string::npos;) w.erase(p, 2);
                w.erase(std::remove(w.begin(), w.end(), '\\'), w.end());
                reserved.insert(w);
            }
        };
        reserve(kCInstrc);
        reserve(kCJumpIn);
        reserve(kCFuncDl);
        reserve(kCFuncNm);
        reserve(kCOperat);
        reserved.insert("DATA");
        // c_func_stop_kw: as listas CRUAS (com as barras)
        for (const auto *l : {&kCInstrc, &kCJumpIn, &kCFuncDl, &kCFuncNm}) func_stop.insert(l->begin(), l->end());
    }
};

const Words &W() {
    static const Words w;
    return w;
}

bool IsIdentTail(unsigned char c) { return IsAlpha(c) || IsDigit(c) || c == '_'; }

// ^(\d*\.?)?\d+((D|E)(\+|-)\d+)?([%#!])?$
bool IsDecimal(const std::string &s) {
    size_t i = 0;
    const size_t n = s.size();
    size_t a = 0;
    while (i < n && IsDigit(static_cast<unsigned char>(s[i]))) {
        ++i;
        ++a;
    }
    if (i < n && s[i] == '.') {
        ++i;
        size_t b = 0;
        while (i < n && IsDigit(static_cast<unsigned char>(s[i]))) {
            ++i;
            ++b;
        }
        if (b == 0) return false;
    } else if (a == 0) {
        return false;
    }
    if (i < n && (s[i] == 'd' || s[i] == 'D' || s[i] == 'e' || s[i] == 'E')) {
        ++i;
        if (i >= n || (s[i] != '+' && s[i] != '-')) return false;
        ++i;
        size_t e = 0;
        while (i < n && IsDigit(static_cast<unsigned char>(s[i]))) {
            ++i;
            ++e;
        }
        if (e == 0) return false;
    }
    if (i < n && (s[i] == '%' || s[i] == '#' || s[i] == '!')) ++i;
    return i == n;
}

bool IsRadix(const std::string &s) {
    if (s.size() < 3 || s[0] != '&') return false;
    const char k = static_cast<char>(s[1] | 0x20);
    for (size_t i = 2; i < s.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (k == 'h') {
            if (!(IsDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) return false;
        } else if (k == 'o') {
            if (c < '0' || c > '7') return false;
        } else if (k == 'b') {
            if (c != '0' && c != '1') return false;
        } else {
            return false;
        }
    }
    return k == 'h' || k == 'o' || k == 'b';
}

// ^[a-z][a-z_0-9]*$ (re.I) depois de `skip` caracteres
bool IsNameFrom(const std::string &s, size_t skip) {
    if (s.size() <= skip || !IsAlpha(static_cast<unsigned char>(s[skip]))) return false;
    for (size_t i = skip + 1; i < s.size(); ++i) {
        if (!IsIdentTail(static_cast<unsigned char>(s[i]))) return false;
    }
    return true;
}

} // namespace

const char *Description::Classify(const std::string &s) {
    if (s.empty()) return "";
    // principais: newline e espacos
    if (std::all_of(s.begin(), s.end(), [](char c) { return c == '\r'; })) return "NEWLINE";
    if (s.size() == 1 && s[0] != '\r' && s[0] != '\n' && IsPySpace(static_cast<unsigned char>(s[0]))) return "SPACES";

    const std::string u = PyUpper(s);
    const Words &w = W();
    // .nome (Dignified) vem antes das palavras classicas mas nao se sobrepoe a elas
    if (s[0] == '.' && s.size() > 1 && IsNameFrom(s, 1)) return "D_FUNCNM";
    const auto it = w.map.find(u);
    if (it != w.map.end()) {
        // um grupo com palavra igual mais cedo na ordem vence: o mapa ja' foi montado na ordem do original
        return it->second;
    }
    if (u.rfind("USR", 0) == 0 && (u.size() == 3 || (u.size() == 4 && IsDigit(static_cast<unsigned char>(u[3]))))) return "C_FUNCTS";
    if (u.rfind("DEFUSR", 0) == 0 && (u.size() == 6 || (u.size() == 7 && IsDigit(static_cast<unsigned char>(u[6]))))) return "C_INSTRC";
    if (s[0] == '#' && s.size() > 1 && IsNameFrom(s, 1)) return "D_TGLREM";
    if (IsDecimal(s) || IsRadix(s)) return "C_NUMBER";
    // identificador com tipo: ^[a-z][a-z_0-9]*[%!#$]?$
    if (IsAlpha(static_cast<unsigned char>(s[0]))) {
        std::string body = s;
        const char last = body.back();
        if (body.size() > 1 && (last == '%' || last == '!' || last == '#' || last == '$')) body.pop_back();
        if (IsNameFrom(body, 0)) return "D_IDNTTP";
    }
    return "";
}

bool Description::IsReserved(const std::string &upper) { return W().reserved.count(upper) != 0; }
bool Description::IsFuncStop(const std::string &upper) { return W().func_stop.count(upper) != 0; }

bool Description::IsSymbComp(const std::string &v) {
    static const std::set<std::string> k = {">", "=", "<", "+", "*", "/", "^", "", "-", ",", "(", ")", ";", ":", "_"};
    return k.count(v) != 0;
}

bool Description::IsIdentf(const std::string &s) { return IsNameFrom(s, 0); }

} // namespace badig
