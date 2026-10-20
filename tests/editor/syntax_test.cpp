// Teste do destaque de sintaxe do editor (src/editor/cpp/syntax*; fase E4): as linguagens, o estado de varias linhas, a escolha pela extensao
// e o nucleo (C, Fortran e Assembly). Sem terminal. Ver doc/editor-spec.md.
#include <cstdio>
#include <string>
#include <vector>

#include "../../src/editor/asm/ed_hash.h"
#include "../../src/editor/c/ed_syntax.h"
#include "../../src/editor/cpp/syntax.h"

namespace {

using ed::HlState;
using ed::Span;

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

// "classe(texto)" de cada trecho, separados por espaco. `st` carrega o estado entre linhas.
std::string Fmt(const char *lang, const std::string &line, HlState *st = nullptr) {
    const ed::Lang *l = ed::LangById(lang);
    if (!l) return "SEM LINGUAGEM";
    HlState local;
    std::vector<Span> spans;
    l->Highlight(line, st ? *st : local, spans);
    std::string out;
    for (const Span &s : spans) {
        if (!out.empty()) out += " ";
        out += std::string(ed::SynClassName(s.cls)) + "(" + line.substr(static_cast<size_t>(s.from), static_cast<size_t>(s.to - s.from)) + ")";
    }
    return out;
}

bool Sane(const char *lang, const std::string &line) {
    const ed::Lang *l = ed::LangById(lang);
    HlState st;
    std::vector<Span> spans;
    l->Highlight(line, st, spans);
    int prev = 0;
    for (const Span &s : spans) {
        if (s.from < prev || s.to <= s.from || s.to > static_cast<int>(line.size())) return false;
        prev = s.to;
    }
    return true;
}

void test_kernels() {
    check(ed_hash_ci(reinterpret_cast<const unsigned char *>("PRINT"), 5) == ed_hash_ci(reinterpret_cast<const unsigned char *>("print"), 5), "ed_hash_ci (NASM): ignora a caixa");
    check(ed_hash_ci(reinterpret_cast<const unsigned char *>("print"), 5) != ed_hash_ci(reinterpret_cast<const unsigned char *>("prinT2"), 6), "ed_hash_ci: palavras diferentes, hashes diferentes");
    check(ed_hash_ci(reinterpret_cast<const unsigned char *>("a"), 0) == 2166136261u, "ed_hash_ci: vazio = deslocamento do FNV-1a");
    struct { const char *t; int kind; } nums[] = {
        {"123", 1}, {"12%", 1}, {"1.5", 5}, {"1E5", 5}, {"1.5E-3", 5}, {"3#", 5}, {"0.5!", 5}, {"&H1F", 2}, {"&h1f", 2}, {"&B101", 3}, {"&O17", 4}, {"&17", 4},
        {"0x1F", 2}, {"$FF", 2}, {"#FF", 2}, {"%101", 3}, {"0FFh", 2}, {"12h", 2}, {"101b", 3}, {"1abc", 0}, {"&H", 0}, {"$", 0}, {"abc", 0}, {"12b", 0 + 0},
    };
    bool ok = true;
    for (const auto &c : nums) {
        const int k = ed_number_kind(c.t, static_cast<int>(std::string(c.t).size()));
        // "12b" nao e' binario nem decimal valido
        const int want = std::string(c.t) == "12b" ? 0 : c.kind;
        if (k != want) {
            std::printf("    ed_number_kind(\"%s\") = %d (esperado %d)\n", c.t, k, want);
            ok = false;
        }
    }
    check(ok, "ed_number_kind (Fortran): todas as formas de numero");
    check(ed_hl_ident_end("abc_1$ x", 8, 0, ED_ID_SUFFIX) == 6 && ed_hl_ident_end("abc.def", 7, 0, ED_ID_DOT) == 7 && ed_hl_ident_end("1abc", 4, 0, 0) == 0, "ed_hl_ident_end (C)");
    check(ed_hl_string_end("\"a\"\"b\" x", 8, 0, '"') == 6 && ed_hl_string_end("'abc", 4, 0, '\'') == 4, "ed_hl_string_end: aspa dobrada continua; sem fechar vai ate' o fim");
}

void test_basic() {
    check(Fmt("msxbasic", "10 PRINT \"HELLO\":GOTO 10") == "lineno(10) keyword(PRINT) string(\"HELLO\") keyword(GOTO) label(10)", "MSX-BASIC: numero de linha, comando, string, GOTO <linha>");
    check(Fmt("msxbasic", "20 FORI=1TO10") == "lineno(20) keyword(FOR) number(1) keyword(TO) number(10)", "MSX-BASIC: listagem compacta em maiusculas (FORI=1TO10)");
    check(Fmt("msxbasic", "30 REM um comentario") == "lineno(30) comment(REM um comentario)", "MSX-BASIC: REM ate' o fim da linha");
    check(Fmt("msxbasic", "40 A$=CHR$(65)+\"X\" 'nota") == "lineno(40) keyword2(CHR$) number(65) string(\"X\") comment('nota)", "MSX-BASIC: funcao, string e comentario com apostrofo");
    check(Fmt("msxbasic", "50 PRINT &HFF,&B101,3.5E2") == "lineno(50) keyword(PRINT) number(&HFF) number(&B101) number(3.5E2)", "MSX-BASIC: &H, &B e real com expoente");
    check(Fmt("msxbasic", "60 CALL MUSIC(0,1)") == "lineno(60) keyword(CALL) directive(MUSIC) number(0) number(1)", "MSX-BASIC: CALL <extensao>");
    check(Fmt("msxbasic", "70 _TURBO ON") == "lineno(70) directive(_TURBO) keyword(ON)", "MSX-BASIC: _EXTENSAO no inicio da instrucao");
    check(Fmt("msxbasic", "80 IF A=1 THEN 100 ELSE 200") == "lineno(80) keyword(IF) number(1) keyword(THEN) label(100) keyword(ELSE) label(200)", "MSX-BASIC: THEN/ELSE <linha> sao rotulos");
    check(Fmt("msxbasic", "90 ON X GOTO 10,20,30") == "lineno(90) keyword(ON) keyword(GOTO) label(10) label(20) label(30)", "MSX-BASIC: ON x GOTO lista");
    check(Fmt("msxbasic", "print total") == "keyword(print)", "MSX-BASIC: 'total' em minusculas nao e' fatiado");
    check(Fmt("msxbasic", "? \"hi\"") == "keyword(?) string(\"hi\")", "MSX-BASIC: ? e' PRINT");
    check(Fmt("msxbasic", "100 A=B AND NOT C") == "lineno(100) operator(AND) operator(NOT)", "MSX-BASIC: operadores por extenso");
    check(Fmt("msxbasic", "110 PRINT \"sem fechar") == "lineno(110) keyword(PRINT) string(\"sem fechar)", "MSX-BASIC: string sem fechar vai ate' o fim");
    check(Fmt("msxbasic", "120 X=USR(0):POKE &HF3DB,0") == "lineno(120) keyword2(USR) number(0) keyword(POKE) number(&HF3DB) number(0)", "MSX-BASIC: USR e POKE");
}

void test_dignified() {
    check(Fmt("dignified", "{start}") == "label({start})", "Dignified: {rotulo}");
    check(Fmt("dignified", "loop{") == "label(loop{)", "Dignified: nome{ abre um loop label");
    check(Fmt("dignified", "}") == "label(})", "Dignified: } fecha");
    check(Fmt("dignified", "## so para mim") == "comment(## so para mim)", "Dignified: ## e' comentario exclusivo");
    check(Fmt("dignified", "##BB:arguments=-ss") == "directive(##BB:arguments=-ss)", "Dignified: remtag ##BB:");
    check(Fmt("dignified", "print \"a\" ' fica") == "keyword(print) string(\"a\") comment(' fica)", "Dignified: print, string e comentario normal");
    check(Fmt("dignified", "define [pk][poke 100,[10]]") == "directive(define) directive([pk][poke 100,[10]])", "Dignified: define e os [alias]");
    check(Fmt("dignified", "x = .upper(\"a\") + ~total") == "keyword2(.upper) string(\"a\") variable(~total)", "Dignified: .funcao(...) e ~variavel");
    check(Fmt("dignified", "if inkey$ <> \"A\" then goto {@}") == "keyword(if) keyword2(inkey$) string(\"A\") keyword(then) keyword(goto) label({@})", "Dignified: goto {rotulo}");
    check(Fmt("dignified", "10 print 5") == "number(10) keyword(print) number(5)", "Dignified: nao ha' numero de linha (e' numero)");
    HlState st;
    check(Fmt("dignified", "''", &st) == "comment('')" && st.mode == 1, "Dignified: '' abre um bloco de comentario");
    check(Fmt("dignified", "print \"dentro\"", &st) == "comment(print \"dentro\")", "Dignified: dentro do bloco tudo e' comentario");
    check(Fmt("dignified", "''", &st) == "comment('')" && st.mode == 0, "Dignified: '' fecha o bloco");
    check(Fmt("dignified", "print 1", &st) == "keyword(print) number(1)", "Dignified: depois do bloco volta ao normal");
    HlState st2;
    Fmt("dignified", "###", &st2);
    check(st2.mode == 2, "Dignified: ### abre o outro tipo de bloco");
    check(Fmt("dignified", "'' fecha?", &st2) == "comment('' fecha?)" && st2.mode == 2, "Dignified: '' nao fecha um bloco ###");
}

void test_dialects() {
    check(Fmt("msxbas2rom", "FILE \"SCREEN.SC1\"") == "directive(FILE) string(\"SCREEN.SC1\")", "MSXBAS2ROM: FILE");
    check(Fmt("msxbas2rom", "10 CMD TURBO 1") == "lineno(10) keyword(CMD) directive(TURBO) number(1)", "MSXBAS2ROM: CMD <comando estendido>");
    check(Fmt("nestorbasic", "10 ?USR(0)") == "lineno(10) keyword(?) keyword2(USR) number(0)", "NestorBASIC: usa o MSX-BASIC + as funcoes NB_");
    check(Fmt("nestorbasic", "x=nb_playmusic").find("directive(nb_playmusic)") != std::string::npos, "NestorBASIC: nomes NB_ sao diretivas");
    check(Fmt("kunbasic", "120 _TURBO ON") == "lineno(120) directive(_TURBO) keyword(ON)", "Kun BASIC: _TURBO ON");
    check(Fmt("kunbasic", "120 CALL TURBO ON(A,C())") == "lineno(120) keyword(CALL) directive(TURBO) keyword(ON)", "Kun BASIC: CALL TURBO ON(A,C())");
    check(Fmt("kunbasic", "10 _RUN") == "lineno(10) directive(_RUN)", "Kun BASIC: _RUN");
    check(Fmt("kunbasic", "120 '#I &HF3,&HCD,@150,&HFB") == "lineno(120) directive('#I) number(&HF3) number(&HCD) label(@150) number(&HFB)", "Kun BASIC: '#I e @linha");
    check(Fmt("kunbasic", "20 '#C- sem recorte") == "lineno(20) directive('#C-) comment( sem recorte)", "Kun BASIC: '#C-");
    check(Fmt("kunbasic", "30 '#N+") == "lineno(30) directive('#N+)", "Kun BASIC: '#N+");
    check(Fmt("kunbasic", "40 'um comentario comum") == "lineno(40) comment('um comentario comum)", "Kun BASIC: comentario comum");
    {
        HlState st;
        check(Fmt("kunbasic", "100 OPEN \"A\" AS #1", &st) == "lineno(100) keyword(OPEN) string(\"A\") keyword(AS) number(1)" && st.mode == 0, "Kun BASIC: fora do bloco OPEN e' comando normal");
        Fmt("kunbasic", "110 _TURBO ON", &st);
        check(st.mode == 1, "Kun BASIC: _TURBO ON abre o bloco (estado de varias linhas)");
        check(Fmt("kunbasic", "120 OPEN \"A\" AS #1", &st) == "lineno(120) error(OPEN) string(\"A\") keyword(AS) number(1)", "Kun BASIC: dentro do bloco OPEN nao compila (erro)");
        check(Fmt("kunbasic", "130 FOR X=0 TO 255:LINE(X,0)-(X,211),0:NEXT X", &st).find("error(") == std::string::npos, "Kun BASIC: FOR/LINE/NEXT compilam");
        check(Fmt("kunbasic", "135 PRINT #1,A$", &st).find("error(PRINT)") != std::string::npos && Fmt("kunbasic", "136 PRINT USING \"#\";A", &st).find("error(USING)") != std::string::npos, "Kun BASIC: PRINT# e PRINT USING nao compilam");
        check(Fmt("kunbasic", "137 CALL MUSIC", &st).find("error(CALL)") != std::string::npos && Fmt("kunbasic", "138 TAB(3)", &st).find("error(TAB)") != std::string::npos, "Kun BASIC: CALL (menos TURBO) e TAB nao compilam");
        check(Fmt("kunbasic", "139 CALL TURBO OFF", &st) == "lineno(139) keyword(CALL) directive(TURBO) keyword(OFF)" && st.mode == 0, "Kun BASIC: CALL TURBO OFF fecha o bloco e nao e' erro");
        check(Fmt("kunbasic", "140 OPEN \"A\" AS #1", &st).find("keyword(OPEN)") != std::string::npos, "Kun BASIC: depois do bloco OPEN volta ao normal");
    }
}

void test_asm() {
    check(Fmt("z80asm", "loop: ld a,(hl) ; le o byte") == "label(loop) keyword(ld) register(a) register(hl) comment(; le o byte)", "ASM: rotulo, mnemonico, registradores, comentario");
    check(Fmt("z80asm", "    call CHPUT") == "keyword(call) bios(CHPUT)", "ASM: chamada da BIOS");
    check(Fmt("z80asm", "    ld a,(LINL40)") == "keyword(ld) register(a) bios(LINL40)", "ASM: variavel da BIOS");
    check(Fmt("z80asm", ".org 0x4000") == "directive(.org) number(0x4000)", "ASM: .org");
    check(Fmt("z80asm", "msg: db \"Hi\",0") == "label(msg) directive(db) string(\"Hi\") number(0)", "ASM: db com string e numero");
    check(Fmt("z80asm", "  ld a,#FF") == "keyword(ld) register(a) number(#FF)", "ASM: #FF");
    check(Fmt("z80asm", "  ld b,0FFh") == "keyword(ld) register(b) number(0FFh)", "ASM: 0FFh");
    check(Fmt("z80asm", "  ld c,%1010") == "keyword(ld) register(c) number(%1010)", "ASM: %1010 depois de virgula");
    check(Fmt("z80asm", "  ld hl,$C000") == "keyword(ld) register(hl) number($C000)", "ASM: $C000");
    check(Fmt("z80asm", "  jr nz,.again") == "keyword(jr) register(nz) label(.again)", "ASM: condicao e rotulo local");
    check(Fmt("z80asm", ".again: djnz .again") == "label(.again) keyword(djnz) label(.again)", "ASM: definicao de rotulo local");
    check(Fmt("z80asm", "name equ 5") == "label(name) directive(equ) number(5)", "ASM: nome equ valor (o nome e' rotulo)");
    check(Fmt("z80asm", "  ex af,af'") == "keyword(ex) register(af) register(af')", "ASM: af'");
    check(Fmt("z80asm", "  ld a,'x'") == "keyword(ld) register(a) string('x')", "ASM: caractere");
    HlState st;
    check(Fmt("z80asm", "  nop /* abre", &st) == "keyword(nop) comment(/* abre)" && st.mode == 1, "ASM: /* abre um comentario de varias linhas");
    check(Fmt("z80asm", "ld a,1 ; ainda dentro", &st) == "comment(ld a,1 ; ainda dentro)", "ASM: a linha de dentro e' comentario");
    check(Fmt("z80asm", "fecha */ ld a,2", &st) == "comment(fecha */) keyword(ld) register(a) number(2)" && st.mode == 0, "ASM: */ fecha e o codigo volta");
    check(Fmt("z80asm", "  ld a,b // c") == "keyword(ld) register(a) register(b) comment(// c)", "ASM: // tambem comenta");
    check(Fmt("z80asm", "  rept 4") == "directive(rept) number(4)", "ASM: rept");
}

void test_pascal() {
    check(Fmt("pascal", "begin { comentario }") == "keyword(begin) comment({ comentario })", "Pascal: begin e { }");
    HlState st;
    check(Fmt("pascal", "{ abre", &st) == "comment({ abre)" && st.mode == 1, "Pascal: { sem fechar atravessa a linha");
    check(Fmt("pascal", "meio da coisa", &st) == "comment(meio da coisa)", "Pascal: a linha do meio e' comentario");
    check(Fmt("pascal", "fecha } x := 1;", &st) == "comment(fecha }) number(1)" && st.mode == 0, "Pascal: } fecha e o codigo volta");
    check(Fmt("pascal", "s := 'it''s'; // fim") == "string('it''s') comment(// fim)", "Pascal: string com aspa dobrada e //");
    check(Fmt("pascal", "{$I file.inc}") == "directive({$I file.inc})", "Pascal: diretiva do compilador {$...}");
    check(Fmt("pascal", "x := $FF + 3.14;") == "number($FF) number(3.14)", "Pascal: hexa e real");
    check(Fmt("pascal", "var i: integer;") == "keyword(var) keyword2(integer)", "Pascal: var e tipo");
    check(Fmt("pascal", "(* velho *) x") == "comment((* velho *))", "Pascal: (* *)");
    HlState st2;
    check(Fmt("pascal", "(* abre", &st2) == "comment((* abre)" && st2.mode == 2, "Pascal: (* sem fechar");
    check(Fmt("pascal", "} nao fecha (*", &st2) == "comment(} nao fecha (*)" && st2.mode == 2, "Pascal: } nao fecha um (*");
    check(Fmt("pascal", "BEGIN").find("keyword(BEGIN)") == 0, "Pascal: palavras-chave sem diferenciar a caixa");
    check(Fmt("pascal", "procedure Foo; bank 2; public;").find("keyword(bank)") != std::string::npos, "Pascal: BANK (Kizuna)");
}

void test_bat() {
    check(Fmt("bat", "@echo off") == "operator(@) keyword(echo) keyword(off)", "BAT: @echo off");
    check(Fmt("bat", ":loop") == "label(:loop)", "BAT: :rotulo");
    check(Fmt("bat", "rem comentario") == "comment(rem comentario)", "BAT: rem");
    check(Fmt("bat", ":: outro") == "comment(:: outro)", "BAT: ::");
    check(Fmt("bat", "if %1 == \"x\" goto fim") == "keyword(if) variable(%1) string(\"x\") keyword(goto)", "BAT: if, %1, string, goto");
    check(Fmt("bat", "set A=%PATH%") == "keyword(set) variable(%PATH%)", "BAT: %VAR%");
    check(Fmt("bat", "for %%i in (*.asm) do echo %%i") == "keyword(for) variable(%%i) keyword(in) keyword(do) keyword(echo) variable(%%i)", "BAT: for %%i");
    check(Fmt("bat", "dir > nul").find("keyword(dir)") == 0, "BAT: dir");
}

void test_markdown() {
    check(Fmt("markdown", "# Titulo") == "heading(# Titulo)", "Markdown: titulo");
    check(Fmt("markdown", "- item com `codigo`") == "operator(-) string(`codigo`)", "Markdown: lista e codigo em linha");
    check(Fmt("markdown", "texto **forte** aqui") == "keyword2(**forte**)", "Markdown: negrito");
    check(Fmt("markdown", "veja [aqui](http://x)") == "label([aqui]) comment((http://x))", "Markdown: link");
    check(Fmt("markdown", "> citacao") == "comment(> citacao)", "Markdown: citacao");
    check(Fmt("markdown", "1. primeiro") == "operator(1.)", "Markdown: lista numerada");
    check(Fmt("markdown", "---") == "operator(---)", "Markdown: linha horizontal");
    HlState st;
    check(Fmt("markdown", "```cpp", &st) == "directive(```cpp)" && st.mode == 1, "Markdown: ``` abre um bloco");
    check(Fmt("markdown", "# nao e' titulo", &st) == "string(# nao e' titulo)", "Markdown: dentro do bloco e' codigo");
    check(Fmt("markdown", "```", &st) == "string(```)" && st.mode == 0, "Markdown: ``` fecha");
    check(Fmt("plain", "qualquer coisa PRINT 10") == "", "texto puro: nada destacado");
}

void test_detection() {
    auto id = [](const char *path, const std::string &sample = std::string()) { return ed::LangForFile(path, sample)->Id(); };
    check(id("a.dmx") == "dignified" && id("A.DMX") == "dignified", "extensao .dmx = Basic Dignified");
    check(id("x.amx") == "msxbasic" && id("x.bas") == "msxbasic" && id("x.asc") == "msxbasic", ".amx/.bas/.asc = MSX-BASIC");
    check(id("a.asm") == "z80asm" && id("a.inc") == "z80asm" && id("c:\\p\\a.z80") == "z80asm", ".asm/.inc/.z80 = Assembly Z80");
    check(id("a.pas") == "pascal" && id("a.bat") == "bat" && id("a.btm") == "bat" && id("a.md") == "markdown", ".pas/.bat/.btm/.md");
    check(id("a.txt") == "plain" && id("semextensao") == "plain" && id("dir.v1/arq") == "plain", "sem extensao conhecida = texto puro");
    check(id("a.bas", "10 PRINT 1\n20 FILE \"X.SC2\"") == "msxbasic" && id("a.bas", "FILE \"SCREEN.SC1\"\n10 SCREEN 1") == "msxbas2rom", ".bas com FILE \"...\" = MSXBAS2ROM");
    check(id("a.bas", "10 CMD TURBO 1") == "msxbas2rom", ".bas com CMD <comando estendido> = MSXBAS2ROM");
    check(id("a.bas", "10 X=NB_ExecCode") == "nestorbasic", ".bas com NB_ = NestorBASIC");
    check(id("a.bas", "100 SCREEN 8:DEFINT A-Z\n120 _TURBO ON") == "kunbasic" && id("a.bas", "10 CALL TURBO ON") == "kunbasic" && id("x.kun") == "kunbasic", ".bas com _TURBO ON / CALL TURBO = MSX-BASIC KUN");
    check(ed::LangById("pascal") != nullptr && ed::LangById("nada") == nullptr && ed::AllLangs().size() == 10 && ed::AllLangs()[0]->Id() == "plain", "registro: 10 linguagens, a primeira e' o texto puro");
}

void test_sanity() {
    const char *langs[] = {"plain", "msxbasic", "dignified", "msxbas2rom", "nestorbasic", "kunbasic", "z80asm", "pascal", "bat", "markdown"};
    const std::string lines[] = {"", " ", "10", "10 PRINT \"", "\"\"\"", "{{{", "}}}", "(*", "*)", "/*", "*/", "'", "''", "```", "[[[", "]]]", "~~~", "%%%", "&&&", "$$$", "###",
                                 "FORI=1TO10:NEXT", ".", "..", ":", "::", "@", "@@@", "1.2.3", "&H", "0x", "af''", "{$", "(*)", "-", "***", "\t\t", "label:", "a\tb"};
    bool ok = true;
    for (const char *l : langs) {
        for (const std::string &s : lines) {
            if (!Sane(l, s)) {
                std::printf("    %s: linha '%s' gerou trechos invalidos\n", l, s.c_str());
                ok = false;
            }
        }
    }
    check(ok, "10 linguagens x 37 linhas estranhas: trechos sempre em ordem, sem sobrepor e dentro da linha");
}

} // namespace

int main(int argc, char **argv) {
    // `syntaxtest --show <linguagem> <linha>`: mostra os trechos de uma linha (para depurar um caso)
    if (argc == 4 && std::string(argv[1]) == "--show") {
        std::printf("%s\n", Fmt(argv[2], argv[3]).c_str());
        return 0;
    }
    test_kernels();
    test_basic();
    test_dignified();
    test_dialects();
    test_asm();
    test_pascal();
    test_bat();
    test_markdown();
    test_detection();
    test_sanity();
    std::printf("\n%s (%d falha%s)\n", g_failures == 0 ? "TODOS OS TESTES PASSARAM" : "HA' FALHAS", g_failures, g_failures == 1 ? "" : "s");
    return g_failures == 0 ? 0 : 1;
}
