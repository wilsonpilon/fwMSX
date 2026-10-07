// fwMSX -- leitor de .CAS (imagem de fita crua do MSX). Codigo ORIGINAL
// do fwMSX (BSD-3-Clause). Ver doc/tape-spec.md.
#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "tape_image.h"

namespace tape {

// Le um arquivo .CAS e preenche `out` para os dois modos de carregamento
// (fast_bytes = o proprio arquivo; pulses = sintetizados com os
// parametros padrao do MSX, ver cas_format.h). Devolve false com `error`
// se o arquivo nao abrir.
bool LoadCasImage(const std::string &path, TapeImage &out, std::string &error);

// Posicao do proximo marcador de 8 bytes (kCasHeader) a partir de `from`,
// ou `bytes.size()` se nao achar (nunca devolve "nao achado" como um
// valor magico separado -- ver os loops que usam isto). Compartilhado
// pelo leitor de .TSX/.TZX (tzx_reader.cpp) e pelo escritor (tsx_writer.cpp).
std::size_t FindCasHeader(const std::vector<uint8_t> &bytes, std::size_t from);

// Varre um buffer no formato .CAS e lista os arquivos logicos (entre
// cabecalhos de 8 bytes) para exibicao e para marcar o ponto de carga
// (TapeEngine::SeekToFile()). `marks` (opcional): pares (posicao do
// cabecalho em `bytes`, indice do pulso correspondente em `pulses`) --
// ver SynthesizeCasPulses(). Sem `marks`, os arquivos saem com
// `pulse_index` sempre 0 (so' faz sentido pedir pulsos reais de um
// buffer com pulsos de verdade, como o modo normal precisa).
std::vector<TapeFileEntry> ScanCasFiles(const std::vector<uint8_t> &bytes,
                                        const std::vector<std::pair<std::size_t, std::size_t>> &marks = {});

// Acrescenta a `pulses` os pulsos sintetizados (piloto + bytes KCS, ver
// cas_format.h) de cada bloco delimitado por cabecalhos de 8 bytes em
// `bytes`. Se `marks_out` nao for nulo, acrescenta um par (posicao do
// cabecalho, `pulses.size()` ANTES de acrescentar os pulsos daquele
// bloco) por bloco encontrado -- para ScanCasFiles() preencher
// `pulse_index` dos arquivos.
void SynthesizeCasPulses(const std::vector<uint8_t> &bytes, std::vector<uint32_t> &pulses,
                         std::vector<std::pair<std::size_t, std::size_t>> *marks_out = nullptr);

} // namespace tape
