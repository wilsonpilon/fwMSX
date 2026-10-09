// fwMSX -- captura da tela em PNG. Ver screenshot.h.
#include "screenshot.h"

#include <miniz.h>

#include <cstdint>
#include <cstdio>
#include <ctime>
#include <vector>

namespace machine {

std::string SaveScreenshotPng(const Machine &machine, std::string &error, const std::string &path) {
    std::vector<uint32_t> rgba;
    const FrameSize fs = machine.RenderFrame(rgba);
    const int out_h = fs.height * fs.y_scale;
    std::vector<uint8_t> rgb(static_cast<size_t>(fs.width) * out_h * 3);
    for (int y = 0; y < out_h; ++y) {
        const uint32_t *src = rgba.data() + static_cast<size_t>(y / fs.y_scale) * fs.width;
        uint8_t *dst = rgb.data() + static_cast<size_t>(y) * fs.width * 3;
        for (int x = 0; x < fs.width; ++x) {
            dst[x * 3 + 0] = static_cast<uint8_t>(src[x] & 0xFF);
            dst[x * 3 + 1] = static_cast<uint8_t>((src[x] >> 8) & 0xFF);
            dst[x * 3 + 2] = static_cast<uint8_t>((src[x] >> 16) & 0xFF);
        }
    }
    size_t png_len = 0;
    void *png = tdefl_write_image_to_png_file_in_memory(rgb.data(), fs.width, out_h, 3, &png_len);
    if (!png) {
        error = "falha ao codificar o PNG";
        return "";
    }
    const std::time_t t = std::time(nullptr);
    char auto_name[64];
    std::strftime(auto_name, sizeof(auto_name), "fwmsx-%Y%m%d-%H%M%S.png", std::localtime(&t));
    const std::string name = path.empty() ? std::string(auto_name) : path;
    std::FILE *f = std::fopen(name.c_str(), "wb");
    const bool ok = f && std::fwrite(png, 1, png_len, f) == png_len;
    if (f) std::fclose(f);
    mz_free(png);
    if (!ok) {
        error = "nao foi possivel gravar '" + name + "'";
        return "";
    }
    return name;
}

} // namespace machine
