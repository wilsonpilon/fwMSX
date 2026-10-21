// fwMSX -- editor de textos: completar e snippets (fase E5). Ver completion.h.
#include "completion.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <unordered_set>

extern "C" {
#include "../c/ed_syntax.h"
}
#include "../asm/ed_hash.h"

namespace ed {

namespace {

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

bool IsBasicId(const std::string &id) {
    return id == "msxbasic" || id == "dignified" || id == "msxbas2rom" || id == "nestorbasic" || id == "kunbasic";
}

struct Builtin {
    const char *lang; // id da linguagem ("basic" = a familia toda do BASIC)
    const char *trigger;
    const char *body;
};

const Builtin kBuiltins[] = {
    // MSX-BASIC e dialetos
    {"basic", "for", "FOR $1=$2 TO $3:$0:NEXT"},
    {"basic", "ifthen", "IF $1 THEN $0 ELSE"},
    {"basic", "scr5", "SCREEN 5:COLOR 15,1,1:CLS$0"},
    {"basic", "scr8", "SCREEN 8:COLOR 255,0,0:CLS$0"},
    {"basic", "sprite", "SCREEN 5,2:SPRITE$($1)=$0"},
    {"basic", "onerr", "ON ERROR GOTO $1:$0"},
    {"basic", "onkey", "KEY ON:ON KEY GOSUB $1:$0"},
    {"basic", "onstrig", "ON STRIG GOSUB $1:STRIG(0) ON:$0"},
    {"basic", "waitkey", "A$=INKEY$:IF A$=\"\" THEN $1$0"},
    {"basic", "vpoke", "VPOKE $1,$2$0"},
    {"kunbasic", "turbo", "_TURBO ON\n$0\n_TURBO OFF"},
    // Basic Dignified: o programa que carrega o NestorBASIC (BLOAD "nbasic.bin",R mexe na pilha: carregador com GOTO, nunca func/ret)
    {"dignified", "nbload",
     "{inicio}\ndefint p\ndim p(15)\ndim f$(1)\n\ngoto {NBasicLoad}\n{VoltaNBasicLoad}\n$0\nend\n\n{NBasicLoad}\n"
     "bload \"nbasic.bin\",r\nif p(0) >= 5 then goto {VoltaNBasicLoad}\nprint \"Error loading NestorBASIC: \";\n"
     "if p(0) = 0 then print \"the computer has no mapped RAM (128K minimum).\"\n"
     "if p(0) = 1 then print \"disk error reading nbasic.bin.\"\n"
     "if p(0) = 2 then print \"no free segments in the primary mapper (DOS 2).\"\n"
     "if p(0) = 3 then print \"NestorBASIC was already installed.\"\n"
     "if p(0) = 4 then print \"undefined error in this version.\"\nend\n\n' the library goes after the END: its functions are GOSUB targets\ninclude \"nbasic.dmx\""},
    // Z80 (asMSX/fwasm)
    {"z80asm", "rom", ".BIOS\n.PAGE 1\n.ROM\n.START $1\n$1:\n  $0"},
    {"z80asm", "proc", "$1:\n  $0\n  ret"},
    {"z80asm", "ldir", "  ld hl,$1\n  ld de,$2\n  ld bc,$3\n  ldir$0"},
    {"z80asm", "djnz", "  ld b,$1\n$2:\n  $0\n  djnz $2"},
    {"z80asm", "chput", "  ld a,$1\n  call CHPUT$0"},
    {"z80asm", "wrtvdp", "  ld b,$1\n  ld c,$2\n  call WRTVDP$0"},
    {"z80asm", "filvrm", "  ld hl,$1\n  ld bc,$2\n  ld a,$3\n  call FILVRM$0"},
    {"z80asm", "ifz", "  or a\n  jr nz,$1\n  $0\n$1:"},
    // Pascal
    {"pascal", "prog", "program $1;\nbegin\n  $0\nend."},
    {"pascal", "for", "for $1 := $2 to $3 do\nbegin\n  $0\nend;"},
    {"pascal", "while", "while $1 do\nbegin\n  $0\nend;"},
    {"pascal", "if", "if $1 then\nbegin\n  $0\nend;"},
    {"pascal", "proc", "procedure $1;\nbegin\n  $0\nend;"},
    {"pascal", "func", "function $1: $2;\nbegin\n  $0\nend;"},
    // Batch
    {"bat", "off", "@echo off\n$0"},
    {"bat", "ifex", "if exist $1 goto $2$0"},
    {"bat", "for", "for %%f in ($1) do $0"},
    // Markdown
    {"markdown", "link", "[$1]($2)$0"},
    {"markdown", "code", "```$1\n$0\n```"},
    {"markdown", "table", "| $1 | $2 |\n|---|---|\n| $0 |  |"},
};

std::map<std::string, std::vector<SnippetDef>> &User() {
    static std::map<std::string, std::vector<SnippetDef>> m;
    return m;
}

std::string Unescape(const std::string &s) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            const char n = s[i + 1];
            if (n == 'n') { out += '\n'; ++i; continue; }
            if (n == 't') { out += '\t'; ++i; continue; }
            if (n == '\\') { out += '\\'; ++i; continue; }
        }
        out += s[i];
    }
    return out;
}

std::string Trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

} // namespace

void CollectBufferWords(const std::vector<std::string> &lines, std::vector<std::string> &out) {
    std::unordered_set<unsigned> seen; // uma colisao de hash so' faz uma palavra rara deixar de ser oferecida: sem problema
    const size_t cap = 20000;          // textos enormes: as primeiras linhas bastam para o completar
    for (size_t l = 0; l < lines.size() && l < cap; ++l) {
        const std::string &s = lines[l];
        const int n = static_cast<int>(s.size());
        int i = 0;
        while (i < n) {
            if (!ed_hl_ident_start(static_cast<unsigned char>(s[static_cast<size_t>(i)]))) {
                ++i;
                continue;
            }
            const int e = ed_hl_ident_end(s.data(), n, i, 0);
            if (e - i >= 3) {
                const unsigned h = ed_hash_ci(reinterpret_cast<const unsigned char *>(s.data()) + i, e - i);
                if (seen.insert(h).second) out.push_back(s.substr(static_cast<size_t>(i), static_cast<size_t>(e - i)));
            }
            i = std::max(e, i + 1);
        }
    }
}

std::vector<SnippetDef> SnippetsFor(const Lang *lang) {
    const std::string id = lang ? lang->Id() : std::string("plain");
    std::vector<SnippetDef> out;
    auto add = [&](const std::string &trig, const std::string &body) {
        for (SnippetDef &d : out) {
            if (Lower(d.trigger) == Lower(trig)) {
                d.body = body; // o do usuario (que vem depois) ganha do de fabrica
                return;
            }
        }
        out.push_back({trig, body});
    };
    for (const Builtin &b : kBuiltins) {
        const std::string l = b.lang;
        if (l == id || (l == "basic" && IsBasicId(id))) add(b.trigger, b.body);
    }
    for (const std::string &key : {std::string("all"), id}) {
        const auto it = User().find(key);
        if (it == User().end()) continue;
        for (const SnippetDef &d : it->second) add(d.trigger, d.body);
    }
    return out;
}

bool FindSnippet(const Lang *lang, const std::string &trigger, SnippetDef &out) {
    const std::string t = Lower(trigger);
    for (const SnippetDef &d : SnippetsFor(lang)) {
        if (Lower(d.trigger) == t) {
            out = d;
            return true;
        }
    }
    return false;
}

std::vector<Candidate> CompletionCandidates(const Lang *lang, const std::string &prefix, const std::vector<std::string> &buffer_words, size_t max) {
    struct Scored {
        Candidate c;
        int score;
    };
    std::vector<Scored> found;
    std::unordered_set<std::string> have; // por texto em minuscula: a palavra da linguagem ganha do mesmo texto vindo do arquivo
    const int np = static_cast<int>(prefix.size());
    auto consider = [&](const std::string &w, bool snippet) {
        if (np == 0) return;
        int sc = ed_match_score(prefix.data(), np, w.data(), static_cast<int>(w.size()));
        // o gatilho inteiro digitado ainda aparece: e' como se descobre que ele existe (Tab expande)
        if (sc <= 0 && snippet && Lower(w) == Lower(prefix)) sc = 3;
        if (sc <= 0) return;
        const std::string key = (snippet ? "s:" : "w:") + Lower(w);
        if (!have.insert(key).second) return;
        found.push_back({{w, snippet}, sc});
    };
    std::vector<std::string> words;
    if (lang) lang->Words(words);
    for (const std::string &w : words) consider(w, false);
    for (const SnippetDef &d : SnippetsFor(lang)) consider(d.trigger, true);
    for (const std::string &w : buffer_words) {
        if (have.count("w:" + Lower(w))) continue; // a palavra que a linguagem ja' conhece nao entra de novo
        consider(w, false);
    }
    std::stable_sort(found.begin(), found.end(), [](const Scored &a, const Scored &b) {
        if (a.score != b.score) return a.score > b.score;
        if (a.c.snippet != b.c.snippet) return a.c.snippet; // snippets primeiro: sao o que o usuario escreveu de proposito
        if (a.c.text.size() != b.c.text.size()) return a.c.text.size() < b.c.text.size();
        return a.c.text < b.c.text;
    });
    std::vector<Candidate> out;
    for (const Scored &s : found) {
        if (out.size() >= max) break;
        out.push_back(s.c);
    }
    return out;
}

void SetUserSnippets(const std::string &text) {
    User().clear();
    std::istringstream in(text);
    std::string line, section = "all";
    while (std::getline(in, line)) {
        const std::string t = Trim(line);
        if (t.empty() || t[0] == '#') continue;
        if (t.front() == '[' && t.back() == ']') {
            section = Lower(Trim(t.substr(1, t.size() - 2)));
            continue;
        }
        const size_t eq = t.find('=');
        if (eq == std::string::npos || eq == 0) continue;
        const std::string trig = Trim(t.substr(0, eq));
        if (trig.empty()) continue;
        User()[section].push_back({trig, Unescape(t.substr(eq + 1))});
    }
}

bool LoadUserSnippets(const std::string &path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    SetUserSnippets(ss.str());
    return true;
}

Expansion ExpandSnippet(const std::string &body, const std::string &indent) {
    Expansion ex;
    int line = 0, col = 0;
    std::pair<int, int> stop[10];
    bool has[10] = {false};
    for (size_t i = 0; i < body.size(); ++i) {
        const char c = body[i];
        if (c == '$' && i + 1 < body.size() && std::isdigit(static_cast<unsigned char>(body[i + 1]))) {
            const int d = body[i + 1] - '0';
            if (!has[d]) { // so' a primeira ocorrencia vale (nao ha' espelhamento)
                has[d] = true;
                stop[d] = {line, col};
            }
            ++i;
            continue;
        }
        ex.text += c;
        if (c == '\n') {
            ex.text += indent;
            ++line;
            col = static_cast<int>(indent.size());
        } else {
            ++col;
        }
    }
    for (int d = 1; d <= 9; ++d) {
        if (has[d]) ex.stops.push_back(stop[d]);
    }
    ex.stops.push_back(has[0] ? stop[0] : std::make_pair(line, col));
    return ex;
}

} // namespace ed
