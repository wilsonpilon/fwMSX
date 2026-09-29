//
// msxdisk (fwMSX): estilo visual MODERNO da GUI -- Fase 5 (ajuste a
// pedido do autor: a paleta retro compartilhada com a TUI, otima em
// terminal, ficava "anos 90" numa janela grafica de verdade). A TUI
// continua com src/msxdisk/config/theme.h (Norton Commander/XTree, de
// proposito); a GUI tem sua propria paleta aqui, sem nenhuma relacao com
// aquela.
//
#pragma once

struct ImGuiIO;

namespace msxdisk::gui {

// Aplica a paleta + espacamento/arredondamento moderno ao ImGuiStyle
// atual. 'dark' escolhe entre o tema escuro (default) e o claro.
void ApplyModernStyle(bool dark);

// Cor de fundo (glClearColor) correspondente ao tema moderno atual, para
// nao deixar uma faixa da cor antiga aparecendo atras da janela do ImGui.
void ModernClearColor(bool dark, float *r, float *g, float *b);

// Troca a fonte padrao do ImGui por uma fonte do sistema mais legivel
// (Segoe UI no Windows), com fallback silencioso pra fonte embutida do
// ImGui se nao encontrar nenhuma candidata. Chamar depois de
// ImGui::CreateContext() e antes de inicializar os backends.
void LoadModernFont(ImGuiIO &io);

} // namespace msxdisk::gui
