// fwMSX -- filtros de video da janela. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver video_filters.h.
#include "video_filters.h"

#include <algorithm>
#include <cmath>

namespace machine::gui {

namespace {

struct Rgb {
    int r, g, b;
};

Rgb Unpack(uint32_t px) {
    return Rgb{static_cast<int>(px & 0xFF), static_cast<int>((px >> 8) & 0xFF), static_cast<int>((px >> 16) & 0xFF)};
}

uint32_t Pack(Rgb c) {
    const auto clamp = [](int v) { return static_cast<uint32_t>(std::clamp(v, 0, 255)); };
    return 0xFF000000u | (clamp(c.b) << 16) | (clamp(c.g) << 8) | clamp(c.r);
}

Rgb Mix(Rgb a, Rgb b) { return Rgb{(a.r + b.r) / 2, (a.g + b.g) / 2, (a.b + b.b) / 2}; }

// Pixel com as coordenadas presas nas bordas da imagem.
class Source {
public:
    Source(const std::vector<uint32_t> &img, int w, int h) : img_(img), w_(w), h_(h) {}
    uint32_t at(int x, int y) const {
        return img_[static_cast<size_t>(std::clamp(y, 0, h_ - 1)) * w_ + std::clamp(x, 0, w_ - 1)];
    }

private:
    const std::vector<uint32_t> &img_;
    int w_, h_;
};

// Interpolacao 2x: cada pixel vira um bloco 2x2.
void Interpolate2x(const std::vector<uint32_t> &in, int w, int h, Interpolation mode, std::vector<uint32_t> &out) {
    const Source src(in, w, h);
    const int ow = w * 2;
    out.assign(static_cast<size_t>(ow) * h * 2, 0xFF000000u);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const uint32_t e = src.at(x, y);
            const uint32_t b = src.at(x, y - 1);  // acima
            const uint32_t d = src.at(x - 1, y);  // esquerda
            const uint32_t f = src.at(x + 1, y);  // direita
            const uint32_t hh = src.at(x, y + 1); // abaixo
            uint32_t tl = e, tr = e, bl = e, br = e;
            switch (mode) {
            case Interpolation::Epx:
            case Interpolation::Scale2x:
                // Regras de Scale2x: um canto recebe a cor vizinha quando as duas arestas
                // que o tocam concordam entre si e diferem do centro.
                if (d == b && b != f && d != hh) tl = d;
                if (b == f && b != d && f != hh) tr = f;
                if (d == hh && d != b && hh != f) bl = d;
                if (hh == f && d != hh && b != f) br = f;
                break;
            case Interpolation::Eagle: {
                const uint32_t ul = src.at(x - 1, y - 1), ur = src.at(x + 1, y - 1);
                const uint32_t dl = src.at(x - 1, y + 1), dr = src.at(x + 1, y + 1);
                if (b == ul && d == ul) tl = ul;
                if (b == ur && f == ur) tr = ur;
                if (hh == dl && d == dl) bl = dl;
                if (hh == dr && f == dr) br = dr;
                break;
            }
            case Interpolation::Sal2x:
                // Aproximacao de 2xSaI: nas mesmas arestas de Scale2x, mistura as duas cores.
                if (d == b && b != f && d != hh) tl = Pack(Mix(Unpack(d), Unpack(b)));
                if (b == f && b != d && f != hh) tr = Pack(Mix(Unpack(f), Unpack(b)));
                if (d == hh && d != b && hh != f) bl = Pack(Mix(Unpack(d), Unpack(hh)));
                if (hh == f && d != hh && b != f) br = Pack(Mix(Unpack(hh), Unpack(f)));
                break;
            case Interpolation::Nearest:
            case Interpolation::Linear:
                break;
            }
            const size_t row0 = static_cast<size_t>(y * 2) * ow;
            const size_t row1 = row0 + ow;
            out[row0 + x * 2] = tl;
            out[row0 + x * 2 + 1] = tr;
            out[row1 + x * 2] = bl;
            out[row1 + x * 2 + 1] = br;
        }
    }
}

// Bilinear 2x: o centro de cada pixel de saida cai entre os pixels de entrada.
void Linear2x(const std::vector<uint32_t> &in, int w, int h, std::vector<uint32_t> &out) {
    const Source src(in, w, h);
    const int ow = w * 2, oh = h * 2;
    out.assign(static_cast<size_t>(ow) * oh, 0xFF000000u);
    for (int oy = 0; oy < oh; ++oy) {
        const float sy = (oy + 0.5f) / 2.0f - 0.5f;
        const int y0 = static_cast<int>(std::floor(sy));
        const float fy = sy - y0;
        for (int ox = 0; ox < ow; ++ox) {
            const float sx = (ox + 0.5f) / 2.0f - 0.5f;
            const int x0 = static_cast<int>(std::floor(sx));
            const float fx = sx - x0;
            const Rgb c00 = Unpack(src.at(x0, y0)), c10 = Unpack(src.at(x0 + 1, y0));
            const Rgb c01 = Unpack(src.at(x0, y0 + 1)), c11 = Unpack(src.at(x0 + 1, y0 + 1));
            const auto blend = [&](int a, int b, int c, int d) {
                const float top = a + (b - a) * fx;
                const float bot = c + (d - c) * fx;
                return static_cast<int>(std::lround(top + (bot - top) * fy));
            };
            out[static_cast<size_t>(oy) * ow + ox] = Pack(Rgb{blend(c00.r, c10.r, c01.r, c11.r), blend(c00.g, c10.g, c01.g, c11.g),
                                                              blend(c00.b, c10.b, c01.b, c11.b)});
        }
    }
}

// Escurece a cor por um fator (0..1).
Rgb Scale(Rgb c, float k) {
    return Rgb{static_cast<int>(std::lround(c.r * k)), static_cast<int>(std::lround(c.g * k)), static_cast<int>(std::lround(c.b * k))};
}

Rgb ColorTransform(Rgb c, ColorFilter mode, int x) {
    const int luma = (77 * c.r + 150 * c.g + 29 * c.b) >> 8;
    switch (mode) {
    case ColorFilter::Monochrome:
        return Rgb{luma, luma, luma};
    case ColorFilter::Sepia:
        return Rgb{static_cast<int>(0.393f * c.r + 0.769f * c.g + 0.189f * c.b),
                   static_cast<int>(0.349f * c.r + 0.686f * c.g + 0.168f * c.b),
                   static_cast<int>(0.272f * c.r + 0.534f * c.g + 0.131f * c.b)};
    case ColorFilter::GreenCrt:
        return Rgb{0, luma, 0};
    case ColorFilter::AmberCrt:
        return Rgb{luma, static_cast<int>(luma * 0.7f), static_cast<int>(luma * 0.2f)};
    case ColorFilter::CmyRaster:
        // Subpixels ciano, magenta e amarelo, em colunas de 3 pixels.
        switch (x % 3) {
        case 0: return Rgb{static_cast<int>(c.r * 0.5f), c.g, c.b};
        case 1: return Rgb{c.r, static_cast<int>(c.g * 0.5f), c.b};
        default: return Rgb{c.r, c.g, static_cast<int>(c.b * 0.5f)};
        }
    case ColorFilter::RgbRaster:
        switch (x % 3) {
        case 0: return Rgb{c.r, static_cast<int>(c.g * 0.5f), static_cast<int>(c.b * 0.5f)};
        case 1: return Rgb{static_cast<int>(c.r * 0.5f), c.g, static_cast<int>(c.b * 0.5f)};
        default: return Rgb{static_cast<int>(c.r * 0.5f), static_cast<int>(c.g * 0.5f), c.b};
        }
    case ColorFilter::None:
        break;
    }
    return c;
}

// Scanlines e grades sobre a imagem de saida: linhas (y) e colunas (x).
float ScanlineFactor(Scanlines mode, int x, int y) {
    switch (mode) {
    case Scanlines::Tv:
        return (y % 2 == 1) ? 0.6f : 1.0f;
    case Scanlines::Lcd:
        return (y % 3 == 2) ? 0.7f : 1.0f;
    case Scanlines::LcdRaster:
        if (y % 3 == 2 || x % 3 == 2) return 0.6f;
        return 1.0f;
    case Scanlines::None:
        break;
    }
    return 1.0f;
}

} // namespace

bool VideoFiltersActive(const VideoFilterOptions &options) {
    return options.interp != Interpolation::Nearest || options.scanlines != Scanlines::None ||
           options.color != ColorFilter::None;
}

void ApplyVideoFilters(const std::vector<uint32_t> &in, int w, int h, const VideoFilterOptions &options,
                       std::vector<uint32_t> &out, int &out_w, int &out_h) {
    const bool scaled = options.interp != Interpolation::Nearest;
    if (scaled) {
        if (options.interp == Interpolation::Linear) Linear2x(in, w, h, out);
        else Interpolate2x(in, w, h, options.interp, out);
        out_w = w * 2;
        out_h = h * 2;
    } else {
        out = in;
        out_w = w;
        out_h = h;
    }

    const bool colored = options.color != ColorFilter::None;
    const bool lined = options.scanlines != Scanlines::None;
    if (!colored && !lined) return;
    for (int y = 0; y < out_h; ++y) {
        for (int x = 0; x < out_w; ++x) {
            uint32_t &px = out[static_cast<size_t>(y) * out_w + x];
            Rgb c = Unpack(px);
            if (colored) c = ColorTransform(c, options.color, x);
            if (lined) c = Scale(c, ScanlineFactor(options.scanlines, x, y));
            px = Pack(c);
        }
    }
}

} // namespace machine::gui
