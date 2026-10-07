// fwMSX -- monta blocos .CAS (cabecalho de sincronismo + conteudo) fora
// do gancho de BIOS, para a ferramenta de linha de comando empacotar um
// .BIN/.BAS solto num .TSX/.CAS sem passar pelo emulador (ver
// src/tape/cli/cas_tool.cpp e doc/tape-spec.md, secao 8). Codigo
// ORIGINAL do fwMSX (BSD-3-Clause).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "tape_image.h"

namespace tape {

// Alinha `bytes` a um multiplo de 8 (mesma regra do TAPOON de verdade),
// acrescenta o marcador de sincronismo (kCasHeader) e `content`, e
// registra a marca correspondente em `marks` (pulse_index fica 0 -- so'
// importa para tocar a fita ja carregada na memoria, o que esta
// ferramenta nunca faz; WriteTsxFromCas() so' usa fast_byte_offset e
// content_length). Equivale, byte a byte, ao que TapeEngine::OnTapoon()
// + FinalizeWrite() fazem para UM TAPOON/TAPOOF -- so' que sem motor,
// sem BIOS, sem Z80.
void AppendCasBlock(std::vector<uint8_t> &bytes, const std::vector<uint8_t> &content, std::vector<TapeMark> &marks);

// Acrescenta um arquivo completo: o bloco do cabecalho (10 bytes do tipo
// + o nome, truncado/preenchido com espaco para 6 bytes) seguido do
// bloco de dados -- os mesmos DOIS blocos que um CSAVE/BSAVE de verdade
// grava (ver doc/tape-spec.md, secao 6). `type_id` e' kCasIdBinary/
// kCasIdBasic/kCasIdAscii (cas_format.h).
void AppendCasFile(std::vector<uint8_t> &bytes, std::vector<TapeMark> &marks, uint8_t type_id, const std::string &name,
                    const std::vector<uint8_t> &data);

} // namespace tape
