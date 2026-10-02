// fwMSX -- saida de audio ao vivo (miniaudio): abre o dispositivo padrao do
// sistema e toca, em tempo real, as amostras que a emulacao empurra.
// Codigo ORIGINAL do fwMSX (BSD-3-Clause); miniaudio e' de dominio publico /
// MIT-0 (ver LICENSE-THIRD-PARTY.md). Ver doc/audio-spec.md.
//
// Sempre declarado, independente de FWMSX_AUDIO: audio_output.cpp (real) ou
// audio_output_stub.cpp (Start() falha com mensagem) fornecem a mesma classe,
// escolhidos no CMakeLists.txt -- quem usa nao precisa de #ifdef.
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace audio {

class AudioOutput {
public:
    // Formato fixo: mono, 16 bits, `sample_rate` Hz (o do PSG).
    explicit AudioOutput(int sample_rate = 44100);
    ~AudioOutput();
    AudioOutput(const AudioOutput &) = delete;
    AudioOutput &operator=(const AudioOutput &) = delete;

    // Abre e inicia o dispositivo padrao. false + `error` se nao houver
    // dispositivo de audio (o emulador segue mudo).
    bool Start(std::string &error);
    void Stop();
    bool running() const;

    // Entrega amostras ao dispositivo (chamada pela thread de emulacao, uma
    // vez por quadro). Nunca bloqueia: o que nao couber no buffer e'
    // descartado (e conta em dropped_samples()).
    void Push(const int16_t *samples, size_t count);

    // Volume 0.0-1.0 (aplicado no callback); 0 = mudo.
    void SetGain(float gain);
    float gain() const;

    // Descarta o que ainda nao tocou (Reset/pausa: nao arrastar som velho).
    void Flush();

    // Estatisticas (para diagnostico e testes).
    uint64_t consumed_samples() const; // amostras entregues ao dispositivo
    uint64_t underruns() const;        // vezes que o dispositivo pediu e nao havia amostras
    uint64_t dropped_samples() const;  // amostras descartadas por buffer cheio
    std::string device_name() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace audio
