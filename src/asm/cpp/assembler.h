// fwMSX -- montador (fwasm): interface publica (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause), escrito a partir
// do manual do asMSX 1.2.0 e do comportamento observado do programa (caixa-preta); NAO e' derivado do fonte
// dele (GPLv3). Ver doc/asm-spec.md.
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace fwasm {

struct AsmOptions {
    bool zilog = false;    // -z: sintaxe Zilog sem precisar da diretiva .ZILOG
    bool silent = true;    // -s: sem mensagens de progresso
    bool verbose = false;  // -vv
    std::string out_base;  // -o: nome-base (sem extensao) das saidas; vazio = nome do fonte
    bool want_listing = false; // preenche AsmResult::listing (usado pelo monitor Mamute para montar a listagem)
    bool want_tape = false;    // monta tambem o .cas de um programa .BASIC mesmo sem a diretiva .CAS (o "montar e rodar" da TUI carrega por fita)
};

// Acesso a arquivos de .INCLUDE / .INCBIN (o teste usa um provedor em memoria).
class FileProvider {
public:
    virtual ~FileProvider() = default;
    // `path` ja' relativo ao diretorio do fonte principal. false se nao existe.
    virtual bool Read(const std::string &path, std::vector<uint8_t> &data) = 0;
};

// Uma linha do fonte na passada final: o endereco ($) onde ela comecou e os bytes que gerou (vazio para comentario, rotulo, EQU...).
// is_const: a linha definiu uma constante (EQU) de valor `value`. So' vem com AsmOptions::want_listing.
struct ListEntry {
    std::string file;
    int line = 0;
    int32_t addr = 0;
    std::vector<uint8_t> bytes;
    bool is_const = false;
    int32_t value = 0;
};

// Um simbolo da tabela final: rotulo (is_label) ou constante/EQU. `page` = subpagina da MegaROM (0 nos outros formatos).
struct SymbolRec {
    std::string name;
    int32_t value = 0;
    int page = 0;
    bool is_label = false;
};

struct AsmResult {
    int rc = 0;                       // codigo de saida (0 = ok)
    std::vector<std::string> msgs;    // erros e avisos, formatados ("x.asm, line 3: ...")
    // Arquivos gerados: extensao ("z80", "bin", "rom", "sym", "txt"...) -> conteudo.
    std::map<std::string, std::vector<uint8_t>> outs;
    std::string base_name;            // nome-base das saidas (sem extensao)
    std::vector<ListEntry> listing;   // so' com want_listing; na ordem do fonte (as linhas depois de um erro nao entram)
    std::vector<SymbolRec> symbols;   // a tabela de simbolos (a mesma do .sym), para o depurador
    std::string format;               // "rom", "megarom", "basic", "msxdos", "sinclair" ou "" (so' os bytes)
    std::string mapper;               // MegaROM: "konami", "konamiscc", "ascii8" ou "ascii16"
    bool uses_markers = false;        // o fonte usa .DEBUG/.BREAK (marcadores do depurador do blueMSX no codigo)
};

// true se `name` (sem diferenciar maiusculas) e' o nome de uma diretiva (org, start, print, size...): nao pode ser rotulo. Quem gera fonte
// para o fwasm (o Mamute) renomeia os rotulos que colidem.
bool IsDirectiveName(const std::string &name);

// Monta `text` (conteudo do arquivo `source_name`). `files` pode ser nullptr (sem includes).
AsmResult Assemble(const std::string &source_name, const std::string &text, const AsmOptions &opts, FileProvider *files);

} // namespace fwasm
