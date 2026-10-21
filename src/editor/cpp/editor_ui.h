// fwMSX -- editor de textos: a APLICACAO (menus abertos, caixas de dialogo, duas janelas, mouse, configuracao) sobre o
// modelo `ed::Editor`. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/editor-spec.md. Continua SEM terminal e sem
// FTXUI: recebe teclas por nome e eventos de mouse e devolve uma grade; quem pinta (editor_tui.cpp) so' traduz entrada e
// cores. Por isso tudo aqui e' testavel com texto puro.
#pragma once

#include <functional>
#include <set>
#include <string>
#include <vector>

#include "../../badig/cpp/badig.h"
#include "../../project/cpp/project.h"
#include "editor.h"

namespace ed {

// ---------------------------------------------------------------------------------------------------------------
// Configuracao de tela, mouse e impressao (o que as caixas Display/Mouse/Print setup editam). O que e' do editor
// (tabulacao, backup, margens...) fica em ed::Settings.
// ---------------------------------------------------------------------------------------------------------------

struct UiSettings {
    int scheme = 0;          // 0 LCD, 1 Color 1 (azul), 2 Color 2 (ciano), 3 VGA monocromatico, 4 monocromatico
    int lines = 0;           // 0 = usar o terminal todo (Don't change); 25, 35, 43 ou 50
    bool scrollbar = true;
    bool blinking_bracket = true;
    bool zoom_effects = true;
    bool large_cursor = false;
    int mouse_button = 0;    // 0 esquerdo, 1 direito
    int scroll_delay = 1;    // 0 sem atraso, 1 = 12 ms, 2 = 18 ms, 3 = 26 ms, 4 = sincronizado com o relogio
    bool adj_mouse_speed = true;
    int format_begin = 1;    // Text-layout setup: 0 no cursor, 1 no topo do paragrafo
    int print_left_margin = 0;
    bool formfeed_between = true;
    int formfeed_after = 1;  // 0 nenhum, 1 uma pagina, 2 duas paginas
};

// Grava/le "chave=valor" (um por linha). `path` vazio = sem arquivo.
bool SaveConfigFile(const std::string &path, const Settings &s, const UiSettings &u, std::string &err);
bool LoadConfigFile(const std::string &path, Settings &s, UiSettings &u);

// ---------------------------------------------------------------------------------------------------------------
// Caixas de dialogo: widgets em posicoes absolutas dentro da caixa, como as telas do NE 2.
// ---------------------------------------------------------------------------------------------------------------

struct Widget {
    enum Type { Label, Input, Radio, Check, Button, List };
    Type type = Label;
    int x = 0, y = 0, w = 0, h = 0;
    std::string text;      // rotulo / valor do campo / texto do botao
    int group = 0;         // Radio: grupo
    int index = 0;         // Radio: posicao dentro do grupo
    bool checked = false;  // Check
    int id = 0;            // Button: numero devolvido; Input/List/Check/Radio: identificador do campo
    bool is_default = false;
    size_t cur = 0;        // Input: cursor
    bool fresh = false;    // Input: o primeiro caractere digitado substitui o texto (como o NE)
    int maxlen = 64;
    bool numeric = false;
    std::vector<std::string> items; // List
    int sel = 0;
    int top = 0;
};

enum Btn { kBtnOk = 1, kBtnCancel, kBtnNext, kBtnPrev, kBtnYes, kBtnNo, kBtnAll, kBtnSave, kBtnDontSave, kBtnTop, kBtnCursor,
           kBtnHelp, kBtnSkip, kBtnProject, kBtnFolder, kBtnActions };

// O que o menu de contexto de um arquivo/pasta do projeto faz (a arvore do TUI e da janela grafica).
enum class ProjAct { Open, Push, Pull, Rename, Add, Remove };

struct DialogBox {
    Dialog kind = Dialog::None;
    std::string title;
    int w = 40, h = 8;
    std::vector<Widget> ws;
    int focus = 0;                 // indice em `fields`
    std::vector<int> fields;       // indices em `ws` dos widgets que recebem o foco (uma entrada por grupo de radio)
    std::vector<int> group_sel;    // opcao marcada de cada grupo de radio
    std::string footer;            // "More" nas ajudas
    int page = 0;                  // ajuda: pagina atual
    int help_topic = 0;
    int tag = 0;                   // livre para quem abriu (ex.: o que fazer depois do Save)
    std::string dir;               // dialogo de arquivo: diretorio mostrado
    std::string mask = "*";        // dialogo de arquivo: mascara
    int box_x = -1, box_y = -1;    // posicao na tela (-1 = centrada)

    Widget *Find(int id);
    const Widget *Find(int id) const;
    std::string Text(int id) const;       // valor de um Input
    bool Checked(int id) const;           // valor de um Check
    int Group(int g) const { return group_sel[static_cast<size_t>(g)]; }
    void BuildFields();
    // Tecla ja' traduzida. Devolve o botao apertado (Btn) ou 0.
    int Key(const std::string &canon);
    int ClickAt(int x, int y);            // x, y dentro da caixa (a borda e' 0); devolve o botao apertado ou 0
};

// ---------------------------------------------------------------------------------------------------------------
// A aplicacao
// ---------------------------------------------------------------------------------------------------------------

// Uma mensagem do montador apontando (ou nao) para uma linha de um fonte.
struct BuildMsg {
    std::string file;   // o nome como o montador o escreveu ("jogo.asm", "inc/x.inc"); vazio se a mensagem nao traz arquivo
    int line = 0;       // base 1; 0 = mensagem geral
    bool warning = false;
    std::string text;   // so' o texto, sem "arquivo, line N:"
    std::string path;   // um .include: o caminho para abrir (vazio = o proprio fonte)
};

class EditorApp {
public:
    EditorApp();

    // --- menu Build (fase E6) ---
    // Ganchos do hospedeiro: `run` recebe o caminho do .asm ja' gravado e devolve "" se rodou, ou a mensagem de erro; `help` recebe a
    // palavra e devolve o texto do manual (vazio = sem pagina). Sem gancho o item fica cinza / avisa. O editor nao conhece a maquina nem o `man`.
    void SetRunHook(std::function<std::string(const std::string &)> f) { run_hook_ = std::move(f); }
    void SetHelpHook(std::function<std::string(const std::string &)> f) { help_hook_ = std::move(f); }
    const std::vector<BuildMsg> &BuildMessages() const { return build_msgs_; }
    int BuildCurrent() const { return build_cur_; }

    // --- projeto .msxproject (menu Project; editor_project.cpp; doc/project-spec.md) ---
    // Com um projeto aberto, todo arquivo salvo dentro da pasta do projeto vai tambem para o banco (Save, Save As, autosave, F5/F9), e "Sync..." compara
    // o projeto com a pasta (a copia mais nova e' a sugestao; conflito pergunta).
    bool ProjectNew(const std::string &file, std::string &err);
    bool ProjectOpen(const std::string &file, std::string &err);
    void ProjectClose(bool forget = true); // forget = false: so' troca de projeto (New/Open); true = o usuario fechou, nao reabre na proxima vez
    bool ProjectRestore();                 // reabre o ultimo projeto (fwedit.project ao lado do fwedit.cfg); false se nao havia ou sumiu
    std::string ProjectName() const { return project_.IsOpen() ? project_.Name() : std::string(); }
    bool HasProject() const { return project_.IsOpen(); }
    void OpenProjectFile(const std::string &path) { ProjectOpenFile(path); } // a arvore da janela grafica: abre (extraindo antes, se so' existir no projeto)
    // Menu de contexto da arvore (clique direito na janela, "Actions..." no TUI). `path` termina em '/' quando e' uma pasta.
    void ProjectAct(ProjAct a, const std::string &path) { ProjectDo(a, path, false); }
    prj::Project &ProjectRef() { return project_; }
    const std::string &ProjectRoot() const { return project_root_; }
    void ProjectFlush();  // leva para o banco os arquivos salvos desde a ultima vez (chamado depois de cada tecla, clique e salvamento automatico)

    // --- abrir/estado ---
    bool Open(const std::string &path, std::string &err, bool view = false); // primeira janela (ou a ativa)
    bool Open2(const std::string &path, std::string &err, bool view = false); // abre/ativa a segunda janela
    void SetSize(int cols, int rows);
    int Cols() const { return cols_; }
    int Rows() const { return rows_; }
    Editor &Ed() { return wins_[static_cast<size_t>(active_)]; }
    const Editor &Ed() const { return wins_[static_cast<size_t>(active_)]; }
    Editor &Win(int i) { return wins_[static_cast<size_t>(i)]; }
    int WindowCount() const { return static_cast<int>(wins_.size()); }
    int Active() const { return active_; }
    int DividerRows() const { return divider_; }
    bool QuitRequested() const { return quit_; }
    void ClearQuit() { quit_ = false; exit_index_ = -1; confirm_purpose_ = 0; }
    void RequestExit() { ExecuteCmd(Cmd::Exit, 0); }
    // Embutido no fwMSX: "Sair" so' ESCONDE a janela (os arquivos e o texto nao salvo continuam la', para voltar depois);
    // quem decide o que fazer com o que sobrou e' a sessao (SaveSession/LoadSession).
    void SetEmbedded(bool e) { embedded_ = e; }
    bool TakeHideRequest() { const bool r = hide_request_; hide_request_ = false; return r; }
    void CancelDialog() { if (HasDialog()) DialogDone(kBtnCancel); }
    // Abas sem nome recebem noname01, noname02... (o menor numero livre; fechar uma aba libera o numero).
    void EnsureUntitled();
    // Salvamento automatico (Options > Editor: a cada N minutos; 0 = nunca). `now` em segundos de qualquer relogio crescente. Grava a sessao
    // de recuperacao (se SetSessionPath) e, com `autosave_files`, tambem os arquivos que ja' tem nome. Devolve true se salvou agora.
    void SetSessionPath(const std::string &p) { session_path_ = p; }
    const std::string &SessionPath() const { return session_path_; }
    bool AutosaveTick(double now);
    bool AutosaveNow();
    void DiscardSession();                            // apaga o arquivo de sessao (saida limpa do editor standalone)
    bool SaveSession(const std::string &path) const;  // grava as abas (inclusive o texto nao salvo); sem nada util, apaga o arquivo
    bool LoadSession(const std::string &path);        // restaura as abas gravadas (so' se o editor ainda esta' vazio) // fechar a janela: o mesmo caminho de Alt-F X (pergunta por arquivos alterados)
    UiSettings &ui() { return ui_; }
    const UiSettings &ui() const { return ui_; }
    void SetConfigPaths(const std::string &main_cfg, const std::string &dir_cfg) { cfg_main_ = main_cfg; cfg_dir_ = dir_cfg; }
    void LoadConfig();
    void SetStartDir(const std::string &d) { cwd_ = d; }
    const std::string &Dir() const { return cwd_; }
    bool BarActive() const { return bar_active_; }
    int MenuIndex() const { return menu_; }
    int MenuItemIndex() const { return item_; }
    const DialogBox *Current() const { return dlg_.kind == Dialog::None ? nullptr : &dlg_; }
    bool HasDialog() const { return dlg_.kind != Dialog::None; }

    // --- interface grafica (janela flutuante do fwMSX / editor standalone) ---
    // No modo grafico os arquivos viram ABAS (nao ha' limite de 2 janelas), "2nd window" divide a area em 2 paineis lado a lado,
    // `New`/`Open` abrem uma aba nova e a grade de caracteres (Render) nao e' usada: a interface le' o estado e desenha com widgets.
    void SetGuiMode(bool g) {
        gui_mode_ = g;
        EnsureUntitled();
    }
    bool GuiMode() const { return gui_mode_; }
    bool OpenTab(const std::string &path, std::string &err, bool view = false);
    void NewTab();
    void CloseTab(int i);                     // pergunta se o arquivo foi alterado
    void SetActive(int i);
    bool Split() const { return split_; }
    int PaneDoc(int pane) const { return split_ ? pane_[pane] : active_; }
    int ActivePane() const { return split_ ? ap_ : 0; }
    void ToggleSplit();                       // com 2+ abas divide; sem outra aba pede um arquivo (2nd window)
    void OpenMenuAt(int m) { OpenMenu(m); }
    void CloseMenuNow() { CloseMenu(); }
    void SetMenuItem(int i) { item_ = i; }
    void RunMenu(int m, int i);
    bool MenuItemEnabled(const MenuItem &it) const { return Enabled(it); }
    DialogBox *CurrentMutable() { return HasDialog() ? &dlg_ : nullptr; }
    void PressDialog(int id) { DialogDone(id); }
    void ShowLanguage() { ShowDialog(Dialog::Language); } // escolher a linguagem do destaque
    void DialogEnter();                       // Enter no dialogo (nos de arquivo: abre/entra no diretorio)

    // --- entrada ---
    bool Key(const std::string &canon);      // nome canonico (KeyNameFromInput)
    bool Text(const std::string &bytes);     // texto digitado (ja' em bytes do editor)
    void MouseDown(int x, int y);
    void MouseDrag(int x, int y);
    void MouseUp(int x, int y);
    void Wheel(int x, int y, int dy);

    // --- tela ---
    void Render(Grid &g) const;
    bool CursorScreen(int &x, int &y) const; // onde o cursor de texto aparece (dialogo ou janela ativa)

private:
    struct WinRect { int y0, h; };
    WinRect Rect(int i) const;
    void Layout();
    void AfterEditorCommand();
    bool ExecuteCmd(Cmd cmd, int arg);
    void OpenMenu(int m);
    void CloseMenu();
    bool MenuKey(const std::string &canon);
    void RunMenuItem(const MenuItem &it);
    bool Enabled(const MenuItem &it) const;
    void ShowDialog(Dialog d, int arg = 0);
    void DialogDone(int button);
    void FinishExit();
    void NextExitStep();
    bool DoCloseWindow();
    void RemoveTab(int i);
    bool WindowDifferences();
    void Msg(const std::string &m) { Ed().SetMessage(m); }

    // dialogos (editor_dialogs.cpp)
    void BuildFileDialog(Dialog kind, const std::string &title);
    void RefreshFileList();
    void BuildSimpleInput(Dialog kind, const std::string &title, const std::string &label, const std::string &value, int width);
    void BuildFind(bool replace);
    void BuildConfirm(const std::vector<std::string> &lines, const std::vector<int> &buttons, int default_button, Dialog kind);
    void BuildTextSetup();
    void BuildPrintSetup();
    void BuildInfo();
    void BuildAbout();
    void BuildDisplay();
    void BuildMouse();
    void BuildLanguage();
    void BuildEditorSetup();
    void BuildSaveConfig();
    void BuildHelp(int topic, int page);
    void BuildPageBreakPrompt(Dialog kind, const std::string &text);
    void FileDialogEnter();
    void RenderDialog(Grid &g) const;
    void RenderMenu(Grid &g) const;
    void ReplacePrompt();
    void SaveConfigNow(bool this_dir);
    bool SaveActive();
    void ContinueAfterConfirm();
    // menu Build (editor_build.cpp)
    bool BuildAssemble();
    bool BuildDignified();
    void BuildBadigSetup(int page);
    void ReadBadigPage(const DialogBox &d, int page);
    bool BuildRun();
    void BuildStep(int dir);
    void BuildGoto(int i);
    void BasicLineGoto(const std::string &text); // erro de execucao do BASIC ("Syntax error in 130") -> a linha do .dmx
    void BuildErrors();
    void BuildMessagesDialog();
    bool HelpOnWord();
    void BuildReference(const std::string &title, const std::string &text);
    void BuildProjectList(Dialog kind, const std::string &title, const std::vector<std::string> &items, int sel); // editor_dialogs.cpp
    // menu Project (editor_project.cpp)
    std::string ProjectMemoPath() const;                        // fwedit.project: guarda o ultimo projeto entre sessoes ("" sem configuracao)
    void ProjectRemember() const;
    std::string ProjectRel(const std::string &abs_path) const;  // o caminho dentro do projeto, ou "" se o arquivo esta' fora da pasta
    void ProjectSyncStart();                                    // compara; se ha' diferencas mostra a lista (OK aplica o que e' seguro)
    void ProjectSyncApply();
    void ProjectAskNext();                                      // o proximo conflito / apagado da fila
    void ProjectAnswer(int button);
    void ProjectReloadBuffers(const std::vector<std::pair<std::string, long long>> &before);
    std::vector<std::pair<std::string, long long>> ProjectStamps() const;
    void ProjectFilesDialog(int sel = 0);
    void ProjectTreeEnter(int row);
    void ProjectActionsDialog(int row);                            // o menu de contexto da linha `row` da arvore do Files...
    void ProjectDo(ProjAct a, const std::string &path, bool from_files);
    void ProjectRenameDo(const std::string &from, const std::string &to);
    void ProjectAddDo(const std::string &rel);
    void ProjectBackToFiles() { if (act_from_files_) ProjectFilesDialog(act_row_); } // depois de uma acao, a arvore volta no mesmo item
    void ProjectOpenFile(const std::string &path);
    void ProjectAddCurrent();

    prj::Project project_;
    std::string project_root_;
    std::string last_project_;                // o ultimo .msxproject aberto: Open project... o propoe de novo
    std::vector<prj::SyncItem> sync_queue_;  // conflitos e apagados que esperam a resposta do usuario
    size_t sync_pos_ = 0;
    std::vector<std::pair<std::string, bool>> tree_rows_; // as linhas da arvore do Files...: (caminho, e' pasta)
    std::string act_path_;                                // o alvo do menu de contexto aberto (pasta termina em '/')
    std::vector<ProjAct> act_list_;                       // as linhas do dialogo Actions
    int act_row_ = 0;
    bool act_from_files_ = false;                         // veio do Files...: depois da acao a arvore reabre
    std::set<std::string> tree_folded_;                  // as pastas recolhidas ("src/"), lembradas entre aberturas do dialogo
    std::vector<prj::FileInfo> project_list_; // o que o dialogo Files... mostra (o indice da lista = o indice aqui)
    std::vector<std::pair<std::string, long long>> sync_before_;
    std::vector<Editor> wins_;
    int active_ = 0;
    int divider_ = 10;
    int cols_ = kDefaultCols;
    int rows_ = kDefaultRows;
    bool quit_ = false;
    UiSettings ui_;
    std::string cfg_main_;
    std::string cfg_dir_;
    std::string cwd_;
    bool bar_active_ = false;
    int menu_ = -1;
    int item_ = 0;
    DialogBox dlg_;
    bool gui_mode_ = false;
    std::string session_path_;
    double autosave_next_ = 0;
    double autosave_period_ = 0;
    bool embedded_ = false;
    bool hide_request_ = false;
    bool split_ = false;
    int pane_[2] = {0, 0};      // modo grafico: a aba mostrada em cada painel quando ha' divisao
    int ap_ = 0;                // painel ativo
    int close_index_ = -1;      // modo grafico: aba que esta' sendo fechada (confirmacao)
    int exit_index_ = -1;       // sequencia de saida: janela que esta' sendo conferida
    int confirm_purpose_ = 0;   // 1 = sair, 2 = novo arquivo, 3 = abrir outro arquivo, 4 = fechar aba, 5 = salvar a configuracao
    int after_save_ = 0;        // >0: o Save As aberto continua o fluxo de confirmacao (sair, novo, abrir)
    bool config_dirty_ = false; // alguma caixa de Options mudou a configuracao
    int repl_count_ = 0;
    std::string pending_path_;  // purpose 3: o arquivo a abrir depois do Save
    // busca/troca interativa
    std::string find_text_, repl_text_;
    bool find_case_ = false, find_back_ = false;
    // mouse
    bool dragging_ = false;
    Pos drag_anchor_;
    int drag_win_ = 0;
    // menu Build
    std::vector<BuildMsg> build_msgs_;
    int build_cur_ = -1;
    badig::Options badig_edit_;  // a configuracao do Basic Dignified enquanto o dialogo esta' aberto
    int badig_page_ = 0;
    std::vector<badig::LineMapEntry> basic_map_; // linha BASIC -> linha do fonte, da ultima conversao Dignified
    std::string build_name_;   // o nome com que o montador viu o fonte (as mensagens do proprio fonte trazem este nome)
    std::string build_src_;     // arquivo montado por ultimo (caminho completo; vazio = buffer sem nome)
    std::function<std::string(const std::string &)> run_hook_;
    std::function<std::string(const std::string &)> help_hook_;
};

} // namespace ed
