// fwMSX -- editor de textos na janela (E3): a parte que nao depende de OpenGL/ImGui -- nomes de tecla do teclado "de verdade"
// (a janela recebe cada tecla com os modificadores, ao contrario do terminal) e a paleta de cores em RGB dos esquemas do NE.
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Fica fora do codigo da janela para ser testado sem janela. Ver doc/editor-spec.md.
#pragma once

#include <cstdint>
#include <string>

namespace ed {

// As teclas que a janela traduz. `None` = nao tratada.
enum class GuiKey {
    None,
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    Left, Right, Up, Down, Home, End, PgUp, PgDn, Ins, Del, Backspace, Enter, Tab, Escape, Space,
    LBracket, RBracket, Minus, Equal, Slash, Backslash, Comma, Period, Semicolon, Apostrophe, Grave
};

// Nome da tecla para `Editor::Key` ("C-w", "A-2", "S-Tab", "C-Home", "F10", "C-]"), ou "" quando a tecla e' texto
// (letra, digito ou pontuacao sem Ctrl/Alt: quem digita o texto e' a fila de caracteres do sistema, que ja' trata
// maiuscula, acento e teclado internacional).
std::string GuiKeyName(GuiKey key, bool ctrl, bool alt, bool shift);

struct Rgb {
    uint8_t r = 0, g = 0, b = 0;
};

struct PalRgb {
    Rgb fg;
    Rgb bg;
    bool bold = false;
    bool underline = false;
};

// Cor do destaque de sintaxe de uma classe (SynClass 1..13) para tema escuro (`dark`) ou claro; `bold` pede negrito.
Rgb SyntaxRgb(uint8_t cls, bool dark, bool *bold = nullptr);

// Cor de cada estilo da grade (`ed::Style`) nos esquemas do NE: 0 LCD, 1 Color 1 (azul), 2 Color 2 (ciano), 3 VGA mono, 4 mono.
PalRgb PaletteRgb(int scheme, uint8_t style);

} // namespace ed
