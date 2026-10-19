// fwMSX -- Mamute: o editor de fonte (EDIT) e a montagem (A, MAP) dentro do monitor (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/mamute-spec.md, "Fase 2". As operacoes sobre o fonte estao em mamute_source.cpp; aqui ficam so' os comandos.
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "mamute_monitor.h"
#include "mamute_pdf.h"

namespace mamute {

namespace {

std::string Upper(std::string s) {
    for (char &c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

std::string Trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

std::string Hex(unsigned v, int digits) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%0*X", digits, v);
    return buf;
}

bool ParseDec(const std::string &s, int &out) {
    const std::string t = Trim(s);
    if (t.empty() || t.size() > 5 || t.find_first_not_of("0123456789") != std::string::npos) return false;
    out = std::atoi(t.c_str());
    return out >= 1 && out <= 65535;
}

// "a", "a-b", "a-" (b < 0 = ate' o fim). false se nao e' um intervalo de linhas.
bool ParseRange(const std::string &s, int &a, int &b) {
    const std::string t = Trim(s);
    const size_t dash = t.find('-');
    if (dash == std::string::npos) {
        if (!ParseDec(t, a)) return false;
        b = a;
        return true;
    }
    if (!ParseDec(t.substr(0, dash), a)) return false;
    const std::string rest = Trim(t.substr(dash + 1));
    if (rest.empty()) {
        b = -1;
        return true;
    }
    return ParseDec(rest, b) && b >= a;
}

// Os textos entre apostrofos de CHANGE/SEARCH: "'abc'" ou "'abc'" , "'def'". Devolve quantos achou (0, 1 ou 2).
int QuotedStrings(const std::string &s, std::string &first, std::string &second) {
    int n = 0;
    size_t pos = 0;
    while (n < 2) {
        const size_t a = s.find('\'', pos);
        if (a == std::string::npos) break;
        const size_t b = s.find('\'', a + 1);
        if (b == std::string::npos) break;
        (n == 0 ? first : second) = s.substr(a + 1, b - a - 1);
        ++n;
        pos = b + 1;
    }
    return n;
}

std::string JoinLines(const std::vector<std::string> &v) {
    std::string out;
    for (const std::string &l : v) out += (out.empty() ? "" : "\n") + l;
    return out;
}

const char *const kEditHelp =
    "EDITOR DE FONTE Z80 (linhas numeradas em DECIMAL; numeros no texto em HEXA: 0C100H, 1010B, 10D)\n"
    "  NN [Label:] instrucao operandos [;comentario]   grava a linha NN (so' o numero apaga a linha)\n"
    "  LIST [a[-[b]]] | NEW | DELETE a[-[b]] | RENUM [novo[,antigo[,passo]]]\n"
    "  CHANGE 'a'[,'b']          troca em todas as linhas (sem 'b', apaga)\n"
    "  SEARCH|FIND 'x' | x       procura (com aspas: maiusculas contam); LSEARCH grava as achadas em mamute.srch\n"
    "  SAVE arq | LOAD arq | MERGE arq     arquivo .mza (ASCII)\n"
    "  A [opcoes][/desloc][>nome]   monta (veja HELP A);  MAP   inicio e fim do ultimo codigo montado\n"
    "  QUIT   volta ao monitor (o programa fica);  BA   sai do monitor";

const char *const kAsmHelp =
    "A [opcoes][/desloc][>nome]   monta o fonte (PASSO-1, PASSO-2 e a listagem); opcoes coladas, em qualquer ordem:\n"
    "  O  grava o codigo-objeto na memoria (no endereco do ORG, so' onde for RAM)\n"
    "  N  listagem sem a coluna do numero da linha\n"
    "  P  tambem grava a listagem em NOME.lst (texto)\n"
    "  I  grava o codigo-objeto em NOME.bin no formato BSAVE do MSX (FE inicio fim exec)\n"
    "  R  anexa a referencia cruzada dos simbolos;  S  lista simples (alfabetica);  D  lista em ordem de aparicao\n"
    "  H  grava as listas S e/ou D em NOME.sym\n"
    "  T  grava o codigo-objeto na FITA (arquivo binario): >nome.cas|.tsx|.wav cria essa fita; sem isso acrescenta na fita inserida (TAPE)\n"
    "  /desloc  soma desloc (hexa) a TODO ORG;  >nome  nome-base de P, I e H (padrao: mamute)\n"
    "  Exemplo: A OPR/8000>jogo";

} // namespace

std::string Monitor::ExecuteEdit(const std::string &line_in) {
    const std::string line = Trim(line_in);
    if (line.empty()) return "";

    // NN texto: grava (ou substitui) a linha; so' o numero apaga
    if (std::isdigit(static_cast<unsigned char>(line[0]))) {
        size_t j = 0;
        while (j < line.size() && std::isdigit(static_cast<unsigned char>(line[j]))) ++j;
        int n = 0;
        if (!ParseDec(line.substr(0, j), n)) return "O NUMERO DA LINHA VAI DE 1 A 65535";
        src_.Set(n, line.substr(j));
        last_ok_ = false;
        return "";
    }

    size_t sp = 0;
    while (sp < line.size() && !std::isspace(static_cast<unsigned char>(line[sp])) && line[sp] != '/' && line[sp] != '>') ++sp; // A/10 e A>nome
    const std::string cmd = Upper(line.substr(0, sp));
    const std::string rest = Trim(line.substr(sp));

    if (cmd == "HELP" || cmd == "?") return Upper(rest) == "A" ? kAsmHelp : kEditHelp;
    if (cmd == "LIST") {
        int a = 1, b = -1;
        if (!rest.empty() && !ParseRange(rest, a, b)) return "SINTAXE: LIST [a[-[b]]]";
        const std::string t = src_.List(a, b);
        return t.empty() ? "(NENHUMA LINHA)" : t;
    }
    if (cmd == "NEW") {
        src_.Clear();
        last_ok_ = false;
        return "PROGRAMA APAGADO";
    }
    if (cmd == "DELETE") {
        int a = 0, b = 0;
        if (rest.empty() || !ParseRange(rest, a, b)) return "SINTAXE: DELETE a[-[b]]";
        const int n = src_.Delete(a, b);
        last_ok_ = false;
        return "APAGADAS " + std::to_string(n) + " LINHAS";
    }
    if (cmd == "RENUM") {
        const std::vector<std::string> f = [&] {
            std::vector<std::string> v;
            size_t pos = 0;
            if (rest.empty()) return v;
            while (true) {
                const size_t c = rest.find(',', pos);
                v.push_back(Trim(rest.substr(pos, c == std::string::npos ? std::string::npos : c - pos)));
                if (c == std::string::npos) break;
                pos = c + 1;
            }
            return v;
        }();
        int novo = 10, antigo = 1, inc = 10;
        if (!f.empty() && !f[0].empty() && !ParseDec(f[0], novo)) return "SINTAXE: RENUM [novo[,antigo[,passo]]]";
        if (f.size() > 1 && !f[1].empty() && !ParseDec(f[1], antigo)) return "SINTAXE: RENUM [novo[,antigo[,passo]]]";
        if (f.size() > 2 && !f[2].empty() && !ParseDec(f[2], inc)) return "SINTAXE: RENUM [novo[,antigo[,passo]]]";
        std::string err;
        if (!src_.Renum(novo, antigo, inc, err)) return err;
        last_ok_ = false;
        return "RENUMERADO";
    }
    if (cmd == "CHANGE") {
        std::string a, b;
        const int n = QuotedStrings(rest, a, b);
        if (n == 0 || a.empty()) return "SINTAXE: CHANGE 'texto'[,'novo']";
        const int count = src_.Change(a, n == 2 ? b : "");
        last_ok_ = false;
        return "TROCADAS " + std::to_string(count) + " OCORRENCIAS";
    }
    if (cmd == "SEARCH" || cmd == "FIND" || cmd == "LSEARCH") {
        std::string a, b;
        const bool quoted = QuotedStrings(rest, a, b) >= 1;
        const std::string needle = quoted ? a : rest;
        if (needle.empty()) return "SINTAXE: " + cmd + " 'texto'  (maiusculas contam)  ou  " + cmd + " texto";
        const std::vector<int> found = src_.Search(needle, quoted);
        if (found.empty()) return "NAO ENCONTRADO";
        std::string out;
        for (const int n : found) out += (out.empty() ? "" : "\n") + src_.List(n, n);
        if (cmd != "LSEARCH") return out;
        std::ofstream f("mamute.srch", std::ios::trunc);
        if (!f) return "NAO FOI POSSIVEL GRAVAR mamute.srch";
        f << out << "\n";
        return "GRAVADO mamute.srch - " + std::to_string(found.size()) + " LINHAS";
    }
    if (cmd == "SAVE" || cmd == "LOAD" || cmd == "MERGE") {
        if (rest.empty()) return "SINTAXE: " + cmd + " <arquivo>";
        std::string err;
        if (cmd == "SAVE") {
            if (!src_.SaveFile(rest, err)) return err;
            return "SALVO " + rest + " - " + std::to_string(src_.size()) + " LINHAS";
        }
        if (!src_.LoadFile(rest, cmd == "MERGE", err)) return err;
        last_ok_ = false;
        return std::string(cmd == "LOAD" ? "CARREGADO " : "FUNDIDO ") + rest + " - " + std::to_string(src_.size()) + " LINHAS NO PROGRAMA";
    }
    if (cmd == "A") return CmdAssemble(rest);
    if (cmd == "MAP") return CmdMap();
    if (cmd == "QUIT") return ""; // fora do editor nao ha' o que sair
    return "COMANDO DESCONHECIDO NO EDITOR: " + cmd + " (HELP lista)";
}

std::string Monitor::CmdMap() {
    if (!last_ok_ || !last_.has_code) return "NADA MONTADO AINDA (use A)";
    return "INICIO " + Hex(static_cast<unsigned>(last_.start) & 0xFFFF, 4) + " FIM " + Hex(static_cast<unsigned>(last_.end) & 0xFFFF, 4) + " (" +
           std::to_string(last_.end - last_.start + 1) + " BYTES)";
}

std::string Monitor::CmdAssemble(const std::string &args_in) {
    // "opcoes[/desloc][>nome]"
    std::string args = Trim(args_in);
    std::string base = "mamute";
    std::string tape_file; // >nome.cas / .tsx / .wav: com a opcao T, cria essa fita em vez de acrescentar na inserida
    bool text_out = false; // >nome.txt ou >nome.lst: P e H gravam texto; senao PDF, como o Mamute original
    const size_t gt = args.find('>');
    if (gt != std::string::npos) {
        base = Trim(args.substr(gt + 1));
        args = Trim(args.substr(0, gt));
        const size_t dot = base.rfind('.');
        if (dot != std::string::npos) {
            std::string ext = base.substr(dot);
            for (char &c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            text_out = ext == ".txt" || ext == ".lst";
            if (ext == ".cas" || ext == ".tsx" || ext == ".tzx" || ext == ".wav") tape_file = base; // o nome inteiro, com a extensao
            base = base.substr(0, dot);
        }
        if (base.empty()) return "SINTAXE: A [opcoes][/desloc][>nome]";
    }
    int offset = 0;
    const size_t slash = args.find('/');
    if (slash != std::string::npos) {
        const std::string hex = Trim(args.substr(slash + 1));
        if (hex.empty() || hex.size() > 4 || hex.find_first_not_of("0123456789ABCDEFabcdef") != std::string::npos) return "O DESLOCAMENTO E' HEXA DE 0000 A FFFF";
        offset = static_cast<int>(std::strtol(hex.c_str(), nullptr, 16));
        args = Trim(args.substr(0, slash));
    }
    const std::string opt = Upper(args);
    if (opt.find_first_not_of("ONPIRSDHT") != std::string::npos) return "OPCAO DESCONHECIDA EM '" + opt + "' (O N P I R S D H T) - HELP A";
    auto has = [&](char c) { return opt.find(c) != std::string::npos; };
    if (has('H') && !has('S') && !has('D')) return "A OPCAO H EXIGE S OU D";
    if (src_.empty()) return "NENHUM PROGRAMA (use EDIT e digite linhas numeradas)";

    std::string out = "PASSO-1\nPASSO-2";
    const AsmProduct p = src_.Assemble(offset);
    if (!p.ok) {
        last_ok_ = false;
        out += "\nERRO" + (p.error_line ? " NA LINHA " + std::to_string(p.error_line) : std::string()) + ": " + p.error;
        if (p.error_line) out += "\n> " + src_.List(p.error_line, p.error_line);
        return out;
    }
    last_ = p;
    last_ok_ = true;

    std::vector<std::string> listing = FormatListing(p, !has('N'));
    std::vector<std::string> extra;
    if (has('R')) {
        const std::vector<std::string> r = FormatSymbols(p, 'R');
        extra.insert(extra.end(), r.begin(), r.end());
    }
    std::vector<std::string> labels; // o que o H manda para o arquivo
    if (has('S')) {
        const std::vector<std::string> s = FormatSymbols(p, 'S');
        extra.insert(extra.end(), s.begin(), s.end());
        labels.insert(labels.end(), s.begin(), s.end());
    }
    if (has('D')) {
        if (!labels.empty()) labels.push_back("");
        const std::vector<std::string> d = FormatSymbols(p, 'D');
        extra.insert(extra.end(), d.begin(), d.end());
        labels.insert(labels.end(), d.begin(), d.end());
    }
    if (!listing.empty()) out += "\n" + JoinLines(listing);
    if (!extra.empty()) out += "\n\n" + JoinLines(extra);
    for (const std::string &w : p.warnings) out += "\nAVISO " + w;

    if (has('P')) {
        std::vector<std::string> all = listing;
        if (!extra.empty()) {
            all.push_back("");
            all.insert(all.end(), extra.begin(), extra.end());
        }
        const std::string file = base + (text_out ? ".lst" : ".pdf");
        if (text_out) {
            std::ofstream f(file, std::ios::trunc);
            if (!f) return out + "\nNAO FOI POSSIVEL GRAVAR " + file;
            f << JoinLines(all) << "\n";
        } else {
            std::string err;
            if (!WritePdf(file, "MAMUTE - LISTAGEM", all, err)) return out + "\n" + err;
        }
        out += "\nGRAVADO " + file + (text_out ? "" : " (PDF)");
    }
    if (has('H')) {
        const std::string file = text_out ? base + ".sym" : base + "_labels.pdf";
        if (text_out) {
            std::ofstream f(file, std::ios::trunc);
            if (!f) return out + "\nNAO FOI POSSIVEL GRAVAR " + file;
            f << JoinLines(labels) << "\n";
        } else {
            std::string err;
            if (!WritePdf(file, "MAMUTE - SIMBOLOS", labels, err)) return out + "\n" + err;
        }
        out += "\nGRAVADO " + file + (text_out ? "" : " (PDF)");
    }
    if (has('I')) {
        if (!p.has_code) {
            out += "\nI: NAO HA' CODIGO-OBJETO PARA GRAVAR";
        } else {
            std::vector<uint8_t> bin = {0xFE};
            for (const unsigned v : {static_cast<unsigned>(p.start), static_cast<unsigned>(p.end), static_cast<unsigned>(p.exec)}) {
                bin.push_back(static_cast<uint8_t>(v & 0xFF));
                bin.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
            }
            for (int a = p.start; a <= p.end; ++a) {
                const auto it = p.image.find(a);
                bin.push_back(it == p.image.end() ? 0 : it->second);
            }
            std::ofstream f(base + ".bin", std::ios::binary | std::ios::trunc);
            if (!f) return out + "\nNAO FOI POSSIVEL GRAVAR " + base + ".bin";
            f.write(reinterpret_cast<const char *>(bin.data()), static_cast<std::streamsize>(bin.size()));
            out += "\nGRAVADO " + base + ".bin (BSAVE " + Hex(static_cast<unsigned>(p.start), 4) + "-" + Hex(static_cast<unsigned>(p.end), 4) + ")";
        }
    }
    if (has('T')) out += "\n" + StoreOnTape(base, tape_file, p);
    if (has('O')) {
        unsigned ok = 0;
        for (const auto &kv : p.image)
            if (Mem().Write(static_cast<uint16_t>(kv.first), kv.second)) ++ok;
        out += "\nGRAVADOS " + std::to_string(ok) + " BYTES NA MEMORIA (" + Upper(Mem().Name()) + ") EM " + Hex(static_cast<unsigned>(p.start), 4) + "-" +
               Hex(static_cast<unsigned>(p.end), 4);
        if (ok < p.image.size()) out += " - " + std::to_string(p.image.size() - ok) + " IGNORADOS (NAO E' RAM)";
    }
    out += p.has_code ? "\nMONTADO: " + Hex(static_cast<unsigned>(p.start) & 0xFFFF, 4) + "-" + Hex(static_cast<unsigned>(p.end) & 0xFFFF, 4) + " (" +
                            std::to_string(p.end - p.start + 1) + " BYTES)"
                      : "\nMONTADO: SEM CODIGO-OBJETO";
    return out;
}

} // namespace mamute
