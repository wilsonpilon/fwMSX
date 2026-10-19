// fwMSX -- simbolos e mapa de linhas do fonte para o depurador (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/debug-spec.md, secao 8.
//
// Duas tabelas pequenas que o montador (fwasm) alimenta e o depurador consulta:
//  - simbolos: nome <-> endereco (rotulos e constantes), para `break meu_loop`, `dis` com rotulos e "parou em loop+2";
//  - linhas: endereco -> arquivo:linha do fonte, para "parou em jogo.asm:42" e para o comando `src`.
// Nao conhece a maquina nem o montador: quem monta entrega os pares.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace dbg {

struct Symbol {
    std::string name;
    uint16_t addr = 0;
    bool is_label = true; // false = constante (EQU): vale para busca por nome, mas nao aparece como "rotulo do endereco"
};

struct SourceLine {
    uint16_t addr = 0;
    std::string file;
    int line = 0;
};

class Symbols {
public:
    void Clear() {
        symbols_.clear();
        lines_.clear();
    }
    void ClearSymbols() { symbols_.clear(); }
    void ClearLines() { lines_.clear(); }
    size_t symbol_count() const { return symbols_.size(); }
    size_t line_count() const { return lines_.size(); }
    const std::vector<Symbol> &symbols() const { return symbols_; }

    void Add(const std::string &name, uint16_t addr, bool is_label = true);
    void AddLine(uint16_t addr, const std::string &file, int line);

    // Por nome: exato primeiro, depois sem diferenciar maiusculas. Constantes tambem valem.
    bool Find(const std::string &name, uint16_t &addr) const;
    // "nome" ou "nome+N" / "nome-N" (N em decimal ou 0x/$hexa): o endereco do rotulo mais o deslocamento. false se o nome nao existe.
    bool Resolve(const std::string &text, uint16_t &addr) const;
    // O rotulo EXATAMENTE neste endereco (o primeiro definido).
    bool LabelAt(uint16_t addr, std::string &name) const;
    // O rotulo mais proximo ABAIXO de `addr` (ou nele), a no maximo `max_distance` bytes: "loop" + 2.
    bool Nearest(uint16_t addr, std::string &name, uint16_t &offset, uint16_t max_distance = 0x100) const;
    // "loop+2", "loop" ou "" (sem rotulo por perto).
    std::string Describe(uint16_t addr, uint16_t max_distance = 0x100) const;

    // A linha do fonte que gerou os bytes em `addr` (a ultima registrada para esse endereco exato).
    bool LineAt(uint16_t addr, std::string &file, int &line) const;
    // "jogo.asm:42" ou "".
    std::string DescribeLine(uint16_t addr) const;
    // Os enderecos que a linha `line` de `file` gerou (o primeiro); false se nenhum. Serve para `break jogo.asm:42`.
    bool AddressOfLine(const std::string &file, int line, uint16_t &addr) const;

    // Le um .sym do asMSX/fwasm ("0C000h nome" por linha; MegaROM: "01h:8000h nome"; ';' = comentario) e ACRESCENTA os simbolos.
    // Devolve quantos entraram.
    size_t ParseSym(const std::string &text);

private:
    std::vector<Symbol> symbols_;
    std::vector<SourceLine> lines_;
};

} // namespace dbg
