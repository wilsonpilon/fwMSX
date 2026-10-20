// Teste da aplicacao do editor (src/editor/cpp/editor_ui): menus abertos, caixas de dialogo, duas janelas, mouse,
// configuracao e a traducao das teclas do terminal. Sem terminal: as teclas entram por nome e o teste le' a grade.
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

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
    test_model_extras();
    test_translation();
    std::printf("\n%s (%d falha%s)\n", g_failures == 0 ? "TODOS OS TESTES PASSARAM" : "HA' FALHAS", g_failures, g_failures == 1 ? "" : "s");
    return g_failures == 0 ? 0 : 1;
}
