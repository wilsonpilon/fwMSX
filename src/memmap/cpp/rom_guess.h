// Adaptado de fMSX (GuessROM() em resource/fMSX/fMSX/MSX.c), Copyright (C)
// Marat Fayzullin 1994-2021. O fwMSX evolui a partir do fMSX com o aval do
// autor original para adaptar/estudar seu codigo (ver README.md) -- isso nao
// e' uma relicenciacao: este arquivo continua sob os termos originais dele
// (nao-comercial, aviso ao autor em caso de mudanca), nao o BSD-3-Clause do
// restante do fwMSX. Ver LICENSE-THIRD-PARTY.md.
//
// Deteccao automatica do mapper de uma MegaROM (o MAP_GUESS do fMSX): conta
// as ocorrencias de `LD (nnnn),A` (opcode 32h) cujo endereco e' um registrador
// de banco caracteristico de cada mapper. NAO inclui a consulta a CARTS.CRC /
// CARTS.SHA do fMSX (tabelas de hashes de jogos conhecidos) -- so' a
// heuristica; um jogo que ela errar roda com `--cart <arq> <mapper>`.
// Ver doc/memory-map-spec.md.
#pragma once

#include <cstddef>
#include <cstdint>

#include "../common/memmap_types.h"

namespace memmap {

// Mapper mais provavel de uma MegaROM (Gen8, Gen16, Konami5, Konami4, ASCII8
// ou ASCII16). Empate -> o de menor indice, como o fMSX; o Gen8 comeca com
// uma vantagem e o ASCII8 com uma desvantagem (o ASCII16 e' preferido).
MemMapMapperType GuessMapper(const uint8_t *data, std::size_t size);

} // namespace memmap
