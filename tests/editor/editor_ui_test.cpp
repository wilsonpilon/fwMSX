// Teste da aplicacao do editor (src/editor/cpp/editor_ui): menus abertos, caixas de dialogo, duas janelas, mouse,
// configuracao e a traducao das teclas do terminal. Sem terminal: as teclas entram por nome e o teste le' a grade.
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "../../src/editor/c/ed_syntax.h"
#include "../../src/editor/cpp/editor_gui_input.h"
#include "../../src/editor/cpp/editor_i18n.h"
#include "../../src/editor/cpp/editor_keys.h"
#include "../../src/editor/cpp/editor_ui.h"

namespace {

namespace fs = std::filesystem;
using ed::EditorApp;
using ed::Pos;

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

void K(EditorApp &a, std::initializer_list<const char *> ks) {
    for (const char *k : ks) a.Key(k);
}

std::string row(const ed::Grid &g, int y) {
    std::string s;
    for (int x = 0; x < g.cols; ++x) {
        const unsigned c = g.at(x, y).ch;
        s.push_back(c < 256 ? static_cast<char>(c) : '#');
    }
    return s;
}

bool row_has(const ed::Grid &g, int y, const std::string &needle) {
    return row(g, y).find(needle) != std::string::npos;
}

bool screen_has(EditorApp &a, const std::string &needle) {
    ed::Grid g;
    a.Render(g);
    for (int y = 0; y < g.rows; ++y) {
        if (row(g, y).find(needle) != std::string::npos) return true;
    }
    return false;
}

std::string slurp(const fs::path &p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

void spit(const fs::path &p, const std::string &d) {
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    f.write(d.data(), static_cast<std::streamsize>(d.size()));
}

std::string &err_unused() {
    static std::string e;
    return e;
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

void test_key_names() {
    using ed::KeyNameFromInput;
    check(KeyNameFromInput("\x17") == "C-w", "Ctrl-W");
    check(KeyNameFromInput(std::string("\x1b") + "x") == "A-x", "Alt-X (ESC + x)");
    check(KeyNameFromInput("\x1b[1;5D") == "C-Left", "Ctrl-Esquerda");
    check(KeyNameFromInput("\x1b[1;2C") == "S-Right", "Shift-Direita");
    check(KeyNameFromInput("\x1b[1;6C") == "C-S-Right", "Ctrl-Shift-Direita");
    check(KeyNameFromInput("\x1b[3~") == "Del" && KeyNameFromInput("\x1b[2~") == "Ins", "Del e Ins");
    check(KeyNameFromInput("\x1b[5;5~") == "C-PgUp" && KeyNameFromInput("\x1b[6~") == "PgDn", "PgUp/PgDn com Ctrl");
    check(KeyNameFromInput("\x1b[Z") == "S-Tab" && KeyNameFromInput("\t") == "Tab", "Tab e Shift-Tab");
    check(KeyNameFromInput("\x7f") == "BS" && KeyNameFromInput("\x08") == "C-BS", "Backspace e Ctrl-Backspace");
    check(KeyNameFromInput(std::string(1, '\0')) == "C-2" && KeyNameFromInput("\x1d") == "C-]", "Ctrl-2 e Ctrl-]");
    check(KeyNameFromInput("\x1bOP") == "F1" && KeyNameFromInput("\x1b[21~") == "F10" && KeyNameFromInput("\x1bOQ") == "F2", "teclas F");
    check(KeyNameFromInput("\x1b[A") == "Up" && KeyNameFromInput("\x1b[H") == "Home" && KeyNameFromInput("\x1b[F") == "End", "setas, Home, End");
    check(KeyNameFromInput("\r") == "Enter" && KeyNameFromInput("\n") == "Enter" && KeyNameFromInput("\x1b") == "Esc", "Enter e Esc");
    check(KeyNameFromInput("\x1b\x7f") == "A-BS" && KeyNameFromInput("\x1b\r") == "A-Enter", "Alt-Backspace e Alt-Enter");
    check(KeyNameFromInput("a") == "a" && ed::IsTextKey("a") && !ed::IsTextKey("C-w") && !ed::IsTextKey("Left"), "texto x tecla");
    check(ed::IsTextKey("\xc3\xa9") && ed::Utf8ToBytes("\xc3\xa9") == "\xe9" && ed::Utf8ToBytes("\xe2\x82\xac").empty(), "UTF-8 -> Latin-1");

    // ponta a ponta: o que o terminal manda aciona o comando do NE
    EditorApp a;
    a.Ed().SetText("hello world");
    a.Ed().SetCursor({0, 11});
    a.Key(ed::KeyNameFromInput("\x17"));
    check(a.Ed().Text() == "hello ", "o byte 0x17 do terminal apaga a palavra (Ctrl-W)");
}

void test_menus() {
    EditorApp a;
    a.SetSize(80, 25);
    a.Key("A-f");
    check(a.MenuIndex() == 0 && a.BarActive(), "Alt-F abre o menu File");
    check(screen_has(a, "New") && screen_has(a, "Save As...") && screen_has(a, "2nd window...") && screen_has(a, "F2") && screen_has(a, "eXit"),
          "o menu File desenha os itens e o atalho F2");
    ed::Grid g;
    a.Render(g);
    check(g.at(3, 0).style == ed::kBarSel, "o titulo do menu aberto fica em destaque");
    a.Key("Right");
    check(a.MenuIndex() == 1, "Seta direita troca para o menu Block");
    a.Key("Left");
    a.Key("Down");
    a.Key("Down");
    check(a.MenuItemIndex() == 2, "Setas descem pelos itens");
    a.Key("Up");
    a.Key("Enter");
    check(a.HasDialog() && a.Current()->kind == ed::Dialog::Open, "Enter no item Open... abre o dialogo");
    a.Key("Esc");
    check(!a.HasDialog(), "Esc fecha o dialogo");
    a.Key("F10");
    check(a.MenuIndex() == 0, "F10 abre a barra de menus");
    a.Key("Esc");
    check(a.MenuIndex() < 0 && !a.BarActive(), "Esc fecha o menu");
    a.Key("A-f");
    a.Key("x");
    check(a.QuitRequested(), "Alt-F, X sai quando nada mudou");

    // modo leitura: itens destrutivos desabilitados e pulados
    EditorApp v;
    v.SetSize(80, 25);
    v.Ed().SetText("abc");
    v.Ed().SetReadOnly(true);
    v.Key("A-b");
    check(v.MenuItemIndex() == 3, "view: o menu Block abre no 1o item habilitado (Set marker)");
    v.Render(g);
    bool off = false;
    for (int y = 2; y < 8; ++y) {
        if (row(g, y).find("Copy") != std::string::npos) off = g.at(row(g, y).find("Copy") + 1, y).style == ed::kPanelOff;
    }
    check(off, "view: Copy aparece como desabilitado");
    v.Key("Down");
    v.Key("Down");
    check(v.MenuItemIndex() >= 3, "view: as setas nao param em item desabilitado");
    v.Key("Esc");
    v.Key("A-t");
    v.Key("w");
    check(!v.Ed().settings().word_wrap, "view: Alt-T W e' ignorado");

    EditorApp h;
    h.SetSize(80, 25);
    h.Key("F1");
    check(h.MenuIndex() == static_cast<int>(ed::Menus().size()) - 1 && screen_has(h, "General") && screen_has(h, "About"), "F1 abre o menu F1=Help");
    h.Key("k");
    check(h.HasDialog() && h.Current()->kind == ed::Dialog::Help && h.Current()->help_topic == 1, "Help > Keys");
    check(screen_has(h, "Help on keys") && screen_has(h, "Moves the cursor left") && screen_has(h, "More"), "a ajuda de teclas (pagina 1)");
    h.Key("n");
    check(h.Current()->page == 1 && screen_has(h, "Ctrl-W") && screen_has(h, "Alt-K"), "Next: a pagina 2 lista Ctrl-W e Alt-K");
    h.Key("p");
    check(h.Current()->page == 0, "Previous volta");
    h.Key("Esc");
    h.Key("F1");
    h.Key("f");
    check(screen_has(h, "Alt-F N") && screen_has(h, "File menu help"), "Help > File mostra os mnemonicos Alt-F N");
}

void test_dialogs() {
    const fs::path dir = fs::temp_directory_path() / "fwmsx_editor_ui_test";
    fs::remove_all(dir);
    fs::create_directories(dir / "sub");
    spit(dir / "a.txt", "alpha");
    spit(dir / "b.txt", "beta\r\nline2");
    spit(dir / "c.dat", "data");

    EditorApp a;
    a.SetSize(80, 25);
    a.SetStartDir(dir.string());

    // --- Find ---
    a.Ed().SetText("Foo foo");
    K(a, {"C-f"});
    check(a.HasDialog() && a.Current()->kind == ed::Dialog::Find, "Ctrl-F abre Search for text");
    check(screen_has(a, "Search for text") && screen_has(a, "Upper/lower case must match") && screen_has(a, "Search backwards") && screen_has(a, "F1=Help"),
          "o dialogo Find tem os campos do NE");
    a.Text("foo");
    K(a, {"Tab", " ", "Enter"});
    check(!a.HasDialog() && a.Ed().Cursor() == Pos{0, 4}, "Find com 'Upper/lower case must match' acha a segunda");
    a.Ed().SetCursor({0, 0});
    K(a, {"C-f", "Esc"});
    check(!a.HasDialog() && a.Ed().Cursor() == Pos{0, 0}, "Esc cancela o Find");

    // --- Replace interativo ---
    a.Ed().SetText("a a a");
    a.Ed().SetCursor({0, 0});
    K(a, {"C-r"});
    a.Text("a");
    K(a, {"Tab"});
    a.Text("b");
    K(a, {"Enter"});
    check(a.HasDialog() && screen_has(a, "Replace this occurrence?"), "Replace pergunta a cada ocorrencia");
    a.Key("y");
    check(a.Ed().Text() == "b a a" && a.HasDialog(), "Yes troca e vai para a proxima");
    a.Key("n");
    check(a.Ed().Text() == "b a a" && a.HasDialog(), "No pula");
    a.Key("a");
    check(a.Ed().Text() == "b a b" && !a.HasDialog(), "All troca o resto");

    // --- ir para a linha (campo numerico) ---
    a.Ed().SetText(many_lines(30));
    K(a, {"A-s", "l"});
    check(a.HasDialog() && a.Current()->kind == ed::Dialog::LineNumber, "Alt-S L abre Find line");
    a.Text("x5");
    K(a, {"Enter"});
    check(a.Ed().Cursor().line == 4, "o campo de linha so' aceita digitos");

    // --- Open ---
    a.Ed().SetText("");
    K(a, {"A-f", "o"});
    check(a.HasDialog() && a.Current()->kind == ed::Dialog::Open && screen_has(a, "Open file") && screen_has(a, "File name:") && screen_has(a, "Dirs:"),
          "Alt-F O abre o dialogo de arquivo");
    check(screen_has(a, "a.txt") && screen_has(a, "c.dat") && screen_has(a, "[sub]") && screen_has(a, ".."), "lista os arquivos e os diretorios");
    ed::Grid g;
    a.Render(g);
    check(g.at(9, 3).ch == 0x2554 && g.at(9 + 62, 4).style == ed::kShadow, "caixa de borda dupla com sombra");
    a.Text("*.txt");
    K(a, {"Enter"});
    check(a.HasDialog() && screen_has(a, "b.txt") && !screen_has(a, "c.dat"), "uma mascara no campo filtra a lista");
    K(a, {"Tab", "Down", "Enter"});
    check(!a.HasDialog() && fs::path(a.Ed().Name()).filename() == "b.txt" && a.Ed().Text() == "beta\nline2", "escolher na lista abre o arquivo");

    // --- diretorio ---
    K(a, {"A-f", "o", "Tab", "Tab"});
    K(a, {"Down", "Enter"});
    check(a.HasDialog() && a.Current()->dir.find("sub") != std::string::npos, "escolher [sub] entra no diretorio");
    K(a, {"Esc"});

    // --- Save As ---
    a.Ed().NewFile();
    a.Ed().Type("novo");
    K(a, {"A-f", "a"});
    check(a.HasDialog() && a.Current()->kind == ed::Dialog::SaveAs, "Alt-F A abre Save file as");
    a.Text("out.txt");
    K(a, {"Enter"});
    check(!a.HasDialog() && slurp(dir / "out.txt") == "novo", "Save As grava o arquivo no diretorio mostrado");

    // --- sair com alteracoes ---
    a.Ed().Type("x");
    K(a, {"A-f", "x"});
    check(a.HasDialog() && screen_has(a, "Save it before exiting?") && !a.QuitRequested(), "sair com alteracao pergunta");
    K(a, {"Esc"});
    check(!a.HasDialog() && !a.QuitRequested(), "Cancel volta ao editor");
    K(a, {"A-f", "x", "d"});
    check(a.QuitRequested(), "Don't Save sai");
    EditorApp s;
    s.SetSize(80, 25);
    s.SetStartDir(dir.string());
    s.Ed().Type("conteudo");
    K(s, {"A-f", "x", "s"});
    check(s.HasDialog() && s.Current()->kind == ed::Dialog::SaveAs, "Save sem nome na saida abre Save As");
    s.Text("saida.txt");
    K(s, {"Enter"});
    check(s.QuitRequested() && slurp(dir / "saida.txt") == "conteudo", "depois do Save As a saida continua");

    // --- information, about ---
    EditorApp i;
    i.SetSize(80, 25);
    i.Ed().SetText("um dois\ntres");
    K(i, {"A-v", "i"});
    check(i.HasDialog() && screen_has(i, "Words in edit buffer:") && screen_has(i, "Lines in edit buffer:") && screen_has(i, "Character at cursor:"),
          "Information display");
    K(i, {"Enter"});
    K(i, {"A-v", "a"});
    check(i.HasDialog() && i.Current()->kind == ed::Dialog::About, "Alt-V A: About");

    fs::remove_all(dir);
}

void test_setup_and_config() {
    const fs::path dir = fs::temp_directory_path() / "fwmsx_editor_cfg_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    EditorApp a;
    a.SetSize(80, 25);
    a.SetConfigPaths((dir / "main.cfg").string(), (dir / "local.cfg").string());

    // Display: cor 2 e 33/35 linhas; tab 4
    K(a, {"A-o", "d"});
    check(a.HasDialog() && screen_has(a, "Display setup") && screen_has(a, "VGA Monochrome") && screen_has(a, "Blinking bracket"), "Options > Display");
    K(a, {"Down", "Down"}); // LCD -> Color 1 -> Color 2
    K(a, {"Tab", "Up", "Up", "Enter"});  // linhas: Don't change -> 50 -> 40/43
    check(!a.HasDialog() && a.ui().scheme == 2, "Display: escolher Color 2 muda o esquema");
    check(a.ui().lines == 35 || a.ui().lines == 43 || a.ui().lines == 0 || a.ui().lines == 50 || a.ui().lines == 25, "Display: linhas validas");

    // Editor setup: Tab key = inserts spaces
    K(a, {"A-o", "e", "Tab", "Down", "Enter"});
    check(a.Ed().settings().tab_key == ed::TabKey::InsertSpaces, "Editor setup: Tab key inserts spaces");
    // Text-layout setup
    K(a, {"A-t", "s"});
    a.Text("40");
    K(a, {"Enter"});
    check(a.Ed().settings().line_length == 40, "Text-layout setup: Line Length");
    // salvar e recarregar
    K(a, {"A-o", "s", "Enter"});
    check(fs::exists(dir / "main.cfg") && slurp(dir / "main.cfg").find("scheme=2") != std::string::npos, "Save configuration grava o arquivo");
    K(a, {"A-o", "s", "Down", "Enter"});
    check(fs::exists(dir / "local.cfg"), "Save configuration para o diretorio");
    EditorApp b;
    b.SetConfigPaths((dir / "main.cfg").string(), "");
    b.LoadConfig();
    check(b.ui().scheme == 2 && b.Ed().settings().line_length == 40 && b.Ed().settings().tab_key == ed::TabKey::InsertSpaces, "LoadConfig restaura tudo");

    // configuracao alterada e nao salva: a saida pergunta
    EditorApp c;
    c.SetSize(80, 25);
    c.SetConfigPaths((dir / "c.cfg").string(), "");
    K(c, {"A-o", "m", "Down", "Enter"});
    check(c.ui().mouse_button == 1, "Mouse setup: botao direito");
    K(c, {"A-f", "x"});
    check(c.HasDialog() && screen_has(c, "Save this configuration?"), "sair com a configuracao alterada pergunta");
    K(c, {"Enter"});
    check(c.QuitRequested() && fs::exists(dir / "c.cfg"), "Save grava a configuracao e sai");

    // quebras de pagina
    EditorApp p;
    p.SetSize(80, 25);
    p.Ed().SetText("1\n2\n3\n4\n5");
    p.Ed().settings().page_length = 2;
    K(p, {"A-t", "i"});
    check(p.HasDialog() && screen_has(p, "Insert page breaks starting at page..."), "Insert page breaks pergunta o ponto de partida");
    K(p, {"t"});
    check(p.Ed().Text() == "1\n2\n\f\n3\n4\n\f\n5", "Top insere desde o topo");
    K(p, {"A-t", "b", "t"});
    check(p.Ed().Text() == "1\n2\n3\n4\n5", "Remove page breaks");
    p.Ed().SetText("\tx");
    K(p, {"A-t", "e", "t"});
    check(p.Ed().Text() == std::string(8, ' ') + "x", "Expand tabs");

    // bloco: Indent pede a quantidade
    p.Ed().SetText("a\nb");
    p.Ed().SetCursor({0, 0});
    K(p, {"A-b", "s"});
    p.Ed().SetCursor({1, 1});
    K(p, {"A-b", "s", "A-b", "i"});
    check(p.HasDialog() && screen_has(p, "Amount:"), "Block > Indent pergunta a quantidade");
    a.Text("");
    p.Text("2");
    K(p, {"Enter"});
    check(p.Ed().Text() == "  a\n  b", "Indent de 2 colunas");
    fs::remove_all(dir);
}

void test_windows() {
    const fs::path dir = fs::temp_directory_path() / "fwmsx_editor_win_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    spit(dir / "one.txt", "abc\ndef\nghi");
    spit(dir / "two.txt", "abc\ndeX\nghi");

    EditorApp a;
    a.SetSize(80, 25);
    std::string err;
    check(a.Open((dir / "one.txt").string(), err) && a.WindowCount() == 1, "Open abre o arquivo");
    check(a.Open2((dir / "two.txt").string(), err) && a.WindowCount() == 2 && a.Active() == 1, "Open2 abre a segunda janela");
    ed::Grid g;
    a.Render(g);
    const int bar = 1 + a.DividerRows();
    check(row(g, bar).find("one.txt") != std::string::npos && g.at(0, bar).style == ed::kTitleBar, "a barra do meio mostra o nome da janela de cima");
    check(row(g, 24).find("two.txt") != std::string::npos, "a barra de estado mostra a janela ativa");
    check(row(g, 1).find("abc") != std::string::npos && row(g, bar + 1).find("abc") != std::string::npos, "as duas janelas mostram o texto");
    a.Key("F2");
    check(a.Active() == 0, "F2 passa para a outra janela");
    a.Key("F2");
    check(a.Active() == 1, "F2 de volta");
    const int d0 = a.DividerRows();
    a.Key("A-Down");
    check(a.DividerRows() == d0 + 1, "o divisor desce (Alt-Down: alternativa de Alt-2)");
    a.Key("A-2");
    check(a.DividerRows() == d0 + 2, "Alt-2 desce o divisor");
    a.Key("C-2");
    a.Key("C-2");
    check(a.DividerRows() == d0, "Ctrl-2 sobe o divisor");

    a.Win(0).SetCursor({0, 0});
    a.Win(1).SetCursor({0, 0});
    K(a, {"A-s", "d"});
    check(a.Win(0).Cursor() == Pos{1, 2} && a.Win(1).Cursor() == Pos{1, 2}, "Window Differences leva os dois cursores a diferenca");
    K(a, {"A-s", "d"});
    check(a.Win(0).Cursor() == Pos{1, 2}, "sem mais diferencas nada se move");

    K(a, {"A-f", "c"});
    check(a.WindowCount() == 1 && a.Active() == 0, "Close window fecha a segunda");
    K(a, {"A-f", "c"});
    check(a.WindowCount() == 1, "Close window com uma so' nao faz nada");
    K(a, {"A-f", "2"});
    check(a.HasDialog() && a.Current()->kind == ed::Dialog::Window2, "2nd window... abre o dialogo de arquivo");
    K(a, {"Esc"});

    // sair com as duas janelas alteradas: pergunta por cada uma
    EditorApp b;
    b.SetSize(80, 25);
    b.Open((dir / "one.txt").string(), err);
    b.Open2((dir / "two.txt").string(), err);
    b.Win(0).Type("1");
    b.Win(1).Type("2");
    K(b, {"A-f", "x"});
    check(b.HasDialog() && b.Active() == 0, "a saida comeca pela primeira janela alterada");
    K(b, {"d"});
    check(b.HasDialog() && b.Active() == 1 && !b.QuitRequested(), "e depois pergunta pela segunda");
    K(b, {"d"});
    check(b.QuitRequested(), "Don't Save nas duas sai");
    fs::remove_all(dir);
}

void test_mouse() {
    EditorApp a;
    a.Ed().settings().line_numbers = false; // as coordenadas do mouse destes testes partem da coluna 0
    a.SetSize(80, 25);
    a.Ed().SetText("hello world\nsecond line\nthird");
    a.MouseDown(3, 2);
    check(a.Ed().Cursor() == Pos{1, 3}, "clique posiciona o cursor");
    a.MouseDown(2, 1);
    a.MouseDrag(8, 2);
    a.MouseUp(8, 2);
    check(a.Ed().HasBlock() && a.Ed().BlockText() == "llo world\nsecond l", "arrastar marca um bloco");
    a.MouseDown(5, 3);
    check(!a.Ed().HasBlock() && a.Ed().Cursor() == Pos{2, 5}, "novo clique remove o bloco");

    a.Ed().SetText(many_lines(100));
    a.Wheel(10, 5, 1);
    check(a.Ed().TopLine() == 3, "a roda do mouse rola o texto");
    a.MouseDown(79, 23); // seta para baixo da barra de rolagem
    check(a.Ed().TopLine() == 4, "a seta da barra de rolagem rola uma linha");
    a.MouseDown(79, 20);
    check(a.Ed().TopLine() > 4, "o trilho rola uma pagina");

    // menu pelo mouse
    EditorApp m;
    m.SetSize(80, 25);
    m.MouseDown(3, 0);
    check(m.MenuIndex() == 0, "clicar no titulo abre o menu");
    m.MouseDown(5, 3); // 2o item: Open...
    check(m.HasDialog() && m.Current()->kind == ed::Dialog::Open, "clicar no item o executa");
    const ed::DialogBox *d = m.Current();
    int bx = 0, by = 0;
    for (const ed::Widget &w : d->ws) {
        if (w.type == ed::Widget::Button && w.id == ed::kBtnCancel) {
            bx = (80 - d->w) / 2 + 1 + w.x + 1;
            by = (25 - d->h) / 2 + 1 + w.y;
        }
    }
    m.MouseDown(bx, by);
    check(!m.HasDialog(), "clicar no botao Cancel fecha o dialogo");
    m.MouseDown(3, 0);
    check(m.MenuIndex() == 0, "o menu reabre");
    m.MouseDown(40, 10);
    check(m.MenuIndex() < 0, "clicar fora fecha o menu");
    m.MouseDown(72, 0);
    check(m.MenuIndex() == static_cast<int>(ed::Menus().size()) - 1, "clicar em F1=Help abre o menu de ajuda");
}

void test_gui_input() {
    using ed::GuiKey;
    using ed::GuiKeyName;
    // teclado "de verdade": as teclas do NE chegam todas, com os modificadores
    check(GuiKeyName(GuiKey::W, true, false, false) == "C-w" && GuiKeyName(GuiKey::W, false, true, false) == "A-w", "Ctrl-W e Alt-W");
    check(GuiKeyName(GuiKey::Enter, true, false, false) == "C-Enter" && GuiKeyName(GuiKey::Enter, false, false, false) == "Enter", "Ctrl-Enter distinto de Enter");
    check(GuiKeyName(GuiKey::PgUp, true, false, false) == "C-PgUp" && GuiKeyName(GuiKey::Home, true, false, false) == "C-Home", "Ctrl-PgUp e Ctrl-Home");
    check(GuiKeyName(GuiKey::D2, true, false, false) == "C-2" && GuiKeyName(GuiKey::D2, false, true, false) == "A-2", "Ctrl-2 e Alt-2");
    check(GuiKeyName(GuiKey::RBracket, true, false, false) == "C-]", "Ctrl-]");
    check(GuiKeyName(GuiKey::Backspace, true, false, false) == "C-BS" && GuiKeyName(GuiKey::Tab, false, false, true) == "S-Tab", "Ctrl-Backspace e Shift-Tab");
    check(GuiKeyName(GuiKey::Left, true, false, true) == "C-S-Left" && GuiKeyName(GuiKey::F10, false, false, false) == "F10", "Ctrl-Shift-Esquerda e F10");
    check(GuiKeyName(GuiKey::A, false, false, false).empty() && GuiKeyName(GuiKey::A, false, false, true).empty() && GuiKeyName(GuiKey::D5, false, false, false).empty() &&
              GuiKeyName(GuiKey::Space, false, false, false).empty(),
          "letra, digito e espaco sem Ctrl/Alt sao texto (vem da fila de caracteres)");
    check(GuiKeyName(GuiKey::Del, false, false, false) == "Del" && GuiKeyName(GuiKey::Escape, false, false, false) == "Esc", "Del e Esc");
    check(GuiKeyName(GuiKey::None, true, true, true).empty(), "tecla desconhecida");
    // todo nome devolvido existe como tecla do modelo ou e' uma tecla de dialogo
    ed::Editor e;
    e.SetText("hello world");
    e.SetCursor({0, 11});
    e.Key(GuiKeyName(GuiKey::W, true, false, false));
    check(e.Text() == "hello ", "o nome de Ctrl-W aciona o comando no modelo");
    // paleta
    bool all = true;
    for (int scheme = 0; scheme < 5; ++scheme) {
        for (int st = 0; st < ed::kStyleCount; ++st) {
            const ed::PalRgb p = ed::PaletteRgb(scheme, static_cast<uint8_t>(st));
            if (p.fg.r == p.bg.r && p.fg.g == p.bg.g && p.fg.b == p.bg.b && st != ed::kShadow) all = false; // texto invisivel
        }
    }
    check(all, "paleta: todo estilo de todo esquema tem contraste (fundo != texto)");
    const ed::PalRgb lcd = ed::PaletteRgb(0, ed::kBar);
    check(lcd.bg.r == 170 && lcd.fg.r == 0, "LCD: barras pretas no cinza");
    const ed::PalRgb blue = ed::PaletteRgb(1, ed::kText);
    check(blue.bg.b == 170 && blue.fg.r == 255, "Color 1: texto branco no azul");
}

void test_gui_mode() {
    const fs::path dir = fs::temp_directory_path() / "fwmsx_editor_guimode_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    spit(dir / "one.txt", "abc\ndef\nghi");
    spit(dir / "two.txt", "abc\ndeX\nghi");
    spit(dir / "three.txt", "third");

    EditorApp a;
    a.SetGuiMode(true);
    a.SetSize(80, 25);
    a.SetStartDir(dir.string());
    std::string err;
    check(a.GuiMode() && a.WindowCount() == 1 && a.Ed().Name().empty(), "modo grafico: comeca com uma aba vazia");
    check(a.OpenTab((dir / "one.txt").string(), err) && a.WindowCount() == 1 && fs::path(a.Ed().Name()).filename() == "one.txt",
          "a primeira abertura entra na aba vazia (nao cria uma aba a mais)");
    check(a.OpenTab((dir / "two.txt").string(), err) && a.WindowCount() == 2 && a.Active() == 1, "abrir outro arquivo cria uma aba");
    check(a.OpenTab((dir / "one.txt").string(), err) && a.WindowCount() == 2 && a.Active() == 0, "abrir um arquivo ja' aberto so' ativa a aba");
    K(a, {"A-f", "n"});
    check(a.WindowCount() == 3 && a.Active() == 2 && a.Ed().Name().empty(), "Arquivo > Novo abre uma aba nova");
    a.Ed().Type("novo texto");

    // divisao em paineis
    a.SetActive(0);
    a.Key("F2");
    check(a.Split() && a.PaneDoc(0) == 0 && a.PaneDoc(1) == 1 && a.ActivePane() == 0, "F2 divide a area em 2 paineis (abas 0 e 1)");
    a.Key("F2");
    check(a.Split() && a.ActivePane() == 1 && a.Active() == 1, "F2 com a divisao passa para o outro painel");
    a.Win(0).SetCursor({0, 0});
    a.Win(1).SetCursor({0, 0});
    K(a, {"A-s", "d"});
    check(a.Win(0).Cursor() == Pos{1, 2} && a.Win(1).Cursor() == Pos{1, 2}, "Window Differences compara os dois paineis");
    a.SetActive(2);
    check(a.Split() && a.PaneDoc(a.ActivePane()) == 2 && a.PaneDoc(1 - a.ActivePane()) != 2, "ativar uma terceira aba troca o conteudo do painel ativo");
    K(a, {"A-f", "c"});
    check(!a.Split() && a.WindowCount() == 3, "Close window com a divisao: desfaz a divisao");

    // fechar abas
    a.CloseTab(1);
    check(a.WindowCount() == 2, "fechar uma aba sem alteracoes a remove");
    a.CloseTab(1); // a aba do 'novo texto' esta' alterada
    check(a.HasDialog() && screen_has(a, "Save it before closing?") && a.WindowCount() == 2, "fechar uma aba alterada pergunta");
    K(a, {"Esc"});
    check(!a.HasDialog() && a.WindowCount() == 2, "Cancel mantem a aba");
    a.CloseTab(1);
    K(a, {"d"});
    check(a.WindowCount() == 1 && fs::path(a.Ed().Name()).filename() == "one.txt", "Don't Save fecha a aba");
    a.CloseTab(0);
    check(a.WindowCount() == 1 && a.Ed().Name().empty(), "fechar a ultima aba deixa uma aba vazia");

    // Abrir pelo dialogo abre uma aba
    a.Ed().SetText("");
    K(a, {"A-f", "o"});
    a.Text((dir / "three.txt").string());
    K(a, {"Enter"});
    check(!a.HasDialog() && fs::path(a.Ed().Name()).filename() == "three.txt" && a.Ed().Text() == "third", "o dialogo Abrir carrega o arquivo");
    K(a, {"A-f", "2"});
    check(a.HasDialog() && a.Current()->kind == ed::Dialog::Window2, "2nd window sem outra aba pede um arquivo");
    a.Text((dir / "one.txt").string());
    K(a, {"Enter"});
    check(a.Split() && a.WindowCount() == 2 && fs::path(a.Win(a.PaneDoc(0)).Name()).filename() == "three.txt" && a.ActivePane() == 1, "o arquivo escolhido abre na divisao");

    // API de menu e dialogo para a interface grafica
    EditorApp m;
    m.SetGuiMode(true);
    m.OpenMenuAt(0);
    check(m.MenuIndex() == 0 && m.MenuItemEnabled(ed::Menus()[0].items[0]), "OpenMenuAt abre o menu");
    m.SetMenuItem(2);
    check(m.MenuItemIndex() == 2, "SetMenuItem");
    m.RunMenu(0, 1);
    check(m.MenuIndex() < 0 && m.HasDialog() && m.CurrentMutable()->kind == ed::Dialog::Open, "RunMenu executa o item e fecha o menu");
    ed::DialogBox *d = m.CurrentMutable();
    d->ws[0].text.clear();
    m.PressDialog(ed::kBtnCancel);
    check(!m.HasDialog(), "PressDialog(Cancel) fecha o dialogo");
    m.RunMenu(3, 0); // Busca > Find
    d = m.CurrentMutable();
    check(d && d->kind == ed::Dialog::Find, "Busca > Localizar abre o dialogo");
    d->Find(1)->text = "x";
    d->Find(10)->checked = true;
    m.DialogEnter();
    check(!m.HasDialog(), "DialogEnter confirma pelo botao padrao");

    // tabs nao mudam o comportamento do terminal
    EditorApp t;
    t.SetSize(80, 25);
    check(!t.GuiMode(), "a TUI continua sem modo grafico");
    fs::remove_all(dir);
}

void test_embedded_and_session() {
    const fs::path dir = fs::temp_directory_path() / "fwmsx_editor_session_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    spit(dir / "nota.txt", "linha um\nlinha dois");
    const std::string sess = (dir / "fwedit.session").string();

    // Sair no editor embutido so' esconde: nada de pergunta, o texto continua la' e o editor segue funcionando
    EditorApp a;
    a.SetGuiMode(true);
    a.SetEmbedded(true);
    a.SetSize(80, 25);
    a.SetStartDir(dir.string());
    a.Ed().Type("ideia 1");
    a.RequestExit();
    check(a.TakeHideRequest() && !a.TakeHideRequest() && !a.HasDialog() && !a.QuitRequested(), "embutido: Sair so' pede para esconder (sem pergunta, sem sair)");
    check(a.Ed().Text() == "ideia 1" && a.Ed().Modified(), "embutido: o texto nao salvo continua la'");
    // uma janela escondida com menu e dialogo abertos volta limpa e funcional
    K(a, {"A-f"});
    K(a, {"o"});
    check(a.HasDialog() && a.MenuIndex() < 0, "dialogo aberto antes de esconder");
    a.CancelDialog();
    a.OpenMenuAt(1);
    a.CloseMenuNow();
    a.ClearQuit();
    check(!a.HasDialog() && a.MenuIndex() < 0, "esconder cancela dialogo e menu");
    a.Text(" e mais");
    check(a.Ed().Text() == "ideia 1 e mais", "depois de voltar o editor aceita texto");
    K(a, {"A-s", "f"});
    check(a.HasDialog() && a.Current()->kind == ed::Dialog::Find, "depois de voltar os menus funcionam");
    K(a, {"Esc"});
    check(!a.HasDialog(), "e os dialogos fecham");

    // sessao: abas, texto nao salvo, cursor
    a.NewTab();
    a.Ed().Type("segunda anotacao");
    check(a.OpenTab((dir / "nota.txt").string(), err_unused(), false), "abre um arquivo em outra aba");
    a.Ed().SetCursor({1, 3});
    check(a.SaveSession(sess) && fs::exists(sess), "SaveSession grava a sessao");
    spit(dir / "nota.txt", "linha um\nlinha dois\nlinha tres"); // o arquivo mudou no disco entre as sessoes

    EditorApp b;
    b.SetGuiMode(true);
    b.SetSize(80, 25);
    check(b.LoadSession(sess), "LoadSession restaura");
    check(b.WindowCount() == 3, "tres abas restauradas");
    check(b.Win(0).Text() == "ideia 1 e mais" && b.Win(0).Modified() && b.Win(0).Name().empty(), "aba 1: texto nao salvo volta, ainda alterado");
    check(b.Win(1).Text() == "segunda anotacao" && b.Win(1).Modified(), "aba 2: idem");
    check(fs::path(b.Win(2).Name()).filename() == "nota.txt" && !b.Win(2).Modified() && b.Win(2).LineCount() == 3, "aba 3: arquivo sem alteracoes e' relido do disco");
    check(b.Active() == 2 && b.Ed().Cursor() == Pos{1, 3}, "a aba ativa e o cursor voltam");
    check(!b.LoadSession(sess), "LoadSession nao sobrescreve um editor que ja' tem conteudo");

    // sem nada util a guardar, a sessao e' apagada
    EditorApp c;
    c.SetGuiMode(true);
    check(c.SaveSession(sess) && !fs::exists(sess), "editor vazio: apaga o arquivo de sessao");
    fs::remove_all(dir);
}

void test_untitled_names() {
    const fs::path dir = fs::temp_directory_path() / "fwmsx_editor_untitled_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    EditorApp a;
    a.SetGuiMode(true);
    check(a.Win(0).Title() == "noname01", "a primeira aba sem nome e' noname01");
    a.NewTab();
    a.NewTab();
    check(a.Win(1).Title() == "noname02" && a.Win(2).Title() == "noname03", "as seguintes: noname02, noname03");
    a.CloseTab(1);
    a.NewTab();
    check(a.WindowCount() == 3 && a.Win(2).Title() == "noname02", "fechar uma aba libera o numero (o menor livre e' reaproveitado)");
    // arquivo com nome mostra o nome do arquivo; salvar um sem nome libera o numero
    a.SetActive(0);
    a.Ed().Type("x");
    std::string err;
    check(a.Ed().SaveAs((dir / "doc.txt").string(), err) && a.Ed().Title() == "doc.txt", "salvo com nome: o titulo e' o do arquivo");
    a.EnsureUntitled();
    a.NewTab();
    check(a.Win(3).Title() == "noname01", "o numero do que foi salvo (noname01) volta a ficar livre");
    // a sessao guarda os nomes automaticos
    EditorApp b;
    b.SetGuiMode(true);
    b.NewTab();
    b.NewTab();
    b.CloseTab(0);
    b.Win(0).Type("dois");
    b.Win(1).Type("tres");
    check(b.Win(0).Title() == "noname02" && b.Win(1).Title() == "noname03", "(antes da sessao) noname02 e noname03");
    const std::string sess = (dir / "s.session").string();
    check(b.SaveSession(sess), "SaveSession");
    EditorApp c;
    c.SetGuiMode(true);
    check(c.LoadSession(sess) && c.Win(0).Title() == "noname02" && c.Win(1).Title() == "noname03", "a sessao preserva noname02 e noname03");
    // sessao da 1.41.4 (sem a linha UNT) ainda abre
    spit(dir / "old.session", std::string("FWEDIT-SESSION 1\nACTIVE 0 1\nDOC\nNAME 0\n\nMOD 1 1 0 3\nTEXT 3\nabc\n"));
    EditorApp d;
    d.SetGuiMode(true);
    check(d.LoadSession((dir / "old.session").string()) && d.Win(0).Text() == "abc" && d.Win(0).Title() == "noname01", "sessao antiga (sem UNT) ainda abre e recebe um noname");
    EditorApp t; // a TUI nao e' afetada: continua "Untitled"
    check(t.Win(0).Title() == "Untitled", "sem o modo grafico o titulo e' Untitled (NE)");
    fs::remove_all(dir);
}

void test_autosave() {
    const fs::path dir = fs::temp_directory_path() / "fwmsx_editor_autosave_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    spit(dir / "doc.txt", "orig");
    const std::string sess = (dir / "fwedit.session").string();

    // as opcoes na caixa Editor setup
    EditorApp a;
    a.SetGuiMode(true);
    a.SetSize(80, 25);
    a.SetSessionPath(sess);
    check(!a.AutosaveTick(0) && !a.AutosaveTick(100000), "autosave desligado por padrao (0 minutos)");
    K(a, {"A-o", "e"});
    check(a.HasDialog() && screen_has(a, "Auto-save every (minutes, 0 = off):") && screen_has(a, "Also overwrite files that have a name"), "Editor setup tem as opcoes de autosave");
    ed::DialogBox *d = a.CurrentMutable();
    d->Find(12)->text = "2";
    d->Find(13)->checked = true;
    a.DialogEnter();
    check(!a.HasDialog() && a.Ed().settings().autosave_minutes == 2 && a.Ed().settings().autosave_files, "OK grava os minutos e a opcao de sobrescrever");

    // o relogio
    std::string err;
    check(a.OpenTab((dir / "doc.txt").string(), err), "abre o arquivo");
    a.Ed().Type("X");
    check(!a.AutosaveTick(1000), "o primeiro quadro so' arma o relogio");
    check(!a.AutosaveTick(1000 + 119), "antes dos 2 minutos nao salva");
    check(a.AutosaveTick(1000 + 121), "passados 2 minutos salva");
    check(slurp(dir / "doc.txt") == "Xorig" && !a.Ed().Modified(), "o arquivo com nome foi gravado");
    check(slurp(dir / "doc.{xt") == "orig", "a copia { guarda o ORIGINAL");
    check(fs::exists(sess), "a sessao de recuperacao foi gravada");
    check(a.Ed().Message() == "Auto-saved", "a barra de estado avisa");
    a.Ed().Type("Y");
    check(!a.AutosaveTick(1000 + 200) && a.AutosaveTick(1000 + 121 + 121), "o relogio recomeca depois de cada salvamento");
    check(slurp(dir / "doc.txt") == "XYorig" && slurp(dir / "doc.{xt") == "orig", "o segundo autosave NAO troca a copia { pela versao anterior");
    // um Save manual volta a fazer a copia
    a.Ed().Type("Z");
    std::string e2;
    check(a.Ed().Save(e2) && slurp(dir / "doc.{xt") == "XYorig", "Save manual refaz a copia { com a versao anterior");

    // so' a sessao (sem sobrescrever arquivos)
    EditorApp b;
    b.SetGuiMode(true);
    b.SetSessionPath((dir / "b.session").string());
    b.Ed().settings().autosave_minutes = 1;
    check(b.OpenTab((dir / "doc.txt").string(), err), "abre o arquivo na outra instancia");
    b.Ed().Type("Q");
    b.NewTab();
    b.Ed().Type("anotacao solta");
    b.AutosaveTick(0);
    check(b.AutosaveTick(61) && fs::exists(dir / "b.session"), "so' a sessao: grava a sessao");
    check(slurp(dir / "doc.txt") == "XYZorig", "so' a sessao: o arquivo de verdade NAO e' sobrescrito");
    EditorApp r;
    r.SetGuiMode(true);
    check(r.LoadSession((dir / "b.session").string()) && r.WindowCount() == 2 && r.Win(1).Text() == "anotacao solta" && r.Win(0).Text() == "QXYZorig" && r.Win(0).Modified(),
          "a sessao guarda o texto alterado e as anotacoes");

    // desligar (0) para o relogio; arquivo somente leitura nao e' gravado
    b.Ed().settings().autosave_minutes = 0;
    for (int i = 0; i < b.WindowCount(); ++i) b.Win(i).settings().autosave_minutes = 0;
    check(!b.AutosaveTick(100000), "0 minutos desliga");
    b.DiscardSession();
    check(!fs::exists(dir / "b.session"), "DiscardSession apaga a sessao");

    // configuracao em arquivo
    ed::Settings s1;
    s1.autosave_minutes = 7;
    s1.autosave_files = true;
    ed::UiSettings u;
    std::string cerr;
    check(ed::SaveConfigFile((dir / "c.cfg").string(), s1, u, cerr), "grava a configuracao");
    ed::Settings s2;
    ed::UiSettings u2;
    check(ed::LoadConfigFile((dir / "c.cfg").string(), s2, u2) && s2.autosave_minutes == 7 && s2.autosave_files, "le' o autosave da configuracao");
    fs::remove_all(dir);
}

void test_syntax_in_editor() {
    const fs::path dir = fs::temp_directory_path() / "fwmsx_editor_syntax_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    spit(dir / "a.asm", "  ld a,1 ; x\n");
    spit(dir / "b.pas", "{ abre\nfecha }\nbegin\n");
    spit(dir / "c.bas", "FILE \"X.SC2\"\n10 PRINT 1\n");
    spit(dir / "d.txt", "print 1\n");
    ed::Editor e;
    std::string err;
    check(e.Load((dir / "a.asm").string(), err) && e.Language()->Id() == "z80asm" && !e.LanguageManual(), "Load escolhe a linguagem pela extensao (.asm)");
    check(e.Load((dir / "c.bas").string(), err) && e.Language()->Id() == "msxbas2rom", "Load de .bas olha o inicio do texto (MSXBAS2ROM)");
    check(e.Load((dir / "d.txt").string(), err) && e.Language()->Id() == "plain", "texto puro para .txt");
    check(e.Load((dir / "b.pas").string(), err) && e.Language()->Id() == "pascal", "Pascal para .pas");

    // trechos por linha e o estado de varias linhas
    std::vector<ed::Span> sp;
    e.HighlightLine(0, sp);
    check(sp.size() == 1 && sp[0].cls == ed::kSynComment, "linha 1 de { ... : comentario");
    e.HighlightLine(1, sp);
    check(sp.size() == 1 && sp[0].cls == ed::kSynComment && sp[0].to == 7, "linha 2 ('fecha }') ainda e' comentario por causa do estado");
    e.HighlightLine(2, sp);
    check(!sp.empty() && sp[0].cls == ed::kSynKeyword, "linha 3 (begin) volta a ser codigo");
    e.SetCursor({0, 0});
    e.Key("Del");
    e.Key("Del"); // apaga o '{' e o espaco: a linha 1 deixa de abrir comentario
    e.HighlightLine(1, sp);
    check(sp.empty() || sp[0].cls != ed::kSynComment || sp[0].from > 0, "editar a linha de cima invalida o estado das de baixo");

    // escolha manual
    e.SetLanguage(ed::LangById("bat"), true);
    check(e.Language()->Id() == "bat" && e.LanguageManual(), "SetLanguage manual");
    std::string e2;
    check(e.SaveAs((dir / "novo.asm").string(), e2) && e.Language()->Id() == "bat", "a escolha manual sobrevive ao Save As");
    e.SetLanguage(nullptr, false);
    e.DetectLanguage();
    check(e.Language()->Id() == "z80asm" && !e.LanguageManual(), "auto: volta a vir da extensao");
    e.NewFile();
    check(e.Language()->Id() == "plain", "arquivo novo: texto puro");

    // a grade da TUI usa o destaque
    ed::EditorApp a;
    a.Ed().settings().line_numbers = false;
    a.SetSize(80, 25);
    a.Ed().SetText("10 PRINT \"HI\" 'c");
    a.Ed().SetLanguage(ed::LangById("msxbasic"), true);
    ed::Grid g;
    a.Render(g);
    check(g.at(0, 1).style == ed::SynStyle(ed::kSynLineNo) && g.at(3, 1).style == ed::SynStyle(ed::kSynKeyword) && g.at(9, 1).style == ed::SynStyle(ed::kSynString) &&
              g.at(14, 1).style == ed::SynStyle(ed::kSynComment) && g.at(2, 1).style == ed::kText,
          "TUI: a grade pinta numero de linha, comando, string e comentario");
    a.Ed().settings().syntax = false;
    a.Render(g);
    check(g.at(3, 1).style == ed::kText, "TUI: com o destaque desligado volta tudo a texto");
    a.Ed().settings().syntax = true;
    K(a, {"A-b", "s"});
    a.Ed().SetCursor({0, 5});
    K(a, {"A-b", "s"});
    a.Render(g);
    check(g.at(3, 1).style == ed::kBlockSel, "TUI: o bloco marcado vence a cor do destaque");

    // dialogo Language (View > Language...)
    K(a, {"A-v", "l"});
    check(a.HasDialog() && a.Current()->kind == ed::Dialog::Language && screen_has(a, "(auto)") && screen_has(a, "Assembly Z80") && screen_has(a, "Basic Dignified"),
          "View > Language... lista as linguagens");
    check(a.Current()->Find(2)->sel == 2, "o dialogo abre na linguagem atual (MSX-BASIC e' o 3o item: depois de (auto) e Texto)");
    ed::DialogBox *d = a.CurrentMutable();
    d->Find(2)->sel = 7; // (auto), plain, msxbasic, dignified, msxbas2rom, nestorbasic, turbobasic, z80asm
    a.DialogEnter();
    check(!a.HasDialog() && a.Ed().Language()->Id() == "z80asm" && a.Ed().LanguageManual(), "escolher na lista troca a linguagem");
    K(a, {"A-v", "l"});
    a.CurrentMutable()->Find(2)->sel = 0;
    a.DialogEnter();
    check(!a.Ed().LanguageManual() && a.Ed().Language()->Id() == "plain", "(auto) volta a detectar (arquivo sem nome = texto puro)");

    // Display setup: a caixa Syntax highlighting
    K(a, {"A-o", "d"});
    check(a.HasDialog() && screen_has(a, "Syntax highlighting"), "Display setup tem 'Syntax highlighting'");
    a.CurrentMutable()->Find(15)->checked = false;
    a.DialogEnter();
    check(!a.Ed().settings().syntax, "desmarcar desliga o destaque");

    // configuracao em arquivo e sessao
    ed::Settings s1;
    s1.syntax = false;
    ed::UiSettings u;
    std::string cerr;
    check(ed::SaveConfigFile((dir / "c.cfg").string(), s1, u, cerr), "grava a configuracao");
    ed::Settings s2;
    ed::UiSettings u2;
    check(ed::LoadConfigFile((dir / "c.cfg").string(), s2, u2) && !s2.syntax, "a configuracao guarda o destaque");
    ed::EditorApp b;
    b.SetGuiMode(true);
    b.Ed().Type("x");
    b.Ed().SetLanguage(ed::LangById("pascal"), true);
    const std::string sess = (dir / "s.session").string();
    check(b.SaveSession(sess), "SaveSession com linguagem manual");
    ed::EditorApp c;
    c.SetGuiMode(true);
    check(c.LoadSession(sess) && c.Ed().Language()->Id() == "pascal" && c.Ed().LanguageManual(), "a sessao guarda a linguagem escolhida");
    fs::remove_all(dir);
}

void test_model_extras() {
    ed::Editor e;
    e.SetText("one two\nthree");
    const uint64_t r0 = e.Revision();
    e.SetCursor({0, 0});
    e.Type("x");
    check(e.Revision() > r0, "Revision muda a cada alteracao");
    check(e.PosAt(0, 100) == Pos{0, 8} && e.PosAt(99, 0) == Pos{1, 0} && e.PosAt(-3, 2) == Pos{0, 2}, "PosAt: coluna/linha de tela -> posicao do texto");
    e.SelectWord({0, 5});
    check(e.BlockText() == "two", "SelectWord (duplo clique)");
    e.SelectLine(0);
    check(e.BlockText() == "xone two\n", "SelectLine (triplo clique)");
    e.SelectLine(1);
    check(e.BlockText() == "three", "SelectLine na ultima linha");
    e.SetText(many_lines(50));
    e.SetTopLine(7);
    e.SetLeftCol(3);
    check(e.TopLine() == 7 && e.LeftCol() == 3, "SetTopLine/SetLeftCol (a rolagem e' do ImGui)");
    e.SetViewSize(80, 10, false);
    check(e.TopLine() == 7, "SetViewSize sem revelar nao mexe na rolagem");
    // as teclas alternativas dos editores graficos
    e.SetText("abc");
    e.SetCursor({0, 3});
    e.Key("C-v");
    check(e.Text() == "abc", "Ctrl-V com a area de transferencia vazia nao faz nada");
    e.SetClipboard("XY");
    e.Key("C-v");
    check(e.Text() == "abcXY", "Ctrl-V cola");
    e.SelectWord({0, 1});
    e.Key("C-x");
    check(e.Text().empty() && e.Clipboard() == "abcXY", "Ctrl-X recorta");
}

void test_translation() {
    using ed::Tr;
    check(Tr("File") == "Arquivo" && Tr("Search") == "Busca" && Tr("F1=Help") == "Ajuda", "Tr: titulos de menu");
    check(Tr("Don't Save") == "Nao salvar" && Tr("Cancel") == "Cancelar" && Tr("Yes") == "Sim", "Tr: botoes");
    check(Tr("Words in edit buffer:      12").rfind("Palavras no buffer:", 0) == 0 && Tr("Words in edit buffer:      12").find("12") != std::string::npos,
          "Tr: 'rotulo: valor' traduz so' o rotulo");
    check(Tr("never seen text") == "never seen text", "Tr: texto desconhecido volta como veio");
    // todo item de menu do NE tem texto em portugues (ou e' igual nas duas linguas)
    bool all = true;
    for (const ed::Menu &m : ed::Menus()) {
        for (const ed::MenuItem &it : m.items) {
            if (!it.label) continue;
            const std::string en = it.label;
            if (Tr(en) == en && en != "Mouse..." && en != "Editor..." && en != "Cursor" && en != "OK") {
                std::printf("    sem traducao: %s\n", en.c_str());
                all = false;
            }
        }
    }
    check(all, "Tr: todos os itens de menu tem traducao");
    bool accents = true;
    for (const ed::Menu &m : ed::Menus()) {
        for (const ed::MenuItem &it : m.items) {
            if (!it.label) continue;
            for (const unsigned char c : Tr(it.label)) {
                if (c > 127) accents = false;
            }
        }
    }
    check(accents, "Tr: sem acentos (convencao do projeto)");
}

void test_view_app() {
    const fs::path dir = fs::temp_directory_path() / "fwmsx_editor_view_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    spit(dir / "v.bas", "10 PRINT \"HI\"\r\n20 END");
    EditorApp a;
    a.SetSize(80, 25);
    std::string err;
    check(a.Open((dir / "v.bas").string(), err, true) && a.Ed().ReadOnly(), "Open em modo view");
    for (const char *k : {"BS", "Del", "Enter", "C-w", "A-k", "Tab"}) a.Key(k);
    a.Text("zzz");
    K(a, {"A-f", "s"});
    K(a, {"A-f", "n"});
    check(a.Ed().Text() == "10 PRINT \"HI\"\n20 END" && !a.Ed().Modified() && slurp(dir / "v.bas") == "10 PRINT \"HI\"\r\n20 END", "view: nada muda, nem o arquivo");
    check(screen_has(a, "View"), "a barra de estado mostra View");
    a.Ed().SetCursor({0, 0});
    K(a, {"C-f"});
    a.Text("END");
    K(a, {"Enter"});
    check(a.Ed().Cursor() == Pos{1, 3}, "view: a busca funciona");
    K(a, {"A-f", "a"});
    check(a.HasDialog() && a.Current()->kind == ed::Dialog::SaveAs, "view: Save As (copia) esta' habilitado");
    a.Text("copia.bas");
    K(a, {"Enter"});
    check(fs::exists(dir / "copia.bas"), "view: Save As grava a copia");
    K(a, {"C-r"});
    check(!a.HasDialog(), "view: Replace nao abre");
    K(a, {"A-f", "x"});
    check(a.QuitRequested(), "view: sair nao pergunta nada");
    fs::remove_all(dir);
}

} // namespace

void test_complete_and_numbers() {
    const fs::path dir = fs::temp_directory_path() / "fwedit_e5";
    fs::remove_all(dir);
    fs::create_directories(dir);

    // numeros de linha na grade (TUI): ligados por padrao; o dialogo Display setup liga e desliga
    EditorApp a;
    a.SetSize(80, 25);
    a.Ed().SetText("hello\nworld");
    ed::Grid g;
    a.Render(g);
    check(g.at(4, 1).ch == 'h' && g.at(2, 1).ch == '1', "TUI: a grade mostra os numeros de linha por padrao");
    K(a, {"A-o", "d"});
    check(a.HasDialog() && screen_has(a, "Line numbers"), "Display setup tem 'Line numbers'");
    check(a.Current()->Find(16)->checked, "a caixa abre marcada");
    a.CurrentMutable()->Find(16)->checked = false;
    a.DialogEnter();
    a.Render(g);
    check(!a.Ed().settings().line_numbers && g.at(0, 1).ch == 'h', "desmarcar tira a margem");
    ed::Settings s1;
    s1.line_numbers = false;
    s1.auto_complete = true;
    ed::UiSettings u;
    std::string cerr;
    check(ed::SaveConfigFile((dir / "c.cfg").string(), s1, u, cerr), "grava a configuracao");
    ed::Settings s2;
    ed::UiSettings u2;
    check(ed::LoadConfigFile((dir / "c.cfg").string(), s2, u2) && !s2.line_numbers && s2.auto_complete, "a configuracao guarda os numeros de linha e o completar sozinho");
    // o mouse conta a margem
    a.Ed().settings().line_numbers = true;
    a.Render(g);
    a.MouseDown(6, 1);
    check(a.Ed().Cursor() == Pos{0, 2}, "TUI: o clique desconta a margem de numeros");

    // completar pelo app: Alt-/ abre, a lista aparece na grade, Enter aceita
    EditorApp b;
    b.SetSize(80, 25);
    b.Ed().settings().line_numbers = false;
    b.Ed().SetLanguage(ed::LangById("msxbasic"), true);
    b.Ed().SetText("10 LE");
    b.Ed().SetCursor({0, 5});
    K(b, {"A-/"});
    check(b.Ed().CompletionOpen(), "Alt-/ no app abre a lista");
    b.Render(g);
    bool shown = false;
    for (int y = 2; y < 12; ++y) shown = shown || row_has(g, y, "LEFT$");
    check(shown, "a lista aparece na grade, abaixo do cursor");
    K(b, {"Enter"});
    check(!b.Ed().CompletionOpen() && b.Ed().Text() != "10 LE", "Enter aceita (o app repassa a tecla ao editor)");
    K(b, {"A-s", "w"});
    check(!b.Ed().CompletionOpen(), "o menu Search > Complete word abre a lista (ou completa direto)");

    // Editor setup: Completar sozinho
    K(b, {"A-o", "e"});
    check(b.HasDialog() && screen_has(b, "Auto completion"), "Editor setup tem 'Auto completion'");
    b.CurrentMutable()->Find(15)->checked = true;
    b.DialogEnter();
    check(b.Ed().settings().auto_complete, "marcar liga o completar sozinho");

    // snippets do usuario vem de fwedit.snippets, ao lado da configuracao
    spit(dir / "fwedit.snippets", "[msxbasic]\nhi=PRINT \"$1\"\n");
    EditorApp c;
    c.SetConfigPaths((dir / "fwedit.cfg").string(), "");
    c.LoadConfig();
    ed::SnippetDef d;
    check(ed::FindSnippet(ed::LangById("msxbasic"), "hi", d) && d.body == "PRINT \"$1\"", "LoadConfig le o fwedit.snippets");
    ed::SetUserSnippets("");
    fs::remove_all(dir);
}

bool menu_item_enabled(EditorApp &app, ed::Cmd cmd) {
    for (const ed::Menu &m : ed::Menus()) {
        for (const ed::MenuItem &it : m.items) {
            if (it.cmd == cmd) return app.MenuItemEnabled(it);
        }
    }
    return false;
}

void test_build() {
    // o parser de mensagens (C) e o ciclo (Fortran)
    int ne = 0, ln = 0, kd = 0;
    const std::string m1 = "jogo.asm, line 12: Undefined identifier";
    check(ed_build_msg(m1.data(), static_cast<int>(m1.size()), &ne, &ln, &kd) == 1 && ne == 8 && ln == 12 && kd == 1, "ed_build_msg: erro com arquivo e linha");
    const std::string m2 = "a, b.asm, line 3: Warning: algo";
    check(ed_build_msg(m2.data(), static_cast<int>(m2.size()), &ne, &ln, &kd) == 1 && ne == 8 && ln == 3 && kd == 2, "ed_build_msg: aviso; virgula no nome do arquivo");
    const std::string m3 = "Warning: only 2 out of 4 megaROM pages are defined";
    check(ed_build_msg(m3.data(), static_cast<int>(m3.size()), &ne, &ln, &kd) == 0, "ed_build_msg: mensagem geral nao aponta para linha");
    check(ed_cycle(3, -1, 1) == 0 && ed_cycle(3, -1, -1) == 2 && ed_cycle(3, 2, 1) == 0 && ed_cycle(3, 0, -1) == 2 && ed_cycle(0, 0, 1) == 0, "ed_cycle: circular");

    const fs::path dir = fs::temp_directory_path() / "fwedit_e6";
    fs::remove_all(dir);
    fs::create_directories(dir);

    // montar o buffer: bom e com erros
    EditorApp a;
    a.SetSize(80, 25);
    a.Ed().SetLanguage(ed::LangById("z80asm"), true);
    a.Ed().SetText(" org #4000\n ld a,1\n ret\n");
    K(a, {"F9"});
    check(a.Ed().Message().rfind("Assemble OK", 0) == 0 && a.BuildMessages().empty(), "F9: monta o buffer sem erros");
    // dois avisos (linhas 2 e 3) e um erro (linha 4): o fwasm para no primeiro erro
    a.Ed().SetText(" org #4000\n ld a,300\n ld b,-200\n ld a,undef_one\n ret\n");
    K(a, {"F9"});
    check(a.BuildMessages().size() == 3 && a.BuildMessages()[0].warning && a.BuildMessages()[0].line == 2 && !a.BuildMessages()[2].warning &&
              a.BuildMessages()[2].line == 4 && a.BuildMessages()[2].file == "noname.asm",
          "F9: avisos e erro viram mensagens com arquivo e linha");
    check(a.BuildCurrent() == 2 && a.Ed().Cursor().line == 3 && a.Ed().Message().find("1 error(s), 2 warning(s)") != std::string::npos,
          "o cursor vai para a linha do primeiro ERRO (pula os avisos)");
    K(a, {"F8"});
    check(a.BuildCurrent() == 0 && a.Ed().Cursor().line == 1, "F8: proxima mensagem (volta ao comeco da lista)");
    K(a, {"S-F8"});
    check(a.BuildCurrent() == 2 && a.Ed().Cursor().line == 3, "Shift-F8: mensagem anterior (circular)");
    K(a, {"A-u"});
    check(a.BarActive() && screen_has(a, "Run on MSX") && screen_has(a, "Messages..."), "Alt-U abre o menu Build");
    K(a, {"Esc"});
    K(a, {"A-u", "m"});
    check(a.HasDialog() && screen_has(a, "Build messages") && screen_has(a, "undefined identifier"), "Build > Messages... lista as mensagens");
    K(a, {"Up", "Enter"});
    check(!a.HasDialog() && a.BuildCurrent() == 1 && a.Ed().Cursor().line == 2, "Enter na lista vai para a linha escolhida");

    // outra linguagem: avisa
    EditorApp b;
    b.SetSize(80, 25);
    b.Ed().SetLanguage(ed::LangById("msxbasic"), true);
    b.Ed().SetText("10 PRINT LEFT$(A$,1)");
    K(b, {"F9"});
    check(b.Ed().Message().find("not a Z80") != std::string::npos, "F9 num fonte BASIC avisa");
    K(b, {"F8"});
    check(b.Ed().Message().find("No messages") != std::string::npos, "F8 sem mensagens avisa");

    // executar: sem gancho o item fica desligado; com gancho grava, monta e chama
    check(!b.BuildMessages().size() && !menu_item_enabled(b, ed::Cmd::RunMsx), "Run on MSX: desligado sem gancho");
    EditorApp c;
    c.SetSize(80, 25);
    c.SetEmbedded(true);
    std::string got;
    c.SetRunHook([&](const std::string &p) { got = p; return std::string(); });
    check(menu_item_enabled(c, ed::Cmd::RunMsx), "Run on MSX: ligado com gancho");
    const fs::path src = dir / "jogo.asm";
    spit(src, " org #4000\n ret\n");
    std::string err;
    check(c.Open(src.string(), err), "abre o fonte");
    c.Ed().SetLanguage(ed::LangById("z80asm"), true);
    c.Ed().SetText(" org #4000\n ld a,2\n ret\n");
    c.Ed().MarkModified();
    K(c, {"F5"});
    check(got == c.Ed().Name() && slurp(src).find("ld a,2") != std::string::npos, "F5: grava o buffer e entrega o caminho ao gancho");
    check(c.TakeHideRequest(), "F5: a janela do editor sai da frente (embutido)");
    c.Ed().SetText(" bogus\n");
    got.clear();
    K(c, {"F5"});
    check(got.empty() && !c.TakeHideRequest(), "F5 com erro de montagem nao executa");
    c.SetRunHook([&](const std::string &) { return std::string("sem maquina"); });
    c.Ed().SetText(" ret\n");
    K(c, {"F5"});
    check(c.Ed().Message() == "sem maquina", "o erro do gancho vai para a barra de mensagens");

    // ajuda da palavra
    EditorApp d;
    d.SetSize(80, 25);
    d.Ed().SetLanguage(ed::LangById("msxbasic"), true);
    d.Ed().SetText("10 PRINT LEFT$(A$,1)");
    d.Ed().SetCursor({0, 12});
    K(d, {"C-F1"});
    check(d.Ed().Message().find("not attached") != std::string::npos, "Ctrl-F1 sem manual avisa");
    std::string asked;
    d.SetHelpHook([&](const std::string &w) { asked = w; return w == "LEFT$" ? std::string("LEFT$(texto,n)\nPega os n primeiros caracteres.") : std::string(); });
    K(d, {"C-F1"});
    check(asked == "LEFT$" && d.HasDialog() && screen_has(d, "Pega os n primeiros"), "Ctrl-F1: abre a pagina da palavra sob o cursor (BASIC com $)");
    K(d, {"Esc"});
    d.Ed().SetCursor({0, 3});
    K(d, {"C-F1"});
    check(!d.HasDialog() && d.Ed().Message().find("No manual page") != std::string::npos, "palavra sem pagina avisa");

    fs::remove_all(dir);
}

void test_badig_build() {
    const fs::path dir = fs::temp_directory_path() / "fwedit_badig";
    fs::remove_all(dir);
    fs::create_directories(dir);
    badig::SetConfigPath((dir / "badig.cfg").string());

    // converter o buffer .dmx: o erro leva a' linha; o que esta' certo grava .amx e .bmx ao lado do arquivo
    EditorApp a;
    a.SetSize(80, 25);
    const fs::path src = dir / "prog.dmx";
    spit(src, "print \"a\"\n");
    std::string err;
    check(a.Open(src.string(), err), "abre o .dmx");
    check(a.Ed().Language() && a.Ed().Language()->Id() == "dignified", "a extensao .dmx escolhe o Basic Dignified");
    a.Ed().SetText("print \"a\"\n{top}\nx = 1\ngoto {nowhere}\n");
    K(a, {"F9"});
    check(a.BuildMessages().size() == 1 && !a.BuildMessages()[0].warning && a.BuildMessages()[0].line == 4 &&
              a.BuildMessages()[0].text.find("Label does not exist") == 0,
          "F9 num .dmx: o erro vira mensagem com a linha do .dmx");
    check(a.Ed().Cursor().line == 3 && a.Ed().Message().find("Convert: 1 error(s)") == 0, "o cursor vai para a linha do erro");
    check(!fs::exists(dir / "prog.amx"), "com erro nada e' gravado");

    a.Ed().SetText("print \"a\"\n{top}\nx = 1\ngoto {top}\n");
    K(a, {"F9"});
    check(a.BuildMessages().empty() && a.Ed().Message().rfind("Converted: ", 0) == 0, "converte sem erro");
    check(fs::exists(dir / "prog.amx") && fs::exists(dir / "prog.bmx"), "grava o .amx e o .bmx ao lado do fonte");
    check(slurp(dir / "prog.amx").find("goto 40") != std::string::npos, "o label virou numero de linha");
    check(slurp(dir / "prog.bmx").size() > 8 && static_cast<unsigned char>(slurp(dir / "prog.bmx")[0]) == 0xFF, "o .bmx e' BASIC tokenizado (FF...)");

    // buffer sem nome: so' confere
    EditorApp u;
    u.SetSize(80, 25);
    u.Ed().SetLanguage(ed::LangById("dignified"), true);
    u.Ed().SetText("print 1\n");
    K(u, {"F9"});
    check(u.Ed().Message().find("not saved") != std::string::npos, "sem nome: converte mas nao grava");

    // executar: o gancho recebe o caminho do .dmx
    EditorApp c;
    c.SetSize(80, 25);
    c.SetEmbedded(true);
    std::string got;
    c.SetRunHook([&](const std::string &p) { got = p; return std::string(); });
    check(c.Open(src.string(), err), "abre o .dmx para executar");
    K(c, {"F5"});
    check(got == c.Ed().Name() && c.TakeHideRequest(), "F5 num .dmx converte e chama o gancho com o caminho do .dmx");

    // o snippet do carregador do NestorBASIC
    EditorApp n;
    n.SetSize(80, 25);
    n.Ed().SetLanguage(ed::LangById("dignified"), true);
    n.Ed().SetText("nbload");
    n.Ed().SetCursor({0, 6});
    K(n, {"Tab"});
    check(n.Ed().Text().find("bload \"nbasic.bin\",r") != std::string::npos && n.Ed().Text().rfind("include \"nbasic.dmx\"") > n.Ed().Text().find("end"),
          "snippet nbload: o carregador do NestorBASIC com a biblioteca depois do END");

    // um .include com erro abre a aba do arquivo (janela grafica)
    spit(dir / "inc.dmx", "print 1\ngoto {nope}\n");
    EditorApp g;
    g.SetSize(80, 25);
    g.SetGuiMode(true);
    check(g.OpenTab(src.string(), err), "abre o principal");
    g.Ed().SetText("include \"inc.dmx\"\nprint 2\n");
    K(g, {"F9"});
    check(g.BuildMessages().size() == 1 && g.BuildMessages()[0].line == 2 && !g.BuildMessages()[0].path.empty(), "o erro do include traz o caminho do include");
    check(g.Ed().Name().find("inc.dmx") != std::string::npos && g.Ed().Cursor().line == 1, "a janela abre o include na linha do erro");

    // a tela de configuracao: duas paginas, OK grava o badig.cfg
    EditorApp s;
    s.SetSize(80, 25);
    K(s, {"A-u", "d"});
    check(s.HasDialog() && screen_has(s, "Basic Dignified setup (1/2)") && screen_has(s, "Strip all spaces"), "Build > Dignified setup... abre a pagina 1");
    s.CurrentMutable()->Find(24)->checked = true;       // strip spaces
    s.CurrentMutable()->Find(20)->text = "100";          // line start
    s.CurrentMutable()->Find(29)->text = "16";           // lmx
    s.PressDialog(ed::kBtnNext);
    check(s.HasDialog() && screen_has(s, "(2/2)") && screen_has(s, "Lexer tokens"), "Next abre a pagina 2");
    s.CurrentMutable()->Find(30)->checked = true;        // label report
    check(screen_has(s, "NestorBASIC") && screen_has(s, "Files of the source folder"), "a pagina 2 tem os arquivos de apoio e o NestorBASIC");
    s.CurrentMutable()->Find(38)->text = "*.bin dados.dat";
    s.CurrentMutable()->Find(39)->checked = true;
    s.PressDialog(ed::kBtnPrev);
    check(s.CurrentMutable()->Find(24)->checked && s.CurrentMutable()->Find(20)->text == "100", "Prev volta com os valores da pagina 1");
    s.PressDialog(ed::kBtnNext);
    s.DialogEnter();
    check(!s.HasDialog() && s.Ed().Message().find("saved") != std::string::npos, "OK grava");
    const badig::Options l = badig::LoadDefaults();
    check(l.strip_spaces && l.line_start == 100 && l.tk_list == 16 && l.label_report && !l.lexer_report, "o badig.cfg tem o que a tela mostrou");
    check(l.disk_files == "*.bin dados.dat" && l.nbasic, "e os arquivos de apoio / NestorBASIC");
    K(s, {"A-u", "d"});
    check(s.CurrentMutable()->Find(24)->checked, "a tela reabre com o que foi gravado");
    s.PressDialog(ed::kBtnCancel);

    // as opcoes valem na proxima conversao: sem espacos, a partir da linha 100
    a.Ed().SetText("print \"a\"\n");
    K(a, {"F9"});
    check(slurp(dir / "prog.amx").rfind("100'Converted", 0) == 0, "a configuracao gravada vale na conversao (-ss, linha inicial)");

    badig::SetConfigPath("");
    fs::remove_all(dir);
}

int main() {
    test_key_names();
    test_menus();
    test_dialogs();
    test_setup_and_config();
    test_windows();
    test_mouse();
    test_view_app();
    test_gui_input();
    test_gui_mode();
    test_embedded_and_session();
    test_untitled_names();
    test_autosave();
    test_syntax_in_editor();
    test_model_extras();
    test_translation();
    test_complete_and_numbers();
    test_build();
    test_badig_build();
    std::printf("\n%s (%d falha%s)\n", g_failures == 0 ? "TODOS OS TESTES PASSARAM" : "HA' FALHAS", g_failures, g_failures == 1 ? "" : "s");
    return g_failures == 0 ? 0 : 1;
}
