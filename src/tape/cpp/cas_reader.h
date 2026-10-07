// fwMSX -- leitor de .CAS (imagem de fita crua do MSX). Codigo ORIGINAL
// do fwMSX (BSD-3-Clause). Ver doc/tape-spec.md.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "tape_image.h"

namespace tape {

// Le um arquivo .CAS e preenche `out` para os dois modos de carregamento
// (fast_bytes = o proprio arquivo; pulses = sintetizados com os
// parametros padrao do MSX, ver cas_format.h). Devolve false com `error`
// se o arquivo nao abrir.
bool LoadCasImage(const std::string &path, TapeImage &out, std::string &error);

// Varre um buffer no formato .CAS e lista os arquivos logicos (entre
// cabecalhos de 8 bytes) para exibicao -- tambem usado pelo leitor de
// TSX/TZX sobre o fluxo reconstruido (ver tzx_reader.cpp).
std::vector<TapeFileEntry> ScanCasFiles(const std::vector<uint8_t> &bytes);

// Acrescenta a `pulses` os pulsos sintetizados (piloto + bytes KCS, ver
// cas_format.h) de cada bloco delimitado por cabecalhos de 8 bytes em
// `bytes`.
void SynthesizeCasPulses(const std::vector<uint8_t> &bytes, std::vector<uint32_t> &pulses);

} // namespace tape
