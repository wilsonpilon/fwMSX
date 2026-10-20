// fwMSX -- editor de textos na janela (E3): nomes de tecla e paleta. Ver editor_gui_input.h.
#include "editor_gui_input.h"

#include "editor.h"

namespace ed {

namespace {

bool IsTextKey(GuiKey k) {
    return (k >= GuiKey::A && k <= GuiKey::D9) || (k >= GuiKey::LBracket && k <= GuiKey::Grave);
}

std::string BaseName(GuiKey k) {
    if (k >= GuiKey::A && k <= GuiKey::Z) return std::string(1, static_cast<char>('a' + (static_cast<int>(k) - static_cast<int>(GuiKey::A))));
    if (k >= GuiKey::D0 && k <= GuiKey::D9) return std::string(1, static_cast<char>('0' + (static_cast<int>(k) - static_cast<int>(GuiKey::D0))));
    if (k >= GuiKey::F1 && k <= GuiKey::F12) return "F" + std::to_string(static_cast<int>(k) - static_cast<int>(GuiKey::F1) + 1);
    switch (k) {
    case GuiKey::Left: return "Left";
    case GuiKey::Right: return "Right";
    case GuiKey::Up: return "Up";
    case GuiKey::Down: return "Down";
    case GuiKey::Home: return "Home";
    case GuiKey::End: return "End";
    case GuiKey::PgUp: return "PgUp";
    case GuiKey::PgDn: return "PgDn";
    case GuiKey::Ins: return "Ins";
    case GuiKey::Del: return "Del";
    case GuiKey::Backspace: return "BS";
    case GuiKey::Enter: return "Enter";
    case GuiKey::Tab: return "Tab";
    case GuiKey::Escape: return "Esc";
    case GuiKey::Space: return " ";
    case GuiKey::LBracket: return "[";
    case GuiKey::RBracket: return "]";
    case GuiKey::Minus: return "-";
    case GuiKey::Equal: return "=";
    case GuiKey::Slash: return "/";
    case GuiKey::Backslash: return "\\";
    case GuiKey::Comma: return ",";
    case GuiKey::Period: return ".";
    case GuiKey::Semicolon: return ";";
    case GuiKey::Apostrophe: return "'";
    case GuiKey::Grave: return "`";
    default: return {};
    }
}

} // namespace

std::string GuiKeyName(GuiKey key, bool ctrl, bool alt, bool shift) {
    if (key == GuiKey::None) return {};
    // letra, digito e pontuacao: so' viram tecla com Ctrl ou Alt; sozinhas sao texto
    if (IsTextKey(key) && !ctrl && !alt) return {};
    // espaco: texto sozinho; com Ctrl/Alt e' uma tecla ("C-Space" fica para o completar)
    if (key == GuiKey::Space) {
        if (!ctrl && !alt) return {};
        std::string n = std::string(ctrl ? "C-" : "") + (alt ? "A-" : "") + (shift ? "S-" : "") + "Space";
        return n;
    }
    const std::string base = BaseName(key);
    if (base.empty()) return {};
    std::string name;
    if (ctrl) name += "C-";
    if (alt) name += "A-";
    if (shift) name += "S-";
    return CanonicalKey(name + base);
}

Rgb SyntaxRgb(uint8_t cls, bool dark, bool *bold) {
    if (bold) *bold = false;
    // tema escuro: no estilo do VS Code Dark+; tema claro: tons mais fechados
    switch (cls) {
    case kSynKeyword: if (bold) *bold = false; return dark ? Rgb{86, 156, 214} : Rgb{0, 0, 200};
    case kSynKeyword2: return dark ? Rgb{78, 201, 176} : Rgb{0, 128, 128};
    case kSynDirective: return dark ? Rgb{197, 134, 192} : Rgb{175, 0, 219};
    case kSynComment: return dark ? Rgb{106, 153, 85} : Rgb{0, 128, 0};
    case kSynString: return dark ? Rgb{206, 145, 120} : Rgb{163, 21, 21};
    case kSynNumber: return dark ? Rgb{181, 206, 168} : Rgb{9, 134, 88};
    case kSynLabel: return dark ? Rgb{220, 220, 170} : Rgb{121, 94, 38};
    case kSynRegister: return dark ? Rgb{156, 220, 254} : Rgb{0, 16, 128};
    case kSynOperator: return dark ? Rgb{212, 212, 212} : Rgb{60, 60, 60};
    case kSynLineNo: return dark ? Rgb{133, 133, 133} : Rgb{120, 120, 120};
    case kSynVariable: return dark ? Rgb{156, 220, 254} : Rgb{0, 16, 128};
    case kSynBios: if (bold) *bold = false; return dark ? Rgb{79, 193, 255} : Rgb{0, 112, 193};
    case kSynHeading: if (bold) *bold = true; return dark ? Rgb{86, 156, 214} : Rgb{0, 0, 200};
    default: return dark ? Rgb{212, 212, 212} : Rgb{30, 30, 30};
    }
}

PalRgb PaletteRgb(int scheme, uint8_t style) {
    if (style >= kSynKeywordSt && style < kStyleCount) { // destaque: a cor da classe sobre o fundo do texto do esquema
        const PalRgb base = PaletteRgb(scheme, kText);
        PalRgb p = base;
        bool bold = false;
        p.fg = SyntaxRgb(static_cast<uint8_t>(style - kSynKeywordSt + 1), true, &bold);
        p.bold = bold;
        return p;
    }
    const Rgb grey{170, 170, 170}, dark{85, 85, 85}, black{0, 0, 0}, white{255, 255, 255};
    const Rgb blue{0, 0, 170}, cyan{0, 170, 170}, yellow{255, 255, 85}, red{170, 0, 0}, lred{255, 85, 85};
    if (scheme == 1 || scheme == 2) {
        const bool c2 = scheme == 2;
        const Rgb panel = c2 ? cyan : grey;
        const Rgb hot = c2 ? yellow : red;
        switch (style) {
        case kText: return {white, blue};
        case kBar: case kTitleBar: return {black, panel};
        case kBarHot: return {hot, panel};
        case kBarSel: return {white, black};
        case kBlockSel: return {black, cyan};
        case kBlockMark: return {yellow, red, true};
        case kScrollTrack: return {cyan, blue};
        case kScrollThumb: case kScrollArrow: return {black, panel};
        case kPanel: return {black, panel};
        case kPanelHot: return {c2 ? yellow : lred, panel, true};
        case kPanelSel: return {white, black};
        case kPanelOff: return {dark, panel};
        case kPanelField: return {black, panel, false, true};
        case kShadow: return {dark, black};
        case kBracket: return {black, yellow};
        default: return {white, blue};
        }
    }
    // LCD, VGA monocromatico e monocromatico: texto claro no preto, barras e caixas cinza (como as capturas do NE 2)
    switch (style) {
    case kText: return {grey, black};
    case kBar: case kTitleBar: return {black, grey};
    case kBarHot: return {black, grey, true};
    case kBarSel: return {grey, black};
    case kBlockSel: return {black, grey};
    case kBlockMark: return {white, black, true};
    case kScrollTrack: return {dark, black};
    case kScrollThumb: case kScrollArrow: return {black, grey};
    case kPanel: return {black, grey};
    case kPanelHot: return {white, black};
    case kPanelSel: return {grey, black};
    case kPanelOff: return {dark, grey};
    case kPanelField: return {black, grey, false, true};
    case kShadow: return {dark, black};
    case kBracket: return {black, white};
    default: return {grey, black};
    }
}

} // namespace ed
