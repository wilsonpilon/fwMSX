#include "wav_reader.h"

#include <cstring>
#include <fstream>

namespace tape {

namespace {

uint16_t ReadLe16(const uint8_t *p) { return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8); }

uint32_t ReadLe32(const uint8_t *p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

bool ReadWholeFile(const std::string &path, std::vector<uint8_t> &out, std::string &error) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) {
        error = "nao foi possivel abrir '" + path + "'";
        return false;
    }
    const std::streamsize size = f.tellg();
    f.seekg(0);
    out.resize(static_cast<std::size_t>(size));
    if (size > 0 && !f.read(reinterpret_cast<char *>(out.data()), size)) {
        error = "nao foi possivel ler '" + path + "'";
        return false;
    }
    return true;
}

} // namespace

bool LoadWav(const std::string &path, WavSamples &out, std::string &error) {
    out = WavSamples{};

    std::vector<uint8_t> bytes;
    if (!ReadWholeFile(path, bytes, error)) return false;

    if (bytes.size() < 12 || std::memcmp(bytes.data(), "RIFF", 4) != 0 || std::memcmp(bytes.data() + 8, "WAVE", 4) != 0) {
        error = "'" + path + "' nao e' um .wav RIFF/WAVE valido";
        return false;
    }

    bool got_fmt = false;
    bool got_data = false;
    uint16_t format_tag = 0, channels = 0, bits_per_sample = 0;
    uint32_t sample_rate = 0;
    std::size_t data_start = 0, data_size = 0;

    std::size_t pos = 12;
    while (pos + 8 <= bytes.size() && (!got_fmt || !got_data)) {
        char chunk_id[4];
        std::memcpy(chunk_id, bytes.data() + pos, 4);
        const uint32_t chunk_size = ReadLe32(bytes.data() + pos + 4);
        const std::size_t payload_start = pos + 8;
        if (payload_start + chunk_size > bytes.size()) {
            error = "'" + path + "': bloco RIFF truncado";
            return false;
        }

        if (!got_fmt && std::memcmp(chunk_id, "fmt ", 4) == 0) {
            if (chunk_size < 16) {
                error = "'" + path + "': bloco 'fmt ' invalido (menor que 16 bytes)";
                return false;
            }
            const uint8_t *fmt = bytes.data() + payload_start;
            format_tag = ReadLe16(fmt);
            channels = ReadLe16(fmt + 2);
            sample_rate = ReadLe32(fmt + 4);
            bits_per_sample = ReadLe16(fmt + 14);
            got_fmt = true;
        } else if (!got_data && std::memcmp(chunk_id, "data", 4) == 0) {
            data_start = payload_start;
            data_size = chunk_size;
            got_data = true;
        }

        pos = payload_start + chunk_size + (chunk_size & 1); // blocos RIFF sao alinhados a 2 bytes
    }

    if (!got_fmt || !got_data) {
        error = "'" + path + "': faltam os blocos 'fmt '/'data'";
        return false;
    }
    if (format_tag != 1) {
        error = "'" + path + "': so' PCM e' suportado (formato " + std::to_string(format_tag) + " nao)";
        return false;
    }
    if (channels != 1) {
        error = "'" + path + "': so' mono e' suportado (" + std::to_string(channels) + " canais encontrados)";
        return false;
    }
    if (bits_per_sample != 8 && bits_per_sample != 16) {
        error = "'" + path + "': so' 8 ou 16 bits por amostra sao suportados (" + std::to_string(bits_per_sample) +
                " encontrado)";
        return false;
    }
    if (sample_rate == 0) {
        error = "'" + path + "': taxa de amostragem zero";
        return false;
    }

    out.sample_rate = sample_rate;
    const std::size_t bytes_per_sample = bits_per_sample / 8;
    const std::size_t sample_count = data_size / bytes_per_sample;
    out.samples.resize(sample_count);
    const uint8_t *data = bytes.data() + data_start;
    if (bits_per_sample == 8) {
        for (std::size_t i = 0; i < sample_count; ++i) {
            out.samples[i] = static_cast<int16_t>((static_cast<int32_t>(data[i]) - 128) * 256);
        }
    } else {
        for (std::size_t i = 0; i < sample_count; ++i) {
            out.samples[i] = static_cast<int16_t>(ReadLe16(data + i * 2));
        }
    }
    return true;
}

} // namespace tape
