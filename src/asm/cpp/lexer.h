// fwMSX -- montador (fwasm): analisador lexico de uma linha de fonte (C++). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause), escrito a partir do manual do asMSX e do comportamento observado do asMSX 1.2.0
// (nao do fonte dele). Ver doc/asm-spec.md.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace fwasm {

struct Token {
    enum Kind { End, Ident, Int, Float, String, Char, Op };
    Kind kind = End;
    std::string text;   // Ident: o nome; String: o conteudo; Op: o operador
    int32_t ival = 0;   // Int / Char
    double fval = 0.0;  // Float
};

// Resultado do lexer: `error` != "" se a linha tem um token invalido (ou "unterminated").
struct LexResult {
    std::vector<Token> tokens;
    std::string error; // "syntax error" ou "Unterminated string"
};

// Quebra uma linha (ja' sem comentarios) em tokens. `zilog` nao afeta o lexer (so' o analisador).
LexResult Lex(const std::string &line);

} // namespace fwasm
