// fwMSX -- gravador de WAV PCM mono de 16 bits para inspecionar o som do
// PSG sem placa de audio (par do ppm_writer do VDP). Codigo ORIGINAL do
// fwMSX (BSD-3-Clause). Ver doc/psg-spec.md.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace psg {

// Escreve `samples` em `path` como WAV PCM mono 16 bits a `sample_rate` Hz.
// Devolve true em caso de sucesso; senao false e `error` explica.
bool WriteWav(const std::string &path, const std::vector<int16_t> &samples, int sample_rate, std::string &error);

} // namespace psg
