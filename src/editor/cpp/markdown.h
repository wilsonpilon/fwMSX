// fwMSX -- editor de textos: o preview do Markdown (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/editor-spec.md (E7).
//
// Transforma o texto Markdown em linhas de texto simples, ja' quebradas na largura: igual na TUI e na janela grafica (a lista de rolagem do
// dialogo so' mostra texto). Cobre o que se escreve nos docs do projeto: titulos, listas, citacoes, blocos de codigo, tabelas, linhas
// horizontais e, dentro do texto, negrito/italico/codigo/links/imagens.
#pragma once

#include <string>
#include <vector>

namespace ed {

// `width` = colunas disponiveis (minimo 20). Os titulos de nivel 1 e 2 levam um sublinhado (=== e ---); a lista vira "* item"; a citacao "| texto";
// o codigo fica recuado em 4 colunas; [texto](url) vira "texto (url)" e ![alt](url) vira "[imagem: alt]".
std::vector<std::string> RenderMarkdown(const std::vector<std::string> &lines, int width);

// So' o tratamento do texto corrido de uma linha (negrito, italico, codigo, links, imagens): exposto para os testes.
std::string MarkdownInline(const std::string &s);

} // namespace ed
