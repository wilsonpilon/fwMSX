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
    {Cmd::Complete, "complete", true, D::None, true},
    {Cmd::Print, "print", false, D::None, true},
    {Cmd::PrintBlock, "print-block", false, D::None, true},
    {Cmd::EjectPage, "eject-page", false, D::None, false},
    {Cmd::PrintSetup, "print-setup", false, D::PrintSetup, true},
    {Cmd::Information, "information", false, D::Information, true},
    {Cmd::Outline, "outline", false, D::Outline, false},
    {Cmd::About, "about", false, D::About, true},
    {Cmd::Language, "language", false, D::Language, true},
    {Cmd::Assemble, "assemble", false, D::None, true},
    {Cmd::RunMsx, "run-msx", false, D::None, true},
    {Cmd::NextError, "next-error", false, D::None, true},
    {Cmd::PrevError, "prev-error", false, D::None, true},
    {Cmd::ErrorList, "error-list", false, D::None, true},
    {Cmd::HelpWord, "help-word", false, D::None, true},
    {Cmd::BadigSetup, "badig-setup", false, D::None, true},
    {Cmd::BasicLine, "basic-line", false, D::None, true},
    {Cmd::ProjectNew, "project-new", false, D::None, true},
    {Cmd::ProjectOpen, "project-open", false, D::None, true},
    {Cmd::ProjectClose, "project-close", false, D::None, true},
    {Cmd::ProjectFiles, "project-files", false, D::None, true},
    {Cmd::ProjectAdd, "project-add", false, D::None, true},
    {Cmd::ProjectSync, "project-sync", false, D::None, true},
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
    // {rotulo, letra, coluna, itens}; item = {rotulo, letra, atalho, comando, arg, separador depois, escondido}
    // Organizacao "moderna" (1.43.8): oito menus como nos IDEs atuais. Os dez menus do Norton Editor foram juntados -- Block e Text-layout em
    // Edit/Tools, Print em File, Options em Tools, Build em Run -- mas as TECLAS DIRETAS do NE (Alt-K, Alt-B..., Ctrl-F...) nao mudaram, e a
    // letra de cada item continua a mesma dentro do menu que o recebeu (Alt-E S = marcar bloco, como era Alt-B S).
    static const std::vector<Menu> menus = {
        {"File", 'f', 3,
         {
             {"New", 'n', "", Cmd::New, 0, false, false},
             {"Open...", 'o', "", Cmd::Open, 0, false, false},
             {"Save", 's', "", Cmd::Save, 0, false, false},
             {"Save As...", 'a', "", Cmd::SaveAs, 0, false, false},
             {"Merge...", 'm', "", Cmd::Merge, 0, true, false},
             {"Close window", 'c', "", Cmd::CloseWindow, 0, true, false},
             {"Load more", 'l', "", Cmd::LoadMore, 0, false, false},
             {"Write to cursor", 'w', "", Cmd::WriteToCursor, 0, false, false},
             {"Top of file", 't', "", Cmd::TopOfFile, 0, true, false},
             {"Print", 'p', "", Cmd::Print, 0, false, false},
             {"Print Block", 'b', "", Cmd::PrintBlock, 0, false, false},
             {"Eject page", 'e', "", Cmd::EjectPage, 0, false, false},
             {"Print setup...", 'u', "", Cmd::PrintSetup, 0, true, false},
             {"eXit", 'x', "", Cmd::Exit, 0, false, false},
         }},
        {"Edit", 'e', 9,
         {
             {"Undo", 'z', "Alt-BS", Cmd::Undo, 0, false, false},
             {"Redo", 'y', "Alt-S-BS", Cmd::Redo, 0, true, false},
             {"Copy", 'c', "", Cmd::BlockCopy, 0, false, false},
             {"Move", 'm', "", Cmd::BlockMove, 0, false, false},
             {"Delete", 'd', "", Cmd::BlockDelete, 0, true, false},
             {"Set marker", 's', "", Cmd::BlockSetMarker, 0, false, false},
             {"Remove markers", 'r', "", Cmd::BlockRemoveMarkers, 0, false, false},
             {"Find marker", 'f', "", Cmd::BlockFindMarker, 0, true, false},
             {"Indent...", 'i', "", Cmd::BlockIndent, 0, false, false},
             {"Unindent...", 'u', "", Cmd::BlockUnindent, 0, false, false},
             {"Output to file...", 'o', "", Cmd::BlockOutput, 0, true, false},
             {"Paragraph format", 'p', "", Cmd::ParagraphFormat, 0, false, false},
             {"Document format...", 'g', "", Cmd::DocumentFormat, 0, false, false},
             {"Center line", 't', "", Cmd::CenterLine, 0, false, false},
             {nullptr, 0x7F, "", Cmd::BlockDeleteNoAsk, 0, false, true},
             {nullptr, 'l', "", Cmd::BlockLine, 0, false, true},
             {nullptr, 'e', "", Cmd::BlockToEol, 0, false, true},
         }},
        {"Search", 's', 15,
         {
             {"Find string...", 'f', "Ctrl-F", Cmd::Find, 0, false, false},
             {"Find and Replace...", 'r', "Ctrl-R", Cmd::Replace, 0, false, false},
             {"Continue fwd", 'c', "Alt-C", Cmd::ContinueFwd, 0, false, false},
             {"Continue rev", 0, "Ctrl-C", Cmd::ContinueRev, 0, true, false},
             {"Line number...", 'l', "", Cmd::LineNumber, 0, false, false},
             {"Matching bracket", 0, "Ctrl-]", Cmd::MatchBracket, 0, false, false},
             {"Window Differences", 'd', "", Cmd::WindowDiff, 0, true, false},
             {"Text in Block", 'b', "", Cmd::TextInBlock, 0, false, false},
             {"Complete word", 'w', "Alt-/", Cmd::Complete, 0, false, false},
             {"Block Marker", 'm', "", Cmd::BlockFindMarker, 0, false, false},
         }},
        {"View", 'v', 23,
         {
             {"Information display", 'i', "", Cmd::Information, 0, false, false},
             {"Outline display", 'o', "", Cmd::Outline, 0, true, false},
             {"Language...", 'l', "", Cmd::Language, 0, false, false},
             {"2nd window...", '2', "F2", Cmd::Window2, 0, false, false},
             {nullptr, 'a', "", Cmd::About, 0, false, true},
         }},
        // O menu Project (E6 parte 2): o .msxproject aberto no editor (doc/project-spec.md)
        {"Project", 'j', 29,
         {
             {"New project...", 'n', "", Cmd::ProjectNew, 0, false, false},
             {"Open project...", 'o', "", Cmd::ProjectOpen, 0, false, false},
             {"Close project", 'c', "", Cmd::ProjectClose, 0, true, false},
             {"Files...", 'f', "", Cmd::ProjectFiles, 0, false, false},
             {"Add current file", 'a', "", Cmd::ProjectAdd, 0, false, false},
             {"Sync...", 's', "", Cmd::ProjectSync, 0, false, false},
         }},
        // O menu Run (era o Build da E6): montar/converter, executar no MSX e as mensagens
        {"Run", 'r', 38,
         {
             {"Assemble / Convert", 'a', "F9", Cmd::Assemble, 0, false, false},
             {"Run on MSX", 'r', "F5", Cmd::RunMsx, 0, true, false},
             {"Next message", 'n', "F8", Cmd::NextError, 0, false, false},
             {"Previous message", 'p', "Shift-F8", Cmd::PrevError, 0, false, false},
             {"Messages...", 'm', "", Cmd::ErrorList, 0, true, false},
             {"BASIC line...", 'l', "", Cmd::BasicLine, 0, false, false},
             {"Dignified setup...", 'd', "", Cmd::BadigSetup, 0, false, false},
         }},
        // O menu Tools: as opcoes do editor, as ferramentas de texto (o antigo Text-layout) e o shell
        {"Tools", 't', 43,
         {
             {"Display...", 'd', "", Cmd::DisplaySetup, 0, false, false},
             {"Mouse...", 'm', "", Cmd::MouseSetup, 0, false, false},
             {"Editor...", 'e', "", Cmd::EditorSetup, 0, false, false},
             {"Save configuration...", 's', "", Cmd::SaveConfig, 0, true, false},
             {"Text layout setup...", 't', "", Cmd::TextSetup, 0, false, false},
             {"Word wrap", 'w', "", Cmd::WordWrap, 0, false, false},
             {"Auto indent", 'a', "", Cmd::AutoIndent, 0, true, false},
             {"Expand tabs...", 'x', "", Cmd::ExpandTabs, 0, false, false},
             {"Put in Tabs...", 'p', "", Cmd::PutInTabs, 0, false, false},
             {"Insert page breaks...", 'i', "", Cmd::InsertPageBreaks, 0, false, false},
             {"Remove page breaks...", 'b', "", Cmd::RemovePageBreaks, 0, true, false},
             {"DOS shell", 'h', "", Cmd::DosShell, 0, false, false},
         }},
        // O menu "F1=Help" (so' abre por F1 ou pelo mouse; nao tem letra de Alt). Os topicos 2..8 sao os sete menus acima, na mesma ordem.
        {"F1=Help", 0, 70,
         {
             {"General", 'g', "", Cmd::Help, 0, false, false},
             {"Keys", 'k', "", Cmd::Help, 1, false, false},
             {"File", 'f', "", Cmd::Help, 2, false, false},
             {"Edit", 'e', "", Cmd::Help, 3, false, false},
             {"Search", 's', "", Cmd::Help, 4, false, false},
             {"View", 'v', "", Cmd::Help, 5, false, false},
             {"Project", 'j', "", Cmd::Help, 6, false, false},
             {"Run", 'r', "", Cmd::Help, 7, false, false},
             {"Tools", 't', "", Cmd::Help, 8, true, false},
             {"On word", 'w', "Ctrl-F1", Cmd::HelpWord, 0, false, false},
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
        {"A-/", Cmd::Complete, true}, {"C-Space", Cmd::Complete, true},
        // menu Build (fase E6)
        {"F9", Cmd::Assemble, true}, {"F5", Cmd::RunMsx, true}, {"F8", Cmd::NextError, true}, {"S-F8", Cmd::PrevError, true},
        {"C-F1", Cmd::HelpWord, true},
        // --- alternativas modernas (o Norton Editor nao tem) ---
        {"A-BS", Cmd::Undo, true}, {"A-S-BS", Cmd::Redo, true},
        {"C-Ins", Cmd::Copy, true}, {"S-Ins", Cmd::Paste, true}, {"S-Del", Cmd::Cut, true},
        {"S-Left", Cmd::SelLeft, true}, {"S-Right", Cmd::SelRight, true},
        {"S-Up", Cmd::SelUp, true}, {"S-Down", Cmd::SelDown, true},
        {"S-Home", Cmd::SelHome, true}, {"S-End", Cmd::SelEnd, true},
        {"S-PgUp", Cmd::SelPgUp, true}, {"S-PgDn", Cmd::SelPgDn, true},
        {"C-S-Left", Cmd::SelWordLeft, true}, {"C-S-Right", Cmd::SelWordRight, true},
        // --- alternativas dos editores graficos (nao conflitam com o NE) ---
        {"C-v", Cmd::Paste, true}, {"C-x", Cmd::Cut, true}, {"C-s", Cmd::Save, true}, {"C-n", Cmd::New, true}, {"C-o", Cmd::Open, true},
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
