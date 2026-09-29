//
// Nucleo C++ do msxdisk (fwMSX): entrada de diretorio FAT12 (32 bytes) e
// nomes 8.3 do MSX-DOS.
//
#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace msxdisk {

#pragma pack(push, 1)
struct RawDirEntry {
    char name[8];
    char ext[3];
    uint8_t attr;
    uint8_t reserved[10];
    uint16_t time;
    uint16_t date;
    uint16_t first_cluster;
    uint32_t size;
};
#pragma pack(pop)

static_assert(sizeof(RawDirEntry) == 32,
              "RawDirEntry deve ter exatamente 32 bytes (entrada de diretorio FAT12)");

enum : uint8_t {
    kAttrReadOnly = 0x01,
    kAttrHidden = 0x02,
    kAttrSystem = 0x04,
    kAttrVolumeLabel = 0x08,
    kAttrDirectory = 0x10,
    kAttrArchive = 0x20,
};

// Primeiro byte do nome numa entrada livre (nunca usada) ou apagada.
constexpr uint8_t kEntryFree = 0x00;
constexpr uint8_t kEntryDeleted = 0xE5;

// Converte um nome "de exibicao" (ex.: "arquivo.bas") no par nome(8)/
// extensao(3) maiusculo e preenchido com espaco usado nas entradas de
// diretorio e na comparacao com coringas.
std::array<char, 11> ToFat83Name(const std::string &display_name);

// Converte um padrao com coringas MSX-DOS ('*'/'?') no formato de 11
// bytes usado por msxdisk_name_match: '*' expande preenchendo o restante
// do campo (nome ou extensao) com '?'.
std::array<char, 11> ToFat83Pattern(const std::string &display_pattern);

// Reconstroi o nome "de exibicao" (ex.: "ARQUIVO.BAS") a partir dos
// campos brutos de uma entrada de diretorio.
std::string FromFat83Name(const char name[8], const char ext[3]);

} // namespace msxdisk
