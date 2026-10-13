// fwMSX -- montador (fwasm): tabelas (palavras reservadas, diretivas, BIOS). Ver doc/asm-spec.md.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace fwasm::tables {

struct BiosEntry {
    const char *name;
    int32_t value;
};

// Registradores, condicoes, mnemonicos, funcoes matematicas, `pi`, `random` e os nomes de mapper: nao podem ser
// rotulos nem identificadores (erro de sintaxe).
bool IsSyntaxReserved(const std::string &lower);
// Palavras de diretiva usadas como identificador numa expressao: "reserved word used as identifier".
bool IsDirectiveWord(const std::string &lower);
// `name` (minusculas, sem o ponto) e' uma diretiva? `dotted` = veio com ponto. Algumas diretivas exigem o ponto.
bool IsDirective(const std::string &name, bool dotted);

const std::vector<BiosEntry> &BiosCalls();
const std::vector<BiosEntry> &BiosVars();

} // namespace fwasm::tables
