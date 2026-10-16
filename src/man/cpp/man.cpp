// fwMSX -- HELP <comando> e MAN <topico>. Ver man.h e doc/man-spec.md.
#include "man.h"

#include <algorithm>

#include "../c/man_wrap.h"
#include "man_internal.h"

extern "C" {
int man_fold(int c);                                                  // man_fold.asm
int man_match_score(const char *a, int la, const char *b, int lb);    // man_match.f90
}

namespace man {

namespace {

std::string Lower(const std::string &s) {
    std::string out = s;
    for (char &c : out) c = static_cast<char>(man_fold(static_cast<unsigned char>(c)));
    return out;
}

// Os nomes (nome + apelidos) de um topico.
std::vector<std::string> Names(const Topic &t) {
    std::vector<std::string> names{t.name};
    size_t start = 0;
    while (start < t.aliases.size()) {
        const size_t comma = t.aliases.find(',', start);
        const size_t end = comma == std::string::npos ? t.aliases.size() : comma;
        if (end > start) names.push_back(t.aliases.substr(start, end - start));
        start = end + 1;
    }
    return names;
}

// Categorias dos comandos, na ordem em que o help as mostra.
const char *const kCommandCategories[] = {"Console", "Emulador", "Midia e estado", "Depurador", "Ferramentas"};

std::string Repeat(char c, int n) { return std::string(static_cast<size_t>(std::max(0, n)), c); }

// "  uso<pad>resumo", com o resumo quebrado e alinhado na coluna do resumo.
void AppendRow(std::string &out, const std::string &left, const std::string &right, int col, int width) {
    std::string head = "  " + left;
    if (static_cast<int>(head.size()) + 2 > col) { // o uso estoura a coluna: o resumo vai para a linha de baixo
        out += head + "\n";
        head.clear();
    }
    const std::vector<std::string> lines = Wrap(right, std::max(10, width - col));
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i == 0 && !head.empty()) out += head + Repeat(' ', col - static_cast<int>(head.size())) + lines[i] + "\n";
        else out += Repeat(' ', col) + lines[i] + "\n";
    }
}

} // namespace

const std::vector<Topic> &Topics() {
    static const std::vector<Topic> all = [] {
        std::vector<Topic> v = CommandTopics();
        for (Topic &t : ToolTopics()) v.push_back(std::move(t));
        for (Topic &t : ReferenceTopics()) v.push_back(std::move(t));
        // Palavras do MSX-BASIC: quando o nome e' o de um comando do fwMSX (stop, wait), a pagina do BASIC vira uma secao a mais da pagina
        // do comando em vez de um segundo topico com o mesmo nome.
        for (Topic &b : BasicTopics()) {
            Topic *same = nullptr;
            for (Topic &t : v)
                if (t.command && Lower(t.name) == Lower(b.name)) same = &t;
            if (same) same->page += "\n\nNO MSX-BASIC (palavra reservada de mesmo nome)\n" + b.page;
            else v.push_back(std::move(b));
        }
        return v;
    }();
    return all;
}

namespace {

// minusculas, sem espacos nas pontas e com os espacos repetidos reduzidos a um
std::string Normalize(const std::string &s) {
    std::string out;
    bool space = false;
    for (const char raw : s) {
        const char c = static_cast<char>(man_fold(static_cast<unsigned char>(raw)));
        if (c == ' ' || c == '\t') {
            space = !out.empty();
            continue;
        }
        if (space) out += ' ';
        space = false;
        out += c;
    }
    return out;
}

} // namespace

const Topic *Find(const std::string &name) {
    const std::string want = Normalize(name);
    if (want.empty()) return nullptr;
    for (const Topic &t : Topics()) // 1a passada: o nome do topico
        if (Normalize(t.name) == want) return &t;
    for (const Topic &t : Topics()) // 2a: os apelidos
        for (const std::string &n : Names(t))
            if (Normalize(n) == want) return &t;
    return nullptr;
}

const Topic *Resolve(const std::string &name) {
    if (const Topic *t = Find(name)) return t;
    const std::string want = Normalize(name);
    if (want.empty()) return nullptr;
    const Topic *hit = nullptr;
    int count = 0;
    for (const Topic &t : Topics())
        for (const std::string &n : Names(t))
            if (man_match_score(n.data(), static_cast<int>(n.size()), want.data(), static_cast<int>(want.size())) == 2) {
                if (hit != &t) ++count;
                hit = &t;
                break;
            }
    return count == 1 ? hit : nullptr;
}

std::vector<const Topic *> Search(const std::string &query) {
    std::vector<std::pair<int, const Topic *>> scored;
    for (const Topic &t : Topics()) {
        int best = 0;
        for (const std::string &n : Names(t))
            best = std::max(best, man_match_score(n.data(), static_cast<int>(n.size()), query.data(), static_cast<int>(query.size())));
        if (best > 0) scored.emplace_back(best, &t);
    }
    std::stable_sort(scored.begin(), scored.end(), [](const auto &a, const auto &b) { return a.first > b.first; });
    std::vector<const Topic *> out;
    for (const auto &p : scored) out.push_back(p.second);
    return out;
}

std::vector<std::string> Wrap(const std::string &text, int width) {
    std::vector<std::string> out;
    width = std::max(8, width);
    size_t pos = 0;
    while (pos <= text.size()) {
        size_t nl = text.find('\n', pos);
        const size_t end = nl == std::string::npos ? text.size() : nl;
        std::string line = text.substr(pos, end - pos);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) {
            out.push_back(std::string());
        } else {
            size_t indent = 0;
            while (indent < line.size() && line[indent] == ' ') ++indent;
            // Linha de tabela ("99h     VDP comando     escrita...") que precisa quebrar: as continuacoes ficam alinhadas na ultima
            // coluna (depois de uma folga de 3+ espacos que termina antes da coluna 40); texto corrido usa so' a indentacao.
            size_t hang = indent;
            if (static_cast<int>(line.size()) > width) {
                for (size_t j = indent; j + 3 < line.size() && j < 40;) {
                    if (line[j] == ' ' && line[j + 1] == ' ' && line[j + 2] == ' ') {
                        size_t k = j;
                        while (k < line.size() && line[k] == ' ') ++k;
                        if (k < line.size() && k < 40) hang = k;
                        j = k;
                    } else {
                        ++j;
                    }
                }
            }
            const std::string pad(hang, ' ');
            std::string rest = line;
            bool first = true;
            while (!rest.empty()) {
                const int room = static_cast<int>(first ? width : width - hang);
                int n = man_wrap_break(rest.data(), static_cast<int>(rest.size()), room);
                if (n <= 0) n = static_cast<int>(rest.size());
                std::string piece = rest.substr(0, static_cast<size_t>(n));
                while (!piece.empty() && piece.back() == ' ') piece.pop_back();
                out.push_back(first ? piece : pad + piece);
                size_t skip = static_cast<size_t>(n);
                while (skip < rest.size() && rest[skip] == ' ') ++skip;
                rest.erase(0, skip);
                first = false;
            }
        }
        if (nl == std::string::npos) break;
        pos = nl + 1;
    }
    return out;
}

std::string HelpTable(int width) {
    std::string out;
    const std::string rule = Repeat('=', width);
    out += "fwMSX - comandos     (help <comando> = resumo   man <comando> = manual   man = indice)\n";
    out += rule + "\n";
    // coluna do resumo: a maior linha de uso que cabe em ~38 colunas
    int col = 20;
    for (const Topic &t : Topics())
        if (t.command && t.usage.size() + 4 <= 32) col = std::max(col, static_cast<int>(t.usage.size()) + 4);
    for (const char *cat : kCommandCategories) {
        std::string block;
        for (const Topic &t : Topics())
            if (t.command && t.category == cat) AppendRow(block, t.usage, t.summary, col, width);
        if (block.empty()) continue;
        out += std::string(" ") + cat + "\n";
        out += " " + Repeat('-', width - 1) + "\n";
        out += block;
        out += "\n";
    }
    out += "Qualquer comando que o console nao conheca vai para o emulador conectado (emu start).\n";
    out += "MSX-BASIC (print, screen, for...) e referencia do MSX (bios, sysvars, hooks, cores...): man <topico>; man lista todos.\n";
    return out;
}

std::string HelpFor(const Topic &t) {
    std::string out;
    out += t.usage + "\n";
    for (const std::string &l : Wrap(t.summary, 76)) out += "  " + l + "\n";
    if (!t.aliases.empty()) out += "  (tambem: " + t.aliases + ")\n";
    out += "  man " + t.name + " = manual completo\n";
    return out;
}

std::string Index(int width) {
    std::string out;
    out += "fwMSX - manual (man <topico>)\n" + Repeat('=', width) + "\n";
    std::vector<std::string> cats(std::begin(kCommandCategories), std::end(kCommandCategories));
    cats.push_back("Referencia MSX");
    cats.push_back("Manual MSX-BASIC");
    cats.push_back("MSX-BASIC");
    for (const std::string &cat : cats) {
        std::string block;
        if (cat == "MSX-BASIC") { // ~170 palavras: so' os nomes, em linhas corridas
            std::string names;
            for (const Topic &t : Topics())
                if (t.category == cat) names += (names.empty() ? "" : "  ") + t.name;
            for (const std::string &l : Wrap(names, width - 2)) block += "  " + l + "\n";
        }
        for (const Topic &t : Topics())
            if (t.category == cat && cat != "MSX-BASIC") AppendRow(block, t.name, t.summary, 16, width);
        if (block.empty()) continue;
        out += " " + cat + "\n " + Repeat('-', width - 1) + "\n" + block + "\n";
    }
    return out;
}

std::string Render(const Topic &t, int width) {
    std::string out;
    const std::string title = Lower(t.name) + " -- " + t.summary;
    for (const std::string &l : Wrap(title, width)) out += l + "\n";
    out += Repeat('=', std::min<int>(width, static_cast<int>(std::max<size_t>(title.size(), 20)))) + "\n";
    if (!t.usage.empty() && t.command) out += "USO\n  " + t.usage + "\n\n";
    for (const std::string &l : Wrap(t.page, width)) out += l + "\n";
    return out;
}

std::string NotFound(const std::string &name) {
    std::string out = "nao ha' topico '" + name + "'";
    const std::vector<const Topic *> hits = Search(name);
    if (!hits.empty()) {
        out += " -- parecidos:";
        for (size_t i = 0; i < hits.size() && i < 8; ++i) out += " " + hits[i]->name;
    }
    return out + "\n(man lista todos os topicos)\n";
}

} // namespace man
