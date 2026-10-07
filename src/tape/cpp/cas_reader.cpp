#include "cas_reader.h"

#include <algorithm>
#include <fstream>

#include "../core/kcs_codec.h"
#include "cas_format.h"

namespace tape {

namespace {

// Posicao do proximo kCasHeader a partir de `from`, ou bytes.size() se
// nao achar (facilita os loops abaixo -- nunca devolve npos).
std::size_t FindHeader(const std::vector<uint8_t> &bytes, std::size_t from) {
    if (from + kCasHeader.size() > bytes.size()) return bytes.size();
    const auto it = std::search(bytes.begin() + static_cast<std::ptrdiff_t>(from), bytes.end(), kCasHeader.begin(), kCasHeader.end());
    return it == bytes.end() ? bytes.size() : static_cast<std::size_t>(it - bytes.begin());
}

void SinkAppend(void *ctx, uint32_t t) { static_cast<std::vector<uint32_t> *>(ctx)->push_back(t); }

} // namespace

std::vector<TapeFileEntry> ScanCasFiles(const std::vector<uint8_t> &bytes) {
    std::vector<TapeFileEntry> out;
    std::size_t h = FindHeader(bytes, 0);
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

                // O segundo cabecalho (antes dos dados de verdade) vem logo
                // depois do nome -- se nao estiver ali, os "dados" comecam
                // direto (arquivo sem conteudo, so' o cabecalho).
                std::size_t data_start = id_pos + kCasFileIdBytes + kCasFileNameBytes;
                if (FindHeader(bytes, data_start) == data_start) data_start += kCasHeader.size();
                const std::size_t data_end = FindHeader(bytes, data_start);
                entry.data_bytes = data_end - data_start;
                out.push_back(entry);
                h = FindHeader(bytes, data_end);
                continue;
            }
        }
        h = FindHeader(bytes, h + kCasHeader.size());
    }
    return out;
}

void SynthesizeCasPulses(const std::vector<uint8_t> &bytes, std::vector<uint32_t> &pulses) {
    const KcsByteFraming cfg{1, 0, 2, 1, 0, kMsxZeroPulsesPerBit, kMsxOnePulsesPerBit, kMsxZeroTStates, kMsxOneTStates};
    std::size_t pos = FindHeader(bytes, 0);
    while (pos < bytes.size()) {
        const std::size_t content_start = pos + kCasHeader.size();
        const std::size_t content_end = FindHeader(bytes, content_start);
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
    out.files = ScanCasFiles(bytes);
    SynthesizeCasPulses(bytes, out.pulses);
    out.from_tsx = false;
    return true;
}

} // namespace tape
