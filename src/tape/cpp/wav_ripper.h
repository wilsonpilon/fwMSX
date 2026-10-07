// fwMSX -- "ripper" de .WAV: demodula uma gravacao real de fita (PCM
// mono) para o formato .CAS (fast_bytes/marks, ver tape_image.h),
// detectando o piloto e decodificando os bytes do bloco #4B (Kansas
// City Standard) -- ver doc/tape-spec.md, secao 9. Algoritmo (deteccao
// de piloto por limiar adaptativo + decodificacao byte a byte com
// tolerancia) PORTADO do conceito do makeTSX
// (resource/makeTSX/BlockRipper.cpp e rippers/MSX4B_Ripper.cpp, MIT) --
// nenhum codigo copiado, so' simplificado para o caso fixo do MSX (sem
// os modos interativo/preditivo do original, ver a nota de limites no
// fim deste arquivo). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "tape_image.h"
#include "wav_reader.h"

namespace tape {

struct RipStats {
    std::size_t blocks_found = 0;
    std::size_t bytes_decoded = 0;
    std::vector<std::string> warnings;
};

// Demodula `wav`, ACRESCENTANDO a `fast_bytes`/`marks` (nao limpa o
// que ja estiver la' -- mesmo contrato de AppendCasBlock()/
// AppendCasFile(), para o chamador poder "ripar" direto numa fita
// existente, igual o --anexar do `pack`). Um AppendCasBlock() por
// bloco #4B encontrado -- cabecalho com nome e bloco de dados de um
// mesmo arquivo CSAVE'd saem como DUAS marcas, igual gravar pela BIOS
// de verdade. `tolerance_percent` ajusta a tolerancia de casamento dos
// pulsos contra a duracao esperada (um valor maior aceita gravacoes
// mais ruidosas/com mais jitter, ao custo de mais bytes decodificados
// errado). Devolve false com `error` se nenhum bloco foi encontrado
// (`fast_bytes`/`marks` ficam como estavam antes da chamada).
bool RipWavToCas(const WavSamples &wav, std::vector<uint8_t> &fast_bytes, std::vector<TapeMark> &marks,
                  int tolerance_percent, RipStats &stats, std::string &error);

// Limites desta primeira versao (ver doc/tape-spec.md, secao 9):
// - So' o bloco #4B/KCS do MSX (bitcfg/bytecfg fixos: 1 start bit (0),
//   8 de dados LSb primeiro, 2 stop bits (1), zero = 2 pulsos, um = 4
//   pulsos) -- o unico que o MSX usa de verdade (ver cas_format.h).
// - Sem os modos interativo/preditivo do makeTSX original (que pede
//   ajuda ao usuario ou tenta adivinhar um bit ambiguo olhando os
//   proximos) -- aqui, um bit ambiguo simplesmente termina o bloco
//   corrente (como se fosse silencio). Gravacoes muito ruidosas podem
//   perder o resto de um bloco por causa disso.
// - Sem normalize()/envelopeCorrection() do original (filtros de
//   volume/suavizacao) -- so' deteccao de limiar adaptativo (fracao do
//   pico do arquivo). Gravacoes com volume muito baixo ou MUITO
//   assimetrico podem nao ser detectadas bem.

} // namespace tape
