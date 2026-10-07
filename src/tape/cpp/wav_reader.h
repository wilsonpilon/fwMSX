// fwMSX -- leitor de .WAV (PCM mono, 8 ou 16 bits), usado pelo
// "ripper" de fita (`fwmsx --cas rip`, ver wav_ripper.h e
// doc/tape-spec.md, secao 9). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause) -- layout do formato RIFF/WAVE conferido contra
// resource/makeTSX/WAV.cpp (MIT, so' como referencia, nenhum codigo
// copiado).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace tape {

struct WavSamples {
    uint32_t sample_rate = 0;
    std::vector<int16_t> samples; // mono, centrado em 0 (8 bits e' promovido para a mesma escala de 16)
};

// Le um .WAV PCM mono (8 ou 16 bits) para `out`. So' mono -- fitas de
// MSX sao gravadas num canal so', e um .wav estereo e' rejeitado com
// erro (mixar os canais e' um refinamento futuro, nao um caso comum).
// Devolve false com `error` se o arquivo nao abrir, nao for RIFF/WAVE
// PCM valido, ou for estereo/outro formato que nao PCM inteiro.
bool LoadWav(const std::string &path, WavSamples &out, std::string &error);

} // namespace tape
