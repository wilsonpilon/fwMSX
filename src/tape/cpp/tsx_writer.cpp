#include "tsx_writer.h"

#include <fstream>

#include "cas_format.h"

namespace tape {

namespace {

void AppendU16(std::vector<uint8_t> &v, uint16_t x) {
    v.push_back(static_cast<uint8_t>(x & 0xFF));
    v.push_back(static_cast<uint8_t>((x >> 8) & 0xFF));
}

void AppendU32(std::vector<uint8_t> &v, uint32_t x) {
    v.push_back(static_cast<uint8_t>(x & 0xFF));
    v.push_back(static_cast<uint8_t>((x >> 8) & 0xFF));
    v.push_back(static_cast<uint8_t>((x >> 16) & 0xFF));
    v.push_back(static_cast<uint8_t>((x >> 24) & 0xFF));
}

// Bloco #4B (Kansas City Standard) com os parametros padrao do MSX --
// ver cas_format.h e doc/tape-spec.md. Pausa de 1s depois de cada bloco
// (o mesmo default do makeTSX para o piloto/sync).
void AppendBlock4B(std::vector<uint8_t> &v, const uint8_t *data, std::size_t size) {
    v.push_back(0x4B);
    AppendU32(v, static_cast<uint32_t>(12 + size));
    AppendU16(v, 1000); // pausa
    AppendU16(v, static_cast<uint16_t>(kMsxPilotTStates));
    AppendU16(v, static_cast<uint16_t>(kMsxPilotPulses));
    AppendU16(v, static_cast<uint16_t>(kMsxZeroTStates));
    AppendU16(v, static_cast<uint16_t>(kMsxOneTStates));
    v.push_back(static_cast<uint8_t>((kMsxZeroPulsesPerBit << 4) | kMsxOnePulsesPerBit)); // bitcfg: 0x24
    v.push_back(0x54); // bytecfg: 1 bit de inicio (0), 2 de fim (1), LSb primeiro
    v.insert(v.end(), data, data + size);
}

} // namespace

bool WriteTsxFromCas(const std::vector<uint8_t> &fast_bytes, const std::vector<TapeMark> &marks, const std::string &path,
                      std::string &error) {
    std::vector<uint8_t> tsx = {'Z', 'X', 'T', 'a', 'p', 'e', '!', 0x1A, 1, 20};
    // Usa o tamanho EXATO de cada bloco (`marks`), nao "ate' o proximo
    // cabecalho" -- essa busca generica mistura o preenchimento de
    // alinhamento (que o proprio TAPOON insere antes do PROXIMO cabecalho,
    // ver TapeEngine::OnTapoon()) com o conteudo de verdade do bloco
    // ANTERIOR. Bug real encontrado em 2026-10-08: uma fita gravada e
    // depois RECARREGADA (ejetada e reinserida) tinha esse preenchimento
    // codificado como dado, so' no modo normal. Ver doc/tape-spec.md,
    // secao 5/6.
    for (const TapeMark &mark : marks) {
        const std::size_t content_start = mark.fast_byte_offset + kCasHeader.size();
        if (content_start + mark.content_length > fast_bytes.size()) continue; // marca invalida -- nunca deveria acontecer
        AppendBlock4B(tsx, fast_bytes.data() + content_start, mark.content_length);
    }

    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f || !f.write(reinterpret_cast<const char *>(tsx.data()), static_cast<std::streamsize>(tsx.size()))) {
        error = "nao foi possivel gravar '" + path + "'";
        return false;
    }
    return true;
}

} // namespace tape
