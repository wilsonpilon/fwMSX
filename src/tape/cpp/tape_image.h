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

// Um segmento (entre dois cabecalhos de 8 bytes) com o tamanho EXATO do
// conteudo de verdade -- mais completo que TapeFileEntry (guarda TODO
// segmento, nao so' os que parecem um arquivo) e sem a ambiguidade de
// "procurar o proximo cabecalho" (que mistura preenchimento de
// alinhamento com dado de verdade -- ver doc/tape-spec.md, secao 5).
// TapeEngine usa isto para gerar/persistir pulsos corretos depois de uma
// gravacao (TAPOON/TAPOUT/TAPOOF); o leitor de .CAS so' pode adivinhar
// (content_length = ate' o proximo cabecalho, PODE incluir preenchimento
// se o arquivo .cas de origem tiver sido gravado por este emulador); o
// leitor de .TSX sabe o tamanho exato de verdade (vem do proprio campo
// `N` do bloco #4B).
struct TapeMark {
    std::size_t fast_byte_offset = 0; // posicao do cabecalho de 8 bytes
    std::size_t pulse_index = 0;      // indice do pulso correspondente (o 1o do piloto)
    std::size_t content_length = 0;   // bytes de conteudo DEPOIS do cabecalho
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
    // Um TapeMark por segmento encontrado (ver TapeMark acima).
    std::vector<TapeMark> marks;
};

} // namespace tape
