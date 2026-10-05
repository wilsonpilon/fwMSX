// fwMSX -- filtros de video da janela (menu Video): interpolacao (2x),
// scanlines e filtros de cor. Codigo ORIGINAL do fwMSX (BSD-3-Clause),
// escrito a partir das descricoes publicas dos algoritmos (ver
// doc/machine-spec.md). Nao copia codigo do fMSX.
#pragma once

#include <cstdint>
#include <vector>

namespace machine::gui {

enum class Interpolation { Nearest = 0, Linear, Epx, Eagle, Scale2x, Sal2x };
enum class Scanlines { None = 0, Tv, Lcd, LcdRaster };
enum class ColorFilter { None = 0, Monochrome, Sepia, GreenCrt, AmberCrt, CmyRaster, RgbRaster };

struct VideoFilterOptions {
    Interpolation interp = Interpolation::Nearest;
    Scanlines scanlines = Scanlines::None;
    ColorFilter color = ColorFilter::None;
};

// Verdadeiro se algum filtro muda a imagem (senao a janela envia o quadro como esta).
bool VideoFiltersActive(const VideoFilterOptions &options);

// Aplica os filtros a uma imagem `w` x `h` (pixels empacotados como em
// PackRgba: 0xFF | B<<16 | G<<8 | R). Interpolacao 2x dobra as duas dimensoes;
// Nearest mantem o tamanho. Devolve a imagem em `out` com as dimensoes em
// out_w/out_h.
void ApplyVideoFilters(const std::vector<uint32_t> &in, int w, int h, const VideoFilterOptions &options,
                       std::vector<uint32_t> &out, int &out_w, int &out_h);

} // namespace machine::gui
