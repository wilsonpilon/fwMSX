// fwMSX -- escritor de .TSX a partir de um buffer no formato .CAS (o
// mesmo fluxo que o gancho de BIOS de gravacao produz, ver
// tape_device.h). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver
// doc/tape-spec.md.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "tape_image.h"

namespace tape {

// Converte `fast_bytes` (formato .CAS: marcadores de 8 bytes entre
// blocos) num .TSX valido -- um bloco #4B (Kansas City Standard) por
// marca de `marks` (ver TapeMark em tape_image.h), com os parametros
// padrao do MSX (ver cas_format.h). Usa o tamanho EXATO de cada marca,
// nao uma busca generica pelo proximo cabecalho (que confundiria
// preenchimento de alinhamento com dado de verdade -- ver
// doc/tape-spec.md, secao 5). Devolve false com `error` se nao conseguir
// escrever o arquivo.
bool WriteTsxFromCas(const std::vector<uint8_t> &fast_bytes, const std::vector<TapeMark> &marks, const std::string &path,
                      std::string &error);

} // namespace tape
