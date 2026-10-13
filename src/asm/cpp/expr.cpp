// fwMSX -- montador (fwasm): expressoes numericas. Ver expr.h.
#include "expr.h"

#include <cctype>
#include <cmath>

// Funcoes matematicas em Fortran (src/asm/fortran/asm_math.f90).
extern "C" {
double asm_sin(double);
double asm_cos(double);
double asm_tan(double);
double asm_acos(double);
double asm_asin(double);
double asm_atan(double);
double asm_sqr(double);
double asm_sqrt(double);
double asm_exp(double);
double asm_pow(double, double);
double asm_log(double);
double asm_ln(double);
double asm_abs(double);
int64_t asm_int(double);
int64_t asm_fix(double);
int64_t asm_fixmul(int64_t, int64_t);
int64_t asm_fixdiv(int64_t, int64_t);
}

namespace fwasm {

bool NeedsCrop(int32_t v, int bits) {
    if (bits == 8) return v < -128 || v > 255;
    if (bits == 16) return v < -32768 || v > 65535;
    return false;
}

namespace {

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

int32_t Wrap(int64_t v) { return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(v) & 0xFFFFFFFFull)); }

// Conversao double -> int32 como o x86 (cvttsd2si): fora da faixa ou NaN vira INT_MIN, que depois estoura os 8/16 bits.
int32_t ToInt32(double x) {
    if (!(x > -2147483649.0 && x < 2147483648.0)) return INT32_MIN;
    return static_cast<int32_t>(x);
}

Val MakeInt(int32_t v) {
    Val r;
    r.i = v;
    return r;
}

Val MakeFloat(double f) {
    Val r;
    r.is_float = true;
    r.f = f;
    return r;
}

double AsDouble(const Val &v) { return v.is_float ? v.f : static_cast<double>(v.i); }

class Parser {
public:
    Parser(const std::vector<Token> &t, size_t &pos, ExprContext &ctx) : t_(t), pos_(pos), ctx_(ctx) {}

    bool Run(Val &out, ExprErr &err) {
        out = Level(0);
        if (err_ != ExprErr::None) {
            err = err_;
            return false;
        }
        return true;
    }

private:
    const Token *Peek() const { return pos_ < t_.size() ? &t_[pos_] : nullptr; }
    bool IsOp(const char *op) const {
        const Token *p = Peek();
        return p && p->kind == Token::Op && p->text == op;
    }
    void Fail(ExprErr e) {
        if (err_ == ExprErr::None) err_ = e;
    }
    bool Failed() const { return err_ != ExprErr::None; }

    // Precedencia observada no asMSX 1.2.0 (do mais fraco ao mais forte), todos associativos a esquerda:
    //   0: + - | ^      1: << >>      2: * / % &      3: && ||
    //   4: operadores unarios prefixos (- ~ !): o operando e' uma cadeia de COMPARACOES inteira, entao -3==3 e'
    //      -(3==3); mas -3+2 e' (-3)+2
    //   5: comparacoes (== != < <= > >=), as mais fortes de todas.
    static int LevelOf(const std::string &op) {
        if (op == "+" || op == "-" || op == "|" || op == "^") return 0;
        if (op == "<<" || op == ">>") return 1;
        if (op == "*" || op == "/" || op == "%" || op == "&") return 2;
        if (op == "&&" || op == "||") return 3;
        return -1;
    }
    static bool IsCompare(const std::string &op) {
        return op == "==" || op == "!=" || op == "<" || op == "<=" || op == ">" || op == ">=";
    }

    Val Level(int level) {
        if (level > 3) return Unary();
        Val lhs = Level(level + 1);
        while (!Failed()) {
            const Token *p = Peek();
            if (!p || p->kind != Token::Op || LevelOf(p->text) != level) break;
            const std::string op = p->text;
            ++pos_;
            Val rhs = Level(level + 1);
            if (Failed()) break;
            lhs = Binary(op, lhs, rhs);
        }
        return lhs;
    }

    Val Binary(const std::string &op, const Val &a, const Val &b) {
        const bool unknown = a.unknown || b.unknown;
        if (a.is_float || b.is_float) {
            // Ponto flutuante: so' + - * /.
            if (op != "+" && op != "-" && op != "*" && op != "/") {
                Fail(ExprErr::Syntax);
                return Val{};
            }
            const double x = AsDouble(a), y = AsDouble(b);
            double r = 0.0;
            if (op == "+") r = x + y;
            else if (op == "-") r = x - y;
            else if (op == "*") r = x * y;
            else {
                if (y == 0.0 && !unknown) { // float / 0: "memory overflow", como a divisao inteira
                    Fail(ExprErr::DivZero);
                    return Val{};
                }
                r = y == 0.0 ? 0.0 : x / y;
            }
            Val v = MakeFloat(r);
            v.unknown = unknown;
            return v;
        }
        const int32_t x = a.i, y = b.i;
        int32_t r = 0;
        if (op == "+") r = Wrap(static_cast<int64_t>(x) + y);
        else if (op == "-") r = Wrap(static_cast<int64_t>(x) - y);
        else if (op == "*") r = Wrap(static_cast<int64_t>(x) * y);
        else if (op == "/" || op == "%") {
            if (y == 0) {
                if (!unknown) Fail(ExprErr::DivZero);
                return MakeInt(0);
            }
            if (x == INT32_MIN && y == -1) r = (op == "/") ? INT32_MIN : 0;
            else r = (op == "/") ? x / y : x % y;
        } else if (op == "<<") r = static_cast<int32_t>(static_cast<uint32_t>(x) << (static_cast<uint32_t>(y) & 31u));
        else if (op == ">>") r = x >> (static_cast<uint32_t>(y) & 31u);
        else if (op == "|") r = x | y;
        else if (op == "&") r = x & y;
        else if (op == "^") r = x ^ y;
        else if (op == "&&") r = (x != 0 && y != 0) ? 1 : 0;
        else if (op == "||") r = (x != 0 || y != 0) ? 1 : 0;
        else if (op == "==") r = x == y;
        else if (op == "!=") r = x != y;
        else if (op == "<") r = x < y;
        else if (op == "<=") r = x <= y;
        else if (op == ">") r = x > y;
        else if (op == ">=") r = x >= y;
        Val v = MakeInt(r);
        v.unknown = unknown;
        return v;
    }

    Val ApplyPrefix(const std::string &op, Val v) {
        if (Failed()) return v;
        if (v.is_float) {
            if (op != "-") {
                Fail(ExprErr::Syntax);
                return Val{};
            }
            v.f = -v.f;
            return v;
        }
        if (op == "-") v.i = Wrap(-static_cast<int64_t>(v.i));
        else if (op == "~") v.i = ~v.i;
        else v.i = (v.i == 0) ? 1 : 0;
        return v;
    }

    bool PeekPrefix(std::string &op) const {
        const Token *p = Peek();
        if (p && p->kind == Token::Op && (p->text == "-" || p->text == "~" || p->text == "!")) {
            op = p->text;
            return true;
        }
        return false;
    }

    // Nivel 4: prefixo unario sobre uma cadeia de comparacoes.
    Val Unary() {
        std::string op;
        if (PeekPrefix(op)) {
            ++pos_;
            return ApplyPrefix(op, Unary());
        }
        return Compare();
    }

    // Operando de uma comparacao: um primario, com prefixos unarios "colados" (5==-3 vale 5==(-3)).
    Val CmpOperand() {
        std::string op;
        if (PeekPrefix(op)) {
            ++pos_;
            return ApplyPrefix(op, CmpOperand());
        }
        return Primary();
    }

    // Nivel 5: comparacoes.
    Val Compare() {
        Val lhs = CmpOperand();
        while (!Failed()) {
            const Token *p = Peek();
            if (!p || p->kind != Token::Op || !IsCompare(p->text)) break;
            const std::string op = p->text;
            ++pos_;
            Val rhs = CmpOperand();
            if (Failed()) break;
            lhs = Binary(op, lhs, rhs);
        }
        return lhs;
    }

    // Argumentos de uma funcao: "(" expr {"," expr} ")".
    bool Args(std::vector<Val> &args, size_t want) {
        if (!IsOp("(")) {
            Fail(ExprErr::Syntax);
            return false;
        }
        ++pos_;
        for (size_t k = 0; k < want; ++k) {
            if (k > 0) {
                if (!IsOp(",")) {
                    Fail(ExprErr::Syntax);
                    return false;
                }
                ++pos_;
            }
            args.push_back(Level(0));
            if (Failed()) return false;
        }
        if (!IsOp(")")) {
            Fail(ExprErr::Syntax);
            return false;
        }
        ++pos_;
        return true;
    }

    Val Function(const std::string &name) {
        static const struct {
            const char *name;
            double (*f1)(double);
        } kFloatFns[] = {{"sin", asm_sin}, {"cos", asm_cos}, {"tan", asm_tan}, {"acos", asm_acos}, {"asin", asm_asin},
                         {"atan", asm_atan}, {"sqr", asm_sqr}, {"sqrt", asm_sqrt}, {"exp", asm_exp}, {"log", asm_log},
                         {"ln", asm_ln}, {"abs", asm_abs}};
        std::vector<Val> a;
        for (const auto &fn : kFloatFns) {
            if (name != fn.name) continue;
            if (!Args(a, 1)) return Val{};
            if (!a[0].is_float) {
                Fail(ExprErr::Syntax);
                return Val{};
            }
            Val r = MakeFloat(fn.f1(a[0].f));
            r.unknown = a[0].unknown;
            return r;
        }
        if (name == "pow") {
            if (!Args(a, 2)) return Val{};
            if (!a[0].is_float || !a[1].is_float) {
                Fail(ExprErr::Syntax);
                return Val{};
            }
            Val r = MakeFloat(asm_pow(a[0].f, a[1].f));
            r.unknown = a[0].unknown || a[1].unknown;
            return r;
        }
        if (name == "int" || name == "fix") {
            if (!Args(a, 1)) return Val{};
            if (!a[0].is_float) {
                Fail(ExprErr::Syntax);
                return Val{};
            }
            Val r = MakeInt(ToInt32(name == "int" ? static_cast<double>(asm_int(a[0].f)) : static_cast<double>(asm_fix(a[0].f))));
            r.unknown = a[0].unknown;
            return r;
        }
        if (name == "fixmul" || name == "fixdiv") {
            if (!Args(a, 2)) return Val{};
            if (a[0].is_float || a[1].is_float) {
                Fail(ExprErr::Syntax);
                return Val{};
            }
            const bool unknown = a[0].unknown || a[1].unknown;
            // FIXDIV(x,0) nao da' erro no asMSX: o resultado estoura (INT_MIN) e sai o aviso de corte de 16 bits.
            if (name == "fixdiv" && a[1].i == 0) {
                Val z = MakeInt(INT32_MIN);
                z.unknown = unknown;
                return z;
            }
            Val r = MakeInt(Wrap(name == "fixmul" ? asm_fixmul(a[0].i, a[1].i) : asm_fixdiv(a[0].i, a[1].i)));
            r.unknown = unknown;
            return r;
        }
        if (name == "random" || name == ".random") {
            if (!Args(a, 1)) return Val{};
            if (a[0].is_float) {
                Fail(ExprErr::Syntax);
                return Val{};
            }
            return MakeInt(ctx_.Random(a[0].i));
        }
        Fail(ExprErr::Syntax);
        return Val{};
    }

    static bool IsFunctionName(const std::string &n) {
        static const char *const names[] = {"sin", "cos", "tan", "acos", "asin", "atan", "sqr", "sqrt", "exp", "log", "ln",
                                            "abs", "pow", "int", "fix", "fixmul", "fixdiv", "random", ".random"};
        for (const char *s : names)
            if (n == s) return true;
        return false;
    }

    Val Primary() {
        const Token *p = Peek();
        if (!p) {
            Fail(ExprErr::Syntax);
            return Val{};
        }
        switch (p->kind) {
        case Token::Int:
        case Token::Char: {
            ++pos_;
            return MakeInt(p->ival);
        }
        case Token::Float:
            ++pos_;
            return MakeFloat(p->fval);
        case Token::String: // strings so' existem como itens de DB/DW (tratados na diretiva), nunca em expressoes
            Fail(ExprErr::Syntax);
            return Val{};
        case Token::Op: {
            if (p->text == "$") {
                ++pos_;
                return MakeInt(ctx_.Pc());
            }
            const char *open = ctx_.zilog ? "[" : "(";
            const char *close = ctx_.zilog ? "]" : ")";
            if (p->text == open) {
                ++pos_;
                Val v = Level(0);
                if (Failed()) return v;
                if (!IsOp(close)) {
                    Fail(ExprErr::Syntax);
                    return Val{};
                }
                ++pos_;
                return v;
            }
            Fail(ExprErr::Syntax);
            return Val{};
        }
        case Token::Ident: {
            const std::string lower = Lower(p->text);
            ++pos_;
            if (lower == "pi") return MakeFloat(3.14159265358979323846);
            if (IsFunctionName(lower) && IsOp("(")) return Function(lower);
            if (ctx_.IsSyntaxReserved(lower)) {
                Fail(ExprErr::Syntax);
                return Val{};
            }
            if (ctx_.IsDirectiveWord(lower)) {
                Fail(ExprErr::Reserved);
                return Val{};
            }
            int32_t v = 0;
            bool unknown = false;
            if (!ctx_.Lookup(p->text, v, unknown)) {
                Fail(ExprErr::Undefined);
                return Val{};
            }
            Val r = MakeInt(v);
            r.unknown = unknown;
            return r;
        }
        default:
            Fail(ExprErr::Syntax);
            return Val{};
        }
    }

    const std::vector<Token> &t_;
    size_t &pos_;
    ExprContext &ctx_;
    ExprErr err_ = ExprErr::None;
};

} // namespace

bool ParseExpression(const std::vector<Token> &tokens, size_t &pos, ExprContext &ctx, Val &out, ExprErr &err) {
    Parser p(tokens, pos, ctx);
    return p.Run(out, err);
}

} // namespace fwasm
