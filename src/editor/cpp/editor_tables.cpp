// fwMSX -- editor de textos: tabelas de DADOS (comandos, menus, teclas). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// As teclas e os mnemonicos de menu seguem o Norton Editor 2 (doc/editor-spec.md, secoes 1b.1 e 1b.2); as teclas
// marcadas como alternativas (alt = true) sao as modernas e as que os terminais entregam melhor.
#include "editor.h"

#include <cctype>
#include <cstring>

namespace ed {

namespace {

using D = Dialog;

// destrutivo: muda o texto ou o arquivo (some no modo somente leitura). implementado: false = so' o item existe.
const CmdInfo kInfos[] = {
    {Cmd::None, "none", false, D::None, true},
    {Cmd::Left, "left", false, D::None, true},
    {Cmd::Right, "right", false, D::None, true},
    {Cmd::Up, "up", false, D::None, true},
    {Cmd::Down, "down", false, D::None, true},
    {Cmd::WordLeft, "word-left", false, D::None, true},
    {Cmd::WordRight, "word-right", false, D::None, true},
    {Cmd::Home, "home", false, D::None, true},
    {Cmd::End, "end", false, D::None, true},
    {Cmd::PgUp, "page-up", false, D::None, true},
    {Cmd::PgDn, "page-down", false, D::None, true},
    {Cmd::ScreenTop, "screen-top", false, D::None, true},
    {Cmd::ScreenBottom, "screen-bottom", false, D::None, true},
    {Cmd::FileTop, "file-top", false, D::None, true},
    {Cmd::FileEnd, "file-end", false, D::None, true},
    {Cmd::ScrollUp, "scroll-up", false, D::None, true},
    {Cmd::ScrollDown, "scroll-down", false, D::None, true},
    {Cmd::SelLeft, "sel-left", false, D::None, true},
    {Cmd::SelRight, "sel-right", false, D::None, true},
    {Cmd::SelUp, "sel-up", false, D::None, true},
    {Cmd::SelDown, "sel-down", false, D::None, true},
    {Cmd::SelHome, "sel-home", false, D::None, true},
    {Cmd::SelEnd, "sel-end", false, D::None, true},
    {Cmd::SelPgUp, "sel-page-up", false, D::None, true},
    {Cmd::SelPgDn, "sel-page-down", false, D::None, true},
    {Cmd::SelWordLeft, "sel-word-left", false, D::None, true},
    {Cmd::SelWordRight, "sel-word-right", false, D::None, true},
    {Cmd::DelLeft, "delete-left", true, D::None, true},
    {Cmd::DelRight, "delete-right", true, D::None, true},
    {Cmd::DelWordLeft, "delete-word-left", true, D::None, true},
    {Cmd::DelWordRight, "delete-word-right", true, D::None, true},
    {Cmd::DelToBol, "delete-to-bol", true, D::None, true},
    {Cmd::DelToEol, "delete-to-eol", true, D::None, true},
    {Cmd::KillLine, "kill-line", true, D::None, true},
    {Cmd::Undelete, "undelete", true, D::None, true},
    {Cmd::FlipCaseBack, "flip-case-back", true, D::None, true},
    {Cmd::FlipCaseFwd, "flip-case-fwd", true, D::None, true},
    {Cmd::Literal, "literal", true, D::None, true},
    {Cmd::Enter, "enter", true, D::None, true},
    {Cmd::EnterIndent, "enter-indent", true, D::None, true},
    {Cmd::Tab, "tab", true, D::None, true},
    {Cmd::Unindent, "unindent-line", true, D::None, true},
    {Cmd::ToggleInsert, "toggle-insert", false, D::None, true},
    {Cmd::Undo, "undo", true, D::None, true},
    {Cmd::Redo, "redo", true, D::None, true},
    {Cmd::Copy, "copy", false, D::None, true}, // copiar nao destroi nada: vale na leitura
    {Cmd::Cut, "cut", true, D::None, true},
    {Cmd::Paste, "paste", true, D::None, true},
    {Cmd::DividerUp, "divider-up", false, D::None, false},
    {Cmd::DividerDown, "divider-down", false, D::None, false},
    {Cmd::New, "new", true, D::None, true},
    {Cmd::Open, "open", false, D::Open, true},
    {Cmd::Save, "save", true, D::None, true},
    {Cmd::SaveAs, "save-as", false, D::SaveAs, true}, // grava uma COPIA: vale na leitura
    {Cmd::Merge, "merge", true, D::Merge, true},
    {Cmd::Window2, "window-2", false, D::Window2, false},
    {Cmd::CloseWindow, "close-window", false, D::None, false},
    {Cmd::LoadMore, "load-more", false, D::None, false},
    {Cmd::WriteToCursor, "write-to-cursor", true, D::None, false},
    {Cmd::TopOfFile, "top-of-file", false, D::None, false},
    {Cmd::DosShell, "shell", false, D::None, false},
    {Cmd::Exit, "exit", false, D::ConfirmExit, true},
    {Cmd::BlockCopy, "block-copy", true, D::None, true},
    {Cmd::BlockMove, "block-move", true, D::None, true},
    {Cmd::BlockDelete, "block-delete", true, D::None, true},
    {Cmd::BlockDeleteNoAsk, "block-delete-now", true, D::None, true},
    {Cmd::BlockSetMarker, "block-set-marker", false, D::None, true},
    {Cmd::BlockRemoveMarkers, "block-remove-markers", false, D::None, true},
    {Cmd::BlockFindMarker, "block-find-marker", false, D::None, true},
    {Cmd::BlockIndent, "block-indent", true, D::BlockIndent, true},
    {Cmd::BlockUnindent, "block-unindent", true, D::BlockUnindent, true},
    {Cmd::BlockOutput, "block-output", false, D::BlockOutput, true},
    {Cmd::BlockLine, "block-line", false, D::None, true},
    {Cmd::BlockToEol, "block-to-eol", false, D::None, true},
    {Cmd::ParagraphFormat, "paragraph-format", true, D::None, false},
    {Cmd::DocumentFormat, "document-format", true, D::FormatDocument, false},
    {Cmd::CenterLine, "center-line", true, D::None, true},
    {Cmd::TextSetup, "text-setup", false, D::TextSetup, true},
    {Cmd::WordWrap, "word-wrap", true, D::None, true},
    {Cmd::InsertPageBreaks, "insert-page-breaks", true, D::InsertPageBreaks, true},
    {Cmd::RemovePageBreaks, "remove-page-breaks", true, D::RemovePageBreaks, true},
    {Cmd::ExpandTabs, "expand-tabs", true, D::ExpandTabs, true},
    {Cmd::PutInTabs, "put-in-tabs", true, D::PutInTabs, true},
    {Cmd::AutoIndent, "auto-indent", true, D::None, true},
    {Cmd::Find, "find", false, D::Find, true},
    {Cmd::Replace, "replace", true, D::Replace, true},
    {Cmd::ContinueFwd, "continue-fwd", false, D::None, true},
    {Cmd::ContinueRev, "continue-rev", false, D::None, true},
    {Cmd::LineNumber, "line-number", false, D::LineNumber, true},
    {Cmd::MatchBracket, "match-bracket", false, D::None, true},
    {Cmd::WindowDiff, "window-differences", false, D::None, false},
    {Cmd::TextInBlock, "text-in-block", false, D::None, true},
    {Cmd::Print, "print", false, D::None, false},
    {Cmd::PrintBlock, "print-block", false, D::None, false},
    {Cmd::EjectPage, "eject-page", false, D::None, false},
    {Cmd::PrintSetup, "print-setup", false, D::PrintSetup, true},
    {Cmd::Information, "information", false, D::Information, true},
    {Cmd::Outline, "outline", false, D::Outline, false},
    {Cmd::About, "about", false, D::About, true},
    {Cmd::DisplaySetup, "display-setup", false, D::DisplaySetup, true},
    {Cmd::MouseSetup, "mouse-setup", false, D::MouseSetup, true},
    {Cmd::EditorSetup, "editor-setup", false, D::EditorSetup, true},
    {Cmd::SaveConfig, "save-config", false, D::SaveConfig, true},
    {Cmd::Help, "help", false, D::Help, true},
    {Cmd::MenuOpen, "menu", false, D::None, true},
};

} // namespace

const CmdInfo &Info(Cmd c) {
    for (const CmdInfo &i : kInfos) {
        if (i.cmd == c) return i;
    }
    return kInfos[0];
}

const std::vector<Menu> &Menus() {
    // {rotulo, letra, atalho, comando, arg, separador depois, escondido}
    static const std::vector<Menu> menus = {
        {"File", 'f', 3,
         {
             {"New", 'n', "", Cmd::New, 0, false, false},
             {"Open...", 'o', "", Cmd::Open, 0, false, false},
             {"Save", 's', "", Cmd::Save, 0, false, false},
             {"Save As...", 'a', "", Cmd::SaveAs, 0, false, false},
             {"Merge...", 'm', "", Cmd::Merge, 0, true, false},
             {"2nd window...", '2', "F2", Cmd::Window2, 0, false, false},
             {"Close window", 'c', "", Cmd::CloseWindow, 0, true, false},
             {"Load more", 'l', "", Cmd::LoadMore, 0, false, false},
             {"Write to cursor", 'w', "", Cmd::WriteToCursor, 0, false, false},
             {"Top of file", 't', "", Cmd::TopOfFile, 0, true, false},
             {"DOS shell", 'd', "", Cmd::DosShell, 0, false, false},
             {"eXit", 'x', "", Cmd::Exit, 0, false, false},
         }},
        {"Block", 'b', 9,
         {
             {"Copy", 'c', "", Cmd::BlockCopy, 0, false, false},
             {"Move", 'm', "", Cmd::BlockMove, 0, false, false},
             {"Delete", 'd', "", Cmd::BlockDelete, 0, true, false},
             {"Set marker", 's', "", Cmd::BlockSetMarker, 0, false, false},
             {"Remove markers", 'r', "", Cmd::BlockRemoveMarkers, 0, false, false},
             {"Find marker", 'f', "", Cmd::BlockFindMarker, 0, true, false},
             {"Indent...", 'i', "", Cmd::BlockIndent, 0, false, false},
             {"Unindent...", 'u', "", Cmd::BlockUnindent, 0, true, false},
             {"Output to file...", 'o', "", Cmd::BlockOutput, 0, false, false},
             {nullptr, 0x7F, "", Cmd::BlockDeleteNoAsk, 0, false, true},
             {nullptr, 'l', "", Cmd::BlockLine, 0, false, true},
             {nullptr, 'e', "", Cmd::BlockToEol, 0, false, true},
         }},
        {"Text-layout", 't', 16,
         {
             {"Paragraph format", 'p', "", Cmd::ParagraphFormat, 0, false, false},
             {"Document format...", 'd', "", Cmd::DocumentFormat, 0, false, false},
             {"Center line", 'c', "", Cmd::CenterLine, 0, true, false},
             {"Setup...", 's', "", Cmd::TextSetup, 0, false, false},
             {"Word wrap", 'w', "", Cmd::WordWrap, 0, true, false},
             {"Insert page breaks...", 'i', "", Cmd::InsertPageBreaks, 0, false, false},
             {"Remove page breaks...", 'b', "", Cmd::RemovePageBreaks, 0, true, false},
             {"Expand tabs...", 'e', "", Cmd::ExpandTabs, 0, false, false},
             {"Put in Tabs...", 't', "", Cmd::PutInTabs, 0, true, false},
             {"Auto indent", 'a', "", Cmd::AutoIndent, 0, false, false},
         }},
        {"Search", 's', 29,
         {
             {"Find string...", 'f', "Ctrl-F", Cmd::Find, 0, false, false},
             {"Find and Replace...", 'r', "Ctrl-R", Cmd::Replace, 0, false, false},
             {"Continue fwd", 'c', "Alt-C", Cmd::ContinueFwd, 0, false, false},
             {"Continue rev", 0, "Ctrl-C", Cmd::ContinueRev, 0, true, false},
             {"Line number...", 'l', "", Cmd::LineNumber, 0, false, false},
             {"Matching bracket", 0, "Ctrl-]", Cmd::MatchBracket, 0, false, false},
             {"Window Differences", 'd', "", Cmd::WindowDiff, 0, true, false},
             {"Text in Block", 'b', "", Cmd::TextInBlock, 0, false, false},
             {"Block Marker", 'm', "", Cmd::BlockFindMarker, 0, false, false},
         }},
        {"Print", 'p', 37,
         {
             {"Print", 'p', "", Cmd::Print, 0, false, false},
             {"Print Block", 'b', "", Cmd::PrintBlock, 0, false, false},
             {"Eject page", 'e', "", Cmd::EjectPage, 0, true, false},
             {"Setup...", 's', "", Cmd::PrintSetup, 0, false, false},
         }},
        {"View", 'v', 44,
         {
             {"Information display", 'i', "", Cmd::Information, 0, false, false},
             {"Outline display", 'o', "", Cmd::Outline, 0, false, false},
             {nullptr, 'a', "", Cmd::About, 0, false, true},
         }},
        {"Options", 'o', 50,
         {
             {"Display...", 'd', "", Cmd::DisplaySetup, 0, false, false},
             {"Mouse...", 'm', "", Cmd::MouseSetup, 0, false, false},
             {"Editor...", 'e', "", Cmd::EditorSetup, 0, true, false},
             {"Save configuration...", 's', "", Cmd::SaveConfig, 0, false, false},
         }},
        // O menu "F1=Help" (so' abre por F1 ou pelo mouse; nao tem letra de Alt)
        {"F1=Help", 0, 70,
         {
             {"General", 'g', "", Cmd::Help, 0, false, false},
             {"Keys", 'k', "", Cmd::Help, 1, false, false},
             {"File", 'f', "", Cmd::Help, 2, false, false},
             {"Block", 'b', "", Cmd::Help, 3, false, false},
             {"Text-layout", 't', "", Cmd::Help, 4, false, false},
             {"Search", 's', "", Cmd::Help, 5, false, false},
             {"Print", 'p', "", Cmd::Help, 6, false, false},
             {"View", 'v', "", Cmd::Help, 7, false, false},
             {"Options", 'o', "", Cmd::Help, 8, true, false},
             {"About", 'a', "", Cmd::About, 0, false, false},
         }},
    };
    return menus;
}

const std::vector<KeyBinding> &Bindings() {
    static const std::vector<KeyBinding> b = {
        // --- Norton Editor 2 (Help on keys) ---
        {"Left", Cmd::Left, false}, {"Right", Cmd::Right, false}, {"Up", Cmd::Up, false}, {"Down", Cmd::Down, false},
        {"C-Left", Cmd::WordLeft, false}, {"C-Right", Cmd::WordRight, false},
        {"Home", Cmd::Home, false}, {"End", Cmd::End, false}, {"PgUp", Cmd::PgUp, false}, {"PgDn", Cmd::PgDn, false},
        {"C-PgUp", Cmd::ScreenTop, false}, {"C-PgDn", Cmd::ScreenBottom, false},
        {"C-Home", Cmd::FileTop, false}, {"C-End", Cmd::FileEnd, false},
        {"A-z", Cmd::ScrollUp, false}, {"C-z", Cmd::ScrollDown, false},
        {"BS", Cmd::DelLeft, false}, {"Del", Cmd::DelRight, false},
        {"C-w", Cmd::DelWordLeft, false}, {"A-w", Cmd::DelWordRight, false},
        {"C-l", Cmd::DelToBol, false}, {"A-l", Cmd::DelToEol, false},
        {"A-k", Cmd::KillLine, false}, {"C-u", Cmd::Undelete, false},
        {"C-f", Cmd::Find, false}, {"C-r", Cmd::Replace, false},
        {"A-c", Cmd::ContinueFwd, false}, {"C-c", Cmd::ContinueRev, false},
        {"C-]", Cmd::MatchBracket, false},
        {"C-a", Cmd::FlipCaseBack, false}, {"A-a", Cmd::FlipCaseFwd, false},
        {"C-p", Cmd::Literal, false},
        {"C-Enter", Cmd::EnterIndent, false},
        {"C-BS", Cmd::Unindent, false}, {"S-Tab", Cmd::Unindent, false},
        {"C-2", Cmd::DividerUp, false}, {"A-2", Cmd::DividerDown, false},
        {"Ins", Cmd::ToggleInsert, false},
        {"Enter", Cmd::Enter, false}, {"Tab", Cmd::Tab, false},
        {"F1", Cmd::Help, false}, {"F2", Cmd::Window2, false},
        // --- alternativas: o que o terminal entrega mal (Ctrl+numero, Ctrl+Enter, Ctrl+PgUp...) ---
        {"A-Left", Cmd::WordLeft, true}, {"A-Right", Cmd::WordRight, true},
        {"A-PgUp", Cmd::ScreenTop, true}, {"A-PgDn", Cmd::ScreenBottom, true},
        {"A-Home", Cmd::FileTop, true}, {"A-End", Cmd::FileEnd, true},
        {"A-Enter", Cmd::EnterIndent, true},
        {"A-Up", Cmd::DividerUp, true}, {"A-Down", Cmd::DividerDown, true},
        {"F10", Cmd::MenuOpen, true},
        // --- alternativas modernas (o Norton Editor nao tem) ---
        {"A-BS", Cmd::Undo, true}, {"A-S-BS", Cmd::Redo, true},
        {"C-Ins", Cmd::Copy, true}, {"S-Ins", Cmd::Paste, true}, {"S-Del", Cmd::Cut, true},
        {"S-Left", Cmd::SelLeft, true}, {"S-Right", Cmd::SelRight, true},
        {"S-Up", Cmd::SelUp, true}, {"S-Down", Cmd::SelDown, true},
        {"S-Home", Cmd::SelHome, true}, {"S-End", Cmd::SelEnd, true},
        {"S-PgUp", Cmd::SelPgUp, true}, {"S-PgDn", Cmd::SelPgDn, true},
        {"C-S-Left", Cmd::SelWordLeft, true}, {"C-S-Right", Cmd::SelWordRight, true},
    };
    return b;
}

std::string CanonicalKey(const std::string &name) {
    bool c = false;
    bool a = false;
    bool s = false;
    size_t i = 0;
    while (name.size() - i > 2 && name[i + 1] == '-') {
        const char m = name[i];
        if (m == 'C') {
            c = true;
        } else if (m == 'A') {
            a = true;
        } else if (m == 'S') {
            s = true;
        } else {
            break;
        }
        i += 2;
    }
    std::string base = name.substr(i);
    if (base.empty()) return {};
    if (base.size() == 1 && std::isalpha(static_cast<unsigned char>(base[0]))) {
        base[0] = static_cast<char>(std::tolower(static_cast<unsigned char>(base[0])));
    }
    std::string out;
    if (c) out += "C-";
    if (a) out += "A-";
    if (s) out += "S-";
    return out + base;
}

} // namespace ed
