// fwMSX -- montador (fwasm): expressoes numericas (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause), escrito a
// partir do manual do asMSX (secao 2.3) e do comportamento observado do asMSX 1.2.0. Ver doc/asm-spec.md.
//
// Particularidades do asMSX reproduzidas aqui (conferidas por tests/asm/golden/expr.golden):
//  - aritmetica de 32 bits com volta (wrap); deslocamento com a contagem mascarada em 5 bits (como no x86);
//  - precedencia (do mais fraco ao mais forte): `+ - | ^`, `<< >>`, `* / % &`, `&& ||`, `== != < <= > >=`, todos
//    associativos a esquerda -- NAO e' a de C, apesar do manual;
//  - dois tipos: inteiro e ponto flutuante. Literais decimais e as funcoes matematicas so' existem em contexto de
//    ponto flutuante: INT()/FIX() convertem para inteiro; um inteiro puro em lugar de float e' erro de sintaxe;
//  - `--` comeca um comentario (tratado antes, no pre-processador), entao `2--1` vale 2.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "lexer.h"

namespace fwasm {

struct Val {
    bool is_float = false;
    int32_t i = 0;
    double f = 0.0;
    bool unknown = false; // depende de um simbolo ainda nao definido (primeira passada)
};

enum class ExprErr { None, Syntax, Undefined, DivZero, Reserved, Unterminated };

// O que a expressao precisa do montador.
class ExprContext {
public:
    virtual ~ExprContext() = default;
    virtual int32_t Pc() const = 0; // valor de $
    // Procura um simbolo (rotulo, constante ou variavel). false = nao definido; `unknown` = ainda indefinido na
    // primeira passada (valor provisorio 0).
    virtual bool Lookup(const std::string &name, int32_t &value, bool &unknown) = 0;
    // Palavra reservada que nao pode ser identificador (registrador, mnemonico, funcao...): erro de sintaxe.
    virtual bool IsSyntaxReserved(const std::string &lower) const = 0;
    // Diretiva usada como identificador: "reserved word used as identifier".
    virtual bool IsDirectiveWord(const std::string &lower) const = 0;
    // Sorteio de .RANDOM(n).
    virtual int32_t Random(int32_t n) = 0;
    bool zilog = false; // `[ ]` agrupam em vez de `( )`
};

// Analisa uma expressao a partir de `tokens[pos]`; para no primeiro token que nao continua a expressao e deixa
// `pos` nele. Devolve false em erro (preenche `err`).
bool ParseExpression(const std::vector<Token> &tokens, size_t &pos, ExprContext &ctx, Val &out, ExprErr &err);

// Corta para a faixa de `bits` (8, 16): devolve `true` se o valor passou da faixa (aviso de overflow).
bool NeedsCrop(int32_t v, int bits);

} // namespace fwasm
