// fwMSX -- Mamute: a calculadora do comando CL (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver mamute_calc.h.
#include "mamute_calc.h"

#include <cctype>

#include "mamute_util.h"

namespace mamute {

namespace {

bool InBase(const std::string &digits, int base) {
    if (digits.empty()) return false;
    for (const char c : digits) {
        int d;
        if (std::isdigit(static_cast<unsigned char>(c))) d = c - '0';
        else if (std::isxdigit(static_cast<unsigned char>(c))) d = std::toupper(static_cast<unsigned char>(c)) - 'A' + 10;
        else return false;
        if (d >= base) return false;
    }
    return true;
}

long ToValue(const std::string &digits, int base) {
    long v = 0;
    for (const char c : digits) {
        const int d = std::isdigit(static_cast<unsigned char>(c)) ? c - '0' : std::toupper(static_cast<unsigned char>(c)) - 'A' + 10;
        v = (v * base + d) & 0xFFFF; // a volta em cada passo: "FFFFFFF" nao estoura, so' da' a volta
    }
    return v;
}

class Parser {
public:
    Parser(const std::string &s, const Vars *vars) : s_(s), vars_(vars) {}

    bool Run(int *out, std::string &error) {
        long v = 0;
        if (!Or(v)) {
            error = error_;
            return false;
        }
        Skip();
        if (pos_ < s_.size()) {
            error = "ERRO DE SINTAXE";
            return false;
        }
        *out = static_cast<int>(v & 0xFFFF);
        return true;
    }

private:
    void Skip() {
        while (pos_ < s_.size() && std::isspace(static_cast<unsigned char>(s_[pos_]))) ++pos_;
    }
    bool Peek(char c) {
        Skip();
        return pos_ < s_.size() && s_[pos_] == c;
    }
    bool Fail(const std::string &m) {
        if (error_.empty()) error_ = m;
        return false;
    }

    bool Or(long &v) {
        if (!Xor(v)) return false;
        while (Peek('|')) {
            ++pos_;
            long r = 0;
            if (!Xor(r)) return false;
            v = (v | r) & 0xFFFF;
        }
        return true;
    }
    bool Xor(long &v) {
        if (!And(v)) return false;
        while (Peek('^')) {
            ++pos_;
            long r = 0;
            if (!And(r)) return false;
            v = (v ^ r) & 0xFFFF;
        }
        return true;
    }
    bool And(long &v) {
        if (!Add(v)) return false;
        while (Peek('&')) {
            ++pos_;
            long r = 0;
            if (!Add(r)) return false;
            v = (v & r) & 0xFFFF;
        }
        return true;
    }
    bool Add(long &v) {
        if (!Mul(v)) return false;
        while (Peek('+') || Peek('-')) {
            const char op = s_[pos_++];
            long r = 0;
            if (!Mul(r)) return false;
            v = (op == '+' ? v + r : v - r) & 0xFFFF;
        }
        return true;
    }
    bool Mul(long &v) {
        if (!Unary(v)) return false;
        while (Peek('*') || Peek('/') || Peek('%')) {
            const char op = s_[pos_++];
            long r = 0;
            if (!Unary(r)) return false;
            if (op == '*') v = (v * r) & 0xFFFF;
            else if (r == 0) return Fail("DIVISAO POR ZERO");
            else v = (op == '/' ? v / r : v % r) & 0xFFFF;
        }
        return true;
    }
    bool Unary(long &v) {
        Skip();
        if (Peek('-')) {
            ++pos_;
            if (!Unary(v)) return false;
            v = (-v) & 0xFFFF;
            return true;
        }
        if (Peek('+')) {
            ++pos_;
            return Unary(v);
        }
        if (Peek('!')) {
            ++pos_;
            if (!Unary(v)) return false;
            v = (~v) & 0xFFFF;
            return true;
        }
        return Primary(v);
    }
    bool Primary(long &v) {
        Skip();
        if (pos_ >= s_.size()) return Fail("ERRO DE SINTAXE");
        const char c = s_[pos_];
        if (c == '(') {
            ++pos_;
            if (!Or(v)) return false;
            if (!Peek(')')) return Fail("ERRO DE SINTAXE");
            ++pos_;
            return true;
        }
        if (c == '@') { // @0 .. @3, @B, @E, @S: so' o numero, sem o slot
            if (pos_ + 1 >= s_.size()) return Fail("ERRO DE SINTAXE");
            const Vars::Slot *slot = vars_ ? vars_->Find(s_[pos_ + 1]) : nullptr;
            if (!slot) return Fail(std::string("VARIAVEL INVALIDA: @") + s_[pos_ + 1]);
            if (!slot->has) return Fail(std::string("VARIAVEL SEM VALOR: @") + s_[pos_ + 1]);
            v = static_cast<long>(slot->v.addr & 0xFFFF);
            pos_ += 2;
            return true;
        }
        if (c == '\'' || c == '"') { // literal ASCII de 1 ou 2 caracteres
            const size_t close = s_.find(c, pos_ + 1);
            if (close == std::string::npos) return Fail("ERRO DE SINTAXE");
            const std::string lit = s_.substr(pos_ + 1, close - pos_ - 1);
            if (lit.empty() || lit.size() > 2) return Fail("LITERAL ASCII DE 1 OU 2 CARACTERES");
            v = lit.size() == 1 ? static_cast<unsigned char>(lit[0]) : (static_cast<unsigned char>(lit[0]) << 8) | static_cast<unsigned char>(lit[1]);
            pos_ = close + 1;
            return true;
        }
        if (std::isalnum(static_cast<unsigned char>(c))) {
            size_t e = pos_;
            while (e < s_.size() && std::isalnum(static_cast<unsigned char>(s_[e]))) ++e;
            const std::string tok = s_.substr(pos_, e - pos_);
            pos_ = e;
            return Number(tok, v);
        }
        return Fail(std::string("CARACTERE INVALIDO: ") + c);
    }

    bool Number(const std::string &tok, long &v) {
        const char last = static_cast<char>(std::toupper(static_cast<unsigned char>(tok.back())));
        const std::string head = tok.substr(0, tok.size() - 1);
        // o sufixo so' vale se os digitos antes dele forem validos naquela base; senao e' tudo hexa ("1B" = 1B, "10B" = 2 binario)
        if (last == 'D' && InBase(head, 10)) v = ToValue(head, 10);
        else if (last == 'B' && InBase(head, 2)) v = ToValue(head, 2);
        else if (last == 'H' && InBase(head, 16)) v = ToValue(head, 16);
        else if (last == 'O' && InBase(head, 8)) v = ToValue(head, 8);
        else if (InBase(tok, 16)) v = ToValue(tok, 16);
        else return Fail("NUMERO INVALIDO: " + tok);
        return true;
    }

    const std::string &s_;
    const Vars *vars_;
    size_t pos_ = 0;
    std::string error_;
};

} // namespace

bool CalcEval(const std::string &expr, const Vars *vars, int *out, std::string &error) {
    error.clear();
    if (util::Trim(expr).empty()) {
        error = "ERRO DE SINTAXE";
        return false;
    }
    Parser p(expr, vars);
    return p.Run(out, error);
}

} // namespace mamute
