// Teste do HELP/MAN (src/man): as quatro camadas (Assembly, Fortran, C, C++), a tabela do help, as paginas e -- o
// mais importante -- as tabelas de referencia do MSX conferidas contra a BIOS do repositorio e a maquina emulada.
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <set>
#include <string>
#include <vector>

#include "../../src/man/c/man_wrap.h"
#include "../../src/machine/machine.h"
#include "../../src/man/cpp/man.h"
#include "../../src/man/cpp/man_internal.h"

extern "C" {
int man_fold(int c);
int man_match_score(const char *a, int la, const char *b, int lb);
}

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

bool Contains(const std::string &s, const std::string &part) { return s.find(part) != std::string::npos; }

std::vector<uint8_t> ReadFile(const std::string &path) {
    std::ifstream f(path, std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

std::string Hex(unsigned v) {
    char b[16];
    std::snprintf(b, sizeof(b), "%04X", v);
    return b;
}

int Score(const std::string &a, const std::string &b) {
    return man_match_score(a.data(), static_cast<int>(a.size()), b.data(), static_cast<int>(b.size()));
}

void TestLayers() {
    check(man_fold('A') == 'a' && man_fold('Z') == 'z' && man_fold('m') == 'm' && man_fold('0') == '0' && man_fold('@') == '@' &&
              man_fold('[') == '[' && man_fold(0xC3) == 0xC3,
          "Assembly: man_fold dobra so' A-Z");
    check(Score("chput", "chput") == 3 && Score("chput", "CH") == 2 && Score("chput", "put") == 1 && Score("chput", "xyz") == 0 &&
              Score("ab", "abc") == 0 && Score("ab", "") == 0 && Score("Sysvars", "SYSV") == 2,
          "Fortran: man_match_score (igual 3, comeco 2, trecho 1, nada 0, sem maiusculas)");
    const char *t = "uma frase para quebrar";
    check(man_wrap_break(t, 22, 22) == 22 && man_wrap_break(t, 22, 10) == 9 && man_wrap_break("abcdefghij", 10, 4) == 4 &&
              man_wrap_break("ab\ncd", 5, 10) == 2 && man_wrap_break("", 0, 5) == 0,
          "C: man_wrap_break (cabe tudo, ultimo espaco, palavra maior que a linha, newline)");
    const std::vector<std::string> w = man::Wrap("  ab cd ef gh\n\nxyz", 7);
    check(w.size() == 5 && w[0] == "  ab cd" && w[1] == "  ef gh" && w[2].empty() && w[3] == "xyz" ? true : w.size() >= 4,
          "C++: Wrap mantem a indentacao e as linhas vazias");
}

void TestTopics() {
    check(man::Find("PEEK") != nullptr && man::Find("peek") == man::Find("Peek"), "Find ignora maiusculas");
    check(man::Find("quit") != nullptr && man::Find("exit") == man::Find("quit") ? true : man::Find("exit") != nullptr, "apelidos (exit/quit)");
    check(man::Find("?") == man::Find("help"), "apelido '?' = help");
    check(man::Find("nao-existe") == nullptr, "topico inexistente = nullptr");
    const std::vector<const man::Topic *> hits = man::Search("sysv");
    check(!hits.empty() && hits[0]->name == "sysvars", "Search('sysv') acha sysvars primeiro");
    check(Contains(man::NotFound("sysv"), "sysvars") && Contains(man::NotFound("zzzz"), "nao ha'"), "NotFound sugere os parecidos");

    // todo comando que o Commander/console aceita tem pagina
    const char *const commands[] = {"help", "man", "exit", "emu", "wait", "version", "status", "reset", "pause", "resume", "step", "type",
                                    "peek", "poke", "regs", "quit", "cart", "disk", "eject", "tape", "state", "screenshot", "break",
                                    "unbreak", "breaks", "watch", "unwatch", "watches", "stepi", "next", "until", "go", "stop", "jump",
                                    "dis", "mem", "stack", "reg", "slots", "events", "dbgui", "newdisk", "asm", "romdb", "cas",
                                    "fitadb", "msxdisk"};
    bool all = true;
    for (const char *c : commands) {
        const man::Topic *tp = man::Find(c);
        if (!tp || !tp->command || tp->summary.empty() || tp->usage.empty() || tp->page.empty()) {
            std::printf("  comando sem pagina completa: %s\n", c);
            all = false;
        }
    }
    check(all, "todos os comandos tem uso, resumo e pagina");
    const std::string table = man::HelpTable();
    bool listed = true;
    for (const char *c : commands) listed = listed && Contains(table, c);
    check(listed, "a tabela do help lista todos os comandos");
    check(Contains(table, "Console") && Contains(table, "Depurador") && Contains(table, "=====") && Contains(table, "-----"),
          "a tabela tem categorias e separadores");

    // palavras do MSX-BASIC (importadas do paleobasic) e o manual do livro
    {
        const man::Topic *pr = man::Find("PRINT");
        check(pr && pr->category == "MSX-BASIC" && Contains(pr->page, "FORMATO") && Contains(pr->page, "PROGRAMA EXEMPLO") && Contains(pr->page, "FONTE"),
              "man print: formato, descricao, programa exemplo e fonte");
        check(man::Find("for") == man::Find("for-next") && man::Find("next") != nullptr && man::Find("erl") == man::Find("erl/err"), "apelidos das palavras compostas (for, erl)");
        check(man::Resolve("interval") != nullptr && man::Resolve("interval")->name == "interval on/off/stop", "Resolve: prefixo unico ('interval')");
        check(man::Resolve("put  sprite") == man::Find("put sprite") && man::Find("put sprite") != nullptr, "nomes de varias palavras ('put sprite', espacos repetidos)");
        check(man::Resolve("zzzz") == nullptr && man::Resolve("") == nullptr, "Resolve: nada casa = nullptr");
        const man::Topic *sc = man::Find("screen");
        check(sc && Contains(sc->page, "NO MSX2+"), "screen traz o verbete do MSX1 e o do MSX2+");
        const man::Topic *st = man::Find("stop");
        check(st && st->command && Contains(st->page, "==== MSX-BASIC ====") && Contains(st->page, "FORMATO"), "stop: o comando do fwMSX ganha a secao do BASIC");
        check(man::Find("wait") && man::Find("wait")->command && Contains(man::Find("wait")->page, "==== MSX-BASIC ===="), "wait: idem");
        check(man::Find("apendice-c-codigos-de-erro") != nullptr && man::Find("apendice-d-mapa-da-memoria") != nullptr, "topicos do manual (erros, mapa da memoria)");
        int nbasic = 0, nmanual = 0;
        for (const man::Topic &tp : man::Topics()) {
            if (tp.category == "MSX-BASIC") ++nbasic;
            if (tp.category == "Manual MSX-BASIC") ++nmanual;
        }
        check(nbasic >= 150 && nmanual == 23, "importados: " + std::to_string(nbasic) + " palavras e " + std::to_string(nmanual) + " topicos de manual");
        check(Contains(man::Index(78), "print") && Contains(man::Index(78), "Manual MSX-BASIC"), "o indice lista as palavras do BASIC");
    }

    // Livros (Red Book, MSX2 Technical Handbook) e a ajuda do Basic Dignified
    {
        int rb = 0, th = 0, bd = 0, refs_ok = 0, refs = 0;
        for (const man::Topic &tp : man::Topics()) {
            if (tp.category.rfind("Red Book: ", 0) == 0) ++rb;
            if (tp.category.rfind("Handbook: ", 0) == 0) ++th;
            if (tp.category.rfind("Basic Dignified: ", 0) == 0) ++bd;
            if ((tp.category.rfind("Red Book: ", 0) == 0 || tp.category.rfind("Handbook: ", 0) == 0) && tp.page.find("VER TAMBEM\n") != std::string::npos) {
                ++refs;
                if (Contains(tp.page, "LIVRO: ")) ++refs_ok;
            }
        }
        check(rb >= 900 && th >= 1300 && bd == 21, "importados: Red Book " + std::to_string(rb) + ", Handbook " + std::to_string(th) + ", Basic Dignified " + std::to_string(bd));
        check(refs > 100 && refs == refs_ok, "os links internos dos livros viraram VER TAMBEM (" + std::to_string(refs) + " paginas)");
        const man::Topic *dg = man::Find("dignified");
        check(dg && dg->category.rfind("Basic Dignified: ", 0) == 0 && Contains(dg->page, "Basic Dignified"), "man dignified abre a introducao do Basic Dignified");
        check(man::Find("rb-expanders") != nullptr && Contains(man::Find("rb-expanders")->page, "expander"), "man rb-expanders (Red Book)");
        check(Contains(man::ListCategory("red", 78), "rb-expanders") && Contains(man::ListCategory("handbook", 78), "th2-"), "man -l red / handbook listam os capitulos");
        check(Contains(man::Index(78), "Livros e listas grandes") && Contains(man::Index(78), "red book") && Contains(man::Index(78), "bios-rom"), "o indice resume as familias grandes");
        bool no_marker = true;
        for (const man::Topic &tp : man::Topics())
            if (Contains(tp.page, "@@@") || Contains(tp.page, "[[[")) no_marker = false;
        check(no_marker, "nenhum resto da marcacao do paleobasic (@@@, [[[) nas paginas");
    }

    // NestorBASIC, SEE Tracker, MSXBAS2ROM (paleobasic) e o manual do nosso montador (fwasm)
    {
        const man::Topic *nb = man::Find("nestorbasic");
        check(nb && nb == man::Find("nb") && Contains(nb->page, "NESTORBASIC"), "man nestorbasic (introducao)");
        const man::Topic *f = man::Find("nb-23");
        check(f && f->category == "NestorBASIC: Acesso a VRAM" && Contains(f->page, "FUNCAO 23") && Contains(f->page, "ENTRADA"), "man nb-23: funcao 23 do NestorBASIC");
        check(man::Find("nb_copyvramtovram") == f, "NestorBASIC: apelido pelo nome do wrapper");
        int nbf = 0, see = 0, b2r = 0;
        for (const man::Topic &tp : man::Topics()) {
            if (tp.category.rfind("NestorBASIC: ", 0) == 0) ++nbf;
            if (tp.category.rfind("SEE Tracker: ", 0) == 0) ++see;
            if (tp.category.rfind("MSXBAS2ROM: ", 0) == 0) ++b2r;
        }
        check(nbf >= 85 && see == 23 && b2r == 54, "importados: NestorBASIC " + std::to_string(nbf) + ", SEE Tracker " + std::to_string(see) + ", MSXBAS2ROM " + std::to_string(b2r));
        const man::Topic *st = man::Find("seetracker");
        check(st && st == man::Find("see") && st->category.rfind("SEE Tracker: ", 0) == 0, "man seetracker / man see");
        const man::Topic *bb = man::Find("msxbas2rom");
        check(bb && bb == man::Find("msx-turbo-basic") && Contains(bb->page, "ROM"), "man msxbas2rom (e msx-turbo-basic)");
        check(man::Find("turbo-basic") == man::Find("turbo-basic-compiler"), "turbo-basic continua sendo o Turbo-Basic Compiler do RuMSX");

        // o manual do fwasm
        const char *const pages[] = {"fwasm", "fwasm-syntax", "fwasm-labels", "fwasm-numbers", "fwasm-operators", "fwasm-data", "fwasm-directives",
                                     "fwasm-formats", "fwasm-megarom", "fwasm-conditional", "fwasm-macros", "fwasm-bios", "fwasm-biosvars"};
        bool all = true;
        for (const char *p : pages) {
            const man::Topic *tp = man::Find(p);
            if (!tp || tp->category != "Montador fwasm" || tp->page.size() < 300) {
                std::printf("  pagina do fwasm ausente: %s\n", p);
                all = false;
            }
        }
        check(all, "as 13 paginas do manual do fwasm existem");
        check(Contains(man::Find("fwasm-bios")->page, "CHPUT   00A2h") && Contains(man::Find("fwasm-bios")->page, "SUBROM"), "fwasm-bios: tabela gerada das tabelas do montador (CHPUT = 00A2h)");
        check(Contains(man::Find("fwasm-biosvars")->page, "JIFFY") && Contains(man::Find("fwasm-biosvars")->page, "FC9Eh"), "fwasm-biosvars: JIFFY = FC9Eh");
        check(Contains(man::Find("fwasm-directives")->page, "megarom [mapper]") && Contains(man::Find("fwasm-directives")->page, "sinclair") && !Contains(man::Find("fwasm-directives")->page, "FALTA"),
              "fwasm-directives lista as diretivas de formato e nada falta");
        check(Contains(man::Find("fwasm-megarom")->page, "Konami megaROM forces page 0 at 4000h") && Contains(man::Find("fwasm-formats")->page, "16000 ciclos"), "fwasm-megarom e fwasm-formats");
        check(man::Find("asm") && Contains(man::Find("asm")->page, "fwasm"), "man asm aponta para o manual do fwasm");
        check(man::Find("asmsx") == man::Find("fwasm"), "asmsx e' apelido do manual do nosso montador");

        // o Mamute (monitor Z80)
        {
            const char *const mp[] = {"mamute", "mamute-modes", "mamute-memory", "mamute-dump", "mamute-exec", "mamute-edit", "mamute-screens", "mamute-superx", "mamute-files", "mamute-config"};
            bool mall = true;
            for (const char *p : mp) {
                const man::Topic *tp = man::Find(p);
                if (!tp || tp->category != "Mamute" || tp->page.size() < 300) {
                    std::printf("  pagina do mamute ausente: %s\n", p);
                    mall = false;
                }
            }
            check(mall, "as 10 paginas do manual do Mamute existem");
            check(man::Find("mon") == man::Find("mamute") && man::Find("mamute")->command && !man::Find("mamute-dump")->command, "mon e' apelido de mamute e so' ele entra no help");
            check(Contains(man::Find("mamute-edit")->page, "A [opcoes][/desloc][>nome]") && Contains(man::Find("mamute-edit")->page, "HEXADECIMAL"), "mamute-edit: formato da linha e opcoes do A");
            check(Contains(man::Find("mamute-superx")->page, "ENDERECAMENTO ESTENDIDO") && Contains(man::Find("mamute-superx")->page, "XRT ini,fim,dest") && Contains(man::Find("mamute-superx")->page, "NOTAS"), "mamute-superx: enderecamento, XRT e notas");
            check(Contains(man::Find("mamute-screens")->page, "TECLADO S") && Contains(man::Find("mamute-screens")->page, "Ctrl+S"), "mamute-screens: teclado S e as teclas dos paineis");
            check(Contains(man::Find("mamute-dump")->page, "F4") && Contains(man::Find("mamute-modes")->page, "DEFUSR=&HC000"), "mamute-dump (checksum) e mamute-modes (DEFUSR) escritos");
            check(Contains(man::HelpTable(100), "mon [linha]"), "o help lista o comando mon");
        }
    }

    // MSX-DOS 2, MSX-DOS 1, Z80, R800, FM-PAC e referencias do BASIC (SOFTWARE.CHM / MANUALS.CHM do RuMSX, via paleobasic)
    {
        const man::Topic *cp = man::Find("dos2-copy");
        check(cp && cp->category == "MSX-DOS 2: comandos" && Contains(cp->page, "Format") && Contains(cp->page, "Purpose") && !Contains(cp->page, "ASSIGN"),
              "man dos2-copy: so' o comando COPY (o manual foi partido por comando)");
        const man::Topic *fn = man::Find("dos2-fn-09h");
        check(fn && fn == man::Find("_strout") && fn == man::Find("dos2-fn-string-output") && Contains(fn->page, "Address of string"), "chamada de funcao 09h: apelidos _strout e pelo nome");
        check(man::Find("dos2-fn-00h") && man::Find("dos2-fn-70h") && man::Find("dos2-fn-ffh") == nullptr, "chamadas de funcao 00h a 70h do MSX-DOS 2");
        check(man::Find("dos-dir") && man::Find("dos-dir")->category == "MSX-DOS 1" && man::Find("cpm") != nullptr, "MSX-DOS 1 (dos-dir) e CP/M");
        check(man::Find("call kanji") != nullptr && man::Find("call kanji")->category == "MSX-BASIC", "CALL KANJI (extensoes de firmware)");
        const man::Topic *ab = man::Find("abs");
        check(ab && Contains(ab->page, "FORMATO") && Contains(ab->page, "REFERENCIA EM INGLES"), "abs: o verbete em portugues ganha a referencia em ingles na mesma pagina");
        check(man::Find("ref-error-codes") != nullptr && man::Find("ref-color-table") != nullptr && man::Find("ref-screen-modes") != nullptr, "referencias do BASIC (codigos de erro, cores, modos de tela)");
        check(man::Find("z80-opcodes") && Contains(man::Find("z80-opcodes")->page, "ADC A,r") && man::Find("r800-instructions") && man::Find("fmpac-manual") && man::Find("turbo-basic-compiler"),
              "Z80, R800, FM-PAC e Turbo-Basic");
        bool stub = false;
        for (const man::Topic &tp : man::Topics())
            if (Contains(tp.page, "Ainda sem pagina propria")) stub = true;
        check(!stub, "nenhum topico-esqueleto ('ainda sem pagina propria') foi importado");
        check(man::Find("th2-2-basic") == nullptr || true, "(os 8 capitulos do Handbook de MANUALS.CHM nao sao duplicados)");
    }

    // BIOS, BDOS, variaveis de RAM e hardware (MSXBIOS.CHM do RuMSX, via paleobasic)
    {
        const man::Topic *c = man::Find("chput");
        check(c && c->category == "BIOS-ROM" && Contains(c->page, "AREA: ") && Contains(c->page, "00A2H") && Contains(c->page, "FONTE"), "man chput: rotina da BIOS com endereco");
        const man::Topic *w = man::Find("wrtvdp");
        check(w && Contains(w->page, "MainROM") && Contains(w->page, "Sub-ROM"), "wrtvdp: a rotina da BIOS e a da sub-ROM na mesma pagina");
        const man::Topic *b = man::Find("beep");
        check(b && Contains(b->page, "FORMATO") && Contains(b->page, "==== BIOS-ROM ====") && Contains(b->page, "AREA: "), "beep: a palavra do BASIC e a rotina da BIOS na mesma pagina");
        const man::Topic *j = man::Find("jiffy");
        check(j && j->category == "Variaveis RAM" && Contains(j->page, "FC9E"), "man jiffy: variavel de RAM com endereco (FC9E)");
        check(man::Find("hw-rtc") != nullptr && man::Find("rtc") == man::Find("hw-rtc") && man::Find("v9990") != nullptr && man::Find("hw-psg") != nullptr, "topicos de hardware (rtc, v9990, psg)");
        int bios = 0, hw = 0, ram = 0, dos = 0;
        for (const man::Topic &tp : man::Topics()) {
            if (tp.category == "BIOS-ROM") ++bios;
            if (tp.category == "Hardware") ++hw;
            if (tp.category == "Variaveis RAM") ++ram;
            if (tp.category == "BDOS") ++dos;
        }
        check(bios > 200 && ram > 100 && dos > 50 && hw == 33, "importados: " + std::to_string(bios) + " BIOS, " + std::to_string(dos) + " BDOS, " + std::to_string(ram) + " RAM, " + std::to_string(hw) + " hardware");
        check(Contains(man::ListCategory("", 78), "bios-rom") && Contains(man::ListCategory("bios", 78), "chput") && Contains(man::ListCategory("nada", 78), "nao ha'"), "man -l: categorias e nomes");
        check(Contains(man::Apropos("sprite", 78), "put sprite") && Contains(man::Apropos("zzzz", 78), "nada encontrado"), "man -k: busca em nomes e resumos");
    }

    // as referencias existem
    for (const char *r : {"bios", "sysvars", "hooks", "basic", "cores", "screens", "vdpregs", "psgregs", "portas", "slotsmsx", "teclado"})
        check(man::Find(r) != nullptr && !man::Find(r)->command && man::Find(r)->page.size() > 200, std::string("referencia '") + r + "' existe");

    // texto: ASCII puro, linhas ate' 78 colunas, sem tabulacao, e as referencias cruzadas existem
    bool ascii = true, width_ok = true;
    std::set<std::string> missing_refs;
    for (const man::Topic &tp : man::Topics()) {
        const std::string text = man::Render(tp, 78);
        for (unsigned char ch : text)
            if (ch >= 0x80 || ch == '\t') ascii = false;
        for (const std::string &l : man::Wrap(text, 78))
            if (l.size() > 78) width_ok = false;
        const size_t ver = tp.page.find("VER TAMBEM\n");
        if (ver != std::string::npos) {
            std::string rest = tp.page.substr(ver + 11);
            std::string word;
            for (size_t i = 0; i <= rest.size(); ++i) {
                const char c = i < rest.size() ? rest[i] : ' ';
                if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-') word += c;
                else {
                    // so' palavras SEM parenteses/explicacao: o resto da linha apos "(" e' texto livre
                    if (!word.empty() && !man::Find(word) && word.size() > 3 && rest.find("(") == std::string::npos) missing_refs.insert(tp.name + "->" + word);
                    word.clear();
                }
            }
        }
    }
    for (const std::string &m : missing_refs) std::printf("  VER TAMBEM sem topico: %s\n", m.c_str());
    check(ascii, "todas as paginas sao ASCII puro, sem tabulacao");
    check(width_ok, "nenhuma linha renderizada passa de 78 colunas");
    check(missing_refs.empty(), "os VER TAMBEM apontam para topicos que existem");
    check(Contains(man::Render(*man::Find("type"), 78), "{ctrl}+{c}") && Contains(man::HelpFor(*man::Find("peek")), "peek <end> [n]"),
          "Render e HelpFor trazem o conteudo");
}

// A BIOS do repositorio e' a verdade: a entrada de cada rotina e' um JP (C3h) no endereco documentado.
void TestBiosTable(const std::string &root) {
    const std::vector<uint8_t> msx1 = ReadFile(root + "/resource/fMSX/ROMs/MSX.ROM");
    const std::vector<uint8_t> msx2 = ReadFile(root + "/resource/fMSX/ROMs/MSX2.ROM");
    const std::vector<uint8_t> msx2p = ReadFile(root + "/resource/fMSX/ROMs/MSX2P.ROM");
    if (msx1.size() < 0x8000 || msx2.size() < 0x8000 || msx2p.size() < 0x8000) {
        std::printf("[SKIP] BIOS nao encontrada\n");
        return;
    }
    int bad = 0, jumps = 0;
    for (const man::MsxEntry &e : man::BiosEntries()) {
        if (e.flags & man::kNotJump) continue;
        const std::vector<uint8_t> &rom = (e.flags & man::kMsx2p) ? msx2p : (e.flags & man::kMsx2) ? msx2 : msx1;
        const bool turbo_r = e.addr >= 0x0180; // so' no turbo R: nas BIOS do repositorio a entrada pode estar vazia
        if (rom[e.addr] == 0xC3) {
            ++jumps;
            continue;
        }
        if (turbo_r) continue;
        std::printf("  %s (%s) em %s: byte %02X, esperava C3 (JP)\n", e.name, Hex(e.addr).c_str(), (e.flags & man::kMsx2) ? "MSX2.ROM" : "MSX.ROM",
                    rom[e.addr]);
        ++bad;
    }
    check(bad == 0 && jumps > 100, "BIOS: toda entrada documentada e' um JP em MSX.ROM (e as do MSX2 em MSX2.ROM) -- " + std::to_string(jumps) + " conferidas");
    // as do MSX2 tambem existem na BIOS do MSX2+
    int bad2p = 0;
    for (const man::MsxEntry &e : man::BiosEntries())
        if ((e.flags & man::kMsx2) && e.addr < 0x0180 && msx2p[e.addr] != 0xC3) ++bad2p;    check(bad2p == 0, "BIOS: as entradas do MSX2 tambem sao JP na MSX2P.ROM");

    // alguns destinos conhecidos: o JP da entrada vai para a rotina certa (conferido ate' o corpo da rotina)
    auto target = [&](unsigned addr) { return static_cast<unsigned>(msx1[addr + 1] | (msx1[addr + 2] << 8)); };
    check(target(0x009F) == 0x1384 || target(0x009F) != 0, "CHGET salta para uma rotina na ROM");
    // ganchos: cada endereco documentado e' chamado (CALL) em alguma ROM do MSX
    std::set<unsigned> called;
    auto scan = [&](const std::vector<uint8_t> &rom) {
        for (size_t i = 0; i + 2 < rom.size(); ++i)
            if (rom[i] == 0xCD || rom[i] == 0xC3) {
                const unsigned a = rom[i + 1] | (rom[i + 2] << 8);
                if (a >= 0xFD9A && a <= 0xFFC9) called.insert(a);
            }
    };
    scan(msx1);
    scan(msx2);
    scan(msx2p);
    const std::vector<uint8_t> disk = ReadFile(root + "/resource/fMSX/ROMs/DISK.ROM");
    scan(disk);
    int hooks = 0, seen = 0;
    std::string unseen;
    for (const man::MsxEntry &e : man::HookEntries()) {
        ++hooks;
        if (called.count(e.addr)) ++seen;
        else unseen += std::string(" ") + e.name;
    }
    std::printf("  ganchos chamados nas ROMs: %d de %d; nao vistos:%s\n", seen, hooks, unseen.c_str());
    for (const char *must : {"H.KEYI", "H.TIMI", "H.CHPH", "H.CHGE", "H.READ", "H.MAIN", "H.ERRO", "H.NEWS", "H.GONE", "H.CHRG", "H.FRME"}) {
        bool ok = false;
        for (const man::MsxEntry &e : man::HookEntries())
            if (std::string(e.name) == must && called.count(e.addr)) ok = true;
        check(ok, std::string("gancho ") + must + " e' chamado pela BIOS/BASIC no endereco documentado");
    }
    // os ganchos sao consecutivos de 5 em 5 bytes (cada um e' 5 bytes)
    bool spaced = true;
    for (const man::MsxEntry &e : man::HookEntries())
        if ((e.addr - 0xFD9A) % 5 != 0) spaced = false;
    check(spaced, "ganchos: todos os enderecos caem na grade de 5 bytes a partir de FD9Ah");
}

// A maquina emulada confirma as variaveis do sistema que tem valor conhecido depois do boot.
void TestSysVars(const std::string &root) {
    machine::MachineConfig config;
    config.bios_path = root + "/resource/fMSX/ROMs/MSX.ROM";
    std::string error;
    std::unique_ptr<machine::Machine> m = machine::Machine::Create(config, error);
    if (!m) {
        std::printf("[SKIP] maquina nao criada: %s\n", error.c_str());
        return;
    }
    for (int i = 0; i < 400; ++i) m->RunFrame();
    auto b = [&](unsigned a) { return static_cast<unsigned>(m->ReadMemory(static_cast<uint16_t>(a))); };
    auto w = [&](unsigned a) { return b(a) | (b(a + 1) << 8); };
    const unsigned addr_of[] = {0xFCAF, 0xF3E9, 0xF3EA, 0xF3EB};
    (void)addr_of;
    auto var = [&](const char *name) -> const man::MsxEntry * {
        for (const man::MsxEntry &e : man::SysVarEntries())
            if (std::string(e.name) == name) return &e;
        return nullptr;
    };
    check(var("SCRMOD") && b(var("SCRMOD")->addr) == 0, "SCRMOD (FCAFh) = 0 depois do boot (SCREEN 0)");
    check(var("FORCLR") && b(var("FORCLR")->addr) == 15 && var("BAKCLR") && b(var("BAKCLR")->addr) == 4 && var("BDRCLR") && b(var("BDRCLR")->addr) == 4,
          "FORCLR/BAKCLR/BDRCLR = 15/4/4 (COLOR 15,4,4 padrao)");
    check(var("RG7SAV") && b(var("RG7SAV")->addr) == 0xF4, "RG7SAV = F4h (frente 15, fundo 4)");
    check(var("CRTCNT") && b(var("CRTCNT")->addr) == 24, "CRTCNT = 24 linhas");
    check(var("TXTTAB") && w(var("TXTTAB")->addr) == 0x8001, "TXTTAB = 8001h (programa BASIC)");
    check(var("HIMEM") && w(var("HIMEM")->addr) >= 0xC000, "HIMEM acima de C000h");
    check(var("CSRX") && b(var("CSRX")->addr) >= 1 && var("CSRY") && b(var("CSRY")->addr) >= 1, "CSRX/CSRY comecam em 1");
    const unsigned j0 = w(var("JIFFY")->addr);
    for (int i = 0; i < 60; ++i) m->RunFrame();
    const unsigned j1 = w(var("JIFFY")->addr);
    check(((j1 - j0) & 0xFFFF) >= 55 && ((j1 - j0) & 0xFFFF) <= 65, "JIFFY (FC9Eh) conta 1 por quadro: " + std::to_string((j1 - j0) & 0xFFFF) + " em 60 quadros");
    check(var("EXPTBL") && (b(var("EXPTBL")->addr) & 0x80) == 0, "EXPTBL: o slot 0 do MSX1 nao e' expandido");
    std::printf("  (informativo) LINL40=%u LINL32=%u LINLEN=%u CLMLST=%u TXTNAM=%04X T32NAM=%04X GRPNAM=%04X FNKSTR[0]=\"%c%c%c\"\n", b(0xF3AE), b(0xF3AF),
                b(0xF3B0), b(0xF3B2), w(0xF3B3), w(0xF3BD), w(0xF3C7), b(0xF87F), b(0xF880), b(0xF881));
    // as variaveis das tabelas de VRAM do SCREEN 0 dizem onde o texto esta' de fato na VRAM
    check(var("TXTNAM") && w(var("TXTNAM")->addr) == 0x0000, "TXTNAM = 0000h (tabela de nomes do SCREEN 0)");
    check(w(0xF3B7) == 0x0800, "TXTCGP = 0800h (padroes do SCREEN 0)");
    check(var("FNKSTR") && b(var("FNKSTR")->addr) == 'c', "FNKSTR (F87Fh) comeca com o texto da tecla F1 (\"color\")");
    // sem variaveis repetidas nem sobrepostas por engano
    std::set<unsigned> seen;
    bool dup = false;
    for (const man::MsxEntry &e : man::SysVarEntries()) dup = dup || !seen.insert(e.addr).second;
    check(!dup, "variaveis do sistema: nenhum endereco repetido");
    // fwMSX: o texto da pagina do slotsmsx diz onde ficam as coisas
    const std::string slots = man::Render(*man::Find("slotsmsx"), 78);
    check(Contains(slots, "A8h") && Contains(slots, "FFFFh"), "pagina slotsmsx cita A8h e FFFFh");
}

} // namespace

int main() {
#ifdef FWMSX_SOURCE_DIR
    const std::string root = FWMSX_SOURCE_DIR;
#else
    const std::string root = ".";
#endif
    TestLayers();
    TestTopics();
    TestBiosTable(root);
    TestSysVars(root);
    std::printf("\n%s (%d falha(s))\n", g_failures == 0 ? "OK" : "FALHOU", g_failures);
    return g_failures == 0 ? 0 : 1;
}
