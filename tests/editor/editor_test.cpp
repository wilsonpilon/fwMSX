// Teste do editor de textos (src/editor): o modelo sem interface. As teclas entram por NOME, como a TUI e a janela as
// repassam, e o teste confere o texto, o cursor, os dialogos pedidos e a grade desenhada. Ver doc/editor-spec.md.
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include "../../src/editor/asm/ed_indent.h"
#include "../../src/editor/c/ed_syntax.h"
#include "../../src/editor/c/ed_text.h"
#include "../../src/editor/cpp/editor.h"

namespace {

namespace fs = std::filesystem;
using ed::Cmd;
using ed::Dialog;
using ed::Editor;
using ed::Pos;

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

void keys(Editor &e, std::initializer_list<const char *> ks) {
    for (const char *k : ks) e.Key(k);
}

std::string row(const ed::Grid &g, int y) {
    std::string s;
    for (int x = 0; x < g.cols; ++x) {
        const unsigned c = g.at(x, y).ch;
        s.push_back(c < 256 ? static_cast<char>(c) : '#');
    }
    return s;
}

std::string slurp(const fs::path &p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

void spit(const fs::path &p, const std::string &data) {
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    f.write(data.data(), static_cast<std::streamsize>(data.size()));
}

std::string many_lines(int n) {
    std::string t;
    for (int i = 1; i <= n; ++i) {
        if (i > 1) t += '\n';
        t += "line" + std::to_string(i);
    }
    return t;
}

// ---------------------------------------------------------------------------------------------------------------

void test_kernels() {
    check(ed_find("hello world", 11, "world", 5, 0, 1, 1) == 6, "ed_find: adiante acha");
    check(ed_find("hello world", 11, "WORLD", 5, 0, 1, 0) == 6, "ed_find: ignora a caixa");
    check(ed_find("hello world", 11, "WORLD", 5, 0, 1, 1) == -1, "ed_find: respeita a caixa");
    check(ed_find("abcabc", 6, "abc", 3, 5, 0, 1) == 3, "ed_find: para tras acha a ultima");
    check(ed_find("abcabc", 6, "abc", 3, -1, 0, 1) == -1, "ed_find: para tras sem espaco");
    check(ed_word_right("foo, bar", 8, 0) == 5, "ed_word_right: pula palavra e pontuacao");
    check(ed_word_left("foo, bar", 8, 8) == 5, "ed_word_left: volta ao inicio da palavra");
    const unsigned char ws[] = {' ', ' ', '\t', 'x'};
    check(ed_leading_ws(ws, 4) == 3, "ed_leading_ws (Assembly): 3 bytes de indentacao");
    check(ed_leading_ws(ws, 0) == 0, "ed_leading_ws: linha vazia");
    char buf[] = "aBc1";
    ed_flip_case(buf, 0, 4);
    check(std::string(buf) == "AbC1", "ed_flip_case inverte so' as letras");
}

void test_bindings() {
    std::set<std::string> seen;
    bool unique = true;
    bool all_known = true;
    for (const ed::KeyBinding &b : ed::Bindings()) {
        if (!seen.insert(ed::CanonicalKey(b.key)).second) unique = false;
        if (b.cmd == Cmd::None) all_known = false;
    }
    check(unique, "Bindings: nenhuma tecla repetida");
    check(all_known, "Bindings: toda tecla leva a um comando");
    check(ed::CanonicalKey("A-C-X") == "C-A-x" && ed::CanonicalKey("S-Tab") == "S-Tab", "CanonicalKey ordena os modificadores");
    // as teclas do NE 2 (Help on keys) estao todas la'
    const char *norton[] = {"Left", "Right", "Up", "Down", "C-Left", "C-Right", "Home", "End", "PgUp", "PgDn", "C-PgUp", "C-PgDn",
                            "C-Home", "C-End", "A-z", "C-z", "BS", "Del", "C-w", "A-w", "C-l", "A-l", "A-k", "C-u", "C-f", "C-r",
                            "A-c", "C-c", "C-]", "C-a", "A-a", "C-p", "C-Enter", "C-BS", "S-Tab", "C-2", "A-2", "Ins", "F1", "F2"};
    bool norton_ok = true;
    for (const char *k : norton) {
        bool found = false;
        for (const ed::KeyBinding &b : ed::Bindings()) {
            if (!b.alt && ed::CanonicalKey(b.key) == ed::CanonicalKey(k)) found = true;
        }
        if (!found) {
            norton_ok = false;
            std::printf("    falta a tecla do NE: %s\n", k);
        }
    }
    check(norton_ok, "Bindings: as 40 teclas do Norton Editor 2 existem (alt = false)");
    // todo mnemonico documentado do NE (Alt-F N...) existe
    const struct { char menu; char letter; } mn[] = {
        {'f', 'n'}, {'f', 'o'}, {'f', 's'}, {'f', 'a'}, {'f', 'm'}, {'f', '2'}, {'f', 'c'}, {'f', 'l'}, {'f', 'w'}, {'f', 't'}, {'f', 'd'}, {'f', 'x'},
        {'b', 'c'}, {'b', 'm'}, {'b', 'd'}, {'b', 's'}, {'b', 'r'}, {'b', 'f'}, {'b', 'i'}, {'b', 'u'}, {'b', 'o'}, {'b', 0x7F}, {'b', 'l'}, {'b', 'e'},
        {'t', 'p'}, {'t', 'd'}, {'t', 'c'}, {'t', 's'}, {'t', 'w'}, {'t', 'i'}, {'t', 'b'}, {'t', 'e'}, {'t', 't'}, {'t', 'a'},
        {'s', 'l'}, {'s', 'd'}, {'s', 'b'}, {'s', 'm'}, {'p', 'p'}, {'p', 'b'}, {'p', 'e'}, {'p', 's'},
        {'v', 'i'}, {'v', 'o'}, {'v', 'a'}, {'o', 'd'}, {'o', 'm'}, {'o', 'e'}, {'o', 's'}};
    bool mn_ok = true;
    for (const auto &m : mn) {
        bool found = false;
        for (const ed::Menu &menu : ed::Menus()) {
            if (menu.hot != m.menu) continue;
            for (const ed::MenuItem &it : menu.items) {
                if (it.hot == m.letter) found = true;
            }
        }
        if (!found) {
            mn_ok = false;
            std::printf("    falta o mnemonico Alt-%c %c\n", m.menu, m.letter);
        }
    }
    check(mn_ok, "Menus: os 49 mnemonicos documentados do NE 2 existem");
}

void test_typing_and_movement() {
    Editor e;
    e.Type("hello");
    check(e.Text() == "hello" && e.Cursor() == Pos{0, 5}, "Type insere e move o cursor");
    check(e.Modified(), "Modified apos digitar");
    e.Key("Enter");
    e.Type("x");
    check(e.Text() == "hello\nx" && e.Cursor() == Pos{1, 1}, "Enter divide a linha");
    e.Key("Left");
    e.Key("Left");
    check(e.Cursor() == Pos{0, 5}, "Left no inicio da linha vai ao fim da anterior");
    e.Key("Right");
    check(e.Cursor() == Pos{1, 0}, "Right no fim da linha vai ao inicio da proxima");
    e.Key("C-End");
    check(e.Cursor() == Pos{1, 1}, "C-End: fim do arquivo");
    e.Key("C-Home");
    check(e.Cursor() == Pos{0, 0}, "C-Home: inicio do arquivo");
    e.Key("End");
    check(e.Cursor() == Pos{0, 5}, "End: fim da linha");
    e.Key("Home");
    check(e.Cursor() == Pos{0, 0}, "Home: inicio da linha");

    e.SetText("one two\nthree");
    e.SetCursor({0, 0});
    e.Key("C-Right");
    check(e.Cursor() == Pos{0, 4}, "C-Right: proxima palavra");
    e.Key("C-Right");
    e.Key("C-Right");
    check(e.Cursor() == Pos{1, 0}, "C-Right no fim da linha pula para a proxima");
    e.Key("C-Left");
    e.Key("C-Left");
    check(e.Cursor() == Pos{0, 4}, "C-Left: palavra anterior");
    e.Key("A-Right");
    check(e.Cursor() == Pos{0, 7}, "A-Right (alternativa) = C-Right");

    // a coluna desejada atravessa linhas de tamanhos diferentes e tabulacoes
    e.SetText("a\tb\nxxxxxxxxxxxx");
    e.SetCursor({0, 2});
    e.Key("Down");
    check(e.Cursor() == Pos{1, 8}, "Down mantem a coluna de tela (tab conta 8)");
    e.Key("Up");
    check(e.Cursor() == Pos{0, 2}, "Up volta a coluna com tabulacao");
    check(e.DisplayCol({0, 2}) == 8, "DisplayCol expande a tabulacao");

    e.SetText(many_lines(100));
    e.Key("PgDn");
    check(e.TopLine() == 22 && e.Cursor().line == 22, "PgDn desce uma tela (22 linhas)");
    e.Key("PgUp");
    check(e.TopLine() == 0 && e.Cursor().line == 0, "PgUp sobe uma tela");
    e.Key("A-z");
    check(e.TopLine() == 1 && e.Cursor().line == 1, "A-z rola o texto para cima e o cursor acompanha");
    e.Key("C-z");
    check(e.TopLine() == 0, "C-z rola o texto para baixo");
    e.Key("C-PgDn");
    check(e.Cursor().line == 22, "C-PgDn: fim da tela");
    e.Key("C-PgUp");
    check(e.Cursor().line == 0, "C-PgUp: topo da tela");
    e.Key("A-End");
    check(e.Cursor().line == 99, "A-End (alternativa) = C-End");
}

void test_delete_keys() {
    Editor e;
    e.SetText("hello world");
    e.SetCursor({0, 0});
    e.Key("A-w");
    check(e.Text() == "world", "A-w apaga a palavra a direita");
    e.Key("C-u");
    check(e.Text() == "hello world" && e.Cursor() == Pos{0, 0}, "C-u desfaz a remocao");
    e.SetCursor({0, 11});
    e.Key("C-w");
    check(e.Text() == "hello ", "C-w apaga a palavra a esquerda");
    e.SetText("hello world");
    e.SetCursor({0, 5});
    e.Key("C-l");
    check(e.Text() == " world" && e.Cursor().col == 0, "C-l apaga ate' o inicio da linha");
    e.SetText("hello world");
    e.SetCursor({0, 5});
    e.Key("A-l");
    check(e.Text() == "hello", "A-l apaga ate' o fim da linha");

    e.SetText("a\nb\nc");
    e.SetCursor({1, 0});
    e.Key("A-k");
    check(e.Text() == "a\nc" && e.Cursor() == Pos{1, 0}, "A-k apaga a linha");
    e.SetText("a\nb");
    e.SetCursor({1, 0});
    e.Key("A-k");
    check(e.Text() == "a" && e.Cursor() == Pos{0, 0}, "A-k na ultima linha");
    e.SetText("zzz");
    e.Key("A-k");
    check(e.Text().empty(), "A-k numa linha so' a esvazia");

    e.SetText("ab\ncd");
    e.SetCursor({1, 0});
    e.Key("BS");
    check(e.Text() == "abcd" && e.Cursor() == Pos{0, 2}, "BS no inicio junta com a linha anterior");
    e.SetText("ab\ncd");
    e.SetCursor({0, 2});
    e.Key("Del");
    check(e.Text() == "abcd", "Del no fim junta com a proxima");

    e.SetText("abc");
    e.SetCursor({0, 3});
    e.Key("BS");
    e.Key("C-u");
    check(e.Text() == "abc" && e.Cursor().col == 3, "C-u depois de BS");
    e.Key("BS");
    e.Key("Left");
    e.Key("C-u");
    check(e.Text() == "ab", "C-u nao vale se o cursor se moveu");

    e.SetText("Hello");
    e.SetCursor({0, 3});
    e.Key("C-a");
    check(e.Text() == "hELlo" && e.Cursor().col == 3, "C-a inverte a caixa ate' o inicio da linha");
    e.Key("A-a");
    check(e.Text() == "hELLO", "A-a inverte a caixa ate' o fim da linha");
}

void test_modes_and_indent() {
    Editor e;
    e.SetText("abc");
    e.Key("Ins");
    e.Type("XY");
    check(e.Text() == "XYc" && !e.InsertMode(), "Over substitui os caracteres");
    e.Key("Ins");
    e.Type("1");
    check(e.Text() == "XY1c", "Insert volta a inserir");
    e.settings().over_mode = ed::OverMode::NeverOverstrike;
    e.Key("Ins");
    check(e.InsertMode(), "Never overstrike: Ins nao troca o modo");
    e.settings().over_mode = ed::OverMode::StartInsert;

    e.SetText("");
    e.Key("C-p");
    e.Key("C-a");
    check(e.Text() == std::string(1, '\x01'), "C-p insere a proxima tecla como controle");

    e.SetText("    code");
    e.SetCursor({0, 8});
    e.Key("C-Enter");
    check(e.Text() == "    code\n    " && e.Cursor() == Pos{1, 4}, "C-Enter abre linha com indentacao");
    e.Key("S-Tab");
    check(e.Text() == "    code\n" && e.Cursor() == Pos{1, 0}, "S-Tab remove 1 nivel de indentacao");
    e.SetText("\tx");
    e.SetCursor({0, 2});
    e.Key("C-BS");
    check(e.Text() == "x", "C-BS remove a tabulacao");

    e.SetText("  abc");
    e.SetCursor({0, 5});
    e.Key("Enter");
    check(e.Text() == "  abc\n", "Enter sem auto indent nao indenta");
    e.Key("A-t");
    e.Key("a");
    check(e.settings().auto_indent, "Alt-T A liga o auto indent");
    e.SetText("  abc");
    e.SetCursor({0, 5});
    e.Key("Enter");
    check(e.Text() == "  abc\n  " && e.Cursor() == Pos{1, 2}, "auto indent copia a indentacao");

    e.SetText("");
    e.Key("Tab");
    check(e.Text() == "\t", "Tab insere tabulacao");
    e.SetText("");
    e.settings().tab_key = ed::TabKey::InsertSpaces;
    e.Key("Tab");
    check(e.Text() == std::string(8, ' '), "Tab insere espacos ate' a parada");
    e.SetText("abcdefghijklmnop");
    e.settings().tab_key = ed::TabKey::MoveToStop;
    e.Key("Tab");
    check(e.Cursor().col == 8 && e.Text() == "abcdefghijklmnop", "Tab move para a parada sem inserir");
    e.settings().tab_key = ed::TabKey::InsertTab;

    e.SetText("");
    e.settings().word_wrap = true;
    e.settings().line_length = 10;
    e.Type("aaaa bbbb cccc");
    check(e.Text() == "aaaa bbbb\ncccc" && e.Cursor() == Pos{1, 4}, "word wrap quebra no ultimo espaco");
    e.settings().word_wrap = false;
}

void test_blocks() {
    Editor e;
    e.SetText("one\ntwo\nthree");
    e.SetCursor({0, 0});
    keys(e, {"A-b", "s"});
    e.SetCursor({1, 3});
    keys(e, {"A-b", "s"});
    check(e.HasBlock() && e.BlockText() == "one\ntwo", "Alt-B S duas vezes marca o bloco");
    keys(e, {"A-b", "r"});
    check(!e.HasBlock(), "Alt-B R remove os marcadores");
    e.SetCursor({1, 0});
    keys(e, {"A-b", "l"});
    check(e.BlockText() == "two\n", "Alt-B L marca a linha");
    e.SetCursor({1, 1});
    keys(e, {"A-b", "e"});
    check(e.BlockText() == "wo", "Alt-B E marca ate' o fim da linha");

    // copiar
    e.SetText("abc def");
    e.SetCursor({0, 0});
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 3});
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 7});
    keys(e, {"A-b", "c"});
    check(e.Text() == "abc defabc" && e.Cursor() == Pos{0, 10} && e.BlockText() == "abc", "Alt-B C copia o bloco para o cursor");
    // mover
    e.SetText("abc def");
    e.SetCursor({0, 0});
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 3});
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 7});
    keys(e, {"A-b", "m"});
    check(e.Text() == " defabc", "Alt-B M move o bloco");
    // apagar
    e.SetText("abc def");
    e.SetCursor({0, 0});
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 3});
    keys(e, {"A-b", "s"});
    keys(e, {"A-b", "d"});
    check(e.Text() == " def" && !e.HasBlock(), "Alt-B D apaga o bloco");
    e.SetText("abc def");
    e.SetCursor({0, 0});
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 4});
    keys(e, {"A-b", "s"});
    keys(e, {"A-b", "Del"});
    check(e.Text() == "def", "Alt-B Del apaga sem confirmar");
    // o cursor dentro do bloco nao copia
    e.SetText("abcdef");
    e.SetCursor({0, 0});
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 6});
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 3});
    keys(e, {"A-b", "c"});
    check(e.Text() == "abcdef", "copiar com o cursor dentro do bloco nao faz nada");
    // os marcadores acompanham as edicoes
    e.SetText("abcdef");
    e.SetCursor({0, 2});
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 4});
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 0});
    e.Type("XX");
    check(e.BlockText() == "cd", "os marcadores acompanham o texto inserido antes deles");
    // achar marcador
    e.SetCursor({0, 0});
    keys(e, {"A-b", "f"});
    check(e.Cursor() == Pos{0, 4}, "Alt-B F vai ao proximo marcador");
    // indent / unindent
    e.SetText("a\nb\nc");
    e.SetCursor({0, 0});
    keys(e, {"A-b", "s"});
    e.SetCursor({1, 1});
    keys(e, {"A-b", "s"});
    e.Indent(2);
    check(e.Text() == "  a\n  b\nc", "Block > Indent");
    e.Unindent(1);
    check(e.Text() == " a\n b\nc", "Block > Unindent");
    keys(e, {"A-b", "i"});
    check(e.TakeDialog() == Dialog::BlockIndent, "Alt-B I pede o dialogo");
}

void test_search() {
    Editor e;
    e.SetText("foo bar foo\nBAR foo");
    check(e.Find("foo", false, false) && e.Cursor() == Pos{0, 0}, "Find acha a ocorrencia no cursor");
    e.Key("A-c");
    check(e.Cursor() == Pos{0, 8}, "Alt-C continua adiante");
    e.Key("A-c");
    check(e.Cursor() == Pos{1, 4}, "Alt-C passa de linha");
    e.Key("A-c");
    check(e.Cursor() == Pos{1, 4} && e.Message() == "Text not found", "Alt-C sem mais ocorrencias avisa");
    e.Key("C-c");
    check(e.Cursor() == Pos{0, 8}, "Ctrl-C continua para tras");
    e.Key("C-c");
    check(e.Cursor() == Pos{0, 0}, "Ctrl-C de novo");
    e.SetCursor({0, 0});
    check(e.Find("BAR", true, false) && e.Cursor() == Pos{1, 0}, "Find respeita a caixa");
    e.SetCursor({0, 0});
    check(e.Find("bar", false, false) && e.Cursor() == Pos{0, 4}, "Find ignora a caixa");
    e.SetCursor({1, 7});
    check(e.Find("foo", false, true) && e.Cursor() == Pos{1, 4}, "Find para tras");
    e.Key("C-f");
    check(e.TakeDialog() == Dialog::Find, "Ctrl-F pede o dialogo Find");
    e.Key("C-r");
    check(e.TakeDialog() == Dialog::Replace, "Ctrl-R pede o dialogo Replace");

    e.SetText("a-a-a\na");
    e.SetCursor({0, 0});
    check(e.ReplaceAll("a", "bb", true) == 4 && e.Text() == "bb-bb-bb\nbb", "ReplaceAll troca tudo");
    e.Key("A-BS");
    check(e.Text() == "a-a-a\na", "um desfazer reverte o ReplaceAll inteiro");
    e.SetCursor({0, 0});
    check(e.ReplaceNext("a", "zz", true, false) && e.Text() == "zz-a-a\na" && e.Cursor() == Pos{0, 2}, "ReplaceNext troca uma");

    e.SetText(many_lines(30));
    check(e.GoToLine(12) && e.Cursor() == Pos{11, 0}, "GoToLine");
    check(!e.GoToLine(99) && e.Cursor().line == 29, "GoToLine fora do arquivo vai ao fim");
    e.Key("A-s");
    e.Key("l");
    check(e.TakeDialog() == Dialog::LineNumber, "Alt-S L pede o dialogo");

    e.SetText("f(a, (b))\n");
    e.SetCursor({0, 1});
    e.Key("C-]");
    check(e.Cursor() == Pos{0, 8}, "Ctrl-] acha o fecha-parentese");
    e.Key("C-]");
    check(e.Cursor() == Pos{0, 1}, "Ctrl-] volta ao abre-parentese");
    e.SetText("{\n x\n}");
    e.SetCursor({0, 0});
    e.Key("C-]");
    check(e.Cursor() == Pos{2, 0}, "Ctrl-] atravessa linhas");

    e.SetText("ab foo cd foo");
    e.SetCursor({0, 3});
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 6});
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 6});
    keys(e, {"A-s", "b"});
    check(e.Cursor() == Pos{0, 10}, "Alt-S B acha outra copia do texto do bloco");
}

void test_undo_and_selection() {
    Editor e;
    e.Type("ab");
    e.Type("cd");
    e.Key("Enter");
    e.Type("e");
    e.Key("A-BS");
    check(e.Text() == "abcd\n" && e.Cursor() == Pos{1, 0}, "desfazer: o 'e'");
    e.Key("A-BS");
    check(e.Text() == "abcd", "desfazer: o Enter");
    e.Key("A-BS");
    check(e.Text().empty(), "desfazer: a digitacao seguida e' um passo so'");
    e.Key("A-S-BS");
    check(e.Text() == "abcd", "refazer");
    e.Key("A-S-BS");
    check(e.Text() == "abcd\n", "refazer de novo");

    e.SetText("hello");
    e.SetCursor({0, 0});
    keys(e, {"S-Right", "S-Right", "S-Right"});
    check(e.HasBlock() && e.BlockText() == "hel", "Shift+setas marcam um bloco");
    e.Key("Right");
    check(!e.HasBlock(), "um movimento comum desfaz a selecao de Shift");
    e.SetCursor({0, 3});
    keys(e, {"S-Left", "S-Left"});
    check(e.BlockText() == "el", "Shift+Left seleciona para tras");
    e.Key("C-Ins");
    check(e.Clipboard() == "el", "Ctrl-Ins copia");
    e.Key("S-Del");
    check(e.Text() == "hlo", "Shift-Del recorta");
    e.SetCursor({0, 3});
    e.Key("S-Ins");
    check(e.Text() == "hloel", "Shift-Ins cola");
    e.SetText("one two");
    e.SetCursor({0, 0});
    e.Key("C-S-Right");
    check(e.BlockText() == "one ", "Ctrl-Shift-Right seleciona a palavra");
}

void test_menus_and_dialogs() {
    Editor e;
    keys(e, {"A-f"});
    check(e.MenuPrefix() == 'f', "Alt-F abre o menu File");
    e.Key("x");
    check(e.ExitRequested(), "Alt-F X sai quando nada mudou");
    Editor m;
    m.Type("a");
    keys(m, {"A-f", "x"});
    check(!m.ExitRequested() && m.TakeDialog() == Dialog::ConfirmExit, "Alt-F X pergunta se ha' alteracao");
    keys(m, {"A-f", "o"});
    check(m.TakeDialog() == Dialog::Open, "Alt-F O pede Open");
    keys(m, {"A-f", "a"});
    check(m.TakeDialog() == Dialog::SaveAs, "Alt-F A pede Save As");
    keys(m, {"A-f", "s"});
    check(m.TakeDialog() == Dialog::SaveAs, "Alt-F S sem nome pede Save As");
    keys(m, {"A-o", "e"});
    check(m.TakeDialog() == Dialog::EditorSetup, "Alt-O E");
    keys(m, {"A-v", "i"});
    check(m.TakeDialog() == Dialog::Information, "Alt-V I");
    keys(m, {"A-v", "a"});
    check(m.TakeDialog() == Dialog::About, "Alt-V A (escondido no menu)");
    keys(m, {"A-t", "w"});
    check(m.settings().word_wrap, "Alt-T W liga o word wrap");
    keys(m, {"A-f", "Esc", "x"});
    check(m.MenuPrefix() == 0 && m.Text() == "ax", "Esc cancela o prefixo Alt");
    m.Key("F1");
    check(m.TakeDialog() == Dialog::Help && m.HelpTopic() == 0, "F1 abre a ajuda");
    m.Key("F10");
    check(m.TakeMenuRequest() && !m.TakeMenuRequest(), "F10 pede a barra de menus");
    m.Key("A-p");
    check(m.MenuPrefix() == 'p', "Alt-P abre Print");
    check(!m.Key("p") && m.MenuPrefix() == 0, "Print ainda nao existe: o comando recusa");
    check(!ed::Info(Cmd::Print).implemented && ed::Info(Cmd::Find).implemented, "Info: o que ja' existe");
}

void test_read_only() {
    Editor e;
    e.SetText("abc def");
    e.SetReadOnly(true);
    e.Type("x");
    const char *destructive[] = {"BS", "Del", "C-w", "A-w", "C-l", "A-l", "A-k", "C-a", "A-a", "Enter", "C-Enter", "Tab", "S-Tab", "C-BS",
                                 "C-u", "A-BS", "A-S-BS", "S-Ins", "S-Del", "C-p"};
    for (const char *k : destructive) e.Key(k);
    keys(e, {"A-t", "w"});
    keys(e, {"A-t", "a"});
    keys(e, {"A-f", "n"});
    e.Type("y");
    check(e.Text() == "abc def" && !e.Modified(), "somente leitura: teclas destrutivas nao fazem nada");
    check(!e.settings().word_wrap && !e.settings().auto_indent, "somente leitura: nem os liga/desliga do Text-layout");
    e.Key("End");
    check(e.Cursor().col == 7, "somente leitura: a navegacao funciona");
    e.SetCursor({0, 0});
    check(e.Find("def", false, false) && e.Cursor().col == 4, "somente leitura: a busca funciona");
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 7});
    keys(e, {"A-b", "s"});
    e.Key("C-Ins");
    check(e.Clipboard() == "def", "somente leitura: copiar para a area de transferencia");
    keys(e, {"A-b", "d"});
    check(e.Text() == "abc def", "somente leitura: Block Delete nao faz nada");
    std::string merge_err;
    check(e.ReplaceAll("a", "b", true) == 0 && !e.Merge("x", merge_err), "somente leitura: Replace e Merge recusam");
    auto enabled = [&](const char *label) {
        for (const ed::Menu &m : ed::Menus()) {
            for (const ed::MenuItem &it : m.items) {
                if (it.label && std::string(it.label) == label) return e.ItemEnabled(it);
            }
        }
        return false;
    };
    check(!enabled("Save") && !enabled("Merge...") && !enabled("Find and Replace...") && !enabled("Copy") && !enabled("Center line") &&
              !enabled("Word wrap") && !enabled("Auto indent"),
          "somente leitura: itens destrutivos ficam desabilitados");
    check(enabled("Find string...") && enabled("Save As...") && enabled("Information display") && enabled("Block Marker") &&
              enabled("Continue fwd") && enabled("Line number...") && enabled("Open...") && enabled("Output to file...") &&
              enabled("Display..."),
          "somente leitura: itens nao destrutivos ficam habilitados");
    e.SetReadOnly(false);
    check(enabled("Save") && enabled("Merge..."), "fora da leitura os itens voltam");
    e.SetReadOnly(true);
    ed::Grid g;
    e.Render(g);
    check(row(g, 24).find("View") == 58, "somente leitura: a barra de estado mostra View");
}

void test_files() {
    const fs::path dir = fs::temp_directory_path() / "fwmsx_editor_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    std::string err;

    Editor e;
    check(!e.Load((dir / "nada.txt").string(), err) && !err.empty(), "Load de arquivo inexistente falha");

    spit(dir / "a.txt", std::string("line1\r\nline2\x1A"));
    check(e.Load((dir / "a.txt").string(), err), "Load do arquivo CRLF com Ctrl-Z");
    check(e.Text() == "line1\nline2" && e.Crlf() && e.Eof1A() && !e.Modified(), "CRLF e Ctrl-Z reconhecidos");
    e.Type("X");
    check(e.Save(err) && slurp(dir / "a.txt") == std::string("Xline1\r\nline2\x1A"), "Save preserva CRLF e Ctrl-Z");
    check(slurp(dir / "a.{xt") == std::string("line1\r\nline2\x1A"), "Save guarda o backup com o caractere {");
    check(!e.Modified(), "Save limpa o flag de modificado");

    spit(dir / "b.txt", "a\nb\n");
    check(e.Load((dir / "b.txt").string(), err) && !e.Crlf() && e.LineCount() == 3, "arquivo LF: 2 linhas + final vazio");
    e.Type("z");
    e.settings().delete_backup = true;
    check(e.Save(err) && slurp(dir / "b.txt") == "za\nb\n" && !fs::exists(dir / "b.{xt"), "Save de LF e Delete backup files");

    spit(dir / "c.txt", std::string("\xEF\xBB\xBF") + "bom\r\n");
    check(e.Load((dir / "c.txt").string(), err) && e.Text() == "bom\n", "BOM UTF-8 sai do texto");
    check(e.Save(err) && slurp(dir / "c.txt") == std::string("\xEF\xBB\xBF") + "bom\r\n", "BOM e' preservado ao gravar");

    e.NewFile();
    e.Type("novo");
    check(e.Name().empty(), "NewFile: sem nome");
    check(e.SaveAs((dir / "d.txt").string(), err) && slurp(dir / "d.txt") == "novo" && e.Name() == (dir / "d.txt").string(),
          "SaveAs de arquivo novo (sem Ctrl-Z)");
    check(!e.SaveAs((dir / "nada" / "x.txt").string(), err) && e.Name() == (dir / "d.txt").string(), "SaveAs que falha mantem o nome");

    e.SetText("ab");
    e.SetCursor({0, 1});
    spit(dir / "m.txt", "X\nY");
    check(e.Merge((dir / "m.txt").string(), err) && e.Text() == "aX\nYb", "Merge insere no cursor");

    e.SetText("um\ndois\ntres");
    e.SetCursor({0, 0});
    keys(e, {"A-b", "s"});
    e.SetCursor({1, 4});
    keys(e, {"A-b", "s"});
    check(e.WriteBlock((dir / "blk.txt").string(), err) && slurp(dir / "blk.txt") == "um\r\ndois", "Output to file grava o bloco");

    Editor n;
    n.Type("x");
    n.Key("A-f");
    n.Key("s");
    check(n.TakeDialog() == Dialog::SaveAs, "Save sem nome abre Save As");

    fs::remove_all(dir);
}

void test_text_layout() {
    Editor e;
    e.SetText("\tx");
    e.ApplyExpandTabs(true);
    check(e.Text() == std::string(8, ' ') + "x", "Expand tabs");
    e.ApplyPutInTabs(true);
    check(e.Text() == "\tx", "Put in tabs");
    e.SetText(std::string(10, ' ') + "x");
    e.ApplyPutInTabs(true);
    check(e.Text() == "\t  x", "Put in tabs com resto");
    e.Key("A-BS");
    check(e.Text() == std::string(10, ' ') + "x", "Put in tabs desfaz num passo");

    e.SetText("1\n2\n3\n4\n5");
    e.settings().page_length = 2;
    e.ApplyInsertPageBreaks(true);
    check(e.Text() == "1\n2\n\f\n3\n4\n\f\n5", "Insert page breaks");
    e.ApplyRemovePageBreaks(true);
    check(e.Text() == "1\n2\n3\n4\n5", "Remove page breaks");

    e.SetText("  titulo   ");
    e.settings().line_length = 20;
    e.settings().left_margin = 0;
    e.Execute(Cmd::CenterLine);
    check(e.Text() == std::string(7, ' ') + "titulo", "Center line");

    e.SetText("a bb\n\nccc");
    const ed::DocInfo i = e.Information();
    check(i.lines == 3 && i.words == 3 && i.chars == 11 && i.char_at_cursor == 'a', "Information: linhas, palavras (Fortran) e caracteres");
}

void test_render() {
    Editor e;
    e.settings().line_numbers = false; // estes testes conferem as colunas do NE 2 (sem margem); a margem tem teste proprio
    e.SetSize(80, 25);
    ed::Grid g;
    e.Render(g);
    check(g.cols == 80 && g.rows == 25, "Render: grade 80x25");
    const std::string top = row(g, 0);
    // View e Options andaram uma coluna para a esquerda (o NE 2 tinha 44 e 50) para caber o menu Project antes do F1=Help
    check(top.substr(3, 4) == "File" && top.substr(9, 5) == "Block" && top.substr(16, 11) == "Text-layout" && top.substr(29, 6) == "Search" &&
              top.substr(37, 5) == "Print" && top.substr(43, 4) == "View" && top.substr(48, 7) == "Options" && top.substr(56, 5) == "Build" && top.substr(62, 7) == "Project" &&
              top.substr(70, 7) == "F1=Help",
          "Render: barra de menu nas colunas do NE 2 (View/Options uma coluna a esquerda) com Build e Project");
    const std::string st = row(g, 24);
    check(st.substr(4, 6) == "Line=1" && st.substr(15, 5) == "Col=1" && st.substr(36, 8) == "Untitled" && st.substr(58, 6) == "Insert" &&
              st.substr(66, 6) == "WW=Off",
          "Render: barra de estado nas colunas do NE 2");
    check(g.at(0, 24).ch == 0x2190 && g.at(79, 24).ch == 0x2192, "Render: setas da barra de estado");
    check(g.at(79, 1).ch == 0x2191 && g.at(79, 23).ch == 0x2193 && g.at(79, 5).ch == 0x2591, "Render: barra de rolagem");
    check(g.at(2, 24).ch == ' ', "Render: sem asterisco quando nao ha' alteracao");
    e.Type("x");
    e.Render(g);
    check(g.at(2, 24).ch == '*', "Render: asterisco depois de editar");

    e.SetText("hi\there");
    e.SetCursor({0, 0});
    e.Render(g);
    check(g.at(0, 1).ch == 'h' && g.at(1, 1).ch == 'i' && g.at(2, 1).ch == ' ' && g.at(8, 1).ch == 'h', "Render: tabulacao expandida");
    keys(e, {"A-b", "s"});
    e.SetCursor({0, 2});
    keys(e, {"A-b", "s"});
    e.Render(g);
    check(g.at(0, 1).style == ed::kBlockMark && g.at(1, 1).style == ed::kBlockSel && g.at(8, 1).style == ed::kText,
          "Render: bloco e marcador");
    int x = -1, y = -1;
    e.SetCursor({0, 0});
    check(e.CursorScreen(x, y) && x == 0 && y == 1, "CursorScreen");
    e.Key("Ins");
    e.settings().auto_indent = true;
    e.settings().word_wrap = true;
    e.Render(g);
    const std::string st2 = row(g, 24);
    check(st2.substr(58, 4) == "Over" && st2.substr(66, 5) == "WW=On" && st2.substr(73, 4) == "Auto", "Render: Over, WW=On e Auto");

    e.SetText(many_lines(200));
    e.GoToLine(100);
    e.Render(g);
    check(e.CursorScreen(x, y) && y >= 1 && y <= 23, "o cursor fica visivel depois de GoToLine");
    check(row(g, y).substr(0, 7) == "line100", "Render: a linha do cursor esta' na tela");
    e.SetSize(40, 10);
    e.Render(g);
    check(g.cols == 40 && g.rows == 10 && e.TextRows() == 8, "SetSize: outro tamanho de tela");
}

} // namespace

void test_gutter() {
    Editor e;
    e.SetSize(80, 25);
    e.SetText("hello\nworld\n\ttab");
    check(e.settings().line_numbers && e.Gutter() == 4, "numeros de linha: ligados por padrao, 3 digitos + respiro");
    ed::Grid g;
    e.Render(g);
    check(g.at(2, 1).ch == '1' && g.at(2, 2).ch == '2' && g.at(4, 1).ch == 'h' && g.at(8, 1).ch == 'o', "numeros de linha: o texto comeca depois da margem");
    check(g.at(2, 2).style == ed::kSynCommentSt && g.at(2, 1).style == ed::kText, "numeros de linha: cor discreta, e a linha do cursor em destaque");
    int x = -1, y = -1;
    e.SetCursor({0, 2});
    check(e.CursorScreen(x, y) && x == 6 && y == 1, "numeros de linha: o cursor anda com a margem");
    check(e.ScreenToPos(4, 0) == Pos{0, 0} && e.ScreenToPos(6, 1) == Pos{1, 2} && e.ScreenToPos(1, 0) == Pos{0, 0}, "numeros de linha: o mouse desconta a margem");
    e.SetText(many_lines(1200));
    check(e.Gutter() == 5, "numeros de linha: a margem cresce com os digitos (1000+ linhas)");
    e.settings().line_numbers = false;
    check(e.Gutter() == 0, "numeros de linha: desligados nao ocupam coluna");
    e.settings().line_numbers = true;
    e.SetGridGutter(false);
    check(e.Gutter() == 0, "numeros de linha: a janela grafica desenha a margem dela");
    e.SetGridGutter(true);
    // a rolagem horizontal conta a margem: o fim de uma linha longa continua visivel
    e.SetText(std::string(200, 'x'));
    e.SetCursor({0, 200});
    e.Render(g);
    check(e.CursorScreen(x, y) && x < 79, "numeros de linha: o cursor no fim de uma linha longa continua visivel");
}

void test_completion() {
    Editor e;
    e.settings().line_numbers = false;
    e.SetSize(80, 25);
    ed::SetUserSnippets("");
    e.SetLanguage(ed::LangById("msxbasic"), true);

    // funcoes auxiliares C (prefixo) e Fortran (nota)
    check(ed_word_start("ab cd", 5, 5) == 3 && ed_word_start("ab cd", 5, 2) == 0 && ed_word_start("(x", 2, 2) == 1, "ed_word_start (C)");
    check(ed_match_score("pri", 3, "print", 5) == 3 && ed_match_score("PRI", 3, "print", 5) == 2 && ed_match_score("print", 5, "print", 5) == 0 &&
              ed_match_score("PRINT", 5, "print", 5) == 2 && ed_match_score("x", 1, "abc", 3) == 0 && ed_match_score("abcd", 4, "abc", 3) == 0,
          "ed_match_score (Fortran)");

    // um candidato so': completa direto
    e.SetText("10 PRIN");
    e.SetCursor({0, 7});
    e.Execute(Cmd::Complete);
    check(e.Text() == "10 PRINT" && !e.CompletionOpen(), "Alt-/ com um candidato so' completa direto");
    keys(e, {"A-BS"});
    check(e.Text() == "10 PRIN", "desfazer volta o prefixo");

    // varios: abre a lista; escolher e aceitar
    e.SetText("10 LE");
    e.SetCursor({0, 5});
    keys(e, {"A-/"});
    check(e.CompletionOpen() && e.CompletionItems().size() >= 3, "Alt-/ abre a lista com varios candidatos");
    int len_at = -1;
    for (size_t i = 0; i < e.CompletionItems().size(); ++i) {
        if (e.CompletionItems()[i].text == "LEN") len_at = static_cast<int>(i);
    }
    check(len_at >= 0, "a lista traz LEN (palavra do MSX-BASIC)");
    e.CompletionSelect(len_at);
    keys(e, {"Enter"});
    check(e.Text() == "10 LEN" && !e.CompletionOpen(), "Enter aceita o escolhido");

    // Cima/Baixo, Esc
    e.SetText("10 LE");
    e.SetCursor({0, 5});
    keys(e, {"A-/", "Down"});
    check(e.CompletionOpen() && e.CompletionSel() == 1, "Baixo desce na lista");
    keys(e, {"Up", "Up"});
    check(e.CompletionSel() == static_cast<int>(e.CompletionItems().size()) - 1, "Cima na primeira volta ao fim");
    keys(e, {"Esc"});
    check(!e.CompletionOpen() && e.Text() == "10 LE", "Esc fecha sem mexer no texto");

    // digitar filtra; BS volta a abrir mais
    keys(e, {"A-/"});
    const size_t before = e.CompletionItems().size();
    e.Type("F");
    check(e.CompletionOpen() && e.CompletionItems().size() == 1 && e.CompletionItems()[0].text == "LEFT$", "digitar continua filtrando a lista");
    keys(e, {"BS"});
    check(e.CompletionOpen() && e.CompletionItems().size() == before, "apagar alarga a lista de novo");
    e.Type(" ");
    check(!e.CompletionOpen(), "uma tecla que nao e' de palavra fecha a lista");
    keys(e, {"A-/"});
    check(!e.CompletionOpen(), "sem prefixo nao ha' o que completar");

    // outra tecla fecha e age normalmente
    e.SetText("10 LE");
    e.SetCursor({0, 5});
    keys(e, {"A-/", "Left"});
    check(!e.CompletionOpen() && e.Cursor() == Pos{0, 4}, "Esquerda fecha a lista e move o cursor");

    // palavras do proprio texto (linguagem de texto puro)
    e.SetLanguage(ed::LangById("plain"), true);
    e.SetText("alphabet alpha\nalp");
    e.SetCursor({1, 3});
    keys(e, {"A-/"});
    check(e.CompletionOpen() && e.CompletionItems().size() == 2 && e.CompletionItems()[0].text == "alpha" && e.CompletionItems()[1].text == "alphabet",
          "palavras do texto, a mais curta primeiro");

    // o cursor no meio de uma palavra: ela mesma nao e' candidata
    e.SetText("se10 alpha");
    e.SetCursor({0, 2});
    e.Execute(Cmd::Complete);
    bool self = false;
    for (const ed::Candidate &c : e.CompletionItems()) self = self || c.text == "se10";
    check(!self, "a palavra sob o cursor nao aparece na propria lista");
    e.CompletionClose();

    // BIOS no assembly: a caixa certa (maiuscula)
    e.SetLanguage(ed::LangById("z80asm"), true);
    e.SetText("  call chp");
    e.SetCursor({0, 10});
    e.Execute(Cmd::Complete); // dois candidatos: o snippet "chput" e a rotina da BIOS
    int bios_at = -1;
    for (size_t i = 0; i < e.CompletionItems().size(); ++i) {
        if (e.CompletionItems()[i].text == "CHPUT" && !e.CompletionItems()[i].snippet) bios_at = static_cast<int>(i);
    }
    check(e.CompletionOpen() && bios_at >= 0 && e.CompletionItems()[0].snippet, "BIOS: CHPUT em maiusculas, depois do snippet de mesmo nome");
    e.CompletionSelect(bios_at);
    keys(e, {"Tab"});
    check(e.Text() == "  call CHPUT", "BIOS: completa em maiusculas mesmo digitada em minusculas");

    // modo leitura
    e.SetLanguage(ed::LangById("msxbasic"), true);
    e.SetText("10 PRIN");
    e.SetCursor({0, 7});
    e.SetReadOnly(true);
    keys(e, {"A-/"});
    check(e.Text() == "10 PRIN" && !e.CompletionOpen(), "modo leitura: completar nao faz nada");
    e.SetReadOnly(false);

    // completar sozinho (Editor setup)
    e.SetText("");
    e.settings().auto_complete = true;
    e.Type("L");
    check(!e.CompletionOpen(), "completar sozinho espera 2 letras");
    e.Type("E");
    check(e.CompletionOpen(), "completar sozinho abre com 2 letras");
    e.Type(" ");
    check(!e.CompletionOpen(), "e fecha com um espaco");
    e.settings().auto_complete = false;
    e.SetText("");
    e.Type("LE");
    check(!e.CompletionOpen(), "desligado: digitar nao abre a lista");

    // menu e tecla
    bool found = false;
    for (const ed::Menu &m : ed::Menus()) {
        for (const ed::MenuItem &it : m.items) found = found || it.cmd == Cmd::Complete;
    }
    check(found, "Search > Complete word existe no menu");
}

void test_snippets() {
    Editor e;
    e.settings().line_numbers = false;
    e.SetSize(80, 25);
    ed::SetUserSnippets("");
    e.SetLanguage(ed::LangById("msxbasic"), true);

    e.SetText("for");
    e.SetCursor({0, 3});
    keys(e, {"Tab"});
    check(e.Text() == "FOR = TO ::NEXT" && e.Cursor() == Pos{0, 4} && e.SnippetActive(), "snippet: for + Tab expande e vai ao primeiro ponto");
    e.Type("I");
    keys(e, {"Tab"});
    check(e.Cursor() == Pos{0, 6}, "Tab pula para o ponto seguinte, acompanhando o que foi digitado");
    e.Type("1");
    keys(e, {"Tab"});
    e.Type("9");
    keys(e, {"Tab"});
    check(e.Text() == "FOR I=1 TO 9::NEXT" && e.Cursor() == Pos{0, 13} && !e.SnippetActive(), "o ultimo Tab leva ao $0 e encerra o snippet");
    keys(e, {"Tab"});
    check(e.Text() == "FOR I=1 TO 9:\t:NEXT", "sem snippet, Tab e' tabulacao de novo");

    // em desfazer, um passo
    e.SetText("for");
    e.SetCursor({0, 3});
    keys(e, {"Tab", "A-BS"});
    check(e.Text() == "for" && !e.SnippetActive(), "desfazer o snippet de uma vez");

    // varias linhas com a identacao da linha do gatilho (Pascal)
    e.SetLanguage(ed::LangById("pascal"), true);
    e.SetText("  for");
    e.SetCursor({0, 5});
    keys(e, {"Tab"});
    check(e.LineCount() == 4 && e.Line(0) == "  for  :=  to  do" && e.Line(1) == "  begin" && e.Line(3) == "  end;" && e.Cursor() == Pos{0, 6},
          "snippet de varias linhas mantem a identacao");
    keys(e, {"Tab", "Tab", "Tab"});
    check(e.Cursor() == Pos{2, 4}, "o $0 fica dentro do bloco");

    // no meio de uma palavra, Tab nao expande
    e.SetText("format");
    e.SetCursor({0, 3});
    keys(e, {"Tab"});
    check(e.Text() == "for\tmat", "no meio de uma palavra nao expande");

    // arquivo do usuario
    ed::SetUserSnippets("# meus\n[msxbasic]\nhello=PRINT \"$1\"$0\nfor=FOR $1 TO\n[all]\nsig=-- fwMSX\\nfim\n");
    e.SetLanguage(ed::LangById("msxbasic"), true);
    e.SetText("hello");
    e.SetCursor({0, 5});
    keys(e, {"Tab"});
    check(e.Text() == "PRINT \"\"" && e.Cursor() == Pos{0, 7}, "snippet do usuario (secao da linguagem)");
    e.SetText("for");
    e.SetCursor({0, 3});
    keys(e, {"Tab"});
    check(e.Text() == "FOR  TO", "o snippet do usuario ganha do de fabrica");
    e.SetText("sig");
    e.SetCursor({0, 3});
    keys(e, {"Tab"});
    check(e.Text() == "-- fwMSX\nfim", "secao [all] e \\n no texto");
    ed::SetUserSnippets("");

    // a lista de completar traz o gatilho; aceitar expande
    e.SetText("for");
    e.SetCursor({0, 3});
    keys(e, {"A-/"});
    check(e.CompletionOpen() && e.CompletionItems().size() >= 2 && e.CompletionItems()[0].snippet && e.CompletionItems()[0].text == "for",
          "o gatilho do snippet aparece na lista, antes das palavras");
    keys(e, {"Enter"});
    check(e.Text() == "FOR = TO ::NEXT" && e.SnippetActive(), "aceitar o gatilho da lista expande o snippet");

    // listas
    check(ed::SnippetsFor(ed::LangById("z80asm")).size() >= 5 && ed::SnippetsFor(ed::LangById("plain")).empty(), "snippets de fabrica por linguagem");
    const ed::Expansion ex = ed::ExpandSnippet("a$2b\n$1c$0", "  ");
    check(ex.text == "ab\n  c" && ex.stops.size() == 3 && ex.stops[0] == std::make_pair(1, 2) && ex.stops[1] == std::make_pair(0, 1) && ex.stops[2] == std::make_pair(1, 3),
          "ExpandSnippet: ordem dos pontos e identacao");
    std::vector<std::string> w;
    ed::LangById("z80asm")->Words(w);
    check(std::find(w.begin(), w.end(), "CHPUT") != w.end() && std::find(w.begin(), w.end(), "ldir") != w.end(), "Lang::Words: BIOS em maiusculas e mnemonicos em minusculas");
    w.clear();
    ed::LangById("kunbasic")->Words(w);
    check(std::find(w.begin(), w.end(), "PRINT") != w.end() && std::find(w.begin(), w.end(), "CHR$") != w.end(), "Lang::Words: BASIC em maiusculas, com o $");
}

int main() {
    test_kernels();
    test_bindings();
    test_typing_and_movement();
    test_delete_keys();
    test_modes_and_indent();
    test_blocks();
    test_search();
    test_undo_and_selection();
    test_menus_and_dialogs();
    test_read_only();
    test_files();
    test_text_layout();
    test_render();
    test_gutter();
    test_completion();
    test_snippets();
    std::printf("\n%s (%d falha%s)\n", g_failures == 0 ? "TODOS OS TESTES PASSARAM" : "HA' FALHAS", g_failures, g_failures == 1 ? "" : "s");
    return g_failures == 0 ? 0 : 1;
}
