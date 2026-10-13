// fwMSX -- leitor de PNG minimo. Ver png_reader.h.
#include "png_reader.h"

#include <miniz.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>

namespace splash {

namespace {

uint32_t Be32(const uint8_t *p) { return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) | (static_cast<uint32_t>(p[2]) << 8) | p[3]; }

} // namespace

bool DecodePng(const std::vector<uint8_t> &data, Image &out, std::string &error) {
    static const uint8_t kSig[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    if (data.size() < 8 || std::memcmp(data.data(), kSig, 8) != 0) {
        error = "nao e' um arquivo PNG";
        return false;
    }
    uint32_t w = 0, h = 0;
    int depth = 0, ctype = -1, interlace = 0;
    std::vector<uint8_t> idat, palette, trns;
    size_t pos = 8;
    while (pos + 12 <= data.size()) {
        const uint32_t n = Be32(&data[pos]);
        const char *type = reinterpret_cast<const char *>(&data[pos + 4]);
        if (pos + 12 + static_cast<size_t>(n) > data.size()) {
            error = "PNG truncado";
            return false;
        }
        const uint8_t *body = &data[pos + 8];
        if (std::memcmp(type, "IHDR", 4) == 0 && n >= 13) {
            w = Be32(body);
            h = Be32(body + 4);
            depth = body[8];
            ctype = body[9];
            interlace = body[12];
        } else if (std::memcmp(type, "PLTE", 4) == 0) {
            palette.assign(body, body + n);
        } else if (std::memcmp(type, "tRNS", 4) == 0) {
            trns.assign(body, body + n);
        } else if (std::memcmp(type, "IDAT", 4) == 0) {
            idat.insert(idat.end(), body, body + n);
        } else if (std::memcmp(type, "IEND", 4) == 0) {
            break;
        }
        pos += 12 + n;
    }
    if (w == 0 || h == 0 || w > 16384 || h > 16384) {
        error = "PNG sem cabecalho valido";
        return false;
    }
    if (depth != 8 || interlace != 0 || !(ctype == 0 || ctype == 2 || ctype == 3 || ctype == 4 || ctype == 6)) {
        error = "PNG nao suportado (so' 8 bits por canal, sem entrelacamento)";
        return false;
    }
    if (ctype == 3 && palette.size() < 3) {
        error = "PNG de paleta sem PLTE";
        return false;
    }
    const int bpp = ctype == 0 ? 1 : ctype == 2 ? 3 : ctype == 3 ? 1 : ctype == 4 ? 2 : 4;
    const size_t stride = static_cast<size_t>(w) * static_cast<size_t>(bpp);

    size_t raw_len = 0;
    void *raw_mem = tinfl_decompress_mem_to_heap(idat.data(), idat.size(), &raw_len, TINFL_FLAG_PARSE_ZLIB_HEADER);
    if (raw_mem == nullptr) {
        error = "PNG com dados comprimidos invalidos";
        return false;
    }
    const uint8_t *raw = static_cast<const uint8_t *>(raw_mem);
    if (raw_len < (stride + 1) * h) {
        mz_free(raw_mem);
        error = "PNG com dados incompletos";
        return false;
    }

    // Desfaz os filtros linha a linha.
    std::vector<uint8_t> pix(stride * h);
    for (uint32_t y = 0; y < h; ++y) {
        const uint8_t ft = raw[y * (stride + 1)];
        const uint8_t *src = raw + y * (stride + 1) + 1;
        uint8_t *row = &pix[y * stride];
        const uint8_t *prev = y ? &pix[(y - 1) * stride] : nullptr;
        for (size_t i = 0; i < stride; ++i) {
            const int a = i >= static_cast<size_t>(bpp) ? row[i - bpp] : 0;
            const int b = prev ? prev[i] : 0;
            const int c = (prev && i >= static_cast<size_t>(bpp)) ? prev[i - bpp] : 0;
            int v = src[i];
            switch (ft) {
            case 0: break;
            case 1: v += a; break;
            case 2: v += b; break;
            case 3: v += (a + b) >> 1; break;
            case 4: {
                const int p = a + b - c;
                const int pa = std::abs(p - a), pb = std::abs(p - b), pc = std::abs(p - c);
                v += (pa <= pb && pa <= pc) ? a : (pb <= pc ? b : c);
                break;
            }
            default:
                mz_free(raw_mem);
                error = "PNG com filtro invalido";
                return false;
            }
            row[i] = static_cast<uint8_t>(v);
        }
    }
    mz_free(raw_mem);

    out.width = static_cast<int>(w);
    out.height = static_cast<int>(h);
    out.rgba.assign(static_cast<size_t>(w) * h * 4, 255);
    for (size_t i = 0, n = static_cast<size_t>(w) * h; i < n; ++i) {
        uint8_t *d = &out.rgba[i * 4];
        const uint8_t *s = &pix[i * bpp];
        switch (ctype) {
        case 0: d[0] = d[1] = d[2] = s[0]; break;
        case 2: d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; break;
        case 3: {
            const size_t k = s[0];
            if (k * 3 + 2 < palette.size()) {
                d[0] = palette[k * 3];
                d[1] = palette[k * 3 + 1];
                d[2] = palette[k * 3 + 2];
            }
            if (k < trns.size()) d[3] = trns[k];
            break;
        }
        case 4: d[0] = d[1] = d[2] = s[0]; d[3] = s[1]; break;
        case 6: d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3]; break;
        }
    }
    return true;
}

bool LoadPngFile(const std::string &path, Image &out, std::string &error) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        error = "nao foi possivel abrir '" + path + "'";
        return false;
    }
    const std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return DecodePng(data, out, error);
}

} // namespace splash
