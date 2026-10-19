#include "tape_io.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>

#include "../core/kcs_codec.h"
#include "cas_format.h"
#include "cas_pack.h"
#include "cas_reader.h"
#include "tsx_writer.h"
#include "tzx_reader.h"
#include "wav_reader.h"
#include "wav_ripper.h"

namespace tape {

namespace {

constexpr uint64_t kZ80Clock = 3579545;
constexpr uint32_t kWavRate = 44100;
// Um "pulso" maior que isto nao e' meio-periodo de sinal: e' pausa. Os pulsos reais do MSX tem menos de 3000 T-states; os de
// piloto/dados nunca passam de 2*855.
constexpr uint32_t kSilenceThreshold = 50000;
// Pausa entre blocos no .wav (meio segundo, como o MSX de verdade espera entre o cabecalho do arquivo e os dados).
constexpr uint32_t kGapTStates = static_cast<uint32_t>(kZ80Clock / 2);

void SinkAppend(void *ctx, uint32_t t) { static_cast<std::vector<uint32_t> *>(ctx)->push_back(t); }

bool EndsWithCi(const std::string &s, const char *suffix) {
    const std::size_t n = std::strlen(suffix);
    if (s.size() < n) return false;
    return std::equal(s.end() - static_cast<std::ptrdiff_t>(n), s.end(), suffix,
                       [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == b; });
}

bool WriteBytes(const std::string &path, const std::vector<uint8_t> &bytes, std::string &error) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f || !f.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()))) {
        error = "nao foi possivel gravar '" + path + "'";
        return false;
    }
    return true;
}

void PutLe(std::vector<uint8_t> &v, uint32_t x, int bytes) {
    for (int k = 0; k < bytes; ++k) v.push_back(static_cast<uint8_t>((x >> (8 * k)) & 0xFF));
}

// "os 10 bytes iguais a D0/D3/EA seguidos de 6 de nome" -- um bloco que e' cabecalho de arquivo, nao dado.
bool IsFileHeaderBlock(const std::vector<uint8_t> &b, std::size_t begin, std::size_t len, uint8_t &id) {
    // o cabecalho de um arquivo tem SEMPRE 16 bytes (10 de tipo + 6 de nome); um bloco de dados que por acaso comece com
    // D0 x10 e tenha outro tamanho nao conta
    if (len != kCasFileIdBytes + kCasFileNameBytes) return false;
    id = b[begin];
    if (id != kCasIdBinary && id != kCasIdBasic && id != kCasIdAscii) return false;
    for (std::size_t i = 1; i < kCasFileIdBytes; ++i)
        if (b[begin + i] != id) return false;
    return true;
}

struct Block {
    std::size_t header = 0; // posicao do cabecalho de 8 bytes
    std::size_t begin = 0;  // primeiro byte do conteudo
    std::size_t len = 0;    // bytes do conteudo (exato se ha' marca; senao ate' o proximo cabecalho)
};

std::vector<Block> BlocksOf(const TapeImage &img) {
    std::vector<Block> out;
    const std::vector<uint8_t> &b = img.fast_bytes;
    std::size_t h = FindCasHeader(b, 0);
    while (h < b.size()) {
        Block blk;
        blk.header = h;
        blk.begin = h + kCasHeader.size();
        const std::size_t next = FindCasHeader(b, blk.begin);
        blk.len = next - blk.begin;
        for (const TapeMark &m : img.marks)
            if (m.fast_byte_offset == h && h + kCasHeader.size() + m.content_length <= b.size()) {
                blk.len = m.content_length;
                break;
            }
        out.push_back(blk);
        h = next;
    }
    return out;
}

void RippedToImage(std::vector<uint8_t> bytes, std::vector<TapeMark> marks, TapeImage &out) {
    out = TapeImage{};
    out.fast_bytes = std::move(bytes);
    out.marks = std::move(marks);
    PulsesFromMarks(out.fast_bytes, out.marks, out.pulses);
    out.files = ScanCasFiles(out.fast_bytes, out.marks);
}

} // namespace

std::string TapeExtension(const std::string &path) {
    if (EndsWithCi(path, ".cas")) return "cas";
    if (EndsWithCi(path, ".tsx") || EndsWithCi(path, ".tzx")) return "tsx";
    if (EndsWithCi(path, ".wav")) return "wav";
    return "";
}

void PulsesFromMarks(const std::vector<uint8_t> &bytes, std::vector<TapeMark> &marks, std::vector<uint32_t> &pulses) {
    const KcsByteFraming cfg{1, 0, 2, 1, 0, kMsxZeroPulsesPerBit, kMsxOnePulsesPerBit, kMsxZeroTStates, kMsxOneTStates};
    pulses.clear();
    for (TapeMark &m : marks) {
        const std::size_t begin = m.fast_byte_offset + kCasHeader.size();
        if (begin + m.content_length > bytes.size()) continue;
        m.pulse_index = pulses.size();
        for (uint32_t i = 0; i < kMsxPilotPulses; ++i) pulses.push_back(kMsxPilotTStates);
        for (std::size_t i = 0; i < m.content_length; ++i) kcs_emit_byte(&cfg, bytes[begin + i], &SinkAppend, &pulses);
    }
}

bool WriteWavFromPulses(const std::string &path, const std::vector<uint32_t> &pulses, std::string &error) {
    std::vector<int16_t> samples(kWavRate / 2, 0); // silencio inicial
    uint64_t t_cycles = 0;                         // tempo ja' consumido, em T-states
    bool high = true;
    for (const uint32_t p : pulses) {
        const uint64_t end_cycles = t_cycles + p;
        const uint64_t end_sample = end_cycles * kWavRate / kZ80Clock;
        const std::size_t want = static_cast<std::size_t>(end_sample) + kWavRate / 2;
        const bool silence = p >= kSilenceThreshold;
        if (silence) {
            while (samples.size() < want) samples.push_back(0);
            high = true; // depois da pausa o proximo piloto recomeca em cima
        } else {
            while (samples.size() < want) samples.push_back(high ? int16_t(24000) : int16_t(-24000));
            high = !high;
        }
        t_cycles = end_cycles;
    }
    samples.insert(samples.end(), kWavRate / 2, 0); // silencio final

    std::vector<uint8_t> wav{'R', 'I', 'F', 'F'};
    const uint32_t data_bytes = static_cast<uint32_t>(samples.size() * 2);
    PutLe(wav, 36 + data_bytes, 4);
    for (const char c : std::string("WAVEfmt ")) wav.push_back(static_cast<uint8_t>(c));
    PutLe(wav, 16, 4);
    PutLe(wav, 1, 2); // PCM
    PutLe(wav, 1, 2); // mono
    PutLe(wav, kWavRate, 4);
    PutLe(wav, kWavRate * 2, 4);
    PutLe(wav, 2, 2);
    PutLe(wav, 16, 2);
    for (const char c : std::string("data")) wav.push_back(static_cast<uint8_t>(c));
    PutLe(wav, data_bytes, 4);
    for (const int16_t s : samples) PutLe(wav, static_cast<uint16_t>(s), 2);
    return WriteBytes(path, wav, error);
}

bool LoadAnyTape(const std::string &path, TapeImage &out, std::string &error) {
    const std::string ext = TapeExtension(path);
    if (ext == "tsx") return LoadTzxImage(path, out, error);
    if (ext == "wav") {
        WavSamples wav;
        if (!LoadWav(path, wav, error)) return false;
        std::vector<uint8_t> bytes;
        std::vector<TapeMark> marks;
        RipStats stats;
        if (!RipWavToCas(wav, bytes, marks, 25, stats, error)) return false;
        RippedToImage(std::move(bytes), std::move(marks), out);
        return true;
    }
    return LoadCasImage(path, out, error);
}

bool SaveAnyTape(const std::string &path, const std::vector<uint8_t> &fast_bytes, const std::vector<TapeMark> &marks_in, std::string &error) {
    std::vector<TapeMark> marks = marks_in;
    if (marks.empty() && !fast_bytes.empty()) {
        std::vector<uint32_t> scratch;
        SynthesizeCasPulses(fast_bytes, scratch, &marks);
    }
    const std::string ext = TapeExtension(path);
    if (ext == "tsx") return WriteTsxFromCas(fast_bytes, marks, path, error);
    if (ext == "wav") {
        // pausa entre blocos: so' no .wav (o .tsx guarda a pausa no proprio bloco #4B)
        std::vector<uint32_t> pulses;
        for (std::size_t i = 0; i < marks.size(); ++i) {
            std::vector<TapeMark> one{marks[i]};
            std::vector<uint32_t> p;
            PulsesFromMarks(fast_bytes, one, p);
            if (i) pulses.push_back(kGapTStates);
            pulses.insert(pulses.end(), p.begin(), p.end());
        }
        return WriteWavFromPulses(path, pulses, error);
    }
    return WriteBytes(path, fast_bytes, error);
}

bool ReadTapeFile(const TapeImage &img, std::size_t index, TapeFileData &out, std::string &error) {
    out = TapeFileData{};
    if (index >= img.files.size()) {
        error = "arquivo " + std::to_string(index) + " nao existe na fita (" + std::to_string(img.files.size()) + " arquivos)";
        return false;
    }
    const TapeFileEntry &f = img.files[index];
    const std::vector<Block> blocks = BlocksOf(img);
    std::size_t bi = 0;
    while (bi < blocks.size() && blocks[bi].header != f.fast_byte_offset) ++bi;
    if (bi >= blocks.size()) {
        error = "fita inconsistente (cabecalho do arquivo nao encontrado)";
        return false;
    }
    const Block &hb = blocks[bi];
    out.type = f.type;
    out.name = f.name;
    // o cabecalho pode vir com o primeiro "dado" grudado no mesmo bloco (fitas sem o segundo marcador): aproveita
    std::size_t data_start = hb.begin + kCasFileIdBytes + kCasFileNameBytes;
    std::size_t data_end = hb.begin + hb.len;
    std::size_t next = bi + 1;
    if (data_start >= data_end) {
        data_start = data_end = 0;
        if (next < blocks.size()) {
            uint8_t id = 0;
            if (!IsFileHeaderBlock(img.fast_bytes, blocks[next].begin, blocks[next].len, id)) {
                data_start = blocks[next].begin;
                data_end = data_start + blocks[next].len;
                ++next;
            }
        }
    }
    out.data.assign(img.fast_bytes.begin() + static_cast<std::ptrdiff_t>(data_start), img.fast_bytes.begin() + static_cast<std::ptrdiff_t>(data_end));
    if (f.type == TapeFileType::Ascii) { // texto: blocos de 256 bytes, ate' o proximo arquivo
        for (; next < blocks.size(); ++next) {
            uint8_t id = 0;
            if (IsFileHeaderBlock(img.fast_bytes, blocks[next].begin, blocks[next].len, id)) break;
            out.data.insert(out.data.end(), img.fast_bytes.begin() + static_cast<std::ptrdiff_t>(blocks[next].begin),
                            img.fast_bytes.begin() + static_cast<std::ptrdiff_t>(blocks[next].begin + blocks[next].len));
        }
    } else if (f.type == TapeFileType::Binary) {
        if (out.data.size() < 6) {
            error = "arquivo binario '" + f.name + "' sem o cabecalho de 6 bytes (inicio, fim, execucao)";
            return false;
        }
        out.bin_start = static_cast<uint16_t>(out.data[0] | (out.data[1] << 8));
        out.bin_end = static_cast<uint16_t>(out.data[2] | (out.data[3] << 8));
        out.bin_exec = static_cast<uint16_t>(out.data[4] | (out.data[5] << 8));
        if (out.bin_end < out.bin_start) {
            error = "arquivo binario '" + f.name + "' com fim antes do inicio";
            return false;
        }
        const std::size_t want = static_cast<std::size_t>(out.bin_end - out.bin_start) + 1 + 6;
        if (out.data.size() < want) {
            error = "arquivo binario '" + f.name + "' incompleto (faltam " + std::to_string(want - out.data.size()) + " bytes)";
            return false;
        }
        out.data.resize(want); // descarta o enchimento de alinhamento do final
    }
    return true;
}

void AppendTapeFile(TapeImage &img, uint8_t type_id, const std::string &name, const std::vector<uint8_t> &data) {
    const std::size_t first_new = img.marks.size();
    AppendCasFile(img.fast_bytes, img.marks, type_id, name, data);
    const KcsByteFraming cfg{1, 0, 2, 1, 0, kMsxZeroPulsesPerBit, kMsxOnePulsesPerBit, kMsxZeroTStates, kMsxOneTStates};
    for (std::size_t i = first_new; i < img.marks.size(); ++i) {
        TapeMark &m = img.marks[i];
        m.pulse_index = img.pulses.size();
        for (uint32_t k = 0; k < kMsxPilotPulses; ++k) img.pulses.push_back(kMsxPilotTStates);
        const std::size_t begin = m.fast_byte_offset + kCasHeader.size();
        for (std::size_t k = 0; k < m.content_length; ++k) kcs_emit_byte(&cfg, img.fast_bytes[begin + k], &SinkAppend, &img.pulses);
    }
    img.files = ScanCasFiles(img.fast_bytes, img.marks);
    img.from_tsx = false;
}

std::vector<uint8_t> MakeBinaryContent(uint16_t start, uint16_t exec, const std::vector<uint8_t> &program) {
    const uint16_t end = static_cast<uint16_t>(start + (program.empty() ? 0 : program.size() - 1));
    std::vector<uint8_t> out{static_cast<uint8_t>(start & 0xFF), static_cast<uint8_t>(start >> 8), static_cast<uint8_t>(end & 0xFF),
                             static_cast<uint8_t>(end >> 8),     static_cast<uint8_t>(exec & 0xFF),  static_cast<uint8_t>(exec >> 8)};
    out.insert(out.end(), program.begin(), program.end());
    return out;
}

void TruncateAtFile(TapeImage &img, std::size_t index) {
    if (index >= img.files.size()) return;
    const std::size_t byte_cut = img.files[index].fast_byte_offset;
    const std::size_t pulse_cut = img.files[index].pulse_index;
    if (byte_cut < img.fast_bytes.size()) img.fast_bytes.resize(byte_cut);
    if (pulse_cut < img.pulses.size()) img.pulses.resize(pulse_cut);
    std::vector<TapeMark> kept;
    for (const TapeMark &m : img.marks)
        if (m.fast_byte_offset < byte_cut) kept.push_back(m);
    img.marks = kept;
    img.files = ScanCasFiles(img.fast_bytes, img.marks);
}

} // namespace tape
