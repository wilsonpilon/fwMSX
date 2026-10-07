// fwMSX -- imagem de fita carregada (.CAS ou .TSX/.TZX), pronta para os
// dois modos de carregamento. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/tape-spec.md.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace tape {

// Tipo do arquivo logico, pelo cabecalho de 10 bytes do .CAS (0xD0
// binario, 0xD3 BASIC, 0xEA ASCII -- ver doc/tape-spec.md).
enum class TapeFileType { Unknown, Binary, Basic, Ascii };

struct TapeFileEntry {
    TapeFileType type = TapeFileType::Unknown;
    std::string name;             // 6 bytes do cabecalho, sem espacos a direita
    std::size_t data_bytes = 0;   // bytes de dados depois do segundo cabecalho
    // Posicao do INICIO deste arquivo (o primeiro cabecalho de 8 bytes),
    // em `fast_bytes` e em `pulses` -- usado para marcar o ponto de carga
    // (TapeEngine::SeekToFile()) nos dois modos. Ver doc/tape-spec.md.
    std::size_t fast_byte_offset = 0;
    std::size_t pulse_index = 0;
};

// Imagem pronta para os dois modos de carregamento (ver doc/tape-spec.md):
//  - `fast_bytes`: fluxo equivalente a um .CAS (cabecalhos de 8 bytes
//    1F A6 DE BA CC 13 7D 74 entre blocos, igual ao fMSX/openMSX),
//    consumido pelo gancho de BIOS do modo rapido (TAPION/TAPIN/TAPIOF).
//  - `pulses`: pulsos (meio-periodos, T-states de Z80) da fita inteira,
//    reproduzidos pela porta de verdade no modo normal (com som).
struct TapeImage {
    std::vector<uint8_t> fast_bytes;
    std::vector<uint32_t> pulses;
    std::vector<TapeFileEntry> files;
    bool from_tsx = false;
    // Blocos do TZX reconhecidos mas nao reproduzidos (grupos, laços,
    // saltos, gravacao direta...) -- ver doc/tape-spec.md, "Limites".
    // So' para diagnostico; nao impede o carregamento.
    std::vector<std::string> skipped_blocks;
};

} // namespace tape
