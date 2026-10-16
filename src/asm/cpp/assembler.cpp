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

#include "../asm/wav_synth.h"
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
        if (fmt_ == Fmt::MegaRom) {
            EmitMega(b);
            return;
        }
        if (real_ < -1 || real_ > 0xFFFF) Fail(2, "memory overflow");
        if (fmt_ == Fmt::Rom && real_ >= 0xC000) Fail(29, "cannot write ROM code/data to page 3");
        emitted_any_ = true;
        last_emit_file_ = cur_file_;
        last_emit_line_ = cur_line_;
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
        emitted_any_ = true;
        real_ += n;
        pc_ += n;
    }

    void Finish();
    void ApplyFormat();
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
        int page = 0; // subpagina da MegaROM em que o rotulo foi definido (o .sym a mostra)
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
    int32_t start_addr_ = 0; // .START: ponto de entrada (cabecalho da ROM e do BASIC); vale o ultimo, e na ROM so' o que vem ANTES do .ROM
    bool start_set_ = false;
    // Formato de saida (.ROM / .BASIC / .MSXDOS): so' um por programa; repetir o mesmo tipo e' permitido.
    enum class Fmt { None, Rom, MegaRom, Basic, MsxDos, Sinclair } fmt_ = Fmt::None;
    // Fita: `.CAS`/`.WAV` pedem o arquivo .cas (para emuladores) e/ou o .wav (para um MSX de verdade); vale o PRIMEIRO nome dado.
    bool cas_req_ = false, wav_req_ = false;
    bool tape_name_set_ = false; // o PRIMEIRO .CAS/.WAV decide o nome (dos dois arquivos), mesmo sem nome (= o nome-base)
    std::string tape_name_;
    int32_t rom_hdr_addr_ = -1;  // onde o primeiro .ROM gravou o cabecalho (a fita mostra o INIT final)
    std::vector<uint8_t> tape_raw_; // o bloco montado (sem enchimento), para a fita e o .tap
    uint32_t tape_base_ = 0;
    std::string SourceBase() const; // fonte sem extensao (com a pasta)
    std::string OutBase() const;    // nome-base das saidas, como aparece nas mensagens
    std::string DefaultTapeName() const;
    void BuildTape();
    void BuildSinclair(const std::vector<uint8_t> &raw, uint32_t base);
    // MegaROM: o codigo vive em subpaginas (8 KB; 16 KB no ASCII16). O deslocamento no arquivo e' subpagina*tamanho + (endereco fisico -
    // endereco dado em `.SUBPAGE n AT x`). `.SELECT` gera o codigo que troca a subpagina visivel numa janela.
    enum class Mapper { Konami, KonamiScc, Ascii8, Ascii16 } mapper_ = Mapper::Konami;
    std::vector<uint8_t> mega_rom_; // a imagem inteira: o indice e' subpagina*tamanho + deslocamento (que pode ser negativo dentro da subpagina)
    std::set<int> mega_defined_; // subpaginas definidas ate' aqui NESTA passada
    std::set<int> mega_known_;   // as definidas em passadas anteriores (o .SELECT pode vir antes do .SUBPAGE)
    int mega_cur_ = 0;
    int32_t mega_at_ = 0x4000;
    bool mega_valid_ = false;    // false depois de `.PAGE`: nao ha' subpagina para receber bytes
    int mega_max_ = 0;
    size_t MegaPageSize() const { return mapper_ == Mapper::Ascii16 ? 16384 : 8192; }
    int MegaMaxPages() const { return mapper_ == Mapper::Konami ? 32 : mapper_ == Mapper::KonamiScc ? 64 : 256; }
    void EmitMega(uint8_t b);
    bool rom_hdr_done_ = false;     // ja' houve um .ROM nesta passada
    int32_t rom_default_init_ = 0;  // INIT padrao: o fim do cabecalho do PRIMEIRO .ROM (um segundo .ROM repete esse valor)
    bool has_size_ = false;  // .SIZE n: tamanho do arquivo de saida em KB
    int32_t size_kb_ = 0;
    bool emitted_any_ = false; // algum byte foi emitido ou reservado nesta passada (o .MSXDOS tem que vir antes)
    std::string last_emit_file_;
    int last_emit_line_ = 0;
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
    if (final_) label_order_.push_back({name, static_cast<int32_t>(pc_), fmt_ == Fmt::MegaRom ? mega_cur_ : 0});
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
    fmt_ = Fmt::None;
    cas_req_ = wav_req_ = false;
    tape_name_set_ = false;
    tape_name_.clear();
    rom_hdr_addr_ = -1;
    mapper_ = Mapper::Konami;
    mega_rom_.clear();
    mega_defined_.clear();
    mega_cur_ = 0;
    mega_at_ = 0x4000;
    mega_valid_ = false;
    mega_max_ = 0;
    rom_hdr_done_ = false;
    start_set_ = false;
    start_addr_ = 0;
    has_size_ = false;
    size_kb_ = 0;
    emitted_any_ = false;
    last_emit_line_ = 0;
    last_emit_file_.clear();
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
        // ponto de entrada: vai para o cabecalho da ROM (so' se vier ANTES do .ROM) e para o do arquivo BASIC (em qualquer ordem)
        size_t pos = i;
        const int32_t v = need_expr(pos);
        ExpectEnd(t, pos);
        if (dry_) return true;
        start_addr_ = v;
        start_set_ = true;
        return true;
    }
    if (w == "rom") {
        ExpectEnd(t, i);
        if (dry_) return true;
        if (fmt_ != Fmt::None && fmt_ != Fmt::Rom) Fail(21, "type previously defined");
        fmt_ = Fmt::Rom;
        // cabecalho de 16 bytes no endereco atual: "AB", INIT (o .START, ou o fim do cabecalho) e 12 bytes zero
        if (!rom_hdr_done_) {
            rom_hdr_done_ = true;
            rom_default_init_ = static_cast<int32_t>(pc_ + 16);
            rom_hdr_addr_ = static_cast<int32_t>(real_);
        }
        const int32_t init = start_set_ ? start_addr_ : rom_default_init_;
        Emit('A');
        Emit('B');
        Emit(static_cast<uint8_t>(init & 0xFF));
        Emit(static_cast<uint8_t>((init >> 8) & 0xFF));
        for (int k = 0; k < 12; ++k) Emit(0);
        return true;
    }
    if (w == "megarom") {
        Mapper m = Mapper::Konami;
        if (i < t.size()) {
            const std::string k = t[i].kind == Token::Ident ? Lower(t[i].text) : std::string();
            if (i + 1 == t.size() && k == "konami") m = Mapper::Konami;
            else if (i + 1 == t.size() && k == "konamiscc") m = Mapper::KonamiScc;
            else if (i + 1 == t.size() && k == "ascii8") m = Mapper::Ascii8;
            else if (i + 1 == t.size() && k == "ascii16") m = Mapper::Ascii16;
            else {
                // qualquer outra coisa e' tratada como expressao (e dá o erro de expressao correspondente)
                size_t pos = i;
                EvalInt(t, pos);
                ExpectEnd(t, pos);
            }
        }
        if (dry_) return true;
        if (fmt_ == Fmt::MegaRom) return true; // um segundo .MEGAROM nao faz nada
        if (fmt_ != Fmt::None) Fail(21, "type previously defined");
        fmt_ = Fmt::MegaRom;
        mapper_ = m;
        phase_ = false;
        pc_ = 0x4000; // a subpagina 0 vive em 4000h
        real_ = pc_;
        mega_cur_ = 0;
        mega_at_ = 0x4000;
        mega_valid_ = true;
        mega_defined_.insert(0);
        mega_known_.insert(0);
        mega_max_ = 0;
        const int32_t init = start_set_ ? start_addr_ : 0x4010; // o .START de verdade e' gravado no fim (vale em qualquer ordem)
        Emit('A');
        Emit('B');
        Emit(static_cast<uint8_t>(init & 0xFF));
        Emit(static_cast<uint8_t>((init >> 8) & 0xFF));
        for (int k = 0; k < 12; ++k) Emit(0);
        return true;
    }
    if (w == "subpage") {
        size_t pos = i;
        used_forward_ = false;
        const int32_t n = need_expr(pos);
        const bool forward = used_forward_;
        if (pos >= t.size() || t[pos].kind != Token::Ident || Lower(t[pos].text) != "at") SyntaxError();
        ++pos;
        const int32_t x = need_expr(pos);
        ExpectEnd(t, pos);
        if (dry_) return true;
        if (fmt_ != Fmt::MegaRom) Fail(41, "megaROM-only macro used");
        if (n == 0) Fail(33, "subpage 0 can only be defined by megaROM directive");
        if (n < 0 || n >= MegaMaxPages() || forward) Fail(37, "megaROM mapper subpage out of range");
        if (x < 0x4000 || x > 0xBFFF) Fail(36, "code/data without subpage");
        if (mega_defined_.count(n)) Fail(38, "megaROM subpage already defined");
        phase_ = false;
        pc_ = x;
        real_ = pc_;
        mega_cur_ = n;
        mega_at_ = x;
        mega_valid_ = true;
        mega_defined_.insert(n);
        mega_known_.insert(n);
        mega_max_ = std::max(mega_max_, n);
        return true;
    }
    if (w == "select") {
        // .SELECT n AT x  |  .SELECT registrador AT x : codigo que grava o numero da subpagina no registrador do mapper da janela x
        size_t pos = i;
        int reg = -1; // 0..5 = b c d e h l, 6 = a
        int32_t n = 0;
        if (pos + 1 < t.size() && t[pos].kind == Token::Ident && t[pos + 1].kind == Token::Ident && Lower(t[pos + 1].text) == "at") {
            const std::string r = Lower(t[pos].text);
            if (r.size() == 1 && std::string("bcdehla").find(r[0]) != std::string::npos) {
                reg = static_cast<int>(std::string("bcdehla").find(r[0]));
                ++pos;
            }
        }
        if (reg < 0) n = need_expr(pos);
        if (pos >= t.size() || t[pos].kind != Token::Ident || Lower(t[pos].text) != "at") SyntaxError();
        ++pos;
        const int32_t x = need_expr(pos);
        ExpectEnd(t, pos);
        if (dry_) return true;
        if (fmt_ != Fmt::MegaRom) Fail(41, "megaROM-only macro used");
        const int32_t xv = x < 0 ? 0 : x;
        int32_t addr = 0; // endereco do registrador do mapper para a janela
        switch (mapper_) {
        case Mapper::Konami:
            addr = xv & 0xFFF8;
            if (addr == 0x4000) Fail(39, "Konami megaROM forces page 0 at 4000h");
            break;
        case Mapper::KonamiScc: addr = (xv + 0x1000) & 0xFFF8; break;
        case Mapper::Ascii8: addr = (0x6000 + ((xv - 0x4000) >> 2)) & 0xFFFE; break;
        case Mapper::Ascii16: addr = (xv & 0xFFF8) == 0x4000 ? 0x6000 : 0x7000; break;
        }
        if (reg < 0 && n != 0 && !mega_defined_.count(n) && !mega_known_.count(n)) Fail(40, "megaROM subpage not defined");
        const uint8_t lo = static_cast<uint8_t>(addr & 0xFF), hi = static_cast<uint8_t>((addr >> 8) & 0xFF);
        if (reg < 0) {
            const uint8_t seq[] = {0xF5, 0x3E, static_cast<uint8_t>(n & 0xFF), 0x32, lo, hi, 0xF1};
            for (const uint8_t b : seq) Emit(b);
        } else if (reg == 6) {
            const uint8_t seq[] = {0x32, lo, hi};
            for (const uint8_t b : seq) Emit(b);
        } else {
            const uint8_t seq[] = {0xF5, static_cast<uint8_t>(0x78 + reg), 0x32, lo, hi, 0xF1};
            for (const uint8_t b : seq) Emit(b);
        }
        return true;
    }
    if (w == "cas" || w == "cassette" || w == "wav") {
        // [ "nome" ]: ate' 6 letras para o BLOAD "CAS:" achar o arquivo; sem nome, o nome-base da saida
        std::string name;
        if (i < t.size()) {
            if (t[i].kind != Token::String || i + 1 != t.size() || t[i].text.empty()) SyntaxError();
            name = t[i].text;
        }
        if (dry_) return true;
        if (!tape_name_set_) {
            tape_name_set_ = true;
            tape_name_ = name;
        }
        if (w == "wav") wav_req_ = true;
        else cas_req_ = true;
        return true;
    }
    if (w == "sinclair") {
        ExpectEnd(t, i);
        if (dry_) return true;
        if (fmt_ != Fmt::None && fmt_ != Fmt::Sinclair) Fail(47, "Sinclair directive should preceed any code");
        fmt_ = Fmt::Sinclair;
        return true;
    }
    if (w == "basic") {
        ExpectEnd(t, i);
        if (dry_) return true;
        if (fmt_ != Fmt::None && fmt_ != Fmt::Basic) Fail(21, "type previously defined");
        fmt_ = Fmt::Basic;
        return true;
    }
    if (w == "msxdos") {
        ExpectEnd(t, i);
        if (dry_) return true;
        if (emitted_any_) Fail(24, "MSXDOS directive should preceed any code");
        if (fmt_ != Fmt::None && fmt_ != Fmt::MsxDos) Fail(21, "type previously defined");
        fmt_ = Fmt::MsxDos;
        phase_ = false;
        pc_ = 0x100; // os .COM sao sempre carregados em 0100h
        real_ = pc_;
        return true;
    }
    if (w == "size") {
        size_t pos = i;
        const int32_t v = need_expr(pos);
        ExpectEnd(t, pos);
        if (dry_) return true;
        if (has_size_) Fail(16, "size redefinition");
        has_size_ = true;
        size_kb_ = v;
        return true;
    }
    if (w == "search") {
        ExpectEnd(t, i);
        if (dry_) return true;
        if (fmt_ != Fmt::Rom && fmt_ != Fmt::MegaRom) Fail(42, "only for ROMs and megaROMs");
        // acha o slot da pagina 2 e a seleciona (a mesma sequencia do asMSX): RSLREG, expande o slot, ENASLT
        static const uint8_t kSearch[] = {0xCD, 0x38, 0x01, 0x0F, 0x0F, 0xE6, 0x03, 0x4F, 0x21, 0xC1, 0xFC, 0x85, 0x6F, 0x7E, 0xE6, 0x80,
                                          0xB1, 0x4F, 0x2C, 0x2C, 0x2C, 0x2C, 0x7E, 0xE6, 0x0C, 0xB1, 0x26, 0x80, 0xCD, 0x24, 0x00};
        for (const uint8_t b : kSearch) Emit(b);
        return true;
    }
    if (w == "callbios") {
        size_t pos = i;
        const int32_t v = need_expr(pos);
        ExpectEnd(t, pos);
        if (dry_) return true;
        // LD IY,[EXPTBL-1] / LD IX,rotina / CALL CALSLT
        const uint8_t seq[] = {0xFD, 0x2A, 0xC0, 0xFC, 0xDD, 0x21, static_cast<uint8_t>(v & 0xFF), static_cast<uint8_t>((v >> 8) & 0xFF), 0xCD, 0x1C, 0x00};
        for (const uint8_t b : seq) Emit(b);
        return true;
    }
    if (w == "calldos") {
        size_t pos = i;
        const int32_t v = need_expr(pos);
        ExpectEnd(t, pos);
        if (dry_) return true;
        if (fmt_ != Fmt::MsxDos) Fail(26, "only available for MSXDOS");
        // LD C,funcao / CALL 0005h
        const uint8_t seq[] = {0x0E, static_cast<uint8_t>(v & 0xFF), 0xCD, 0x05, 0x00};
        for (const uint8_t b : seq) Emit(b);
        return true;
    }
    if (w == "debug") {
        // marcador para o depurador do blueMSX: LD D,D / JR (tamanho+4) / texto (sem terminador)
        if (i >= t.size() || t[i].kind != Token::String || i + 1 != t.size() || t[i].text.empty()) SyntaxError();
        if (dry_) return true;
        Emit(0x52);
        Emit(0x18);
        Emit(static_cast<uint8_t>((t[i].text.size() + 4) & 0xFF));
        for (const char c : t[i].text) Emit(static_cast<uint8_t>(c));
        return true;
    }
    if (w == "break" || w == "breakpoint") {
        // breakpoint do blueMSX: LD B,B / JR 0 (no lugar) ou LD B,B / JR 2 / endereco (word)
        int32_t v = 0;
        bool has = false;
        if (i < t.size()) {
            size_t pos = i;
            v = need_expr(pos);
            ExpectEnd(t, pos);
            has = true;
        }
        if (dry_) return true;
        Emit(0x40);
        Emit(0x18);
        if (has) {
            Emit(0x02);
            Emit(static_cast<uint8_t>(v & 0xFF));
            Emit(static_cast<uint8_t>((v >> 8) & 0xFF));
        } else {
            Emit(0x00);
        }
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
        if (fmt_ == Fmt::MegaRom) mega_valid_ = false; // numa MegaROM so' as subpaginas recebem bytes
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
        for (const LabelRec &r : label_order_) {
            if (fmt_ == Fmt::MegaRom) { // numa MegaROM cada rotulo vem com a subpagina: "01h:8000h nome"
                std::snprintf(buf, sizeof(buf), "%02Xh:%04Xh ", static_cast<unsigned>(r.page), static_cast<unsigned>(r.value));
                s += std::string(buf) + r.display + "\r\n";
            } else {
                line(r.value, r.display);
            }
        }
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
        ApplyFormat();
    }
    WriteSym();
    WriteTxt();
}

// Converte o bloco cru (result_.outs["z80"]) no formato pedido pelo programa e aplica o .SIZE.
void Assembler::ApplyFormat() {
    std::vector<uint8_t> raw = std::move(result_.outs["z80"]);
    result_.outs.erase("z80");
    if (fmt_ == Fmt::MegaRom) {
        // Uma pagina por subpagina ate' a MAIOR definida. O aviso do asMSX diz "only N out of M ... defined", mas N e' quantas estao
        // FALTANDO (reproduzido como esta').
        const size_t ps = MegaPageSize();
        const int pages = mega_max_ + 1;
        int missing = 0;
        for (int p = 0; p < pages; ++p)
            if (!mega_defined_.count(p)) ++missing;
        if (missing > 0) msgs_.push_back("Warning: only " + std::to_string(missing) + " out of " + std::to_string(pages) + " megaROM pages are defined");
        std::vector<uint8_t> rom(static_cast<size_t>(pages) * ps, 0);
        std::copy(mega_rom_.begin(), mega_rom_.begin() + static_cast<std::ptrdiff_t>(std::min(mega_rom_.size(), rom.size())), rom.begin());
        if (start_set_) { // o INIT do cabecalho segue o .START final, em qualquer ordem
            rom[2] = static_cast<uint8_t>(start_addr_ & 0xFF);
            rom[3] = static_cast<uint8_t>((start_addr_ >> 8) & 0xFF);
        }
        if (has_size_ && size_kb_ > 0) rom.resize(static_cast<size_t>(size_kb_) * 1024, 0); // .SIZE corta ou completa, sem erro
        result_.outs["rom"] = std::move(rom);
        if (cas_req_ || wav_req_) msgs_.push_back("WARNING: cas file generation is not supported for MEGAROM");
        return;
    }
    const uint32_t base = neg_start_ ? 0u : (has_res_ && res_min_ < min_ ? res_min_ : min_);
    // o erro de tamanho e' dado na linha do ultimo byte emitido
    auto fail_at_last = [&](int rc, const char *msg) {
        if (last_emit_line_ > 0) {
            cur_file_ = last_emit_file_;
            cur_line_ = last_emit_line_;
        }
        final_ = true;
        Fail(rc, msg);
    };
    int64_t total = -1;
    if (has_size_ && size_kb_ != 0) { // .SIZE 0 = sem tamanho fixo
        total = static_cast<int64_t>(size_kb_) * 1024;
        if (total + static_cast<int64_t>(base) > 0x10000) fail_at_last(2, "memory overflow");
        if (static_cast<int64_t>(raw.size()) > total) fail_at_last(18, "code size overflow");
    }
    auto pad_to = [&](std::vector<uint8_t> v, size_t n) {
        v.resize(n, 0);
        return v;
    };
    if (fmt_ == Fmt::Rom && (cas_req_ || wav_req_) && base >= 0x8000 && start_set_ && rom_hdr_addr_ >= 0) {
        // Com fita pedida o asMSX grava o .START FINAL no INIT do cabecalho da ROM, mesmo que ele venha depois do .ROM (e o arquivo .rom
        // tambem sai assim). Sem fita, o que vem depois do .ROM nao conta.
        const int64_t at = static_cast<int64_t>(rom_hdr_addr_) - static_cast<int64_t>(base) + 2;
        if (at >= 0 && at + 1 < static_cast<int64_t>(raw.size())) {
            raw[static_cast<size_t>(at)] = static_cast<uint8_t>(start_addr_ & 0xFF);
            raw[static_cast<size_t>(at) + 1] = static_cast<uint8_t>((start_addr_ >> 8) & 0xFF);
        }
    }
    tape_raw_ = raw; // a fita leva o bloco SEM o enchimento do .SIZE nem o do tamanho da ROM
    tape_base_ = base;
    switch (fmt_) {
    case Fmt::None:
        result_.outs["z80"] = total >= 0 ? pad_to(std::move(raw), static_cast<size_t>(total)) : std::move(raw);
        break;
    case Fmt::Rom: {
        // Sem .SIZE o arquivo tem o tamanho do bloco arredondado para cima em multiplos de 8 KB -- contado a partir do ULTIMO
        // endereco (tamanho-1), como o asMSX: um bloco de exatamente 8 KB + 1 byte perde esse ultimo byte.
        size_t n = total >= 0 ? static_cast<size_t>(total) : ((raw.size() - 1 + 8191) / 8192) * 8192;
        if (n == 0 && total < 0) n = 8192;
        raw.resize(n, 0);
        result_.outs["rom"] = std::move(raw);
        break;
    }
    case Fmt::Basic: {
        // arquivo binario do BASIC (BLOAD "x",R): 0FEh, inicio, fim, execucao e os bytes. O fim NAO conta o enchimento do .SIZE.
        const int32_t end = static_cast<int32_t>(base) + static_cast<int32_t>(raw.size()) - 1;
        const int32_t exec = start_set_ ? start_addr_ : static_cast<int32_t>(base);
        std::vector<uint8_t> out{0xFE,
                                 static_cast<uint8_t>(base & 0xFF),
                                 static_cast<uint8_t>((base >> 8) & 0xFF),
                                 static_cast<uint8_t>(end & 0xFF),
                                 static_cast<uint8_t>((end >> 8) & 0xFF),
                                 static_cast<uint8_t>(exec & 0xFF),
                                 static_cast<uint8_t>((exec >> 8) & 0xFF)};
        if (total >= 0) raw.resize(static_cast<size_t>(total), 0);
        out.insert(out.end(), raw.begin(), raw.end());
        result_.outs["bin"] = std::move(out);
        break;
    }
    case Fmt::MsxDos:
        result_.outs["com"] = total >= 0 ? pad_to(std::move(raw), static_cast<size_t>(total)) : std::move(raw);
        break;
    case Fmt::Sinclair:
        BuildSinclair(tape_raw_, base);
        break;
    case Fmt::MegaRom: break; // tratado acima
    }
    if (cas_req_ || wav_req_) BuildTape();
}

std::string Assembler::SourceBase() const {
    const size_t dot = name_.find_last_of('.');
    const size_t slash = name_.find_last_of("/\\");
    return (dot != std::string::npos && (slash == std::string::npos || dot > slash)) ? name_.substr(0, dot) : name_;
}

std::string Assembler::OutBase() const {
    if (!opts_.out_base.empty()) return opts_.out_base;
    if (!filename_.empty()) return filename_;
    return SourceBase();
}

std::string Assembler::DefaultTapeName() const {
    const std::string b = OutBase();
    const size_t slash = b.find_last_of("/\\");
    return slash == std::string::npos ? b : b.substr(slash + 1);
}

// ---- fita MSX (.cas / .wav) ----
//   .cas : marca de 8 bytes + (arquivo BLOAD: 10x D0h + nome de 6 letras + marca) + os bytes do arquivo (sem o 0FEh)
//   .wav : tom de cabecalho (16000 ciclos de 2400 Hz) + os bytes, em serie (1 start 0, 8 bits LSB primeiro, 2 stop 1); nos arquivos
//          BLOAD ha' um segundo tom (4000 ciclos) entre a identificacao e os dados. 0 = um ciclo de 1200 Hz; 1 = dois de 2400 Hz.
void Assembler::BuildTape() {
    const uint32_t base = tape_base_;
    const std::vector<uint8_t> &raw = tape_raw_;
    bool bin = false; // true = arquivo BLOAD (identificacao + inicio/fim/execucao); false = bloco cru
    int32_t exec = 0;
    if (fmt_ == Fmt::Basic) {
        bin = true;
        exec = start_set_ ? start_addr_ : static_cast<int32_t>(base);
    } else if (fmt_ == Fmt::Rom) {
        if (base < 0x8000) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "0x%04x", static_cast<unsigned>(base));
            msgs_.push_back(std::string("WARNING: cas file generation is not supported for ROM that start below address 0x8000. Current start address is ") + buf);
            return;
        }
        bin = true;
        exec = start_set_ ? start_addr_ : rom_default_init_;
    }
    std::vector<uint8_t> body;
    const std::vector<uint8_t> &data = raw;
    if (bin) {
        const int32_t end = static_cast<int32_t>(base) + static_cast<int32_t>(raw.size()) - 1;
        const uint8_t head[] = {static_cast<uint8_t>(base & 0xFF), static_cast<uint8_t>((base >> 8) & 0xFF), static_cast<uint8_t>(end & 0xFF),
                                static_cast<uint8_t>((end >> 8) & 0xFF), static_cast<uint8_t>(exec & 0xFF), static_cast<uint8_t>((exec >> 8) & 0xFF)};
        body.assign(head, head + sizeof(head));
    }
    body.insert(body.end(), data.begin(), data.end());
    auto name6 = [&](const std::string &given) {
        std::string n = given.empty() ? DefaultTapeName() : given;
        n.resize(6, ' ');
        return n;
    };
    const std::string out_base = OutBase();

    if (cas_req_) {
        static const uint8_t kMark[] = {0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74};
        std::vector<uint8_t> cas(kMark, kMark + sizeof(kMark));
        if (bin) {
            cas.insert(cas.end(), 10, 0xD0);
            const std::string n = name6(tape_name_);
            cas.insert(cas.end(), n.begin(), n.end());
            cas.insert(cas.end(), kMark, kMark + sizeof(kMark));
        }
        cas.insert(cas.end(), body.begin(), body.end());
        result_.outs["cas"] = std::move(cas);
        msgs_.push_back("cas file " + out_base + ".cas");
    }
    if (wav_req_) {
        std::vector<uint8_t> kinds; // 0 = ciclo curto (2400 Hz), 1 = ciclo longo (1200 Hz)
        auto tone = [&](int n) { kinds.insert(kinds.end(), static_cast<size_t>(n), 0); };
        auto put_byte = [&](uint8_t b) {
            kinds.push_back(1); // bit de partida
            for (int k = 0; k < 8; ++k) {
                if ((b >> k) & 1) {
                    kinds.push_back(0);
                    kinds.push_back(0);
                } else {
                    kinds.push_back(1);
                }
            }
            tone(4); // dois bits de parada
        };
        tone(16000);
        if (bin) {
            for (int k = 0; k < 10; ++k) put_byte(0xD0);
            const std::string n = name6(tape_name_);
            for (const char c : n) put_byte(static_cast<uint8_t>(c));
            tone(4000);
        }
        for (const uint8_t b : body) put_byte(b);
        size_t t16 = 0; // tempo total em 1/16 de amostra (so' para dimensionar o buffer)
        for (const uint8_t k : kinds) t16 += k ? 588 : 294;
        std::vector<uint8_t> samples(t16 / 16 + 4, 0);
        size_t n = fwasm_wav_render(kinds.data(), kinds.size(), samples.data());
        if (n % 2) ++n; // o asMSX fecha o arquivo com um numero PAR de amostras (acrescenta uma amostra 0)
        samples.resize(n);
        auto le32 = [](std::vector<uint8_t> &v, uint32_t x) {
            for (int k = 0; k < 4; ++k) v.push_back(static_cast<uint8_t>((x >> (8 * k)) & 0xFF));
        };
        auto le16 = [](std::vector<uint8_t> &v, uint16_t x) {
            v.push_back(static_cast<uint8_t>(x & 0xFF));
            v.push_back(static_cast<uint8_t>(x >> 8));
        };
        std::vector<uint8_t> wav{'R', 'I', 'F', 'F'};
        le32(wav, static_cast<uint32_t>(36 + n));
        for (const char c : std::string("WAVEfmt ")) wav.push_back(static_cast<uint8_t>(c));
        le32(wav, 16);
        le16(wav, 1);     // PCM
        le16(wav, 1);     // mono
        le32(wav, 44100); // amostras por segundo
        le32(wav, 44100); // bytes por segundo
        le16(wav, 1);     // alinhamento
        le16(wav, 8);     // 8 bits
        for (const char c : std::string("data")) wav.push_back(static_cast<uint8_t>(c));
        le32(wav, static_cast<uint32_t>(n));
        wav.insert(wav.end(), samples.begin(), samples.end());
        result_.outs["wav"] = std::move(wav);
        char buf[64];
        std::snprintf(buf, sizeof(buf), " saved [%.2f sec]", static_cast<double>(n) / 44100.0);
        msgs_.push_back("Audio file " + out_base + ".wav" + buf);
    }
}

// ---- arquivo .tap para o ZX Spectrum ----
// Com `.START` (diferente de 0) o .tap leva antes um programa BASIC de uma linha:
//   10 CLEAR VAL "<inicio-1>": LOAD ""CODE : RANDOMIZE USR VAL "<start>"
// seguido da imagem CODE; sem .START so' a imagem CODE. O nome (10 letras) e' o nome-base da saida.
void Assembler::BuildSinclair(const std::vector<uint8_t> &raw, uint32_t base) {
    std::string name = DefaultTapeName();
    name.resize(10, ' ');
    std::vector<uint8_t> tap;
    // Os checksums dos blocos do carregador BASIC do asMSX saem errados e foram reproduzidos assim (casos sinclair_start* e
    // sinclair_filename do formats.golden): o do cabecalho ignora o NOME e leva 11h a mais (fix = 1); o dos dados leva 11h a mais e o
    // XOR do nome de 10 letras (fix = 2). O bloco CODE e' correto.
    uint8_t name_xor = 0;
    for (const char c : name) name_xor ^= static_cast<uint8_t>(c);
    auto block = [&](uint8_t flag, const std::vector<uint8_t> &payload, int fix = 0) {
        const uint32_t len = static_cast<uint32_t>(payload.size()) + 2;
        tap.push_back(static_cast<uint8_t>(len & 0xFF));
        tap.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        uint8_t sum = flag;
        tap.push_back(flag);
        for (const uint8_t b : payload) {
            tap.push_back(b);
            sum ^= b;
        }
        if (fix == 1) sum ^= static_cast<uint8_t>(name_xor ^ 0x11);
        else if (fix == 2) sum ^= static_cast<uint8_t>(0x11 ^ name_xor);
        tap.push_back(sum);
    };
    auto header = [&](uint8_t type, uint16_t length, uint16_t p1, uint16_t p2, int fix = 0) {
        std::vector<uint8_t> h{type};
        h.insert(h.end(), name.begin(), name.end());
        for (const uint16_t v : {length, p1, p2}) {
            h.push_back(static_cast<uint8_t>(v & 0xFF));
            h.push_back(static_cast<uint8_t>(v >> 8));
        }
        block(0x00, h, fix);
    };
    if (start_set_ && start_addr_ != 0) {
        char clear[8], usr[8];
        std::snprintf(clear, sizeof(clear), "%05d", static_cast<int>(base) - 1);
        std::snprintf(usr, sizeof(usr), "%05d", start_addr_ & 0xFFFF);
        std::vector<uint8_t> prog{0x00, 0x0A, 0x1A, 0x00, 0xFD, 0xB0, 0x22};
        prog.insert(prog.end(), clear, clear + 5);
        for (const uint8_t b : {0x22, 0x3A, 0xEF, 0x22, 0x22, 0xAF, 0x3A, 0xF9, 0xC0, 0xB0, 0x22}) prog.push_back(b);
        prog.insert(prog.end(), usr, usr + 5);
        prog.push_back(0x22);
        prog.push_back(0x0D);
        header(0x00, static_cast<uint16_t>(prog.size()), 10, static_cast<uint16_t>(prog.size()), 1);
        block(0xFF, prog, 2);
    }
    header(0x03, static_cast<uint16_t>(raw.size()), static_cast<uint16_t>(base), 0);
    block(0xFF, raw);
    result_.outs["tap"] = std::move(tap);
}

void Assembler::EmitMega(uint8_t b) {
    if (!mega_valid_) Fail(36, "code/data without subpage");
    const int64_t off = real_ - mega_at_;
    if (off >= static_cast<int64_t>(MegaPageSize())) Fail(32, "megaROM subpage overflow");
    const int64_t index = static_cast<int64_t>(mega_cur_) * static_cast<int64_t>(MegaPageSize()) + off;
    if (index < 0) Fail(36, "code/data without subpage"); // (o asMSX original trava aqui)
    if (static_cast<size_t>(index) >= mega_rom_.size()) mega_rom_.resize(static_cast<size_t>(index) + 1, 0);
    mega_rom_[static_cast<size_t>(index)] = b;
    any_written_ = true;
    emitted_any_ = true;
    last_emit_file_ = cur_file_;
    last_emit_line_ = cur_line_;
    ++real_;
    ++pc_;
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
