// fwMSX -- ver ppm_writer.h.
#include "ppm_writer.h"

#include <cstdio>
#include <vector>

namespace vdp {

bool WritePpm(const std::string &path, const VdpRgb888 *pixels, int width, int height, std::string *error) {
    std::FILE *file = std::fopen(path.c_str(), "wb");
    if (!file) {
        if (error) *error = "nao foi possivel abrir '" + path + "' para escrita";
        return false;
    }

    std::fprintf(file, "P6\n%d %d\n255\n", width, height);

    // Grava linha a linha (mesmo que VdpRgb888 seja compativel com um
    // buffer RGB cru em memoria de qualquer plataforma razoavel, evita
    // depender de tamanho/alinhamento de struct exatos entre
    // compiladores -- um fwrite() por componente por pixel seria lento
    // demais, entao monta uma linha por vez num buffer temporario).
    const size_t row_bytes = (size_t)width * 3;
    std::vector<unsigned char> row(row_bytes);
    for (int y = 0; y < height; ++y) {
        const VdpRgb888 *src = pixels + (size_t)y * (size_t)width;
        for (int x = 0; x < width; ++x) {
            row[(size_t)x * 3 + 0] = src[x].r;
            row[(size_t)x * 3 + 1] = src[x].g;
            row[(size_t)x * 3 + 2] = src[x].b;
        }
        if (std::fwrite(row.data(), 1, row_bytes, file) != row_bytes) {
            if (error) *error = "falha ao escrever em '" + path + "'";
            std::fclose(file);
            return false;
        }
    }

    std::fclose(file);
    return true;
}

} // namespace vdp
