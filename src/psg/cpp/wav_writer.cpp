#include "wav_writer.h"

#include <fstream>

namespace psg {

namespace {

void Put16(std::ofstream &f, uint16_t v) {
    const char b[2] = {static_cast<char>(v & 0xFF), static_cast<char>(v >> 8)};
    f.write(b, 2);
}

void Put32(std::ofstream &f, uint32_t v) {
    const char b[4] = {static_cast<char>(v & 0xFF), static_cast<char>((v >> 8) & 0xFF),
                       static_cast<char>((v >> 16) & 0xFF), static_cast<char>(v >> 24)};
    f.write(b, 4);
}

} // namespace

bool WriteWav(const std::string &path, const std::vector<int16_t> &samples, int sample_rate, std::string &error) {
    std::ofstream f(path, std::ios::binary);
    if (!f) {
        error = "nao foi possivel criar '" + path + "'";
        return false;
    }

    const uint32_t data_bytes = static_cast<uint32_t>(samples.size() * 2);
    f.write("RIFF", 4);
    Put32(f, 36 + data_bytes);
    f.write("WAVEfmt ", 8);
    Put32(f, 16);                                        // tamanho do bloco fmt
    Put16(f, 1);                                         // PCM
    Put16(f, 1);                                         // mono
    Put32(f, static_cast<uint32_t>(sample_rate));        // taxa
    Put32(f, static_cast<uint32_t>(sample_rate) * 2);    // bytes por segundo
    Put16(f, 2);                                         // bytes por amostra
    Put16(f, 16);                                        // bits por amostra
    f.write("data", 4);
    Put32(f, data_bytes);
    for (int16_t s : samples) Put16(f, static_cast<uint16_t>(s));

    if (!f) {
        error = "erro de escrita em '" + path + "'";
        return false;
    }
    return true;
}

} // namespace psg
