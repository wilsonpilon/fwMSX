// Teste dos filtros de video da janela (src/machine/gui/video_filters.cpp):
// tamanhos de saida, Scale2x/Eagle nos cantos, filtros de cor e scanlines.
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "video_filters.h"

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf(cond ? "[PASS] %s\n" : "[FAIL] %s\n", what.c_str());
    if (!cond) ++g_failures;
}

uint32_t Px(int r, int g, int b) {
    return 0xFF000000u | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(r);
}

int R(uint32_t p) { return static_cast<int>(p & 0xFF); }
int G(uint32_t p) { return static_cast<int>((p >> 8) & 0xFF); }
int B(uint32_t p) { return static_cast<int>((p >> 16) & 0xFF); }

using machine::gui::ApplyVideoFilters;
using machine::gui::ColorFilter;
using machine::gui::Interpolation;
using machine::gui::Scanlines;
using machine::gui::VideoFilterOptions;

// Imagem 3x3 com um pixel central de cor `e` e cor `n` nas quatro arestas (cima/esquerda/direita/baixo).
std::vector<uint32_t> Cross(uint32_t e, uint32_t n) {
    //   . n .
    //   n e n     (cantos iguais a `e`)
    //   . n .
    std::vector<uint32_t> img(9, e);
    img[1] = n;
    img[3] = n;
    img[5] = n;
    img[7] = n;
    return img;
}

} // namespace

int main() {
    std::vector<uint32_t> out;
    int ow = 0, oh = 0;

    // Sem filtro: a imagem sai igual e no mesmo tamanho.
    {
        const std::vector<uint32_t> in = Cross(Px(10, 20, 30), Px(200, 0, 0));
        ApplyVideoFilters(in, 3, 3, VideoFilterOptions{}, out, ow, oh);
        check(ow == 3 && oh == 3 && out == in, "Nearest sem filtros: imagem identica, tamanho 3x3");
    }

    // Interpolacao 2x dobra as dimensoes.
    {
        const std::vector<uint32_t> in(3 * 2, Px(1, 2, 3));
        for (Interpolation m : {Interpolation::Linear, Interpolation::Epx, Interpolation::Eagle, Interpolation::Scale2x, Interpolation::Sal2x}) {
            VideoFilterOptions o;
            o.interp = m;
            ApplyVideoFilters(in, 3, 2, o, out, ow, oh);
            check(ow == 6 && oh == 4 && out.size() == 24, "interpolacao 2x: 3x2 vira 6x4");
        }
    }

    // Scale2x: centro azul; cima e esquerda vermelhos (concordam); direita verde (difere de cima);
    // embaixo azul (difere de esquerda). So' o canto de cima-esquerda do centro muda, para vermelho.
    {
        const uint32_t red = Px(255, 0, 0), blue = Px(0, 0, 255), green = Px(0, 255, 0);
        std::vector<uint32_t> in(9, blue);
        in[1] = red;   // cima
        in[3] = red;   // esquerda
        in[5] = green; // direita
        in[7] = blue;  // embaixo
        VideoFilterOptions o;
        o.interp = Interpolation::Scale2x;
        ApplyVideoFilters(in, 3, 3, o, out, ow, oh);
        // Centro 3x3 -> bloco 2x2 em (2..3, 2..3) da saida 6x6.
        check(out[2 * ow + 2] == red, "Scale2x: canto de cima-esquerda do centro toma a cor das arestas concordantes");
        check(out[2 * ow + 3] == blue, "Scale2x: canto de cima-direita (cima != direita) fica com a cor do centro");
    }

    // Eagle: canto so' muda quando as duas arestas e o canto diagonal concordam.
    {
        const uint32_t red = Px(255, 0, 0), green = Px(0, 255, 0), blue = Px(0, 0, 255);
        std::vector<uint32_t> in(9, blue);
        in[0] = red; // canto superior-esquerdo
        in[1] = red; // cima
        in[3] = red; // esquerda
        VideoFilterOptions o;
        o.interp = Interpolation::Eagle;
        ApplyVideoFilters(in, 3, 3, o, out, ow, oh);
        check(out[2 * ow + 2] == red, "Eagle: canto do centro vira a cor do canto diagonal quando cima e esquerda concordam");
        check(out[2 * ow + 3] == blue, "Eagle: canto sem concordancia continua com a cor do centro");
        (void)green;
    }

    // Bilinear: meio-caminho entre preto e branco da mesma linha vira cinza.
    {
        const std::vector<uint32_t> in = {Px(0, 0, 0), Px(255, 255, 255)};
        VideoFilterOptions o;
        o.interp = Interpolation::Linear;
        ApplyVideoFilters(in, 2, 1, o, out, ow, oh);
        const int mid = R(out[1]); // pixel entre as duas amostras
        check(mid > 60 && mid < 200 && R(out[1]) == G(out[1]) && G(out[1]) == B(out[1]),
              "Linear: o pixel entre preto e branco e' cinza intermediario");
    }

    // Filtros de cor.
    {
        const std::vector<uint32_t> in(1, Px(255, 0, 0));
        VideoFilterOptions o;
        o.color = ColorFilter::Monochrome;
        ApplyVideoFilters(in, 1, 1, o, out, ow, oh);
        // Luma de vermelho puro: (77*255)>>8 = 76.
        check(out[0] == Px(76, 76, 76), "Monochrome: vermelho puro vira cinza de luma 76");

        o.color = ColorFilter::GreenCrt;
        ApplyVideoFilters(in, 1, 1, o, out, ow, oh);
        check(R(out[0]) == 0 && G(out[0]) == 76 && B(out[0]) == 0, "Green CRT: so' o canal verde, com a luma");

        o.color = ColorFilter::AmberCrt;
        ApplyVideoFilters(in, 1, 1, o, out, ow, oh);
        check(R(out[0]) == 76 && G(out[0]) > 0 && B(out[0]) < G(out[0]), "Amber CRT: tom ambar (R > G > B)");

        o.color = ColorFilter::Sepia;
        ApplyVideoFilters(in, 1, 1, o, out, ow, oh);
        check(R(out[0]) > G(out[0]) && G(out[0]) > B(out[0]), "Sepia: tom quente (R > G > B)");

        o.color = ColorFilter::RgbRaster;
        const std::vector<uint32_t> white3(3, Px(255, 255, 255));
        ApplyVideoFilters(white3, 3, 1, o, out, ow, oh);
        check(R(out[0]) == 255 && G(out[0]) < 255 && B(out[0]) < 255 && G(out[1]) == 255 && R(out[1]) < 255,
              "RGB Raster: as colunas de 3 pixels escurecem canais diferentes");
    }

    // Scanlines.
    {
        const std::vector<uint32_t> in(2 * 2, Px(200, 200, 200));
        VideoFilterOptions o;
        o.scanlines = Scanlines::Tv;
        ApplyVideoFilters(in, 2, 2, o, out, ow, oh);
        check(R(out[0]) == 200 && R(out[2]) == 120, "TV: linhas impares ficam a 60% (200 -> 120)");

        o.scanlines = Scanlines::LcdRaster;
        const std::vector<uint32_t> in3(3 * 3, Px(200, 200, 200));
        ApplyVideoFilters(in3, 3, 3, o, out, ow, oh);
        check(R(out[2 * 3 + 2]) == 120 && R(out[0]) == 200, "LCD Raster: a grade escurece a linha e a coluna 2 de cada 3");
    }

    if (g_failures == 0) {
        std::printf("\nTodos os testes de filtros de video passaram.\n");
        return 0;
    }
    std::printf("\n%d falha(s) nos filtros de video.\n", g_failures);
    return 1;
}
