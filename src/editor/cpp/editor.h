// fwMSX -- editor de textos (estilo Norton Editor 2): modelo SEM interface. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/editor-spec.md. O modelo recebe teclas por NOME (Key) ou texto (Type), age sobre o buffer e desenha numa
// grade de celulas (Render); a TUI e a janela so' pintam a grade e repassam teclas -- por isso os recursos e os
// atalhos sao os MESMOS nos dois modos. Comandos que pedem uma caixa de dialogo (abrir, achar, ...) nao abrem nada
// aqui: deixam um Dialog pendente (TakeDialog) e a interface devolve o resultado pelos metodos Apply*/Open/Find.
#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ed {

constexpr int kDefaultCols = 80;
constexpr int kDefaultRows = 25;

// ---------------------------------------------------------------------------------------------------------------
// Grade de celulas
// ---------------------------------------------------------------------------------------------------------------

enum Style : uint8_t {
    kText = 0,
    kBar,         // barra de menu e de estado
    kBarHot,      // letra de atalho na barra
    kBarSel,      // menu aberto / prefixo Alt pendente
    kBlockSel,    // texto dentro do bloco marcado
    kBlockMark,   // marcador de bloco
    kScrollTrack, // trilho da barra de rolagem
    kScrollThumb, // "elevador" da barra de rolagem
    kScrollArrow, // setas da barra de rolagem
    kPanel,       // menus e caixas de dialogo (fundo)
    kPanelHot,    // letra de atalho nos menus e botoes
    kPanelSel,    // item de menu / botao / campo com o foco
    kPanelOff,    // item de menu desabilitado
    kPanelField,  // campo de texto de um dialogo
    kShadow,      // sombra pontilhada
    kTitleBar,    // barra com o nome do arquivo entre duas janelas
    kBracket,     // o parentese que casa com o que esta' a esquerda do cursor
    kStyleCount
};

struct Cell {
    uint16_t ch = ' '; // 0-255: o byte do texto; acima: ornamentos (setas, trilho) em Unicode
    uint8_t style = kText;
};

struct Grid {
    int cols = 0;
    int rows = 0;
    std::vector<Cell> cells;
    void Resize(int c, int r);
    Cell &at(int x, int y) { return cells[static_cast<size_t>(y) * cols + x]; }
    const Cell &at(int x, int y) const { return cells[static_cast<size_t>(y) * cols + x]; }
    void Put(int x, int y, const std::string &text, Style style = kText);
    void PaintRow(int y, Style style);
};

// ---------------------------------------------------------------------------------------------------------------
// Posicao, comandos, dialogos
// ---------------------------------------------------------------------------------------------------------------

struct Pos {
    int line = 0;
    int col = 0; // indice de byte dentro da linha (nao a coluna de tela: as tabulacoes ocupam varias colunas)
    bool operator==(const Pos &o) const { return line == o.line && col == o.col; }
    bool operator!=(const Pos &o) const { return !(*this == o); }
    bool operator<(const Pos &o) const { return line < o.line || (line == o.line && col < o.col); }
    bool operator<=(const Pos &o) const { return !(o < *this); }
};

enum class Cmd {
    None,
    // cursor
    Left, Right, Up, Down, WordLeft, WordRight, Home, End, PgUp, PgDn, ScreenTop, ScreenBottom, FileTop, FileEnd,
    ScrollUp, ScrollDown,
    // selecao com Shift (blocos "modernos")
    SelLeft, SelRight, SelUp, SelDown, SelHome, SelEnd, SelPgUp, SelPgDn, SelWordLeft, SelWordRight,
    // apagar e editar
    DelLeft, DelRight, DelWordLeft, DelWordRight, DelToBol, DelToEol, KillLine, Undelete, FlipCaseBack,
    FlipCaseFwd, Literal, Enter, EnterIndent, Tab, Unindent, ToggleInsert, Undo, Redo, Copy, Cut, Paste,
    DividerUp, DividerDown,
    // menu File
    New, Open, Save, SaveAs, Merge, Window2, CloseWindow, LoadMore, WriteToCursor, TopOfFile, DosShell, Exit,
    // menu Block
    BlockCopy, BlockMove, BlockDelete, BlockDeleteNoAsk, BlockSetMarker, BlockRemoveMarkers, BlockFindMarker,
    BlockIndent, BlockUnindent, BlockOutput, BlockLine, BlockToEol,
    // menu Text-layout
    ParagraphFormat, DocumentFormat, CenterLine, TextSetup, WordWrap, InsertPageBreaks, RemovePageBreaks, ExpandTabs,
    PutInTabs, AutoIndent,
    // menu Search
    Find, Replace, ContinueFwd, ContinueRev, LineNumber, MatchBracket, WindowDiff, TextInBlock,
    // menu Print
    Print, PrintBlock, EjectPage, PrintSetup,
    // menu View
    Information, Outline, About,
    // menu Options
    DisplaySetup, MouseSetup, EditorSetup, SaveConfig,
    // ajuda e menu
    Help, MenuOpen
};

enum class Dialog {
    None, Open, SaveAs, Merge, Find, Replace, LineNumber, BlockIndent, BlockUnindent, BlockOutput, TextSetup,
    InsertPageBreaks, RemovePageBreaks, FormatDocument, ExpandTabs, PutInTabs, PrintSetup, Information, Outline,
    About, DisplaySetup, MouseSetup, EditorSetup, SaveConfig, Help, ConfirmExit, Window2
};

struct CmdInfo {
    Cmd cmd;
    const char *name;
    bool destructive;  // muda o texto ou o arquivo: desabilitado no modo somente leitura
    Dialog dialog;     // dialogo que o comando abre (None = executa direto)
    bool implemented;  // false = o item aparece no menu, mas o comando ainda nao existe
};
const CmdInfo &Info(Cmd c);

struct MenuItem {
    const char *label;     // texto do item (nullptr = so' atalho escondido)
    char hot;              // letra de atalho depois do Alt+menu (0x7F = Del); 0 = nenhuma
    const char *shortcut;  // "F2", "Ctrl-F"... (so' para mostrar; pode ser "")
    Cmd cmd;
    int arg;               // argumento do comando (ex.: o topico da ajuda)
    bool separator_after;
    bool hidden;           // atalho documentado que nao aparece no menu (Alt-B Del, Alt-V A...)
};

struct Menu {
    const char *title;
    char hot;     // letra que, depois do Alt, abre o menu
    int col;      // coluna do titulo na barra
    std::vector<MenuItem> items;
};
const std::vector<Menu> &Menus();

struct KeyBinding {
    const char *key;  // nome canonico: "C-w", "A-k", "C-Left", "S-Tab", "F2"
    Cmd cmd;
    bool alt;         // true = tecla ALTERNATIVA (moderna ou para terminal), false = a tecla do Norton Editor
};
const std::vector<KeyBinding> &Bindings();

// ---------------------------------------------------------------------------------------------------------------
// Configuracao (o que as caixas Display/Editor/Text-layout setup editam)
// ---------------------------------------------------------------------------------------------------------------

enum class TabKey { InsertTab, InsertSpaces, MoveToStop };
enum class OverMode { StartInsert, StartOverstrike, NeverOverstrike };

struct Settings {
    int tab_width = 8;
    TabKey tab_key = TabKey::InsertTab;
    OverMode over_mode = OverMode::StartInsert;
    char backup_char = '{';
    bool delete_backup = false;
    bool alt_selects_menu = true;
    bool auto_indent = false;
    bool word_wrap = false;
    int line_length = 65;
    int first_margin = 0;
    int left_margin = 0;
    int page_length = 62;
    int top_margin = 2;
    bool show_bracket = true; // realca o parentese que casa (Display setup: Blinking bracket)
    int autosave_minutes = 0;     // salvamento automatico periodico (Editor setup); 0 = desligado
    bool autosave_files = false;  // o autosave tambem grava nos arquivos que ja' tem nome (senao so' a sessao de recuperacao)
};

struct DocInfo {
    std::string file;
    long chars = 0;
    long words = 0;
    long lines = 0;
    int char_at_cursor = -1; // -1 = fim da linha
};

// ---------------------------------------------------------------------------------------------------------------
// O editor
// ---------------------------------------------------------------------------------------------------------------

class Editor {
public:
    Editor();

    // --- entrada -------------------------------------------------------------------------------------------------
    // Tecla por nome: "Left", "C-w", "A-k", "C-A-Left", "S-Tab", "F2", "Enter", "BS", "Del", "Ins", "Esc"; uma tecla
    // de um caractere ("a", "A", "5", "{") e' texto. Devolve true se o editor consumiu a tecla.
    bool Key(const std::string &name);
    // Texto digitado (um ou mais bytes; sem quebra de linha: use Key("Enter")).
    bool Type(const std::string &text);
    // Executa um comando (ja' resolvido de tecla ou de menu). `arg` e' o argumento do item de menu.
    bool Execute(Cmd cmd, int arg = 0);

    // --- arquivo ---------------------------------------------------------------------------------------------------
    bool Load(const std::string &path, std::string &err);
    bool Save(std::string &err, bool make_backup = true); // grava no nome atual (make_backup = false: nao refaz a copia { )
    bool SaveAs(const std::string &path, std::string &err);
    bool Merge(const std::string &path, std::string &err); // insere o arquivo no cursor
    bool WriteBlock(const std::string &path, std::string &err); // Block > Output to file
    void NewFile();
    void SetText(const std::string &text);              // troca o conteudo (EOL interno: '\n')
    std::string Text() const;                           // o conteudo com '\n' entre as linhas

    // --- resultados de dialogos ------------------------------------------------------------------------------------
    Dialog TakeDialog();                                // o dialogo pedido pelo ultimo comando (e limpa)
    int HelpTopic() const { return help_topic_; }
    bool Find(const std::string &text, bool match_case, bool backwards);
    bool ReplaceHere(int n, const std::string &repl);   // troca os n caracteres do cursor por repl (busca interativa)
    int ReplaceAll(const std::string &find, const std::string &repl, bool match_case); // do cursor ao fim; devolve quantos
    bool ReplaceNext(const std::string &find, const std::string &repl, bool match_case, bool backwards);
    bool GoToLine(int line1);                           // 1-based
    void Indent(int amount);                            // Block > Indent
    void Unindent(int amount);                          // Block > Unindent
    void ApplyExpandTabs(bool from_top);
    void ApplyPutInTabs(bool from_top);
    void ApplyInsertPageBreaks(bool from_top);
    void ApplyRemovePageBreaks(bool from_top);
    DocInfo Information() const;

    // --- estado ----------------------------------------------------------------------------------------------------
    void SetCrlf(bool v) { crlf_ = v; }
    void MarkModified() { modified_ = true; ++rev_; }
    const std::string &Name() const { return name_; }
    // Nome para MOSTRAR: o arquivo (so' o nome, sem a pasta) ou o nome automatico dos sem nome (noname01...); "Untitled" se ainda nao recebeu.
    std::string Title() const;
    const std::string &Untitled() const { return untitled_; }
    void SetUntitled(const std::string &u) { untitled_ = u; }
    bool AutosavedOnce() const { return autosaved_; }
    void SetAutosavedOnce(bool v) { autosaved_ = v; }
    void SetName(const std::string &n) { name_ = n; }
    bool Modified() const { return modified_; }
    bool ReadOnly() const { return read_only_; }
    void SetReadOnly(bool v) { read_only_ = v; }
    bool InsertMode() const { return insert_; }
    int LineCount() const { return static_cast<int>(lines_.size()); }
    const std::string &Line(int i) const { return lines_[static_cast<size_t>(i)]; }
    Pos Cursor() const { return cur_; }
    void SetCursor(Pos p);
    int DisplayCol(Pos p) const;                        // coluna de tela (0-based), com tabulacoes expandidas
    bool HasBlock() const { return m1_set_ && m2_set_; }
    bool BlockRange(Pos &a, Pos &b) const;              // [a, b) normalizado
    std::string BlockText() const;
    const std::string &Message() const { return message_; }
    void ClearMessage() { message_.clear(); }
    char MenuPrefix() const { return menu_prefix_; }    // letra do menu aberto por Alt+letra (0 = nenhum)
    bool ItemEnabled(const MenuItem &item) const;       // false = o item aparece cinza e nao faz nada
    bool ReadOnlyFile() const { return read_only_; }
    bool Eof1A() const { return eof_ctrl_z_; }
    bool Crlf() const { return crlf_; }
    Settings &settings() { return set_; }
    const Settings &settings() const { return set_; }
    std::string Profile() const { return profile_; }
    bool Dirty() const { return modified_; }
    bool ExitRequested() const { return exit_requested_; }
    bool TakeMenuRequest() { const bool r = menu_request_; menu_request_ = false; return r; } // F10: abrir a barra de menus
    const std::string &Clipboard() const { return clipboard_; }
    void SetClipboard(const std::string &s) { clipboard_ = s; }

    // --- tela ------------------------------------------------------------------------------------------------------
    void SetSize(int cols, int rows);                   // a janela inteira (menu + texto + estado)
    void SetViewSize(int cols, int text_rows, bool reveal = true); // so' a area de texto (uma janela de varias)
    uint64_t Revision() const { return rev_; }          // muda a cada alteracao do texto (a interface grafica usa para guardar medidas)
    int TextRows() const { return text_rows_; }
    void Render(Grid &grid) const;                      // barra de menu + texto + barra de estado
    // So' a area de texto (e a barra de rolagem) nas linhas [y0, y0 + TextRows()) da grade.
    void RenderBody(Grid &grid, int y0, bool scrollbar = true) const;
    void RenderStatus(Grid &grid, int y) const;         // a barra de estado do NE na linha y
    bool CursorScreen(int &x, int &y) const;            // onde fica o cursor na grade (linha 1 = primeira do texto)
    int TopLine() const { return top_; }
    int LeftCol() const { return left_; }
    // janela grafica: a rolagem e' do ImGui; o modelo so' recebe onde esta' a primeira linha/coluna visivel
    void SetTopLine(int t) { top_ = std::max(0, std::min(t, LineCount() - 1)); }
    void SetLeftCol(int c) { left_ = std::max(0, c); }
    Pos PosAt(int line, int dcol) const;                // posicao do texto sob a linha/coluna de tela (mouse)
    void SelectWord(Pos p);                             // duplo clique
    void SelectLine(int line);                          // triplo clique
    // mouse: (x, row) sao relativos a area de texto desta janela
    Pos ScreenToPos(int x, int row) const;
    void SelectBlock(Pos a, Pos b);                     // marca o bloco entre a e b (arrastar o mouse)
    void ScrollView(int dy);                            // roda do mouse: rola sem mover o cursor para fora da tela
    void SetMessage(const std::string &m) { message_ = m; }
    bool MatchOf(Pos from, Pos &out) const;             // o parentese que casa com o de `from`
    int BodyWidth() const { return cols_; }

private:
    struct Op {
        bool insert;
        Pos at;
        std::string text;
        Pos cursor_before;
        Pos cursor_after;
        int group;
    };

    // primitivas (todas as edicoes passam por aqui, para desfazer e marcadores)
    Pos RawInsert(Pos p, const std::string &s);
    std::string RawDelete(Pos a, Pos b);
    Pos DoInsert(Pos p, const std::string &s);
    std::string DoDelete(Pos a, Pos b);
    void AdjustForInsert(Pos p, Pos end, Pos &q) const;
    void AdjustForDelete(Pos a, Pos b, Pos &q) const;
    void Touch() { modified_ = true; ++rev_; }
    Pos EndOfFile() const;

    bool InsertChars(const std::string &s);
    void InsertNewline(bool indent);
    void MoveTo(Pos p, bool keep_goal = false);
    void MoveVertical(int dy);
    void MoveSel(Cmd base);
    void Reveal();
    void DeleteRangeCmd(Pos a, Pos b);
    int FlipCase(bool to_line_start);
    void BlockSet();
    void BlockCopyMove(bool move);
    void BlockDeleteCmd();
    void BlockIndentImpl(int amount, bool out);
    void CenterLineCmd();
    bool MatchBracketCmd();
    bool MatchScan(Pos from, Pos &out) const;
    bool TextInBlockCmd();
    void DoUndo(bool redo);
    bool DispatchMenuLetter(char menu, const std::string &canon);
    int ColFromDisplay(int line, int dcol) const;
    std::string ExpandedLine(const std::string &s) const;
    std::string LeadingIndent(int line) const;
    void ClearSelectionIfPlain();
    void BeginGroup(bool merge);
    bool Writable() const { return !read_only_; }
    bool InsertLiteral(const std::string &base, bool ctrl);
    bool Search(bool fwd);

    std::vector<std::string> lines_;
    Pos cur_;
    int goal_dcol_ = 0;
    int top_ = 0;
    int left_ = 0;
    int cols_ = kDefaultCols;
    int rows_ = kDefaultRows;
    int text_rows_ = kDefaultRows - 2;
    uint64_t rev_ = 1;
    bool insert_ = true;
    bool modified_ = false;
    bool read_only_ = false;
    bool crlf_ = true;
    bool eof_ctrl_z_ = false;
    bool bom_ = false;
    std::string name_;
    std::string untitled_;   // noname01, noname02...: so' para mostrar (a interface grafica)
    bool autosaved_ = false; // o autosave ja' guardou a copia { do arquivo original
    std::string message_;
    std::string profile_ = "norton";
    Settings set_;

    // bloco: marcadores 1 e 2 (a ordem no texto vale, nao a ordem em que foram postos)
    Pos m1_, m2_;
    bool m1_set_ = false;
    bool m2_set_ = false;
    bool sel_active_ = false;  // o bloco veio de Shift+setas (um movimento comum o desfaz)
    Pos sel_anchor_;

    // busca
    std::string last_find_;
    bool last_case_ = false;
    bool last_back_ = false;
    bool have_find_ = false;
    bool found_here_ = false;  // o cursor esta' no inicio da ultima ocorrencia achada

    // Ctrl-U (desfaz a ultima remocao se o cursor nao se moveu)
    bool undel_valid_ = false;
    std::string undel_text_;
    Pos undel_pos_;
    Pos undel_cursor_;
    Pos undel_before_;

    // desfazer/refazer gerais e area de transferencia interna
    std::vector<Op> undo_;
    std::vector<Op> redo_;
    bool replaying_ = false;
    int group_ = 0;
    bool last_typed_ = false;
    Pos last_type_end_;
    bool exit_requested_ = false;
    bool menu_request_ = false;
    std::string clipboard_;

    bool literal_pending_ = false;
    char menu_prefix_ = 0;
    Dialog pending_dialog_ = Dialog::None;
    int help_topic_ = 0;
};

// Nome canonico de uma tecla ("A-C-x" -> "C-A-x"): modificadores na ordem C-, A-, S-; letra de uma tecla de caractere
// em minuscula. Vazio se o nome for invalido.
std::string CanonicalKey(const std::string &name);

} // namespace ed
