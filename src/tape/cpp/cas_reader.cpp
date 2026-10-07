#include "cas_reader.h"

#include <algorithm>
#include <fstream>

#include "../core/kcs_codec.h"
#include "cas_format.h"

namespace tape {

namespace {

void SinkAppend(void *ctx, uint32_t t) { static_cast<std::vector<uint32_t> *>(ctx)->push_back(t); }

std::size_t PulseIndexFor(const std::vector<TapeMark> &marks, std::size_t h) {
    for (const auto &m : marks) {
        if (m.fast_byte_offset == h) return m.pulse_index;
    }
    return 0;
}

} // namespace

std::size_t FindCasHeader(const std::vector<uint8_t> &bytes, std::size_t from) {
    if (from + kCasHeader.size() > bytes.size()) return bytes.size();
    const auto it = std::search(bytes.begin() + static_cast<std::ptrdiff_t>(from), bytes.end(), kCasHeader.begin(), kCasHeader.end());
    return it == bytes.end() ? bytes.size() : static_cast<std::size_t>(it - bytes.begin());
}

std::vector<TapeFileEntry> ScanCasFiles(const std::vector<uint8_t> &bytes, const std::vector<TapeMark> &marks) {
    std::vector<TapeFileEntry> out;
    std::size_t h = FindCasHeader(bytes, 0);
    while (h < bytes.size()) {
        const std::size_t id_pos = h + kCasHeader.size();
        if (id_pos + kCasFileIdBytes + kCasFileNameBytes <= bytes.size()) {
            const uint8_t id = bytes[id_pos];
            bool uniform = true;
            for (std::size_t i = 1; i < kCasFileIdBytes; ++i) {
                if (bytes[id_pos + i] != id) { uniform = false; break; }
            }
            if (uniform && (id == kCasIdBinary || id == kCasIdBasic || id == kCasIdAscii)) {
                TapeFileEntry entry;
                entry.type = id == kCasIdBinary ? TapeFileType::Binary : id == kCasIdBasic ? TapeFileType::Basic : TapeFileType::Ascii;
                std::string name(reinterpret_cast<const char *>(&bytes[id_pos + kCasFileIdBytes]), kCasFileNameBytes);
                while (!name.empty() && name.back() == ' ') name.pop_back();
                entry.name = name;
                entry.fast_byte_offset = h;
                entry.pulse_index = PulseIndexFor(marks, h);

                // O segundo cabecalho (antes dos dados de verdade) vem logo
                // depois do nome -- se nao estiver ali, os "dados" comecam
                // direto (arquivo sem conteudo, so' o cabecalho).
                std::size_t data_start = id_pos + kCasFileIdBytes + kCasFileNameBytes;
                if (FindCasHeader(bytes, data_start) == data_start) data_start += kCasHeader.size();
                const std::size_t data_end = FindCasHeader(bytes, data_start);
                entry.data_bytes = data_end - data_start;
                out.push_back(entry);
                h = FindCasHeader(bytes, data_end);
                continue;
            }
        }
        h = FindCasHeader(bytes, h + kCasHeader.size());
    }
    return out;
}

void SynthesizeCasPulses(const std::vector<uint8_t> &bytes, std::vector<uint32_t> &pulses,
                         std::vector<TapeMark> *marks_out) {
    const KcsByteFraming cfg{1, 0, 2, 1, 0, kMsxZeroPulsesPerBit, kMsxOnePulsesPerBit, kMsxZeroTStates, kMsxOneTStates};
    std::size_t pos = FindCasHeader(bytes, 0);
    while (pos < bytes.size()) {
        const std::size_t content_start = pos + kCasHeader.size();
        const std::size_t content_end = FindCasHeader(bytes, content_start);
        if (marks_out) marks_out->push_back({pos, pulses.size(), content_end - content_start});
        for (uint32_t i = 0; i < kMsxPilotPulses; ++i) pulses.push_back(kMsxPilotTStates);
        for (std::size_t i = content_start; i < content_end; ++i) kcs_emit_byte(&cfg, bytes[i], &SinkAppend, &pulses);
        pos = content_end;
    }
}

bool LoadCasImage(const std::string &path, TapeImage &out, std::string &error) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) {
        error = "nao foi possivel abrir '" + path + "'";
        return false;
    }
    const std::streamsize size = f.tellg();
    f.seekg(0);
    std::vector<uint8_t> bytes(static_cast<std::size_t>(size));
    if (size > 0 && !f.read(reinterpret_cast<char *>(bytes.data()), size)) {
        error = "nao foi possivel ler '" + path + "'";
        return false;
    }

    out = TapeImage{};
    out.fast_bytes = bytes;
    SynthesizeCasPulses(bytes, out.pulses, &out.marks);
    out.files = ScanCasFiles(bytes, out.marks);
    out.from_tsx = false;
    return true;
}

} // namespace tape
