// fwMSX -- Mamute: listagens em PDF (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md, "Fase 3".
//
// O Mamute original mandava a listagem dos comandos P, LP, V, A P e A H para um PDF A4 em fonte Courier. Este gerador escreve um PDF 1.4
// minimo (texto monoespacado, uma pagina quantas forem precisas) sem biblioteca nenhuma.
#pragma once

#include <string>
#include <vector>

namespace mamute {

// Grava `lines` em `path` como PDF A4 (Courier 9 pt, ~68 linhas por pagina) com `title` e o numero da pagina no cabecalho de cada folha.
// Caracteres fora do ASCII viram '?'. false (+error) se nao foi possivel gravar.
bool WritePdf(const std::string &path, const std::string &title, const std::vector<std::string> &lines, std::string &error);

// Paginacao pedida por quem imprime (o editor: Print setup). Uma linha com so' "\f" (form feed) encerra a folha.
struct PdfLayout {
    int lines_per_page = 0; // linhas de texto por folha (0 = quantas couberem); nunca passa do que cabe na A4
    int top_blank = 0;      // linhas em branco no alto de cada folha (margem superior)
    int left_cols = 0;      // colunas em branco a esquerda (margem esquerda)
};

// Como WritePdf/BuildPdf, mas paginando como `layout` manda; linhas mais largas que a folha continuam na linha seguinte.
std::string BuildPdfLayout(const std::string &title, const std::vector<std::string> &lines, const PdfLayout &layout);
bool WritePdfLayout(const std::string &path, const std::string &title, const std::vector<std::string> &lines, const PdfLayout &layout, std::string &error);

// Monta o PDF na memoria (a gravacao acima usa isto; os testes tambem).
std::string BuildPdf(const std::string &title, const std::vector<std::string> &lines);

// true se `path` termina em .pdf (sem diferenciar maiusculas).
bool IsPdfName(const std::string &path);

} // namespace mamute
