// fwMSX -- Mamute: o programa-fonte Z80 do EDIT e a montagem (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "mamute_source.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "../../asm/cpp/assembler.h"

namespace mamute {

namespace {

std::string Upper(std::string s) {
    for (char &c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string Trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

bool IsIdStart(char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_' || c == '@' || c == '.'; }
bool IsIdChar(char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '@' || c == '.' || c == '$'; }

// O apostrofo abre texto/caractere quando NAO vem colado numa letra ou digito (AF' e' o registrador alternado; DEFM 'TEXTO' tem espaco antes).
bool QuoteOpens(const std::string &s, size_t i) { return i == 0 || !(std::isalnum(static_cast<unsigned char>(s[i - 1])) || s[i - 1] == '_'); }

// Separa o comentario (o primeiro ';' fora de aspas).
void SplitComment(const std::string &text, std::string &code, std::string &comment) {
    char quote = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (quote) {
            if (c == quote) quote = 0;
        } else if ((c == '\'' && QuoteOpens(text, i)) || c == '"') {
            quote = c;
        } else if (c == ';') {
            code = text.substr(0, i);
            comment = text.substr(i);
            return;
        }
    }
    code = text;
    comment.clear();
}

struct Parsed {
    std::string label; // sem os dois pontos
    bool has_label = false;
    std::string op;    // o mnemonico/diretiva, como digitado
    std::string operands;
};

Parsed ParseLine(const std::string &text) {
    std::string code, comment;
    SplitComment(text, code, comment);
    code = Trim(code);
    Parsed p;
    size_t i = 0;
    if (!code.empty() && IsIdStart(code[0])) {
        size_t j = 1;
        while (j < code.size() && IsIdChar(code[j])) ++j;
        if (j < code.size() && code[j] == ':') {
            p.label = code.substr(0, j);
            p.has_label = true;
            i = j + 1;
        }
    }
    const std::string rest = Trim(code.substr(i));
    size_t k = 0;
    while (k < rest.size() && !std::isspace(static_cast<unsigned char>(rest[k]))) ++k;
    p.op = rest.substr(0, k);
    p.operands = Trim(rest.substr(k));
    return p;
}

// Numero do Mamute: sem sufixo = HEXA; H hexa, B binario, D decimal. false se nao e' um numero valido.
bool ParseNumber(const std::string &tok, long &v) {
    std::string t = Upper(tok);
    auto all = [](const std::string &s, const char *set) {
        return !s.empty() && s.find_first_not_of(set) == std::string::npos;
    };
    std::string body;
    int base = 16;
    if (t.size() > 1 && t.back() == 'H' && all(t.substr(0, t.size() - 1), "0123456789ABCDEF")) {
        body = t.substr(0, t.size() - 1);
    } else if (t.size() > 1 && t.back() == 'B' && all(t.substr(0, t.size() - 1), "01")) {
        body = t.substr(0, t.size() - 1);
        base = 2;
    } else if (t.size() > 1 && t.back() == 'D' && all(t.substr(0, t.size() - 1), "0123456789")) {
        body = t.substr(0, t.size() - 1);
        base = 10;
    } else if (all(t, "0123456789ABCDEF")) {
        body = t;
    } else {
        return false;
    }
    v = std::stol(body, nullptr, base);
    return true;
}

// Traduz os operandos para o asMSX: numeros viram decimal; em DEFB/DEFM os textos entre apostrofos viram aspas duplas.
// alias: rotulos do Mamute que o fwasm nao aceita (PRINT, START, SIZE... sao diretivas dele) e que a traducao renomeia.
using Aliases = std::map<std::string, std::string>;

std::string ConvertOperands(const std::string &operands, bool strings_double, const Aliases &alias) {
    std::string out;
    size_t i = 0;
    while (i < operands.size()) {
        const char c = operands[i];
        if ((c == '\'' && QuoteOpens(operands, i)) || c == '"') {
            const size_t end = operands.find(c, i + 1);
            const size_t stop = end == std::string::npos ? operands.size() : end;
            const std::string body = operands.substr(i + 1, stop - i - 1);
            if (strings_double && c == '\'') out += "\"" + body + "\"";
            else out += std::string(1, c) + body + (end == std::string::npos ? "" : std::string(1, c));
            i = end == std::string::npos ? operands.size() : end + 1;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            size_t j = i;
            while (j < operands.size() && IsIdChar(operands[j])) ++j;
            const std::string tok = operands.substr(i, j - i);
            long v = 0;
            out += ParseNumber(tok, v) ? std::to_string(v) : tok; // numero estranho: o fwasm reclama
            i = j;
        } else if (IsIdStart(c) || c == '$') {
            size_t j = i + 1;
            while (j < operands.size() && IsIdChar(operands[j])) ++j;
            const std::string id = operands.substr(i, j - i);
            const auto a = alias.find(id);
            out += a == alias.end() ? id : a->second;
            i = j;
        } else {
            out += c;
            ++i;
        }
    }
    return out;
}

bool HasWord(const std::string &code, const std::string &name) {
    size_t pos = 0;
    while ((pos = code.find(name, pos)) != std::string::npos) {
        const bool left = pos == 0 || !IsIdChar(code[pos - 1]);
        const size_t e = pos + name.size();
        const bool right = e >= code.size() || !IsIdChar(code[e]);
        if (left && right) return true;
        pos = e;
    }
    return false;
}

std::string Hex(unsigned v, int digits) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%0*X", digits, v);
    return buf;
}

} // namespace

void Source::Set(int number, const std::string &text) {
    const std::string t = Trim(text);
    if (t.empty()) lines_.erase(number);
    else lines_[number] = t;
}

std::string Source::List(int from, int to) const {
    std::string out;
    for (const auto &kv : lines_) {
        if (kv.first < from || (to >= 0 && kv.first > to)) continue;
        char head[16];
        std::snprintf(head, sizeof(head), "%-5d", kv.first);
        out += std::string(head) + kv.second + "\n";
    }
    if (!out.empty()) out.pop_back();
    return out;
}

int Source::Delete(int a, int b) {
    int n = 0;
    for (auto it = lines_.begin(); it != lines_.end();) {
        if (it->first >= a && (b < 0 || it->first <= b)) {
            it = lines_.erase(it);
            ++n;
        } else {
            ++it;
        }
    }
    return n;
}

bool Source::Renum(int new_start, int old_start, int inc, std::string &error) {
    if (new_start < 1 || inc < 1) {
        error = "O NUMERO INICIAL E O PASSO PRECISAM SER MAIORES QUE ZERO";
        return false;
    }
    std::vector<std::string> moved;
    std::map<int, std::string> kept;
    for (const auto &kv : lines_) {
        if (kv.first >= old_start) moved.push_back(kv.second);
        else kept.insert(kv);
    }
    if (!kept.empty() && new_start <= kept.rbegin()->first) {
        error = "A NOVA NUMERACAO SE SOBREPOE AS LINHAS ANTERIORES (ATE " + std::to_string(kept.rbegin()->first) + ")";
        return false;
    }
    if (static_cast<long>(new_start) + static_cast<long>(inc) * static_cast<long>(moved.empty() ? 0 : moved.size() - 1) > 65535) {
        error = "A NUMERACAO PASSA DE 65535";
        return false;
    }
    int n = new_start;
    for (const std::string &t : moved) {
        kept[n] = t;
        n += inc;
    }
    lines_ = kept;
    return true;
}

int Source::Change(const std::string &from, const std::string &to) {
    if (from.empty()) return 0;
    int count = 0;
    for (auto &kv : lines_) {
        std::string &t = kv.second;
        size_t pos = 0;
        while ((pos = t.find(from, pos)) != std::string::npos) {
            t.replace(pos, from.size(), to);
            pos += to.size();
            ++count;
        }
    }
    return count;
}

std::vector<int> Source::Search(const std::string &needle, bool case_sensitive) const {
    std::vector<int> found;
    const std::string n = case_sensitive ? needle : Lower(needle);
    for (const auto &kv : lines_) {
        const std::string t = case_sensitive ? kv.second : Lower(kv.second);
        if (t.find(n) != std::string::npos) found.push_back(kv.first);
    }
    return found;
}

bool Source::SaveFile(const std::string &path, std::string &error) const {
    std::ofstream f(path, std::ios::trunc);
    if (!f) {
        error = "NAO FOI POSSIVEL GRAVAR " + path;
        return false;
    }
    for (const auto &kv : lines_) f << kv.first << " " << kv.second << "\n";
    return true;
}

bool Source::LoadFile(const std::string &path, bool merge, std::string &error) {
    std::ifstream f(path);
    if (!f) {
        error = "NAO FOI POSSIVEL ABRIR " + path;
        return false;
    }
    std::map<int, std::string> loaded;
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t i = 0;
        while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) ++i;
        size_t j = i;
        while (j < line.size() && std::isdigit(static_cast<unsigned char>(line[j]))) ++j;
        if (j == i || j - i > 5) continue; // linha sem numero: ignorada
        const int n = std::atoi(line.substr(i, j - i).c_str());
        if (n < 1 || n > 65535) continue;
        const std::string text = Trim(line.substr(j));
        if (!text.empty()) loaded[n] = text;
    }
    if (!merge) lines_.clear();
    for (const auto &kv : loaded) lines_[kv.first] = kv.second;
    return true;
}

AsmProduct Source::Assemble(int offset) const {
    AsmProduct p;
    struct Meta {
        int number;
        Parsed parsed;
        std::string op;
    };
    std::vector<Meta> metas;
    std::string text;
    Aliases alias;
    for (const auto &kv : lines_) {
        const Parsed ps = ParseLine(kv.second);
        if (ps.has_label && fwasm::IsDirectiveName(ps.label)) alias[ps.label] = ps.label + "_m";
    }
    auto name_of = [&](const std::string &label) {
        const auto a = alias.find(label);
        return a == alias.end() ? label : a->second;
    };
    for (const auto &kv : lines_) {
        Meta m{kv.first, ParseLine(kv.second), ""};
        m.op = Upper(m.parsed.op);
        const Parsed &ps = m.parsed;
        const std::string lab = ps.has_label ? name_of(ps.label) + ":" : "";
        std::string line;
        if (m.op.empty()) {
            line = lab;
        } else if (m.op == "ORG") {
            const std::string e = ConvertOperands(ps.operands, false, alias);
            // em sintaxe Zilog os parenteses seriam um acesso a memoria: soma direto no numero (ou "expr+desloc" sem parenteses)
            std::string value = e;
            if (offset) {
                const bool plain = !e.empty() && e.find_first_not_of("0123456789") == std::string::npos;
                value = plain ? std::to_string(std::stol(e) + offset) : e + "+" + std::to_string(offset);
            }
            line = lab + " org " + value;
        } else if (m.op == "END") {
            line = lab;
        } else if (m.op == "EQU") {
            if (!ps.has_label) {
                p.error = "EQU EXIGE UM ROTULO (Label: EQU valor)";
                p.error_line = kv.first;
                return p;
            }
            line = name_of(ps.label) + " equ " + ConvertOperands(ps.operands, false, alias);
        } else if (m.op == "DEFB" || m.op == "DEFM") {
            line = lab + " db " + ConvertOperands(ps.operands, true, alias);
        } else if (m.op == "DEFW") {
            line = lab + " dw " + ConvertOperands(ps.operands, false, alias);
        } else if (m.op == "DEFS") {
            line = lab + " ds " + ConvertOperands(ps.operands, false, alias);
        } else {
            line = lab + " " + ps.op + " " + ConvertOperands(ps.operands, false, alias);
        }
        text += line + "\n";
        metas.push_back(std::move(m));
    }

    fwasm::AsmOptions opts;
    opts.zilog = true; // o Mamute escreve (HL) e nao [HL]
    opts.silent = true;
    opts.want_listing = true;
    const fwasm::AsmResult r = fwasm::Assemble("mamute.asm", text, opts, nullptr);

    // mensagens do fwasm ("mamute.asm, line N: texto"): N vira o numero da linha do Mamute
    auto mamute_line = [&](int n) { return n >= 1 && n <= static_cast<int>(metas.size()) ? metas[static_cast<size_t>(n - 1)].number : 0; };
    auto split_msg = [&](const std::string &m, int &line_no, std::string &body) {
        const size_t a = m.find(", line ");
        if (a == std::string::npos) return false;
        const size_t b = m.find(": ", a);
        if (b == std::string::npos) return false;
        line_no = mamute_line(std::atoi(m.c_str() + a + 7));
        body = m.substr(b + 2);
        return true;
    };
    for (const std::string &m : r.msgs) {
        int ln = 0;
        std::string body;
        if (!split_msg(m, ln, body)) continue;
        if (body.rfind("Warning: ", 0) == 0) {
            p.warnings.push_back("LINHA " + std::to_string(ln) + ": " + body.substr(9));
        } else if (p.error.empty()) {
            p.error = body;
            p.error_line = ln;
        }
    }
    if (r.rc != 0) {
        if (p.error.empty()) p.error = r.msgs.empty() ? "ERRO DE MONTAGEM" : r.msgs.back();
        return p;
    }

    // simbolos: nome -> valor, lidos do .sym
    std::map<std::string, int32_t> symval;
    const auto sym = r.outs.find("sym");
    if (sym != r.outs.end()) {
        const std::string s(sym->second.begin(), sym->second.end());
        size_t pos = 0;
        while (pos < s.size()) {
            size_t nl = s.find('\n', pos);
            if (nl == std::string::npos) nl = s.size();
            const std::string ln = s.substr(pos, nl - pos);
            pos = nl + 1;
            if (ln.empty() || ln[0] == ';') continue;
            const size_t h = ln.find('h');
            if (h == std::string::npos || h + 2 > ln.size() || ln[h + 1] != ' ') continue;
            symval[Trim(ln.substr(h + 2))] = static_cast<int32_t>(std::stoul(ln.substr(0, h), nullptr, 16));
        }
    }

    bool first_code = true;
    for (size_t i = 0; i < metas.size(); ++i) {
        const Meta &m = metas[i];
        fwasm::ListEntry e;
        for (const fwasm::ListEntry &x : r.listing) {
            if (x.line == static_cast<int>(i) + 1) {
                e = x;
                break;
            }
        }
        ListLine l;
        l.number = m.number;
        l.text = lines_.at(m.number);
        l.shown = !(m.op == "ORG" || m.op == "END");
        l.has_addr = !(m.op.empty() && !m.parsed.has_label);
        l.is_equ = m.op == "EQU";
        l.addr = l.is_equ ? e.value : e.addr;
        l.bytes = e.bytes;
        for (size_t k = 0; k < e.bytes.size(); ++k) p.image[e.addr + static_cast<int>(k)] = e.bytes[k];
        if (!e.bytes.empty()) {
            const int last = e.addr + static_cast<int>(e.bytes.size()) - 1;
            if (first_code) {
                p.start = e.addr;
                p.end = last;
                p.exec = e.addr;
                first_code = false;
            }
            p.start = std::min(p.start, e.addr);
            p.end = std::max(p.end, last);
        }
        if (m.parsed.has_label) {
            SymbolInfo s;
            s.name = m.parsed.label;
            s.is_equ = l.is_equ;
            s.def_line = m.number;
            const auto v = symval.find(name_of(s.name));
            s.value = v != symval.end() ? v->second : l.addr;
            p.symbols.push_back(std::move(s));
        }
        p.lines.push_back(std::move(l));
    }
    p.has_code = !first_code;

    // referencia cruzada: os enderecos das linhas cujos operandos citam o simbolo (fora da propria definicao)
    for (SymbolInfo &s : p.symbols) {
        for (size_t i = 0; i < metas.size(); ++i) {
            const Meta &m = metas[i];
            if (m.number == s.def_line || !p.lines[i].has_addr || m.op.empty()) continue;
            if (HasWord(m.parsed.operands, s.name)) s.uses.push_back(p.lines[i].addr);
        }
    }
    p.ok = true;
    return p;
}

std::vector<std::string> FormatListing(const AsmProduct &p, bool number_col) {
    std::vector<std::string> out;
    for (const ListLine &l : p.lines) {
        if (!l.shown) continue;
        std::string head;
        if (number_col) {
            char b[16];
            std::snprintf(b, sizeof(b), "%5d ", l.number);
            head = b;
        }
        const std::string addr = l.has_addr ? Hex(static_cast<unsigned>(l.addr) & 0xFFFF, 4) : "    ";
        std::string bytes;
        for (size_t k = 0; k < l.bytes.size() && k < 4; ++k) bytes += (k ? " " : "") + Hex(l.bytes[k], 2);
        bytes.resize(11, ' ');
        out.push_back(head + addr + "  " + bytes + "  " + l.text);
        for (size_t base = 4; base < l.bytes.size(); base += 4) { // linhas extras: o que passou de 4 bytes
            std::string more;
            for (size_t k = base; k < l.bytes.size() && k < base + 4; ++k) more += (k > base ? " " : "") + Hex(l.bytes[k], 2);
            out.push_back((number_col ? std::string(6, ' ') : std::string()) + Hex((static_cast<unsigned>(l.addr) + static_cast<unsigned>(base)) & 0xFFFF, 4) + "  " + more);
        }
    }
    return out;
}

std::vector<std::string> FormatSymbols(const AsmProduct &p, char kind) {
    std::vector<SymbolInfo> v = p.symbols;
    if (kind != 'D') {
        std::stable_sort(v.begin(), v.end(), [](const SymbolInfo &a, const SymbolInfo &b) { return Lower(a.name) < Lower(b.name); });
    }
    std::vector<std::string> out;
    out.push_back(kind == 'R' ? "REFERENCIA CRUZADA" : kind == 'S' ? "SIMBOLOS (ORDEM ALFABETICA)" : "SIMBOLOS (ORDEM DE APARICAO)");
    for (const SymbolInfo &s : v) {
        std::string name = s.name;
        name.resize(std::max<size_t>(12, name.size()), ' ');
        const std::string head = name + " " + Hex(static_cast<unsigned>(s.value) & 0xFFFF, 4) + (s.is_equ ? " EQU" : "    ");
        if (kind != 'R' || s.uses.empty()) {
            out.push_back(head);
            continue;
        }
        for (size_t k = 0; k < s.uses.size(); k += 4) {
            std::string row = k == 0 ? head : std::string(head.size(), ' ');
            for (size_t j = k; j < s.uses.size() && j < k + 4; ++j) row += " " + Hex(static_cast<unsigned>(s.uses[j]) & 0xFFFF, 4);
            out.push_back(row);
        }
    }
    return out;
}

} // namespace mamute
