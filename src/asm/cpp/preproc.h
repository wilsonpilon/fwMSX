// fwMSX -- montador (fwasm): pre-processador de fonte (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause), escrito
// a partir do manual do asMSX e do comportamento observado do asMSX 1.2.0. Ver doc/asm-spec.md.
//
// Mesma ordem do programa original: (1) leitura, comentarios e INCLUDE; (2) macros; (3) REPT. As condicionais
// (IF/ELSE/ENDIF/IFDEF) NAO sao tratadas aqui: dependem de simbolos e rodam durante a montagem.
#pragma once

#include <string>
#include <vector>

#include "assembler.h"

namespace fwasm {

struct SrcLine {
    std::string file; // arquivo de origem (para as mensagens)
    int line = 0;     // numero da linha (1 = primeira) no arquivo de origem
    std::string text; // texto sem comentarios
    bool commented = false; // a linha tinha comentario (o asMSX nao reconhece MACRO/ENDM com comentario no fim)
};

struct PreResult {
    std::vector<SrcLine> lines;
    int rc = 0;                    // != 0: erro
    std::string error;             // "file, line N: mensagem" (quando rc != 0)
    bool syntax_error = false;     // precede a mensagem com "Parsing error: syntax error"
    std::vector<std::string> warnings;
};

PreResult Preprocess(const std::string &name, const std::string &text, FileProvider *files);

// Retira os comentarios de uma linha (`;`, `//`, `--`, `/* */` e `{ }`); `in_block` carrega o estado entre linhas
// (0 = fora, 1 = dentro de /* */, 2 = dentro de { }). Respeita strings "..." e caracteres 'x'.
std::string StripComments(const std::string &line, int &in_block);

} // namespace fwasm
