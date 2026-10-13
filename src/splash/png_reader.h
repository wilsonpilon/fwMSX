// fwMSX -- leitor de PNG minimo (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/splash-spec.md.
//
// So' o que o splash precisa: PNG de 8 bits por canal (cinza, cinza+alfa, paleta, RGB ou RGBA), sem
// entrelacamento. O resultado e' sempre RGBA de 8 bits. A descompressao e' do miniz.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace splash {

struct Image {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> rgba; // width * height * 4
};

// Decodifica o conteudo de um arquivo PNG. false + `error` se nao for um PNG suportado.
bool DecodePng(const std::vector<uint8_t> &data, Image &out, std::string &error);

// Le e decodifica um arquivo PNG.
bool LoadPngFile(const std::string &path, Image &out, std::string &error);

} // namespace splash
