// fwMSX -- o editor de textos como COMPONENTE grafico (Dear ImGui): barra de menus, abas, painel(is) de texto com rolagem e mouse, caixas
// de dialogo com widgets de verdade e barra de estado, no visual do fwMSX (tema, fonte, textos em portugues). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver doc/editor-spec.md (fase E3b).
//
// O texto, os comandos, os atalhos do Norton Editor 2 e os dialogos (como DADOS) continuam no `ed::EditorApp`/`ed::Editor`: este painel
// so' LE o estado e DESENHA com widgets graficos -- nao e' a grade de caracteres do terminal. Serve a duas formas:
//   * DrawWindow: janela flutuante dentro da janela do emulador (menu "Editor" do fwMSX);
//   * DrawFullscreen: o editor standalone (`fwMSX.exe --edit`), ocupando a janela do sistema.
#pragma once

#include <string>
#include <vector>

#include <imgui.h>

#include "../cpp/editor_ui.h"

namespace ed::gui {

class EditorPanel {
public:
    EditorPanel();
    EditorApp &app() { return app_; }

    // Fonte de largura fixa (Consolas, DejaVu Sans Mono...) para o texto; chamar depois de carregar a fonte da interface.
    void LoadFont(ImGuiIO &io, float size = 17.0f);
    // Caminhos do fwedit.cfg (ao lado do executavel e na pasta atual) e leitura da configuracao.
    void Configure(const std::string &exe_dir, const std::string &cwd);

    void DrawWindow(bool *p_open);   // janela flutuante
    void DrawFullscreen();           // ocupa a janela do sistema

    // true enquanto o teclado e' do editor (janela em foco, menu ou dialogo abertos): o emulador nao deve passar teclas ao MSX.
    bool WantsKeyboard() const { return wants_kb_; }
    void RequestFocus() { request_focus_ = true; }
    void ShowOpenDialog();           // Arquivo > Abrir...
    bool NeedsAttention() const { return app_.HasDialog() || app_.MenuIndex() >= 0; }

private:
    struct PaneState {
        int last_top = -1;
        int last_left = -1;
        bool drag = false;
        Pos anchor;
    };
    struct LenCache {
        uint64_t rev = 0;
        int lines = -1;
        int maxlen = 0;
    };

    void DrawContents();
    void HandleKeyboard();
    void DrawMenuBar();
    void DrawMenuPopup();
    void DrawTabs();
    void DrawPanes(float height);
    void DrawPane(int pane, ImVec2 size);
    void DrawStatus();
    void DrawDialog();
    void DrawDialogWidgets(DialogBox &d, float ux, float uy);
    void ContextMenu(Editor &ed);
    void SyncClipboardIn();
    void SyncClipboardOut();
    int MaxLen(int doc, const Editor &ed);
    void Press(const std::string &name);

    EditorApp app_;
    ImFont *mono_ = nullptr;
    float mono_size_ = 17.0f;
    bool wants_kb_ = false;
    bool request_focus_ = false;
    bool show_numbers_ = true;
    bool alt_down_ = false;
    bool alt_used_ = false;
    double blink_reset_ = 0;
    std::string last_clip_;
    // menu
    bool menu_popup_live_ = false;
    std::vector<ImVec2> anchors_;
    // dialogo
    bool dlg_live_ = false;
    std::string last_dlg_title_;
    // abas / paineis
    int last_active_ = -1;
    PaneState panes_[2];
    std::vector<LenCache> len_cache_;
};

// Carrega uma fonte monoespacada do sistema (ou a embutida do ImGui) e a devolve.
ImFont *LoadMonoFont(ImGuiIO &io, float size);

} // namespace ed::gui
