// fwMSX -- montador (fwasm): tabelas. Ver tables.h.
#include "tables.h"

#include <set>

extern "C" {
#include "../core/z80_enc.h"
}

namespace fwasm::tables {

#include "tables_data.inc"

bool IsSyntaxReserved(const std::string &lower) {
    static const std::set<std::string> words = {
        // registradores e condicoes
        "a", "b", "c", "d", "e", "h", "l", "f", "i", "r", "af", "af'", "bc", "de", "hl", "sp", "ix", "iy", "ixh", "ixl", "iyh",
        "iyl", "nz", "z", "nc", "po", "pe", "p", "m",
        // funcoes e constantes matematicas
        "sin", "cos", "tan", "acos", "asin", "atan", "sqr", "sqrt", "exp", "pow", "log", "ln", "abs", "fix", "fixmul", "fixdiv",
        "int", "pi", "random",
        // tipos de MegaROM
        "konami", "konamiscc", "ascii8", "ascii16",
        // operadores escritos por extenso que o asMSX nao aceita como nome
        "and", "or", "xor"};
    if (words.count(lower)) return true;
    return z80_mnemonic(lower.c_str()) >= 0;
}

bool IsDirectiveWord(const std::string &lower) {
    static const std::set<std::string> words = {
        "org", "page", "zilog", "bios", "biosvars", "rom", "megarom", "basic", "msxdos", "start", "search", "subpage", "select",
        "phase", "dephase", "callbios", "calldos", "size", "incbin", "debug", "break", "breakpoint", "printtext", "print",
        "printhex", "printfix", "cas", "cassette", "wav", "filename", "sinclair", "byte", "word", "skip", "at", "if", "ifdef"};
    return words.count(lower) != 0;
}

bool IsDirective(const std::string &name, bool dotted) {
    static const std::set<std::string> any = {
        "org", "page", "db", "defb", "dt", "deft", "dw", "defw", "ds", "defs", "byte", "word", "zilog", "bios", "biosvars",
        "phase", "dephase", "incbin", "print", "printhex", "printfix", "printtext", "filename", "rom", "megarom", "basic",
        "msxdos", "start", "search", "subpage", "select", "callbios", "calldos", "size", "debug", "break", "breakpoint", "cas",
        "cassette", "wav", "sinclair"};
    static const std::set<std::string> needs_dot = {"printdec", "printstring"};
    if (any.count(name)) return true;
    return dotted && needs_dot.count(name) != 0;
}

const std::vector<BiosEntry> &BiosCalls() {
    static const std::vector<BiosEntry> v(std::begin(kBiosCalls), std::end(kBiosCalls));
    return v;
}

const std::vector<BiosEntry> &BiosVars() {
    static const std::vector<BiosEntry> v(std::begin(kBiosVars), std::end(kBiosVars));
    return v;
}

} // namespace fwasm::tables
