//
// fwMSX - ponto de entrada do projeto (linguagem: C++).
//
// Created by barney on 27/09/2026.
//
// Este e o "main" do sistema.
//
// Sem argumento nenhum, abre a maquina MSX1 completa em janela (os mesmos
// padroes de "--msx" -- ver doc/SPEC.md, secao 5.1, decisao registrada em
// 2026-09-29 e implementada quando o core de emulacao passou a existir de
// verdade).
//
// Com argumentos que nao batem com nenhum modo conhecido (ex.:
// "fwMSX.exe NomeDoProduto 1 2 3"), cai no esqueleto historico do projeto:
//   1. Imprime o aviso de copyright;
//   2. Le (ou assume os valores padrao de version.h para) o nome do
//      produto e a versao (major.minor.patch), recebidos como parametros
//      de main();
//   3. Aciona, em sequencia, o "carregamento" de um modulo escrito em
//      cada uma das quatro linguagens do projeto -- C++, C, Assembly e
//      Fortran --, cada um imprimindo sua propria mensagem com a versao
//      entre colchetes e devolvendo uma assinatura hexadecimal propria;
//   4. Imprime o resumo final com as assinaturas coletadas.
//
// Ver doc/SPEC.md para a especificacao completa do projeto e
// doc/MANUAL.md para como compilar e executar.
//
// "fwmsx --msxdisk <resto dos argumentos>" repassa direto para o
// utilitario de disco embutido (msxdisk::RunEntryPoint -- os mesmos
// pontos de entrada de CLI/shell/TUI/GUI do msxdisk.exe standalone). Ver
// doc/SPEC.md, secao 5.1, e doc/msxdisk-spec.md, Fase 5c.
//
// "fwmsx --msx" liga a maquina MSX1 completa numa janela com teclado do
// host (ou sem janela, com --frames/--shot) -- ver doc/machine-spec.md.
//
// "fwmsx --term [opcoes do --msx]" roda a maquina DENTRO do terminal (FTXUI): menu, linha de comando e
// os mesmos atalhos da janela -- ver doc/term-spec.md.
//
// "fwmsx --tui" abre a TUI de MENUS: os comandos do console em menus, controlando o emulador em JANELA
// pela ponte de controle -- ver doc/tui-spec.md.
//
// "fwmsx --cli" abre o console (REPL): inicia o emulador (emu start), conecta num que ja' esta'
// aberto (emu attach) e encaminha comandos pela ponte de controle -- ver doc/repl-spec.md.
//
// "fwmsx --disknew <arq.dsk> <ss525|ds525|ss35|ds35>" cria um disquete em branco
// e formatado -- ver doc/diskfmt-spec.md.
//
// "fwmsx --z80dbg" abre o REPL de depuracao do nucleo Z80 (RAM plana de
// teste, sem maquina MSX ainda) -- ver doc/z80-core-spec.md, Fase 4.
// "fwmsx --z80dbg --slots" liga o mapa de memoria de verdade (slots/
// subslots) em vez da RAM plana -- ver doc/memory-map-spec.md, Fase 1.
//
// "fwmsx --cas pack --tipo bin|bas --nome NOME ..." empacota um .BIN/.BAS
// solto num .TSX/.CAS valido sem passar pelo emulador; "fwmsx --cas list
// <arquivo>" lista os arquivos de uma fita -- ver doc/tape-spec.md,
// secao 8.
//
// "fwmsx --fitadb <comando>" cadastra metadados (titulo, empresa, ano,
// SHA-1) das fitas que o usuario ja' tem no disco -- SEM download nenhum
// (o site de referencia nao publica termos de uso ainda) -- ver
// doc/tape-spec.md, secao 11.
//

#include "romdb/cli.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "version.h"

#include "init_cpp.h"
#include "init_c.h"
#include "init_asm.h"
#include "init_fortran.h"

#include "msxdisk/entry.h"
#include "diskfmt/cpp/cli.h"
#include "repl/repl.h"
#include "asm/cpp/cli.h"
#include "badig/cpp/badig.h"
#include "mamute/cpp/mamute_config.h"
#include "splash/splash.h"
#include "term/term_app.h"
#include "tui/tui_app.h"
#include "editor/cpp/editor_tui.h"
#include "editor/gui/editor_window.h"
#include "machine/cli.h"
#include "tape/cli/cas_tool.h"
#include "tapedb/cli.h"
#include "z80/debug/z80_debug_shell.h"

namespace {

void print_signature(const char* label, std::uint16_t signature) {
    std::cout << "  " << std::left << std::setw(10) << label
               << std::right << "0x" << std::hex << std::uppercase << std::setfill('0')
               << std::setw(4) << signature
               << std::nouppercase << std::dec << std::setfill(' ') << std::endl;
}

#ifdef _WIN32
// O executavel e' do subsistema "windows" (sem console proprio), para o duplo
// clique nao abrir uma janela preta de console antes da janela do emulador.
// Quando ele e' aberto de dentro de um terminal, religa stdout/stderr/stdin
// ao console do terminal -- mas so' as saidas que NAO foram redirecionadas
// (pipe/arquivo continuam intactos, entao "fwMSX.exe ... | tail" funciona).
// true quando este processo e' do subsistema janela E ficou ligado a um terminal interativo (entrada
// do teclado nao redirecionada): ai' o console interativo nao funciona (ver main()).
bool g_gui_on_terminal = false;

// true no fwMSXc.exe: a copia do executavel com o subsistema "console" (o fwMSX.exe e' do subsistema "janela").
bool IsConsoleExe() {
    const auto *dos = reinterpret_cast<const IMAGE_DOS_HEADER *>(GetModuleHandleA(nullptr));
    const auto *nt = reinterpret_cast<const IMAGE_NT_HEADERS *>(reinterpret_cast<const BYTE *>(dos) + dos->e_lfanew);
    return nt->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI;
}

void attach_parent_console() {
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) return; // fwMSXc.exe (console) ja' tem o seu
    auto redirected = [](DWORD which) {
        const HANDLE h = GetStdHandle(which);
        return h != nullptr && h != INVALID_HANDLE_VALUE;
    };
    // replxx (console interativo) usa GetStdHandle direto, nao o stdio do C: entao os handles do
    // sistema tambem precisam apontar para o console, nao so' stdout/stderr/stdin.
    auto console_handle = [](const char *name, DWORD access) {
        return CreateFileA(name, access, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    };
    if (!redirected(STD_OUTPUT_HANDLE)) {
        SetStdHandle(STD_OUTPUT_HANDLE, console_handle("CONOUT$", GENERIC_READ | GENERIC_WRITE));
        freopen("CONOUT$", "w", stdout);
    }
    if (!redirected(STD_ERROR_HANDLE)) {
        SetStdHandle(STD_ERROR_HANDLE, console_handle("CONOUT$", GENERIC_READ | GENERIC_WRITE));
        freopen("CONOUT$", "w", stderr);
    }
    if (!redirected(STD_INPUT_HANDLE)) {
        g_gui_on_terminal = true;
        SetStdHandle(STD_INPUT_HANDLE, console_handle("CONIN$", GENERIC_READ | GENERIC_WRITE));
        freopen("CONIN$", "r", stdin);
    }
    std::ios::sync_with_stdio(true);
    std::cout.clear();
    std::cerr.clear();
    std::cin.clear();
}
#endif

} // namespace

int main(int argc, char* argv[]) {
#ifdef _WIN32
    attach_parent_console();
#endif
    {
        // o mamute.cfg (configuracao inicial do monitor) mora ao lado do executavel
        std::error_code ec;
        mamute::SetConfigDir(std::filesystem::absolute(std::filesystem::path(argv[0]), ec).parent_path().string());
        // o badig.cfg (opcoes do Basic Dignified) tambem mora ao lado do executavel
        badig::SetConfigPath((std::filesystem::absolute(std::filesystem::path(argv[0]), ec).parent_path() / "badig.cfg").string());
    }
    // Sem argumento nenhum, com o core de emulacao (Z80+VDP+PSG+mapa de
    // memoria+maquina) ja existindo de verdade, abre a maquina completa em
    // janela com os padroes de "--msx" (BIOS MSX1 ao lado do executavel,
    // sem cartucho/disco) -- decisao registrada em doc/SPEC.md, secao 5.1.
    // O esqueleto dos quatro modulos (C++/C/Assembly/Fortran) abaixo so e'
    // alcancado quando ha argumentos explicitos que nao sao um dos modos
    // conhecidos (ex.: "fwMSX.exe NomeDoProduto 1 2 3").
    if (argc == 1) {
        return machine::RunMachineCommand({}, argv[0]);
    }
    if (argc > 1 && std::string(argv[1]) == "--msxdisk") {
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        return msxdisk::RunEntryPoint(tokens);
    }
    if (argc > 1 && std::string(argv[1]) == "--msx") {
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        return machine::RunMachineCommand(tokens, argv[0]);
    }
    if (argc > 1 && std::string(argv[1]) == "--romdb") {
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        return romdb::RunRomDbCommand(tokens, argv[0]);
    }
    if (argc > 1 && std::string(argv[1]) == "--z80dbg") {
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        return z80::debug::RunZ80DebugShell(tokens);
    }
    if (argc > 1 && std::string(argv[1]) == "--term") {
#ifdef _WIN32
        if (g_gui_on_terminal) {
            std::cerr << "fwmsx --term: use o fwMSXc.exe (versao de console) para o modo terminal:\n"
                         "  fwMSXc.exe --term" << std::endl;
            return 2;
        }
#endif
        std::vector<std::string> tokens(argv + 2, argv + argc);
        tokens.insert(tokens.begin(), "--term"); // o mesmo analisador do --msx entende as opcoes da maquina
        return machine::RunMachineCommand(tokens, argv[0]);
    }
    if (argc > 1 && std::string(argv[1]) == "--tui") {
#ifdef _WIN32
        if (g_gui_on_terminal) {
            std::cerr << "fwmsx --tui: use o fwMSXc.exe (versao de console) para a TUI de menus:\n"
                         "  fwMSXc.exe --tui" << std::endl;
            return 2;
        }
#endif
        std::vector<std::string> tokens(argv + 2, argv + argc);
        splash::HandleTerminalArgs(tokens, argv[0]); // splash so' com --splash
        return tui::RunTui(tokens, argv[0]);
    }
    // --edit / --view: o editor de textos (estilo Norton Editor 2) em tela cheia no terminal; --view = somente leitura
    if (argc > 1 && (std::string(argv[1]) == "--edit" || std::string(argv[1]) == "--view")) {
        const bool as_view = std::string(argv[1]) == "--view";
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        // Janela x terminal: --gui / --tui escolhem; sem eles, o fwMSX.exe (subsistema janela) abre a JANELA e o
        // fwMSXc.exe (console) e o Linux abrem o TERMINAL. --dump e' sempre do terminal (imprime a tela e sai).
        bool dump = false, want_gui = false, want_tui = false;
        for (const std::string &t : tokens) {
            dump = dump || t == "--dump";
            want_gui = want_gui || t == "--gui";
            want_tui = want_tui || t == "--tui";
        }
        bool use_gui = want_gui;
#ifdef _WIN32
        if (!want_gui && !want_tui && !dump) use_gui = !IsConsoleExe();
        if (!use_gui && g_gui_on_terminal && !dump) {
            std::cerr << "fwmsx " << argv[1] << " --tui: use o fwMSXc.exe (versao de console) para o editor no terminal:\n"
                         "  fwMSXc.exe " << argv[1] << " arquivo" << std::endl;
            return 2;
        }
#endif
        if (use_gui && !dump) return ed::RunEditorWindow(tokens, argv[0], as_view);
        return ed::RunEditorTui(tokens, argv[0], as_view);
    }
    // --mon / --mamute: o console ja' dentro do monitor Z80 (sem emulador = so' offline; com --attach <porta> tambem online)
    if (argc > 1 && (std::string(argv[1]) == "--cli" || std::string(argv[1]) == "--mon" || std::string(argv[1]) == "--mamute")) {
#ifdef _WIN32
        if (g_gui_on_terminal) {
            // O shell nao espera um programa de janela e disputa o teclado com ele: o console
            // interativo ficaria lento, trocando letras e fechando sozinho.
            std::cerr << "fwmsx --cli: use o fwMSXc.exe (versao de console) para o console interativo:\n"
                         "  fwMSXc.exe --cli" << std::endl;
            return 2;
        }
#endif
        std::vector<std::string> tokens(argv + 2, argv + argc);
        if (std::string(argv[1]) != "--cli") tokens.insert(tokens.begin(), "--mon");
        splash::HandleTerminalArgs(tokens, argv[0]); // splash so' com --splash
        return repl::RunReplCommand(tokens, argv[0]);
    }
    if (argc > 1 && std::string(argv[1]) == "--asm") {
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        return fwasm::RunAsmCommand(tokens);
    }
    if (argc > 1 && std::string(argv[1]) == "--badig") { // converte .dmx (Basic Dignified) em .amx e .bmx
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        return badig::RunBadigCommand(tokens, badig::ConfigPath());
    }
    if (argc > 1 && std::string(argv[1]) == "--disknew") {
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        return diskfmt::RunDiskNewCommand(tokens);
    }
    if (argc > 1 && std::string(argv[1]) == "--cas") {
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        return tape::RunCasToolCommand(tokens, argv[0]);
    }
    if (argc > 1 && std::string(argv[1]) == "--fitadb") {
        const std::vector<std::string> tokens(argv + 2, argv + argc);
        return tapedb::RunTapeDbCommand(tokens, argv[0]);
    }

    std::cout << "Copyright (c) 1972-2026 Cybernostra, Inc." << std::endl;

    // main() recebe o nome do produto e os tres inteiros da versao como
    // parametros de linha de comando (argv[1..4]); na ausencia deles,
    // usamos os valores padrao definidos em src/common/version.h.
    const char* project_name = (argc > 1) ? argv[1] : FWMSX_NAME;
    const int major = (argc > 2) ? std::atoi(argv[2]) : FWMSX_VERSION_MAJOR;
    const int minor = (argc > 3) ? std::atoi(argv[3]) : FWMSX_VERSION_MINOR;
    const int patch = (argc > 4) ? std::atoi(argv[4]) : FWMSX_VERSION_PATCH;

    std::cout << project_name << " [v " << major << "." << minor << "." << patch << "]"
               << std::endl;
    std::cout << std::string(40, '-') << std::endl;

    // Cada modulo recebe a versao corrente, imprime sua propria linha de
    // "Loading module..." com a versao entre colchetes, e devolve sua
    // assinatura hexadecimal (0x0001..0x0004).
    const std::uint16_t sig_cpp = init_cpp(major, minor, patch);
    const std::uint16_t sig_c = init_c(major, minor, patch);
    const std::uint16_t sig_asm = init_asm(major, minor, patch);
    const std::uint16_t sig_fortran = init_fortran(major, minor, patch);

    std::cout << std::string(40, '-') << std::endl;
    std::cout << "Assinaturas dos modulos:" << std::endl;
    print_signature("C++", sig_cpp);
    print_signature("C", sig_c);
    print_signature("Assembly", sig_asm);
    print_signature("Fortran", sig_fortran);

    return 0;
}
