// fwMSX -- montador (fwasm): o montador em si (passadas, simbolos, diretivas, instrucoes). Ver assembler.h.
#include "assembler.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <map>
#include <set>

#include "../core/z80_enc.h"
#include "expr.h"
#include "lexer.h"
#include "preproc.h"

namespace fwasm {

namespace {

#include "bios_tables.inc"

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// Palavras que NAO podem ser usadas como rotulo: erro de sintaxe (mnemonicos, registradores, condicoes, funcoes...).
const std::set<std::string> &SyntaxReserved() {
    static const std::set<std::string> s = {
        "a", "b", "c", "d", "e", "h", "l", "f", "i", "r", "af", "af'", "bc", "de", "hl", "sp", "ix", "iy", "ixh", "ixl", "iyh", "iyl",
        "nz", "z", "nc", "po", "pe", "p", "m",
        "adc", "add", "and", "bit", "call", "ccf", "cp", "cpd", "cpdr", "cpi", "cpir", "cpl", "daa", "dec", "di", "djnz", "ei", "ex",
        "exx", "halt", "im", "in", "inc", "ind", "indr", "ini", "inir", "jp", "jr", "ld", "ldd", "lddr", "ldi", "ldir", "neg", "nop",
        "or", "otdr", "otir", "out", "outd", "outi", "pop", "push", "res", "ret", "reti", "retn", "rl", "rla", "rlc", "rlca", "rld", "rr",
        "rra", "rrc", "rrca", "rrd", "rst", "sbc", "scf", "set", "sla", "sll", "sra", "srl", "sub", "xor",
        "abs", "acos", "asin", "atan", "cos", "exp", "fix", "fixdiv", "fixmul", "int", "ln", "log", "pi", "pow", "random", "sin", "sqr",
        "sqrt", "tan", "konami", "konamiscc", "ascii8", "ascii16", "xor", "and", "or"};
    return s;
}

// Diretivas: usadas como identificador em expressao dao "reserved word used as identifier".
const std::set<std::string> &DirectiveWords() {
    static const std::set<std::string> s = {
        "org", "page", "zilog", "bios", "biosvars", "rom", "megarom", "basic", "msxdos", "start", "search", "subpage", "select",
        "phase", "dephase", "callbios", "calldos", "size", "incbin", "debug", "break", "breakpoint", "printtext", "print", "printhex",
        "printfix", "cas", "cassette", "wav", "filename", "sinclair", "byte", "word", "skip", "at", "if", "ifdef"};
    return s;
}

struct Symbol {
    int32_t value = 0;
    enum Kind { Label, Const, Var } kind = Label;
};

class Assembler : public ExprContext {
public:
    Assembler(const std::string &name, const std::string &text, const AsmOptions &opts, FileProvider *files)
        : name_(name), text_(text), opts_(opts), files_(files) {}

    AsmResult Run();

    // ---- ExprContext ----
    int32_t Pc() const override { return static_cast<int32_t>(pc_); }
    bool Lookup(const std::string &name, int32_t &value, bool &unknown) override;
    bool IsSyntaxReserved(const std::string &lower) const override { return SyntaxReserved().count(lower) != 0; }
    bool IsDirectiveWord(const std::string &lower) const override { return DirectiveWords().count(lower) != 0; }
    int32_t Random(int32_t n) override {
        if (n <= 0) return 0;
        seed_ = seed_ * 1103515245u + 12345u;
        return static_cast<int32_t>((seed_ >> 8) % static_cast<uint32_t>(n));
    }

private:
    struct IfState {
        bool parent_active;
        bool taken;
        bool active;
        bool seen_else;
    };

    // ---- erros e avisos ----
    struct Stop {};
    [[noreturn]] void Fail(int rc, const std::string &msg) {
        if (!final_ || (dry_ && rc != 1)) {
            if (dry_) swallow_ = true;
            throw Stop{};
        }
        if (rc_ == 0) {
            rc_ = rc;
            msgs_.push_back(cur_file_ + ", line " + std::to_string(cur_line_) + ": " + msg);
        }
        throw Stop{};
    }
    [[noreturn]] void SyntaxError() {
        if (!final_) throw Stop{};
        if (rc_ == 0) {
            rc_ = 1;
            // erro do analisador sintatico: os avisos semanticos ja' dados (corte de 8 bits etc.) nao aparecem
            msgs_.erase(std::remove_if(msgs_.begin(), msgs_.end(),
                                       [](const std::string &m) {
                                           return m.find(": Warning: ") != std::string::npos && m.find("Warning: Parameter #") == std::string::npos;
                                       }),
                        msgs_.end());
            msgs_.push_back("Parsing error: syntax error");
            msgs_.push_back(cur_file_ + ", line " + std::to_string(cur_line_) + ": syntax error");
        }
        throw Stop{};
    }
    void Warn(const std::string &msg) {
        if (final_ && !dry_) msgs_.push_back(cur_file_ + ", line " + std::to_string(cur_line_) + ": Warning: " + msg);
    }
    void Crop(int bits) {
        Warn(std::to_string(bits) + "-bit overflow. Cropping to fit " + std::to_string(bits) + " bits. If it's a label, it may not be defined.");
    }

    // ---- passada ----
    void RunPass(bool final);
    void ProcessLine(const SrcLine &line);
    void Statement(const std::vector<Token> &t, size_t i);
    bool Directive(const std::string &name, const std::vector<Token> &t, size_t i);
    void Instruction(const std::string &mnem_name, const std::vector<Token> &t, size_t i);

    // ---- simbolos ----
    std::string LocalKey(const std::string &name) const { return parent_ + "\x01" + name; }
    bool IsLocalName(const std::string &n) const { return n.rfind("@@", 0) == 0 || (!n.empty() && n[0] == '.'); }
    void DefineLabel(const std::string &name);
    void DefineConst(const std::string &name, int32_t value);
    void DefineVar(const std::string &name, int32_t value);

    // ---- expressoes ----
    Val EvalAt(const std::vector<Token> &t, size_t &pos);
    int32_t EvalInt(const std::vector<Token> &t, size_t &pos, bool *unknown = nullptr);
    void ExpectEnd(const std::vector<Token> &t, size_t pos) {
        if (pos != t.size()) SyntaxError();
    }

    // ---- saida ----
    void Emit(uint8_t b) {
        if (dry_) return;
        if (real_ < -1 || real_ > 0xFFFF) Fail(2, "memory overflow");
        if (real_ == -1) {
            // `org -1`: o asMSX grava um byte ANTES do endereco 0 e a saida o inclui (abaixo disso e' lixo de heap
            // la' e nao da' pra reproduzir, entao continua sendo "memory overflow")
            neg1_ = b;
            neg_start_ = true;
            any_written_ = true;
            ++real_;
            ++pc_;
            return;
        }
        const uint32_t a = static_cast<uint32_t>(real_);
        mem_[a] = b;
        if (!any_written_ || a < min_) min_ = a;
        if (!any_written_ || a > max_) max_ = a;
        any_written_ = true;
        ++real_;
        ++pc_;
    }
    void Reserve(int32_t n) {
        if (dry_ || n <= 0) return;
        real_ += n;
        pc_ += n;
    }

    void Finish();
    void WriteSym();
    void WriteTxt();

    std::string name_;
    std::string text_;
    AsmOptions opts_;
    FileProvider *files_;

    std::vector<SrcLine> lines_;
    std::vector<std::string> pre_warnings_;

    bool final_ = false;
    int rc_ = 0;
    std::vector<std::string> msgs_;
    std::string cur_file_;
    int cur_line_ = 0;

    // simbolos
    std::map<std::string, Symbol> table_;                 // chave: nome (global) ou LocalKey (local)
    std::map<std::string, Symbol> prev_table_;            // tabela da passada anterior (referencias adiante)
    struct LabelRec {
        std::string display;
        int32_t value;
    };
    std::vector<LabelRec> label_order_;                   // rotulos na ordem de definicao (so' na ultima passada)
    std::vector<std::string> const_order_, var_order_;
    std::set<std::string> defined_so_far_;                // para IFDEF
    std::map<std::string, int32_t> predefined_;           // .BIOS / .BIOSVARS
    std::string parent_;
    bool changed_ = false;
    bool used_forward_ = false; // a ultima instrucao usou um simbolo definido mais adiante

    // estado do codigo
    int64_t pc_ = 0;   // endereco virtual ($, rotulos); sem volta em 16 bits: passar de FFFFh e' "memory overflow" ao emitir
    int64_t real_ = 0; // endereco fisico (onde os bytes vao)
    bool has_res_ = false;
    uint32_t res_min_ = 0;
    bool dotted_ = false;  // a diretiva em curso foi escrita com ponto
    std::map<std::string, int32_t> prev_vars_; // variaveis ao fim da passada anterior (referencias adiante)
    bool dry_ = false;     // regiao inativa de IF: so' a sintaxe e' conferida
    bool swallow_ = false; // erro semantico engolido no modo seco
    std::map<std::string, int32_t> vars_; // variaveis (NOME = expressao): espaco proprio
    std::set<std::string> ifdef_defs_;    // constantes e variaveis ja' definidas (IFDEF so' as enxerga)
    bool phase_ = false;
    std::string filename_;   // .FILENAME: nome-base das saidas (o -o do CLI tem prioridade)
    int32_t start_addr_ = 0; // .START (ainda sem uso: nao ha' ROM/BASIC)
    bool zilog_ = false;
    bool zilog_msg_ = false;
    std::vector<IfState> ifs_;
    uint32_t seed_ = 12345;

    // memoria
    std::vector<uint8_t> mem_ = std::vector<uint8_t>(65536 + 16, 0);
    std::vector<bool> written_ = std::vector<bool>(65536 + 16, false);
    bool any_written_ = false;
    bool neg_start_ = false; // houve byte emitido em -1 (org -1)
    uint8_t neg1_ = 0;
    uint32_t min_ = 0, max_ = 0;
    bool ds_at_zero_ = false;

    std::vector<std::string> txt_lines_;
    AsmResult result_;
};

bool Assembler::Lookup(const std::string &name, int32_t &value, bool &unknown) {
    unknown = false;
    if (dry_) {
        value = 0;
        unknown = true;
        return true;
    }
    {
        const auto v = vars_.find(name);
        if (v != vars_.end()) {
            value = v->second;
            return true;
        }
        const auto pv = prev_vars_.find(name);
        if (pv != prev_vars_.end() && !table_.count(name)) {
            value = pv->second;
            used_forward_ = true;
            return true;
        }
    }
    const std::map<std::string, Symbol> &prev = prev_table_;
    auto find = [&](const std::string &key, int32_t &v) {
        auto it = table_.find(key);
        if (it != table_.end()) {
            v = it->second.value;
            return true;
        }
        auto jt = prev.find(key);
        if (jt != prev.end()) {
            v = jt->second.value;
            used_forward_ = true; // simbolo definido mais adiante
            return true;
        }
        return false;
    };
    if (IsLocalName(name)) {
        if (find(LocalKey(name), value)) return true;
        if (!final_) {
            unknown = true;
            value = 0;
            return true;
        }
        Fail(14, "undefined local label");
    }
    if (find(name, value)) return true;
    const auto p = predefined_.find(name);
    if (p != predefined_.end()) {
        value = p->second;
        return true;
    }
    if (!final_) {
        unknown = true;
        value = 0;
        return true;
    }
    return false;
}

Val Assembler::EvalAt(const std::vector<Token> &t, size_t &pos) {
    Val v;
    ExprErr err = ExprErr::None;
    ExprContext &ctx = *this;
    zilog = zilog_;
    if (!ParseExpression(t, pos, ctx, v, err)) {
        // O asMSX analisa a sintaxe da instrucao inteira antes de olhar simbolos: se o que vem depois da
        // expressao nao pode continuar uma expressao/operando, o erro e' de SINTAXE, nao de simbolo.
        if (err == ExprErr::Undefined || err == ExprErr::DivZero || err == ExprErr::Reserved) {
            const Token *nx = pos < t.size() ? &t[pos] : nullptr;
            if (nx && (nx->kind == Token::Int || nx->kind == Token::Float || nx->kind == Token::Char || nx->kind == Token::String ||
                       (nx->kind == Token::Op && nx->text == "("))) {
                SyntaxError();
            }
        }
        switch (err) {
        case ExprErr::Undefined: Fail(13, "undefined identifier");
        case ExprErr::DivZero: Fail(2, "memory overflow");
        case ExprErr::Reserved: Fail(17, "reserved word used as identifier");
        default: SyntaxError();
        }
    }
    return v;
}

int32_t Assembler::EvalInt(const std::vector<Token> &t, size_t &pos, bool *unknown) {
    const Val v = EvalAt(t, pos);
    if (v.is_float) SyntaxError();
    if (unknown) *unknown = v.unknown;
    return v.i;
}

// ---- simbolos ----

void Assembler::DefineLabel(const std::string &name) {
    if (dry_) return;
    if (predefined_.count(name)) Fail(15, "symbol redefinition");
    const bool local = IsLocalName(name);
    const std::string key = local ? LocalKey(name) : name;
    if (SyntaxReserved().count(Lower(name))) SyntaxError();
    if (table_.count(key)) Fail(15, "symbol redefinition");
    Symbol s;
    s.value = static_cast<int32_t>(pc_);
    s.kind = Symbol::Label;
    table_[key] = s;
    if (final_) label_order_.push_back({name, static_cast<int32_t>(pc_)});
    if (!local) parent_ = name;
    const auto it = prev_table_.find(key);
    if (it == prev_table_.end() || it->second.value != s.value) changed_ = true;
}

void Assembler::DefineConst(const std::string &name, int32_t value) {
    if (dry_) return;
    if (SyntaxReserved().count(Lower(name))) SyntaxError();
    if (predefined_.count(name)) Fail(15, "symbol redefinition");
    if (table_.count(name) || vars_.count(name)) Fail(15, "symbol redefinition");
    Symbol s;
    s.value = value;
    s.kind = Symbol::Const;
    table_[name] = s;
    if (final_) const_order_.push_back(name);
    ifdef_defs_.insert(name);
    const auto it = prev_table_.find(name);
    if (it == prev_table_.end() || it->second.value != value) changed_ = true;
}

void Assembler::DefineVar(const std::string &name, int32_t value) {
    if (dry_) return;
    if (SyntaxReserved().count(Lower(name))) SyntaxError();
    const bool first = !vars_.count(name);
    vars_[name] = value;
    if (first && final_) var_order_.push_back(name);
    ifdef_defs_.insert(name);
}

// ---- passada ----

void Assembler::RunPass(bool final) {
    final_ = final;
    pc_ = 0;
    real_ = 0;
    phase_ = false;
    zilog_ = opts_.zilog;
    ifs_.clear();
    parent_.clear();
    table_.clear();
    vars_.clear();
    ifdef_defs_.clear();
    dry_ = false;
    swallow_ = false;
    predefined_.clear();
    label_order_.clear();
    const_order_.clear();
    var_order_.clear();
    txt_lines_.clear();
    seed_ = 12345;
    ds_at_zero_ = false;
    has_res_ = false;
    changed_ = false;
    if (final && opts_.zilog && !zilog_msg_) {
        zilog_msg_ = true;
        msgs_.push_back("Using standard Zilog syntax");
    }
    for (const SrcLine &l : lines_) {
        cur_file_ = l.file;
        cur_line_ = l.line;
        try {
            ProcessLine(l);
        } catch (const Stop &) {
            dry_ = false;
            if (swallow_) {
                swallow_ = false;
                continue;
            }
            // Passadas de simbolos: um erro numa linha NAO interrompe a passada (senao os rotulos de baixo nunca
            // seriam definidos e a passada final reclamaria de "undefined identifier" em vez do erro de verdade).
            if (final_) throw;
        }
    }
    if (!ifs_.empty()) {
        cur_line_ = lines_.empty() ? 0 : lines_.back().line;
        Fail(46, "IF not closed");
    }
}

void Assembler::ProcessLine(const SrcLine &line) {
    const LexResult lr = Lex(line.text);
    const std::vector<Token> &t = lr.tokens;

    // Erros de sintaxe valem em qualquer regiao, ate' nas desativadas por IF (o asMSX le o arquivo todo antes).
    if (!lr.error.empty()) {
        if (lr.error == "Unterminated string") Fail(1, "Unterminated string");
        SyntaxError();
    }
    const bool active = ifs_.empty() || ifs_.back().active;
    if (!t.empty() && t[0].kind == Token::Ident) {
        std::string w = Lower(t[0].text);
        if (!w.empty() && w[0] == '.') w = w.substr(1);
        if (w == "if" || w == "ifdef") {
            IfState st{active, false, false, false};
            if (w == "if") {
                if (t.size() < 2) SyntaxError();
                if (active) {
                    size_t pos = 1;
                    bool unknown = false;
                    const int32_t v = EvalInt(t, pos, &unknown);
                    ExpectEnd(t, pos);
                    st.active = v != 0 && !unknown;
                } else {
                    dry_ = true;
                    size_t pos = 1;
                    EvalInt(t, pos);
                    ExpectEnd(t, pos);
                    dry_ = false;
                }
            } else {
                // IFDEF com um unico identificador; qualquer outra coisa o asMSX simplesmente ignora a linha
                if (t.size() != 2 || t[1].kind != Token::Ident) return;
                if (active) {
                    const std::string &nm = t[1].text;
                    st.active = ifdef_defs_.count(nm) != 0 || predefined_.count(nm) != 0;
                }
            }
            st.taken = st.active;
            ifs_.push_back(st);
            return;
        }
        if (w == "ifndef") return; // o asMSX nao conhece IFNDEF: a linha some, sem erro
        if (w == "else") {
            if (ifs_.empty()) Fail(2, "ELSE without IF/IFDEF");
            IfState &st = ifs_.back();
            st.seen_else = true;
            st.active = st.parent_active && !st.active; // cada ELSE inverte (inclusive o segundo, como o asMSX)
            return;
        }
        if (w == "endif") {
            if (ifs_.empty()) Fail(3, "ENDIF without IF/IFDEF");
            ifs_.pop_back();
            return;
        }
    }
    if (t.empty()) return;
    dry_ = !active;

    // ---- rotulos (um ou mais "nome:") ----
    size_t i = 0;
    while (i + 1 < t.size() && t[i].kind == Token::Ident && t[i + 1].kind == Token::Op && t[i + 1].text == ":") {
        DefineLabel(t[i].text);
        i += 2;
    }
    if (i < t.size()) Statement(t, i);
    dry_ = false;
}

void Assembler::Statement(const std::vector<Token> &t, size_t i) {
    if (t[i].kind != Token::Ident) SyntaxError();
    const std::string &raw = t[i].text;
    // variavel: nome = expressao
    if (i + 1 < t.size() && t[i + 1].kind == Token::Op && t[i + 1].text == "=") {
        if (DirectiveWords().count(Lower(raw))) Fail(17, "reserved word used as identifier");
        size_t pos = i + 2;
        if (pos >= t.size()) SyntaxError();
        const int32_t v = EvalInt(t, pos);
        ExpectEnd(t, pos);
        DefineVar(raw, v);
        return;
    }
    // constante: nome EQU expressao
    if (i + 1 < t.size() && t[i + 1].kind == Token::Ident && Lower(t[i + 1].text) == "equ") {
        if (DirectiveWords().count(Lower(raw))) Fail(17, "reserved word used as identifier");
        size_t pos = i + 2;
        if (pos >= t.size()) SyntaxError();
        const int32_t v = EvalInt(t, pos);
        ExpectEnd(t, pos);
        DefineConst(raw, v);
        return;
    }
    std::string w = Lower(raw);
    const bool dotted = !w.empty() && w[0] == '.';
    if (dotted) w = w.substr(1);
    dotted_ = dotted;
    if (Directive(w, t, i + 1)) return;
    if (dotted) SyntaxError();
    Instruction(w, t, i + 1);
}

// ---- diretivas ----

namespace {
std::string FormatSigned(int32_t v) { return std::to_string(v); }
std::string FormatHex(int32_t v) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "$%x", static_cast<unsigned>(static_cast<uint32_t>(v)));
    return buf;
}
std::string FormatFix(int32_t v) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%d.0000", v / 256);
    if (v < 0 && v / 256 == 0) return "0.0000";
    return buf;
}
} // namespace

bool Assembler::Directive(const std::string &w, const std::vector<Token> &t, size_t i) {
    auto need_expr = [&](size_t &pos) {
        if (pos >= t.size()) SyntaxError();
        return EvalInt(t, pos);
    };

    if (w == "org") {
        size_t pos = i;
        int32_t v = need_expr(pos);
        ExpectEnd(t, pos);
        if (dry_) return true;
        phase_ = false;
        pc_ = v; // sem corte: passar de FFFFh so' da' "memory overflow" quando algo for emitido
        real_ = pc_;
        return true;
    }
    if (w == "filename") {
        if (i >= t.size() || t[i].kind != Token::String || i + 1 != t.size() || t[i].text.empty()) SyntaxError();
        if (!dry_) filename_ = t[i].text;
        return true;
    }
    if (w == "start") {
        // so' interessa aos formatos ROM/BASIC/MSXDOS (ainda nao implementados): aqui apenas valida a expressao
        size_t pos = i;
        const int32_t v = need_expr(pos);
        ExpectEnd(t, pos);
        if (dry_) return true;
        start_addr_ = v;
        return true;
    }
    if (w == "page") {
        size_t pos = i;
        const int32_t v = need_expr(pos);
        ExpectEnd(t, pos);
        if (dry_) return true;
        if (v < 0 || v > 3) Fail(23, "page out of range");
        phase_ = false;
        pc_ = static_cast<int64_t>(v) * 0x4000;
        real_ = pc_;
        return true;
    }
    if (w == "phase") {
        size_t pos = i;
        const int32_t v = need_expr(pos);
        ExpectEnd(t, pos);
        if (dry_) return true;
        phase_ = true;
        pc_ = v;
        return true;
    }
    if (w == "dephase") {
        ExpectEnd(t, i);
        if (dry_) return true;
        if (phase_) {
            // a posicao fisica continua de onde parou
            pc_ = real_;
            phase_ = false;
        }
        return true;
    }
    if (w == "zilog") {
        ExpectEnd(t, i);
        if (dry_) return true;
        zilog_ = true;
        if (final_ && !zilog_msg_) {
            zilog_msg_ = true;
            msgs_.push_back("Using standard Zilog syntax");
        }
        return true;
    }
    if (w == "bios" || w == "biosvars") {
        ExpectEnd(t, i);
        if (dry_) return true;
        if (w == "bios") {
            for (const auto &e : kBiosLabels) predefined_[e.name] = e.value;
        } else {
            for (const auto &e : kBiosVars) predefined_[e.name] = e.value;
        }
        return true;
    }
    if (w == "db" || w == "defb" || w == "dt" || w == "deft") {
        size_t pos = i;
        if (pos >= t.size()) SyntaxError();
        while (true) {
            if (pos >= t.size()) SyntaxError();
            if (t[pos].kind == Token::String && (pos + 1 >= t.size() || (t[pos + 1].kind == Token::Op && t[pos + 1].text == ","))) {
                if (t[pos].text.empty()) SyntaxError();
                for (const char c : t[pos].text) Emit(static_cast<uint8_t>(c));
                ++pos;
            } else {
                const int32_t v = EvalInt(t, pos);
                if (NeedsCrop(v, 8)) Crop(8);
                Emit(static_cast<uint8_t>(v & 0xFF));
            }
            if (pos >= t.size()) break;
            if (!(t[pos].kind == Token::Op && t[pos].text == ",")) SyntaxError();
            ++pos;
        }
        return true;
    }
    if (w == "dw" || w == "defw") {
        size_t pos = i;
        if (pos >= t.size()) SyntaxError();
        while (true) {
            if (pos < t.size() && t[pos].kind == Token::String && (pos + 1 >= t.size() || (t[pos + 1].kind == Token::Op && t[pos + 1].text == ","))) {
                // uma string num DW emite os caracteres como bytes (sem completar para 16 bits)
                if (t[pos].text.empty()) SyntaxError();
                for (const char c : t[pos].text) Emit(static_cast<uint8_t>(c));
                ++pos;
            } else {
                const int32_t v = need_expr(pos);
                if (NeedsCrop(v, 16)) Crop(16);
                Emit(static_cast<uint8_t>(v & 0xFF));
                Emit(static_cast<uint8_t>((v >> 8) & 0xFF));
            }
            if (pos >= t.size()) break;
            if (!(t[pos].kind == Token::Op && t[pos].text == ",")) SyntaxError();
            ++pos;
        }
        return true;
    }
    if (w == "ds" || w == "defs") {
        size_t pos = i;
        const int32_t v = need_expr(pos);
        ExpectEnd(t, pos);
        if (NeedsCrop(v, 16)) Crop(16);
        if (dry_) return true;
        if (v < 0) Fail(2, "memory overflow");
        if (pc_ == 0) ds_at_zero_ = true;
        if (real_ >= 0 && real_ <= 0xFFFF && (!has_res_ || real_ < res_min_)) {
            res_min_ = static_cast<uint32_t>(real_); // so' o DS (nao BYTE/WORD) antes do primeiro byte entra na saida
            has_res_ = true;
        }
        Reserve(v & 0xFFFF);
        return true;
    }
    if (w == "byte") {
        ExpectEnd(t, i);
        if (dry_) return true;
        if (pc_ == 0) ds_at_zero_ = true;
        Reserve(1);
        return true;
    }
    if (w == "word") {
        ExpectEnd(t, i);
        if (dry_) return true;
        if (pc_ == 0) ds_at_zero_ = true;
        Reserve(2);
        return true;
    }
    if (w == "incbin") {
        if (i >= t.size() || t[i].kind != Token::String) SyntaxError();
        const std::string file = t[i].text;
        size_t pos = i + 1;
        int32_t skip = 0, size = -1;
        while (pos < t.size()) {
            if (t[pos].kind != Token::Ident) SyntaxError();
            const std::string kw = Lower(t[pos].text);
            ++pos;
            if (kw == "skip") skip = need_expr(pos);
            else if (kw == "size") size = need_expr(pos);
            else SyntaxError();
        }
        if (dry_) return true;
        std::vector<uint8_t> data;
        const size_t slash = name_.find_last_of("/\\");
        const std::string full = (file.find(':') != std::string::npos || (!file.empty() && (file[0] == '/' || file[0] == '\\')))
                                     ? file
                                     : (slash == std::string::npos ? std::string() : name_.substr(0, slash + 1)) + file;
        if (!files_ || !files_->Read(full, data)) Fail(19, "binary file not found");
        const size_t from = static_cast<size_t>(std::max(0, skip));
        if (from > data.size() || (size >= 0 && from + static_cast<size_t>(size) > data.size())) {
            Fail(30, "included binary shorter than expected");
        }
        size_t count = data.size() - from;
        if (size >= 0) count = static_cast<size_t>(size);
        for (size_t k = 0; k < count; ++k) Emit(data[from + k]);
        return true;
    }
    if (w == "print" || w == "printdec" || w == "printhex" || w == "printfix") {
        if (w != "print" && !dotted_) SyntaxError(); // so' PRINT dispensa o ponto
        size_t pos = i;
        const int32_t v = need_expr(pos);
        ExpectEnd(t, pos);
        if (final_ && !dry_) {
            if (w == "printhex") txt_lines_.push_back(FormatHex(v));
            else if (w == "printfix") txt_lines_.push_back(FormatFix(v));
            else txt_lines_.push_back(FormatSigned(v));
        }
        return true;
    }
    if (w == "printtext" || w == "printstring") {
        if (!dotted_ || i >= t.size() || t[i].kind != Token::String || i + 1 != t.size() || t[i].text.empty()) SyntaxError();
        if (final_ && !dry_) txt_lines_.push_back(t[i].text);
        return true;
    }
    return false;
}

// ---- instrucoes ----

namespace {

// Divide os tokens de i..fim em operandos separados por virgula (fora de parenteses/colchetes).
std::vector<std::pair<size_t, size_t>> SplitOperands(const std::vector<Token> &t, size_t i) {
    std::vector<std::pair<size_t, size_t>> ops;
    if (i >= t.size()) return ops;
    int depth = 0;
    size_t start = i;
    for (size_t k = i; k < t.size(); ++k) {
        const Token &tk = t[k];
        if (tk.kind == Token::Op) {
            if (tk.text == "(" || tk.text == "[") ++depth;
            else if (tk.text == ")" || tk.text == "]") --depth;
            else if (tk.text == "," && depth == 0) {
                ops.push_back({start, k});
                start = k + 1;
            }
        }
    }
    ops.push_back({start, t.size()});
    return ops;
}

} // namespace

void Assembler::Instruction(const std::string &mnem_name, const std::vector<Token> &t, size_t i) {
    const int mnem = z80_mnemonic(mnem_name.c_str());
    if (mnem < 0) SyntaxError();

    const auto spans = SplitOperands(t, i);
    used_forward_ = false;
    ZOp ops[3];
    bool unknown[3] = {false, false, false};
    int n = 0;
    const char *open = zilog_ ? "(" : "[";
    const char *close = zilog_ ? ")" : "]";
    const bool jump_like = mnem_name == "jp" || mnem_name == "jr" || mnem_name == "call" || mnem_name == "ret";

    if (spans.size() > 3) SyntaxError();
    for (size_t s = 0; s < spans.size(); ++s) {
        const size_t a = spans[s].first, b = spans[s].second;
        if (a >= b) SyntaxError();
        ZOp &op = ops[n];
        std::memset(&op, 0, sizeof(op));
        bool done = false;
        if (b - a == 1 && t[a].kind == Token::Ident) {
            const std::string r = Lower(t[a].text);
            static const std::map<std::string, std::pair<int, int>> kRegs = {
                {"b", {ZK_R8, 0}}, {"c", {ZK_R8, 1}}, {"d", {ZK_R8, 2}}, {"e", {ZK_R8, 3}}, {"h", {ZK_R8, 4}}, {"l", {ZK_R8, 5}}, {"a", {ZK_R8, 7}},
                {"i", {ZK_I, 0}}, {"r", {ZK_R, 0}}, {"f", {ZK_F, 0}},
                {"bc", {ZK_RR, 0}}, {"de", {ZK_RR, 1}}, {"hl", {ZK_RR, 2}}, {"sp", {ZK_RR, 3}},
                {"af", {ZK_AF, 0}}, {"af'", {ZK_AFP, 0}}, {"ix", {ZK_IX, 0}}, {"iy", {ZK_IY, 0}},
                {"nz", {ZK_COND, 0}}, {"z", {ZK_COND, 1}}, {"nc", {ZK_COND, 2}}, {"po", {ZK_COND, 4}},
                {"pe", {ZK_COND, 5}}, {"p", {ZK_COND, 6}}, {"m", {ZK_COND, 7}}};
            const auto it = kRegs.find(r);
            if (it != kRegs.end()) {
                op.kind = it->second.first;
                op.reg = it->second.second;
                // "c": condicao nos saltos (jp c,nn / jr c,e / call c,nn / ret c)
                if (r == "c" && jump_like && (s == 0) && (spans.size() == 2 || mnem_name == "ret")) {
                    op.kind = ZK_COND;
                    op.reg = 3;
                }
                done = true;
            } else if (r == "ixh" || r == "ixl" || r == "iyh" || r == "iyl") {
                op.kind = ZK_IDXH;
                op.pfx = r[1] == 'x' ? 0xDD : 0xFD;
                op.reg = r[2] == 'h' ? 4 : 5;
                done = true;
            }
        }
        if (!done && t[a].kind == Token::Op && t[a].text == open) {
            // indireto: o fecha-parenteses correspondente tem que ser o ultimo token do operando
            int depth = 0;
            size_t k = a;
            for (; k < b; ++k) {
                if (t[k].kind == Token::Op && (t[k].text == "(" || t[k].text == "[")) ++depth;
                else if (t[k].kind == Token::Op && (t[k].text == ")" || t[k].text == "]")) {
                    if (--depth == 0) break;
                }
            }
            if (k + 1 != b || t[k].text != close) SyntaxError();
            const size_t ia = a + 1, ib = k;
            if (ia >= ib) SyntaxError();
            done = true;
            if (ib - ia == 1 && t[ia].kind == Token::Ident) {
                const std::string r = Lower(t[ia].text);
                if (r == "hl") { op.kind = ZK_IHL; goto indirect_done; }
                if (r == "bc") { op.kind = ZK_IBC; goto indirect_done; }
                if (r == "de") { op.kind = ZK_IDE; goto indirect_done; }
                if (r == "sp") { op.kind = ZK_ISP; goto indirect_done; }
                if (r == "c") { op.kind = ZK_IC; goto indirect_done; }
                if (r == "ix") { op.kind = ZK_IIX; goto indirect_done; }
                if (r == "iy") { op.kind = ZK_IIY; goto indirect_done; }
            }
            if (t[ia].kind == Token::Ident && (Lower(t[ia].text) == "ix" || Lower(t[ia].text) == "iy") && ia + 1 < ib &&
                t[ia + 1].kind == Token::Op && (t[ia + 1].text == "+" || t[ia + 1].text == "-")) {
                op.kind = Lower(t[ia].text) == "ix" ? ZK_IIX : ZK_IIY;
                const bool minus = t[ia + 1].text == "-";
                std::vector<Token> sub(t.begin() + static_cast<std::ptrdiff_t>(ia) + 2, t.begin() + static_cast<std::ptrdiff_t>(ib));
                size_t pos = 0;
                const int32_t d = EvalInt(sub, pos, &unknown[n]);
                if (pos != sub.size()) SyntaxError();
                op.value = minus ? -static_cast<int64_t>(d) : d;
                goto indirect_done;
            }
            {
                std::vector<Token> sub(t.begin() + static_cast<std::ptrdiff_t>(ia), t.begin() + static_cast<std::ptrdiff_t>(ib));
                size_t pos = 0;
                op.kind = ZK_IABS;
                op.value = EvalInt(sub, pos, &unknown[n]);
                if (pos != sub.size()) SyntaxError();
            }
        indirect_done:;
        }
        if (!done) {
            std::vector<Token> sub(t.begin() + static_cast<std::ptrdiff_t>(a), t.begin() + static_cast<std::ptrdiff_t>(b));
            size_t pos = 0;
            op.kind = ZK_IMM;
            op.value = EvalInt(sub, pos, &unknown[n]);
            if (pos != sub.size()) SyntaxError();
        }
        ++n;
    }

    // IM e RST: o valor entra no opcode; o corte para 8 bits (com aviso) vem ANTES de o erro de faixa ser dado.
    // O aviso so' sai quando o valor cortado e' valido (rst 256, im lbl); a excecao do asMSX e' IM com rotulo a
    // frente, que avisa mesmo quando o valor cortado depois da erro (`im lblf` com lblf = 0103h).
    if ((mnem_name == "im" || mnem_name == "rst") && n == 1 && ops[0].kind == ZK_IMM && !unknown[0] &&
        NeedsCrop(static_cast<int32_t>(ops[0].value), 8)) {
        const int cropped = static_cast<int>(ops[0].value & 0xFF);
        const bool valid = mnem_name == "im" ? cropped <= 2 : (cropped & 0xC7) == 0;
        if (valid || (mnem_name == "im" && used_forward_)) Crop(8);
        ops[0].value = cropped;
    }
    ZEnc enc;
    int err = z80_encode(mnem, ops, n, &enc);
    if (err != ZE_OK && !final_ && !dry_) {
        // valor ruim de um simbolo adiante (ainda sem valor definitivo) nao pode abortar a passada: o rotulo
        // seguinte ficaria sem definir. Mede-se o tamanho com 0 e o erro de verdade sai na passada final.
        for (int k = 0; k < n; ++k) {
            if (ops[k].kind == ZK_IMM) ops[k].value = 0;
        }
        err = z80_encode(mnem, ops, n, &enc);
    }
    switch (err) {
    case ZE_OK: break;
    case ZE_DEST_A: Fail(5, "destiny register should be A");
    case ZE_SRC_A: Fail(6, "source register should be A");
    case ZE_IM: Fail(4, "wrong interruption mode");
    case ZE_RST: Fail(11, "wrong restart address");
    case ZE_REGCOMB: Fail(3, "wrong register combination");
    case ZE_ZERO: Fail(7, "value should be 0");
    case ZE_NOCOND: Fail(8, "missing condition");
    case ZE_BADCOND: Fail(10, "wrong condition");
    default: SyntaxError();
    }
    if (enc.bit_warn) Warn("3-bit overflow. Cropping to fit 3 bits. If it's a label, it may not be defined.");

    // preenche os campos
    const int32_t start_pc = pc_;
    for (int f = 0; f < enc.nfields; ++f) {
        const ZField &fld = enc.field[f];
        const int64_t v = ops[fld.op].value;
        const bool unk = unknown[fld.op];
        switch (fld.kind) {
        case ZF_IMM8:
            if (!unk && (v < -128 || v > 255)) Crop(8);
            enc.bytes[fld.pos] = static_cast<uint8_t>(v & 0xFF);
            break;
        case ZF_IMM16:
            if (!unk && (v < -32768 || v > 65535)) Crop(16);
            enc.bytes[fld.pos] = static_cast<uint8_t>(v & 0xFF);
            enc.bytes[fld.pos + 1] = static_cast<uint8_t>((v >> 8) & 0xFF);
            break;
        case ZF_CHK8:
            if (!unk && (v < -128 || v > 255)) Crop(8);
            break;
        case ZF_DISP:
            enc.bytes[fld.pos] = static_cast<uint8_t>(v & 0xFF);
            break;
        case ZF_REL: {
            const int64_t rel = v - (static_cast<int64_t>(start_pc) + enc.len);
            if (!unk && (rel < -128 || rel > 127)) Fail(9, "unreachable address");
            enc.bytes[fld.pos] = static_cast<uint8_t>(rel & 0xFF);
            break;
        }
        }
    }
    for (int k = 0; k < enc.len; ++k) Emit(enc.bytes[k]);
}

// ---- saida ----

void Assembler::WriteSym() {
    if (label_order_.empty() && const_order_.empty() && var_order_.empty()) return;
    std::string s = "; Symbol table from " + name_ + "\r\n; generated by asMSX v1.2.0\r\n\r\n";
    char buf[32];
    auto line = [&](int32_t v, const std::string &n) {
        std::snprintf(buf, sizeof(buf), "%04Xh ", static_cast<unsigned>(v));
        s += std::string(buf) + n + "\r\n";
    };
    if (!label_order_.empty()) {
        s += "; global and local labels\r\n";
        for (const LabelRec &r : label_order_) line(r.value, r.display);
    }
    if (!const_order_.empty()) {
        s += "; other identifiers\r\n";
        for (const std::string &n : const_order_) line(table_[n].value, n);
    }
    if (!var_order_.empty()) {
        s += "; variables - value on exit\r\n";
        for (const std::string &n : var_order_) line(vars_[n], n);
    }
    result_.outs["sym"] = std::vector<uint8_t>(s.begin(), s.end());
}

void Assembler::WriteTxt() {
    if (txt_lines_.empty()) return;
    std::string s = "; Output text file from " + name_ + "\r\n; generated by asMSX v1.2.0\r\n\r\n";
    for (const std::string &l : txt_lines_) s += l + "\r\n";
    result_.outs["txt"] = std::vector<uint8_t>(s.begin(), s.end());
}

void Assembler::Finish() {
    if (!any_written_) {
        if (ds_at_zero_) {
            result_.outs["z80"] = {0};
        } else {
            // linha da ultima linha COM conteudo (0 se o arquivo so' tem comentarios/brancos)
            cur_line_ = 0;
            for (size_t k = lines_.size(); k-- > 0;) {
                if (lines_[k].text.find_first_not_of(" \t\r") != std::string::npos) {
                    cur_line_ = lines_[k].line;
                    break;
                }
            }
            cur_file_ = name_;
            rc_ = 25;
            msgs_.push_back(name_ + ", line " + std::to_string(cur_line_) + ": no code in the whole file");
            WriteTxt();
            return;
        }
    } else {
        if (neg_start_) {
            // a faixa vai de -1 ate o maior endereco gravado (no minimo 0)
            std::vector<uint8_t> z{neg1_};
            z.insert(z.end(), mem_.begin(), mem_.begin() + max_ + 1);
            result_.outs["z80"] = z;
        } else {
            result_.outs["z80"] = std::vector<uint8_t>(mem_.begin() + (has_res_ && res_min_ < min_ ? res_min_ : min_), mem_.begin() + max_ + 1);
        }
    }
    WriteSym();
    WriteTxt();
}

AsmResult Assembler::Run() {
    const PreResult pre = Preprocess(name_, text_, files_);
    for (const std::string &w : pre.warnings) msgs_.push_back(w);
    if (pre.rc != 0) {
        if (pre.syntax_error) msgs_.push_back("Parsing error: syntax error");
        msgs_.push_back(pre.error);
        result_.rc = pre.rc;
        result_.msgs = msgs_;
        return result_;
    }
    lines_ = pre.lines;

    try {
        // passadas de simbolos ate estabilizar (maximo 4), depois a passada final (com emissao e mensagens)
        for (int p = 0; p < 4; ++p) {
            try {
                RunPass(false);
            } catch (const Stop &) {
            }
            prev_table_ = table_;
            prev_vars_ = vars_;
            if (!changed_ && p > 0) break;
        }
        rc_ = 0;
        RunPass(true);
        Finish();
    } catch (const Stop &) {
    }
    if (!filename_.empty() && opts_.out_base.empty() && rc_ == 0) {
        // as saidas passam a se chamar "<nome>.<ext>" (chave com ponto = caminho relativo ao fonte)
        std::map<std::string, std::vector<uint8_t>> renamed;
        for (auto &kv : result_.outs) renamed[filename_ + "." + kv.first] = std::move(kv.second);
        result_.outs = std::move(renamed);
    }
    result_.rc = rc_;
    result_.msgs = msgs_;
    if (rc_ != 0 && rc_ != 25) result_.outs.clear();
    return result_;
}

} // namespace

AsmResult Assemble(const std::string &source_name, const std::string &text, const AsmOptions &opts, FileProvider *files) {
    Assembler a(source_name, text, opts, files);
    AsmResult r = a.Run();
    const size_t dot = source_name.find_last_of('.');
    const size_t slash = source_name.find_last_of("/\\");
    r.base_name = (dot != std::string::npos && (slash == std::string::npos || dot > slash)) ? source_name.substr(0, dot) : source_name;
    return r;
}

} // namespace fwasm
