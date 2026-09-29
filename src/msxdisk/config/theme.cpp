//
// msxdisk (fwMSX): temas da TUI -- ver theme.h.
//

#include "theme.h"

namespace msxdisk::config {

Theme ClassicTheme() {
    Theme t;
    t.name = "classic";
    t.description = "Fundo azul, texto branco/amarelo -- estilo Norton Commander / XTree Gold.";

    t.bg_normal = {0, 0, 170};      // azul classico (paleta CGA/EGA)
    t.fg_normal = {255, 255, 255};  // branco

    t.bg_selected = {0, 170, 170};  // barra do cursor: ciano
    t.fg_selected = {0, 0, 0};      // texto preto sobre o ciano

    t.fg_marked = {255, 255, 85};   // arquivos marcados: amarelo (mesmo fundo azul)
    t.bg_marked = t.bg_normal;

    t.fg_border = {255, 255, 255};

    t.bg_titlebar = {170, 170, 170};
    t.fg_titlebar = {0, 0, 0};

    t.bg_statusbar = {0, 0, 0};
    t.fg_statusbar = {255, 255, 255};

    return t;
}

Theme DarkTheme() {
    Theme t;
    t.name = "dark";
    t.description = "Paleta escura moderna.";

    t.bg_normal = {30, 30, 30};
    t.fg_normal = {220, 220, 220};

    t.bg_selected = {60, 90, 140};
    t.fg_selected = {255, 255, 255};

    t.fg_marked = {255, 180, 60};
    t.bg_marked = t.bg_normal;

    t.fg_border = {100, 100, 100};

    t.bg_titlebar = {45, 45, 48};
    t.fg_titlebar = {255, 255, 255};

    t.bg_statusbar = {20, 20, 20};
    t.fg_statusbar = {180, 180, 180};

    return t;
}

Theme BuiltinThemeByName(const std::string &name) {
    if (name == "dark") return DarkTheme();
    return ClassicTheme();
}

} // namespace msxdisk::config
