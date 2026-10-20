// fwMSX -- editor de textos: COMPLETAR e SNIPPETS (fase E5). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/editor-spec.md, secao 4.
//
// Sem estado e sem tela: o Editor pergunta "quais palavras completam este prefixo?" e "o que e' este gatilho de snippet?". As palavras vem da
// linguagem (as mesmas tabelas do destaque), dos gatilhos de snippet e do proprio texto; a nota de cada candidato vem do Fortran
// (ed_match_score), a medida do prefixo do C (ed_word_start) e a deduplicacao do hash do NASM (ed_hash_ci).
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "syntax.h"

namespace ed {

struct Candidate {
    std::string text;
    bool snippet = false; // true = e' o gatilho de um snippet (aceitar expande o snippet em vez de so' completar a palavra)
};

// Identificadores (3 letras ou mais) do texto, sem repetir (a caixa nao conta). `lines` e' o texto do editor.
void CollectBufferWords(const std::vector<std::string> &lines, std::vector<std::string> &out);

// Candidatos para `prefix`: palavras da linguagem, gatilhos de snippet e `buffer_words`, do melhor para o pior (mesma caixa, depois snippets,
// depois a mais curta, depois ordem alfabetica). Nunca devolve o proprio prefixo. No maximo `max`.
std::vector<Candidate> CompletionCandidates(const Lang *lang, const std::string &prefix, const std::vector<std::string> &buffer_words, size_t max = 60);

struct SnippetDef {
    std::string trigger;
    std::string body; // '\n' = nova linha; $1..$9 = pontos de parada (Tab pula de um para o outro); $0 = onde o cursor termina
};

// Snippets da linguagem: os de fabrica mais os do arquivo do usuario (o do usuario ganha se o gatilho for o mesmo).
std::vector<SnippetDef> SnippetsFor(const Lang *lang);
bool FindSnippet(const Lang *lang, const std::string &trigger, SnippetDef &out);

// Arquivo do usuario (fwedit.snippets): "[idDaLinguagem]" abre uma secao e "gatilho=texto" define um snippet; "\n" e "\t" no texto; '#' comenta.
// O id "all" vale para todas as linguagens. Substitui os snippets do usuario ja' carregados.
void SetUserSnippets(const std::string &text);
bool LoadUserSnippets(const std::string &path);

struct Expansion {
    std::string text;                        // o texto a inserir (sem os $N), com a identacao em todas as linhas depois da primeira
    std::vector<std::pair<int, int>> stops;  // pontos de parada na ordem de pulo: (linha, coluna) dentro de `text`; a ultima e' o fim ($0)
};
Expansion ExpandSnippet(const std::string &body, const std::string &indent);

} // namespace ed
