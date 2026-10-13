// fwMSX -- splash de abertura (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/splash-spec.md.
//
// A politica (quando mostrar) e' uma funcao pura, testavel; o desenho em janela fica em emu_window.cpp; aqui
// ficam a politica, a procura do arquivo da imagem e a versao para terminal (meios blocos com cor de 24 bits).
#pragma once

#include <string>
#include <vector>

#include "png_reader.h"

namespace splash {

// Onde o programa esta' rodando.
enum class Mode {
    Window,   // janela do emulador (padrao: mostra, se a configuracao deixar)
    Terminal, // --cli, --tui, --term: so' com --splash
    Command   // --cmd (um comando e sai): nunca, nem com --splash
};

// Pedido da linha de comando.
enum class Flag {
    Default, // nem --splash nem --no-splash
    ForceOn, // --splash
    ForceOff // --no-splash (vence --splash se vierem os dois)
};

// `enabled_in_settings`: a opcao "Mostrar splash ao iniciar" do menu Ferramentas > Interface.
bool ShouldShow(Mode mode, Flag flag, bool enabled_in_settings);

// Tira "--splash" e "--no-splash" de `args` e devolve o pedido.
Flag ExtractFlag(std::vector<std::string> &args);

// Procura fwMSX-splash.png: ao lado do executavel, em images/ ao lado dele, na pasta de trabalho e em images/.
// Vazio se nao achar (o splash simplesmente nao aparece).
std::string FindSplashFile(const std::string &exe_dir);

// Meios blocos coloridos (ESC[38;2;..m / ESC[48;2;..m, caractere U+2580) para um terminal de `cols` x `rows`
// celulas: a imagem cabe inteira, centralizada, com a proporcao certa (a celula tem o dobro da altura da largura).
std::string RenderHalfBlocks(const Image &image, int cols, int rows);

// Mostra o splash no terminal por `seconds` e limpa a tela. So' age se a saida e' um terminal de verdade (com um
// pipe ou arquivo nao escreve nada) e se a imagem for achada. Devolve true se mostrou.
bool ShowTerminalSplash(const std::string &exe_dir, double seconds);

// Atalho para as entradas de terminal (--cli, --tui, --term): trata --splash/--no-splash em `args` (removendo-os)
// e mostra o splash so' se for pedido com --splash.
void HandleTerminalArgs(std::vector<std::string> &args, const std::string &argv0);

} // namespace splash
