// fwMSX -- HELP <comando> e MAN <topico>: registro dos topicos, tabela do help e formatacao das paginas.
// Ver doc/man-spec.md. C++ (registro e formatacao) + C (quebra de linha) + Fortran (casamento de nomes) +
// Assembly (dobra de maiusculas): as quatro linguagens do projeto.
#pragma once

#include <string>
#include <vector>

namespace man {

struct Topic {
    std::string name;     // "peek", "bios"...
    std::string aliases;  // outros nomes, separados por virgula ("exit" -> "quit")
    std::string category; // "Console", "Emulador", ..., "Referencia MSX"
    std::string usage;    // linha de uso, ex.: "peek <end> [n]"
    std::string summary;  // uma linha
    std::string page;     // o manual completo (texto simples, sem acento)
    bool command;         // true = comando (entra na tabela do help); false = referencia (so' no man)
};

// Todos os topicos, na ordem de exibicao.
const std::vector<Topic> &Topics();

// Procura pelo nome ou apelido, sem diferenciar maiusculas. nullptr se nao existe.
const Topic *Find(const std::string &name);

// Topicos cujo nome (ou apelido) casa com `query` (igual, comeco ou trecho), melhores primeiro.
std::vector<const Topic *> Search(const std::string &query);

// Quebra `text` em linhas de ate `width` colunas, mantendo a indentacao de cada linha.
std::vector<std::string> Wrap(const std::string &text, int width);

// `help` sem argumento: os comandos agrupados por categoria, com separadores.
std::string HelpTable(int width = 78);

// `help <comando>`: uso e resumo (poucas linhas).
std::string HelpFor(const Topic &t);

// `man` sem argumento: o indice de TODOS os topicos (comandos e referencia), por categoria.
std::string Index(int width = 78);

// `man <topico>`: titulo, uso e a pagina inteira, quebrada em `width` colunas.
std::string Render(const Topic &t, int width = 78);

// `help <x>` / `man <x>` quando <x> nao existe: "nao ha' topico 'x'" + sugestoes (Search).
std::string NotFound(const std::string &name);

} // namespace man
