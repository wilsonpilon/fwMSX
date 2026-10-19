#include "symbols.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace dbg {

namespace {

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// Ultimo componente do caminho, para comparar "src/jogo.asm" com "jogo.asm".
std::string BaseName(const std::string &p) {
    const size_t s = p.find_last_of("/\\");
    return s == std::string::npos ? p : p.substr(s + 1);
}

bool ParseNumber(const std::string &t, long &out) {
    if (t.empty()) return false;
    int base = 10;
    std::string s = t;
    if (s[0] == '$') {
        base = 16;
        s.erase(0, 1);
    } else if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        base = 16;
        s.erase(0, 2);
    } else if (s.size() > 1 && (s.back() == 'h' || s.back() == 'H')) {
        base = 16;
        s.pop_back();
    }
    if (s.empty()) return false;
    char *end = nullptr;
    const long v = std::strtol(s.c_str(), &end, base);
    if (*end != '\0') return false;
    out = v;
    return true;
}

} // namespace

void Symbols::Add(const std::string &name, uint16_t addr, bool is_label) {
    if (name.empty()) return;
    symbols_.push_back({name, addr, is_label});
}

void Symbols::AddLine(uint16_t addr, const std::string &file, int line) { lines_.push_back({addr, file, line}); }

bool Symbols::Find(const std::string &name, uint16_t &addr) const {
    for (const Symbol &s : symbols_)
        if (s.name == name) {
            addr = s.addr;
            return true;
        }
    const std::string l = Lower(name);
    for (const Symbol &s : symbols_)
        if (Lower(s.name) == l) {
            addr = s.addr;
            return true;
        }
    return false;
}

bool Symbols::Resolve(const std::string &text, uint16_t &addr) const {
    if (Find(text, addr)) return true;
    // nome+N / nome-N: o ultimo '+' ou '-' separa o deslocamento (um rotulo pode ter '.' e '_' mas nao '+')
    const size_t pos = text.find_last_of("+-");
    if (pos == std::string::npos || pos == 0) return false;
    uint16_t base = 0;
    long off = 0;
    if (!Find(text.substr(0, pos), base) || !ParseNumber(text.substr(pos + 1), off)) return false;
    addr = static_cast<uint16_t>(text[pos] == '+' ? base + off : base - off);
    return true;
}

bool Symbols::LabelAt(uint16_t addr, std::string &name) const {
    for (const Symbol &s : symbols_)
        if (s.is_label && s.addr == addr) {
            name = s.name;
            return true;
        }
    return false;
}

bool Symbols::Nearest(uint16_t addr, std::string &name, uint16_t &offset, uint16_t max_distance) const {
    const Symbol *best = nullptr;
    for (const Symbol &s : symbols_) {
        if (!s.is_label || s.addr > addr) continue;
        if (!best || s.addr > best->addr) best = &s;
    }
    if (!best || addr - best->addr > max_distance) return false;
    name = best->name;
    offset = static_cast<uint16_t>(addr - best->addr);
    return true;
}

std::string Symbols::Describe(uint16_t addr, uint16_t max_distance) const {
    std::string name;
    uint16_t off = 0;
    if (!Nearest(addr, name, off, max_distance)) return "";
    return off ? name + "+" + std::to_string(off) : name;
}

bool Symbols::LineAt(uint16_t addr, std::string &file, int &line) const {
    for (size_t i = lines_.size(); i-- > 0;)
        if (lines_[i].addr == addr) {
            file = lines_[i].file;
            line = lines_[i].line;
            return true;
        }
    return false;
}

std::string Symbols::DescribeLine(uint16_t addr) const {
    std::string file;
    int line = 0;
    if (!LineAt(addr, file, line)) return "";
    return BaseName(file) + ":" + std::to_string(line);
}

bool Symbols::AddressOfLine(const std::string &file, int line, uint16_t &addr) const {
    const std::string want = Lower(BaseName(file));
    for (const SourceLine &l : lines_)
        if (l.line == line && Lower(BaseName(l.file)) == want) {
            addr = l.addr;
            return true;
        }
    return false;
}

size_t Symbols::ParseSym(const std::string &text) {
    size_t added = 0;
    bool labels = true; // o .sym separa "labels" de "other identifiers" (constantes) e "variables" por comentarios
    size_t pos = 0;
    while (pos < text.size()) {
        size_t nl = text.find('\n', pos);
        if (nl == std::string::npos) nl = text.size();
        std::string line = text.substr(pos, nl - pos);
        pos = nl + 1;
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t')) line.pop_back();
        size_t i = 0;
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
        if (i < line.size() && line[i] == ';') {
            if (line.find("other identifiers") != std::string::npos || line.find("variables") != std::string::npos) labels = false;
            else if (line.find("labels") != std::string::npos) labels = true;
            continue;
        }
        if (i >= line.size()) continue;
        // [pp:]aaaah nome   (pp = subpagina da MegaROM, ignorada: o endereco e' o da janela da CPU)
        size_t j = i;
        while (j < line.size() && std::isxdigit(static_cast<unsigned char>(line[j]))) ++j;
        if (j < line.size() && line[j] == 'h' && j + 1 < line.size() && line[j + 1] == ':') { // "01h:8000h"
            i = j + 2;
            j = i;
            while (j < line.size() && std::isxdigit(static_cast<unsigned char>(line[j]))) ++j;
        }
        if (j == i || j >= line.size() || (line[j] != 'h' && line[j] != 'H') || j - i > 4) continue;
        const long value = std::strtol(line.substr(i, j - i).c_str(), nullptr, 16);
        size_t k = j + 1;
        while (k < line.size() && (line[k] == ' ' || line[k] == '\t')) ++k;
        if (k >= line.size()) continue;
        Add(line.substr(k), static_cast<uint16_t>(value & 0xFFFF), labels);
        ++added;
    }
    return added;
}

} // namespace dbg
