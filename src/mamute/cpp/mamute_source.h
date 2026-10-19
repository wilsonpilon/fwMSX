// fwMSX -- Mamute: o programa-fonte Z80 do EDIT e a montagem (comando A) (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/mamute-spec.md, "Fase 2".
//
// O fonte do Mamute e' um programa de linhas NUMERADAS no estilo do MegaAssembler / ZX-81:
//     NN [Label:] instrucao operandos [;comentario]
// com numeros HEXADECIMAIS por padrao (sufixos H, B binario, D decimal). Montar e' TRADUZIR cada linha para o asMSX (uma linha do
// asMSX por linha do Mamute, na mesma ordem) e entregar ao fwasm -- nao ha' um segundo montador. Os erros do fwasm voltam com o
// numero de linha do Mamute; a listagem (endereco e bytes de cada linha) vem de AsmResult::listing.
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace mamute {

// Uma linha da listagem do comando A.
struct ListLine {
    int number = 0;
    std::string text;      // o texto da linha como foi digitado (sem o numero)
    bool shown = true;     // ORG e END nao aparecem na listagem
    bool has_addr = false; // comentario e linha em branco nao tem endereco
    bool is_equ = false;   // a coluna do endereco mostra o VALOR do EQU
    int32_t addr = 0;      // endereco da linha, ou o valor do EQU
    std::vector<uint8_t> bytes;
};

struct SymbolInfo {
    std::string name;
    int32_t value = 0;
    bool is_equ = false;
    int def_line = 0;
    std::vector<int32_t> uses; // enderecos das linhas que usam o simbolo (a referencia cruzada)
};

// O resultado de montar o fonte.
struct AsmProduct {
    bool ok = false;
    std::string error;          // mensagem do erro (vazia se ok)
    int error_line = 0;         // numero da linha do Mamute com o erro (0 = erro que nao e' de uma linha)
    std::vector<ListLine> lines;
    std::vector<SymbolInfo> symbols; // em ordem de aparicao no fonte
    std::map<int, uint8_t> image;    // endereco -> byte do codigo-objeto
    int start = 0, end = 0, exec = 0;
    bool has_code = false;
    std::vector<std::string> warnings;
};

class Source {
public:
    // Grava (ou substitui) a linha `number`; texto vazio apaga.
    void Set(int number, const std::string &text);
    void Clear() { lines_.clear(); }
    bool empty() const { return lines_.empty(); }
    size_t size() const { return lines_.size(); }
    const std::map<int, std::string> &lines() const { return lines_; }

    // "10   ORG 0C100H" para as linhas de from a to (inclusive); to < 0 = ate' o fim.
    std::string List(int from, int to) const;
    // Apaga de a ate' b (b < 0 = ate' o fim); devolve quantas apagou.
    int Delete(int a, int b);
    // Renumera as linhas >= old_start para new_start, new_start+inc...; false (+erro) se a ordem ou o limite (65535) nao permitirem.
    bool Renum(int new_start, int old_start, int inc, std::string &error);
    // Troca todas as ocorrencias de `from` por `to` em cada linha; devolve o numero de trocas.
    int Change(const std::string &from, const std::string &to);
    // Numeros das linhas que contem `needle` (case_sensitive = a forma entre aspas do SEARCH).
    std::vector<int> Search(const std::string &needle, bool case_sensitive) const;

    // Arquivo .mza: "NN texto" por linha, ASCII simples. merge = false substitui o programa; true funde (a linha do arquivo vence).
    bool SaveFile(const std::string &path, std::string &error) const;
    bool LoadFile(const std::string &path, bool merge, std::string &error);

    // Monta. `offset` e' somado a TODO ORG (a opcao /offset do A).
    AsmProduct Assemble(int offset) const;

private:
    std::map<int, std::string> lines_;
};

// A listagem do comando A (uma linha por elemento). number_col = false e' a opcao N.
std::vector<std::string> FormatListing(const AsmProduct &p, bool number_col);
// O bloco de simbolos: kind 'R' = referencia cruzada (alfabetica, com os usos), 'S' = alfabetica simples, 'D' = em ordem de aparicao.
std::vector<std::string> FormatSymbols(const AsmProduct &p, char kind);

} // namespace mamute
