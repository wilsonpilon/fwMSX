// fwMSX -- editor de textos: o preview do Markdown (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/editor-spec.md (E7).
//
// Transforma o texto Markdown em linhas ja' quebradas na largura, cada caractere com a sua CLASSE (titulo, negrito, codigo, link...): o editor
// pinta as classes com as cores do esquema atual, igual na TUI e na janela grafica. Cobre o que se escreve nos docs do projeto: titulos, listas,
// citacoes, blocos de codigo, tabelas, linhas horizontais e, dentro do texto, negrito/italico/codigo/links/imagens.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ed {

enum MdClass : uint8_t {
    kMdText = 0,
    kMdH1,
    kMdH2,
    kMdH3,
    kMdBold,
    kMdItalic,
    kMdCode,      // `codigo` no meio do texto
    kMdLink,      // o texto de [texto](url)
    kMdUrl,       // a (url) que vem depois
    kMdImage,     // [imagem: alt]
    kMdQuote,
    kMdBullet,    // o marcador de lista
    kMdRule,      // linha horizontal e sublinhados dos titulos
    kMdBlock,     // linha de um bloco de codigo
    kMdTable
};

struct MdLine {
    std::string text;
    std::vector<uint8_t> cls; // uma MdClass por byte de `text`
};

// `width` = colunas disponiveis (minimo 20). Os titulos 1 e 2 levam um sublinhado (=== e ---); a lista vira "* item"; a citacao "| texto";
// o codigo fica recuado em 4 colunas; [texto](url) vira "texto (url)" e ![alt](url) vira "[imagem: alt]".
std::vector<MdLine> RenderMarkdownStyled(const std::vector<std::string> &lines, int width);

// So' os textos (sem as classes).
std::vector<std::string> RenderMarkdown(const std::vector<std::string> &lines, int width);

// So' o tratamento do texto corrido de uma linha (negrito, italico, codigo, links, imagens): exposto para os testes.
std::string MarkdownInline(const std::string &s);

} // namespace ed
