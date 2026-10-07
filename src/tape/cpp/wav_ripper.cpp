#include "wav_ripper.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "../core/kcs_codec.h"
#include "cas_format.h"
#include "cas_pack.h"

namespace tape {

namespace {

// Piloto minimo para considerar um trecho valido (o makeTSX usa 500;
// aqui um pouco menos tolerante com gravacoes de duracao curta --
// ainda bem acima de qualquer coincidencia de dado comum).
constexpr std::size_t kMinPilotPulses = 400;

// Fracao do pico absoluto do arquivo usada como limiar alto/baixo
// (zona morta no meio para nao contar ruido perto de zero como
// transicao) -- mais simples que o normalize()/envelopeCorrection() do
// makeTSX original, ver a nota de limites em wav_ripper.h.
constexpr double kThresholdFraction = 0.2;

// Extrai a duracao (em AMOSTRAS) de cada pulso (meio-periodo) do
// sinal, por deteccao de limiar com zona morta -- mesma tecnica do
// BlockRipper::initializeStatesVector()/isLow()/isHigh() do makeTSX
// (ver wav_ripper.h), portada sem a "fase" do original (nao necessaria
// aqui: um pulso e' so' uma duracao, o lado nao importa para
// decodificar KCS).
std::vector<uint32_t> ExtractPulseDurations(const std::vector<int16_t> &samples) {
    std::vector<uint32_t> pulses;
    if (samples.empty()) return pulses;

    int32_t peak = 0;
    for (int16_t s : samples) peak = std::max(peak, static_cast<int32_t>(std::abs(static_cast<int32_t>(s))));
    const int32_t threshold = static_cast<int32_t>(peak * kThresholdFraction);
    if (threshold == 0) return pulses;

    int state = 0; // -1 baixo, 0 indefinido (antes da 1a transicao), 1 alto
    std::size_t last_edge = 0;
    for (std::size_t i = 0; i < samples.size(); ++i) {
        int new_state = state;
        if (samples[i] >= threshold) new_state = 1;
        else if (samples[i] <= -threshold) new_state = -1;
        if (new_state != 0 && new_state != state) {
            if (state != 0) pulses.push_back(static_cast<uint32_t>(i - last_edge));
            last_edge = i;
            state = new_state;
        }
    }
    return pulses;
}

// Mede o piloto a partir de `pos`: conta pulsos consecutivos cuja
// duracao fica dentro da tolerancia da MEDIA acumulada dos pulsos
// anteriores do mesmo trecho (comeca so' com o 1o pulso como
// referencia, refinando a cada pulso aceito -- mais robusto contra
// jitter/arredondamento da gravacao que fixar so' no 1o). `out_width`
// recebe a duracao media medida (usada como `one_len` do bloco).
std::size_t MeasurePilot(const std::vector<uint32_t> &pulses, std::size_t pos, int tolerance_percent,
                          double &out_width) {
    if (pos >= pulses.size()) {
        out_width = 0;
        return 0;
    }
    double width = pulses[pos];
    double sum = 0;
    std::size_t count = 0;
    while (pos + count < pulses.size()) {
        const double p = pulses[pos + count];
        if (std::abs(p - width) > width * tolerance_percent / 100.0) break;
        sum += p;
        ++count;
        width = sum / static_cast<double>(count);
    }
    out_width = count > 0 ? sum / static_cast<double>(count) : 0;
    return count;
}

} // namespace

bool RipWavToCas(const WavSamples &wav, std::vector<uint8_t> &fast_bytes, std::vector<TapeMark> &marks,
                  int tolerance_percent, RipStats &stats, std::string &error) {
    stats = RipStats{};

    if (wav.sample_rate == 0) {
        error = "taxa de amostragem invalida";
        return false;
    }
    const std::vector<uint32_t> pulses = ExtractPulseDurations(wav.samples);
    if (pulses.empty()) {
        error = "nenhum pulso detectado (sinal fraco demais, ou silencio)";
        return false;
    }

    std::size_t pos = 0;
    while (pos < pulses.size()) {
        double pilot_width = 0;
        const std::size_t pilot_count = MeasurePilot(pulses, pos, tolerance_percent, pilot_width);
        if (pilot_count < kMinPilotPulses) {
            ++pos; // nao e' piloto -- avanca 1 pulso (ruido/silencio entre blocos) e tenta de novo
            continue;
        }
        pos += pilot_count;

        const uint32_t one_len = static_cast<uint32_t>(std::lround(pilot_width));
        const uint32_t zero_len = one_len * 2;
        const KcsByteFraming cfg{1, 0, 2, 1, 0, kMsxZeroPulsesPerBit, kMsxOnePulsesPerBit, zero_len, one_len};

        std::vector<uint8_t> block_bytes;
        while (pos < pulses.size()) {
            uint8_t byte = 0;
            std::size_t next = pos;
            if (!kcs_decode_byte(&cfg, pulses.data(), pulses.size(), &next, tolerance_percent, &byte)) break;
            block_bytes.push_back(byte);
            pos = next;
        }

        if (!block_bytes.empty()) {
            AppendCasBlock(fast_bytes, block_bytes, marks);
            ++stats.blocks_found;
            stats.bytes_decoded += block_bytes.size();
        } else {
            stats.warnings.push_back("piloto encontrado sem nenhum dado legivel depois dele");
        }
    }

    if (stats.blocks_found == 0) {
        error = "nenhum bloco #4B (piloto + dados) encontrado no .wav";
        return false;
    }
    return true;
}

} // namespace tape
