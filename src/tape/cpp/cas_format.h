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
// origem nao tem pulsos gravados (.CAS) ou para gerar um #4B novo
// (gravacao, ver tsx_writer.cpp). bitcfg 0x24/bytecfg 0x54 conferidos
// contra os defaults do bloco #4B do makeTSX (resource/makeTSX/TZX_Blocks.h).
//
// BUG corrigido em 2026-10-08: ZERO e UM estavam TROCADOS (e o piloto
// usava o valor do ZERO). A nota original do SPEC.md ("ZERO=855,
// UM=1710") veio dos defaults GENERICOS de ZX Spectrum do makeTSX
// (B10_Standard_Ripper.h/B11_Custom_Ripper.h) -- a propria nota avisava
// "conferir no codigo antes de usar", o que nao tinha sido feito. O #4B
// ESPECIFICO do MSX usa outra regra (resource/makeTSX/rippers/
// MSX4B_Ripper.h/.cpp): `bit0len (ZERO) = bit1len (UM) * 2`, e o piloto
// tem a MESMA duracao do UM (ver o comentario do Block4B em
// TZX_Blocks.h: "Duration of a PILOT pulse {same as ONE pulse}").
// Confirmado batendo meia duracao do ZERO/UM de um .TSX real (Dinamic,
// zero=1404/um=702 -- a mesma proporcao 2:1) contra um bug relatado pelo
// usuario: uma fita GRAVADA por este emulador carregava certo no modo
// rapido (nao usa pulso nenhum) mas nunca no modo normal (CLOAD nunca
// achava o piloto, so' o chiado -- os valores trocados geravam um sinal
// que a BIOS de verdade nao reconhecia como KCS valido).
constexpr uint32_t kMsxOneTStates = 855;
constexpr uint32_t kMsxZeroTStates = kMsxOneTStates * 2;
constexpr uint32_t kMsxPilotTStates = kMsxOneTStates;
constexpr uint32_t kMsxPilotPulses = 8000;
constexpr int kMsxZeroPulsesPerBit = 2;
constexpr int kMsxOnePulsesPerBit = 4;

} // namespace tape
