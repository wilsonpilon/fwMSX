// fwMSX -- editor de textos: DESTAQUE DE SINTAXE (fase E4). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/editor-spec.md.
//
// Uma linguagem e' um objeto que, dada UMA linha (e o estado que veio da linha anterior: comentario de varias linhas, bloco de codigo),
// devolve trechos [de, ate') com uma classe (palavra-chave, comentario, string, numero...). Cada linguagem tem suas palavras em tabelas de
// DADOS (tabela hash por NASM; varredura de identificador/numero/string em C; classificacao do numero em Fortran). O mesmo resultado serve
// a TUI (cores por celula) e a janela (cores do tema).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ed {

enum SynClass : uint8_t {
    kSynNone = 0,
    kSynKeyword,   // comandos e palavras reservadas (PRINT, begin, ld)
    kSynKeyword2,  // funcoes, tipos, extensoes (CHR$, integer, CALL MUSIC)
    kSynDirective, // diretivas e pre-processador (.org, {$I}, #define, ##BB:)
    kSynComment,
    kSynString,
    kSynNumber,
    kSynLabel,     // rotulos, numeros de linha referenciados, links
    kSynRegister,  // registradores Z80
    kSynOperator,  // operadores escritos por extenso, listas, marcadores
    kSynLineNo,    // numero de linha do BASIC
    kSynVariable,  // variaveis (%VAR%, ~nome)
    kSynBios,      // chamadas e variaveis da BIOS (CHPUT, LINL40)
    kSynHeading,   // titulos (Markdown)
    kSynError,     // comando que o compilador do bloco nao aceita (Kun BASIC)
    kSynClassCount
};

struct Span {
    int from = 0; // indice de byte (inclusive)
    int to = 0;   // indice de byte (exclusive)
    uint8_t cls = kSynNone;
};

// O que atravessa o fim da linha. `mode` e' interpretado por cada linguagem (0 = nada).
struct HlState {
    uint8_t mode = 0;
    bool operator==(const HlState &o) const { return mode == o.mode; }
};

class Lang {
public:
    virtual ~Lang() = default;
    const std::string &Id() const { return id_; }
    const std::string &Name() const { return name_; }
    const std::vector<std::string> &Extensions() const { return exts_; }
    // false = nenhuma construcao atravessa linhas: o estado e' sempre o padrao e o editor nem precisa varrer as linhas de cima.
    virtual bool MultiLine() const { return false; }
    // Destaca `line`; `st` entra com o estado do inicio da linha e sai com o do fim. `out` e' limpo antes. Trechos em ordem, sem sobrepor.
    virtual void Highlight(const std::string &line, HlState &st, std::vector<Span> &out) const = 0;
    // As palavras que o completar oferece (comandos, funcoes, diretivas, BIOS), ja' na caixa em que se costuma escrever. Acrescenta em `out`.
    virtual void Words(std::vector<std::string> &out) const { (void)out; }

protected:
    Lang(std::string id, std::string name, std::vector<std::string> exts) : id_(std::move(id)), name_(std::move(name)), exts_(std::move(exts)) {}

private:
    std::string id_;
    std::string name_;
    std::vector<std::string> exts_;
};

// Todas as linguagens (a primeira e' o texto puro).
const std::vector<const Lang *> &AllLangs();
const Lang *LangById(const std::string &id);
// Linguagem de um arquivo: pela extensao; `.bas` olha o inicio do texto (`sample`) para separar MSX-BASIC, MSXBAS2ROM e NestorBASIC.
// Sem extensao conhecida: texto puro.
const Lang *LangForFile(const std::string &path, const std::string &sample = std::string());

// Nome do estilo do editor para uma classe (kSynKeyword -> o Style correspondente): ver editor.h (Style::kSyn*).
const char *SynClassName(uint8_t cls);

} // namespace ed
