// fwMSX -- Basic Dignified: tipos internos (Token, Position, Description, Lexer, Parser). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Portado do badig.py v2.2.1 mantendo a estrutura (lexer por prefixo que cresce, parser em 5 passadas, tokens compartilhados entre as listas):
// o resultado precisa sair IGUAL ao do original, inclusive nos detalhes que parecem acidentais (o `PROGRAM` do fim vira a variavel `zz`).
#pragma once

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "badig.h"

namespace badig {

// ---------------------------------------------------------------------------------------------------------------
// posicao e token
// ---------------------------------------------------------------------------------------------------------------

struct Line {
    int nmbr = 0;
    std::string text;
    std::string file;
};
using Code = std::vector<Line>;
using CodePtr = std::shared_ptr<Code>;

struct Position;
using PosPtr = std::shared_ptr<Position>;

struct Position {
    CodePtr d_code;
    int lin = 0, col = 0, offset = 0;
    std::string text, file;
    int code_len = 0;

    Position() = default;
    // As colunas comecam em 1: o construtor soma 1 ao que recebe (e Copy() passa pelo construtor, entao copiar tambem soma).
    Position(const CodePtr &dc, int lin_, int col_, int offset_ = 0);
    PosPtr Copy(std::optional<int> lin_ = {}, std::optional<int> col_ = {}, std::optional<int> offset_ = {}) const;
};

struct Token {
    std::string tok;       // em maiusculas; vazio = nenhum grupo casou (None)
    std::string val;
    bool is_int = false;   // numero de linha / destino de desvio (o original guarda int)
    long long ival = 0;
    PosPtr pos;
    int bad_cp = 0;        // -tr: ponto de codigo > 256 sem equivalente MSX (o erro sai na geracao)

    Token() = default;
    Token(const std::string &t, const std::string &v, PosPtr p);
    static std::shared_ptr<Token> MakeInt(const std::string &t, long long v, PosPtr p);
    void SetInt(long long v) { is_int = true; ival = v; val = std::to_string(v); }
    void SetStr(const std::string &v) { is_int = false; ival = 0; val = v; }
    bool Truthy() const { return is_int ? ival != 0 : !val.empty(); }
    std::string UVal() const;
    std::string LVal() const;
    std::string VarName() const;   // o identificador sem o tipo, em minusculas
    std::string VarType() const;   // o sufixo ($ % ! #) ou ""
    std::string ValWithFile() const; // "nome@arquivo", em minusculas
    std::shared_ptr<Token> Copy() const;
};
using TokPtr = std::shared_ptr<Token>;

// ---------------------------------------------------------------------------------------------------------------
// utilitarios de texto com a semantica do Python
// ---------------------------------------------------------------------------------------------------------------

std::string PyUpper(const std::string &s);   // so' ASCII
std::string PyLower(const std::string &s);
std::string PyStrip(const std::string &s, const std::string &chars); // strip(chars)
std::string PyStripWs(const std::string &s);                          // strip()
std::string PyLStripWs(const std::string &s);
std::string PyRStripWs(const std::string &s);
bool IsPySpace(unsigned char c);
void SetLatin1Mode(bool latin1); // false com -tr (fonte UTF-8)
int DisplayCol(const Position &p); // a coluna que o original mostraria: com UTF-8 conta caracteres, nao bytes
std::string PyExpandTabs(const std::string &s, int tab);
std::string BaseName(const std::string &path);
std::string DirName(const std::string &path);
std::string JoinPath(const std::string &dir, const std::string &name);
bool IsAbsPath(const std::string &p);
// repr(str)[1:-1] devolvido em UTF-8 (os relatorios sao gravados nessa codificacao). `utf8_input`: `s` ja' e' UTF-8 (-tr antes da traducao);
// senao cada byte e' um caractere latin1.
std::string PyRepr(const std::string &s, bool utf8_input);

// ---------------------------------------------------------------------------------------------------------------
// o registro de mensagens (infolog do original)
// ---------------------------------------------------------------------------------------------------------------

struct BadigStop {}; // lancada depois de registrar um erro: a conversao para

class Logger {
public:
    int level = 5;
    std::vector<std::string> lines;
    std::vector<Diag> diags;

    // lvl 1 = erro (sempre para), 2 = aviso, 3+ = informacao. `tok` e' o token de onde vem a posicao.
    void Log(int lvl, const std::string &desc, const Token *tok = nullptr, const char *bullet = nullptr, bool show_file = false);
    [[noreturn]] void Fatal(const std::string &desc, const Token *tok = nullptr);
    [[noreturn]] void Fatal(const std::string &desc, const TokPtr &tok) { Fatal(desc, tok.get()); }
    void Warn(const std::string &desc, const Token *tok = nullptr) { Log(2, desc, tok); }
    void Warn(const std::string &desc, const TokPtr &tok) { Warn(desc, tok.get()); }
};

// ---------------------------------------------------------------------------------------------------------------
// configuracao ja' processada (o Settings do badig.py)
// ---------------------------------------------------------------------------------------------------------------

struct Cfg {
    Options o;
    std::string file_load;     // o fonte como foi dado
    std::string file_save;     // o .amx
    std::string file_path;     // pasta do fonte (para os .include)
    std::string general_spaces = " ";
    std::string header1 = "Converted with Basic Dignified";
    std::string header2 = "https://github.com/farique1/basic-dignified";
    bool load_utf8 = false;    // -tr: o fonte e' UTF-8; senao latin1 (um byte = um caractere)
    std::map<std::string, std::string> remtags; // ##BB:nome=valor (nome em maiusculas)
};

// Carrega um arquivo como o IO.load_file: a linha 0 e' "PROGRAM"; as outras trazem o texto com os TABs expandidos e a quebra '\n'.
CodePtr LoadFile(const std::string &path, const std::string *text, int tab_length, Logger &log, const Token *tk);

// ---------------------------------------------------------------------------------------------------------------
// a descricao da linguagem (Dignified + MSX): o que o lexer sabe reconhecer e o estado do parser
// ---------------------------------------------------------------------------------------------------------------

struct OrderedMap { // dict do Python (ordem de insercao)
    std::vector<std::pair<std::string, std::string>> items;
    int Find(const std::string &k) const;
    bool Has(const std::string &k) const { return Find(k) >= 0; }
    const std::string &At(const std::string &k) const { return items[static_cast<size_t>(Find(k))].second; }
    void Set(const std::string &k, const std::string &v);
};

struct DefineGroup {
    std::vector<TokPtr> repl;
    std::vector<TokPtr> varv;
    bool has_var = false;
};

struct FuncDef {
    std::vector<std::vector<TokPtr>> args, rets, equl;
};

class Description {
public:
    std::map<std::string, DefineGroup> defines;
    OrderedMap declares;
    std::vector<std::string> keeps;
    std::map<std::string, FuncDef> functions;
    std::set<std::string> hard_short_vars;
    std::vector<TokPtr> hard_long_vars;

    // O grupo (nome do token em maiusculas) que casa a palavra INTEIRA `s`, ou "" se nenhum casa. Ordem do original: principais, Dignified, classicos,
    // e por ultimo o identificador.
    static const char *Classify(const std::string &s);
    static bool IsReserved(const std::string &upper); // c_reserved_kw (com os cortes do original)
    static bool IsFuncStop(const std::string &upper); // c_func_stop_kw
    static bool IsSymbComp(const std::string &v);     // c_symb_comp
    static bool IsIdentf(const std::string &s);       // d_identf
};

// ---------------------------------------------------------------------------------------------------------------
// lexer
// ---------------------------------------------------------------------------------------------------------------

class Lexer {
public:
    Lexer(const Cfg &cfg, CodePtr code, Logger &log);
    std::vector<TokPtr> Lex();
    Description des;

private:
    std::optional<char> Advance();
    std::optional<std::string> AdvanceLine();
    char Lookahead() const;
    TokPtr GetToken();
    // nullopt = erro (bloco nao fechado)
    std::optional<std::vector<TokPtr>> GetLitBlock(const TokPtr &tk);
    TokPtr GetLitLine(const std::string &tok, char end, const std::string &part, PosPtr l_pos);
    std::vector<TokPtr> GetDataLine(bool &has);

    const Cfg &cfg_;
    Logger &log_;
    CodePtr code_;
    PosPtr pos_;
    std::vector<TokPtr> lexer_;
    TokPtr tk_, last_tok_;
};

// ---------------------------------------------------------------------------------------------------------------
// parser
// ---------------------------------------------------------------------------------------------------------------

struct IncludeVars {
    std::set<std::string> shorts;
    std::vector<TokPtr> longs;
    OrderedMap declares;
};

struct Generated {
    std::vector<std::string> c_code;
    OrderedMap declares;
    struct LineRep { long long line; int dig_line; std::string text; std::string file; };
    std::vector<LineRep> line_report;
};

// Tokens a partir de um texto solto (o tok_str do original).
std::vector<TokPtr> TokStr(const Description &des, const std::string &text, PosPtr pos);

class Parser {
public:
    Parser(const Cfg &cfg, std::vector<TokPtr> lexed, Description &des, const IncludeVars &iv, Logger &log);
    // Roda as passadas (as do include so' ate' a 3) e devolve a lista de tokens e as variaveis.
    std::vector<TokPtr> Par(IncludeVars &out);
    Generated Generate();

    std::vector<TokPtr> TokListOut() const { return out_; }

private:
    enum class K { False, None, Tok };
    struct Ctk {
        K k = K::False;
        TokPtr tk;
    };

    // navegacao
    void ResetPass();
    TokPtr NextTok();
    TokPtr PrevTok();
    TokPtr PeekAhead(int offset = 1) const;
    TokPtr LastTok(int offset = -1) const;
    std::vector<TokPtr> TokStrHere(const std::string &text, PosPtr pos = nullptr);

    // defines
    void GetDefines();
    struct DefContent {
        std::vector<TokPtr> part;
        std::vector<DefContent> var;
    };
    DefContent GetDefinedContent(const std::string &delim, int recur);
    std::vector<TokPtr> ReplaceDefine();
    // declares
    void GetDeclares();
    // labels
    void GetLabelLines();
    void GetJumpLabels();
    void GetLoopLabelReturn();
    void GetLabelsExit();
    TokPtr GetInBracket(const std::string &delim, bool &blank);
    // funcoes
    void GetFuncDef();
    void GetFuncRet();
    std::vector<TokPtr> ReplaceFuncCalls();
    static bool CompareFuncArgs(const std::vector<TokPtr> &a, const std::vector<TokPtr> &b);
    std::vector<std::vector<TokPtr>> GetFuncArgs(const TokPtr &func);
    // toggles, include
    void GetKeeps();
    void ToggleLines();
    void IncludeFile();
    void InsertRemHeader(size_t idx, int lin, const std::string &text);
    bool IsMainFile() const;
    // passadas
    void Pass1();
    void Pass2();
    void Pass3();
    void Pass4();
    void Pass5();
    // o modulo classico (MSX)
    Ctk ClcPass5();
    void GetHardVariable(const TokPtr &tk);
    TokPtr ProcessVariable(const TokPtr &tk);
    std::string TransChar(const std::string &text, TokPtr &tk);
    bool ClcGenerate(const TokPtr &token, size_t n);

    const Cfg &cfg_;
    Logger &log_;
    Description &des_;
    std::vector<TokPtr> in_, out_;
    size_t len_ = 0, index_ = 0;
    TokPtr tk_;
    std::vector<TokPtr> loop_labels_;
    TokPtr in_func_;
    std::vector<std::vector<TokPtr>> func_def_args_, func_def_equl_;
    int var_index_ = 676;
    // relatorios da passada 4
    std::vector<Generated::LineRep> line_report_;
    std::map<long long, std::vector<std::string>> label_report_;
    // geracao
    std::vector<TokPtr> line_;
    std::string c_line_;
    size_t gen_n_ = 0;
    TokPtr gen_token_;
};

} // namespace badig
