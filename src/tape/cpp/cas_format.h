// fwMSX -- convencao do arquivo .CAS, compartilhada pelo leitor de .CAS e
// pelo leitor de .TSX/.TZX (que reconstroi um fluxo equivalente a partir
// dos blocos #4B, ver tzx_reader.cpp). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Convencao conferida contra o openMSX
// (resource/openMSX/src/cassette/CasImage.cc, GPL, so' leitura) e o fMSX
// (resource/fMSX/fMSX/Patch.c, TapeHeader[]) -- os tres emuladores usam o
// MESMO marcador, nao e' invencao de nenhum deles. Ver doc/tape-spec.md.
#pragma once

#include <array>
#include <cstdint>

namespace tape {

// Marcador de sincronismo entre blocos logicos de um .CAS.
constexpr std::array<uint8_t, 8> kCasHeader = {0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74};

// Cabecalho de arquivo MSX: 10 bytes do mesmo valor (tipo) seguidos do
// nome (6 bytes, preenchido com espaco).
constexpr std::size_t kCasFileIdBytes = 10;
constexpr std::size_t kCasFileNameBytes = 6;
constexpr uint8_t kCasIdBinary = 0xD0;
constexpr uint8_t kCasIdBasic = 0xD3;
constexpr uint8_t kCasIdAscii = 0xEA;

// Parametros padrao do MSX para sintetizar pulsos (modo normal) quando a
// origem nao tem pulsos gravados (.CAS) ou para os blocos genericos do
// TZX -- conferidos contra os defaults do bloco #4B do makeTSX
// (resource/makeTSX/TZX_Blocks.h: bitcfg 0x24, bytecfg 0x54) e contra
// doc/SPEC.md, secao 5.2.
constexpr uint32_t kMsxPilotTStates = 1710;
constexpr uint32_t kMsxPilotPulses = 2000;
constexpr uint32_t kMsxZeroTStates = 855;
constexpr uint32_t kMsxOneTStates = 1710;
constexpr int kMsxZeroPulsesPerBit = 2;
constexpr int kMsxOnePulsesPerBit = 4;

} // namespace tape
