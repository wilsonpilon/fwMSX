//
// msxdisk (fwMSX): temas da TUI -- Fase 4.
//
// 'Theme' e propositalmente independente do FTXUI (so cores RGB simples)
// para o modulo de configuracao (config_store.h) nao precisar depender
// da biblioteca de interface; a camada de UI (src/msxdisk/tui) e quem
// converte RgbColor -> ftxui::Color na hora de desenhar.
//
#pragma once

#include <cstdint>
#include <string>

namespace msxdisk::config {

struct RgbColor {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct Theme {
    std::string name;
    std::string description;

    RgbColor fg_normal;
    RgbColor bg_normal;
    RgbColor fg_selected; // item sob o cursor
    RgbColor bg_selected;
    RgbColor fg_marked; // item marcado (tag) para enviar/receber
    RgbColor bg_marked;
    RgbColor fg_border;
    RgbColor fg_titlebar;
    RgbColor bg_titlebar;
    RgbColor fg_statusbar;
    RgbColor bg_statusbar;
};

// Tema "classico": fundo azul, texto branco/amarelo -- estilo Norton
// Commander / XTree Gold / PC Tools da era MS-DOS.
Theme ClassicTheme();

// Tema escuro moderno, para quem preferir uma paleta mais atual.
Theme DarkTheme();

// Devolve o tema embutido pelo nome ("classic"/"dark"); cai em
// ClassicTheme() se o nome nao for reconhecido.
Theme BuiltinThemeByName(const std::string &name);

} // namespace msxdisk::config
