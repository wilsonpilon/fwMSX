//
// Nucleo C++ do msxdisk (fwMSX): geometria/BPB de um disco FAT12 MSX.
//
#pragma once

#include <cstdint>

namespace msxdisk {

// Descreve o BPB (BIOS Parameter Block) de um formato de disco MSX FAT12.
// Por enquanto so o formato DS/DD de 720KB esta disponivel para criacao
// (valores conferidos byte a byte contra o boot sector original do fMSX
// em resource/DiskUtilities/Boot.h); Load() consegue ler o BPB de
// qualquer imagem valida, mesmo com outra geometria -- ver
// doc/msxdisk-spec.md, secao 6.
struct Geometry {
    uint16_t bytes_per_sector = 512;
    uint8_t sectors_per_cluster = 2;
    uint16_t reserved_sectors = 1;
    uint8_t fat_count = 2;
    uint16_t root_dir_entries = 112;
    uint16_t total_sectors = 1440;
    uint8_t media_descriptor = 0xF9;
    uint16_t sectors_per_fat = 3;
    uint16_t sectors_per_track = 9;
    uint16_t head_count = 2;

    static Geometry Disk720KB();

    // Le o BPB a partir dos primeiros bytes (setor de boot) de uma
    // imagem existente.
    static Geometry ParseFromBootSector(const uint8_t *boot_sector);

    // Grava este BPB (e um salto/OEM minimos, sem bootloader real) no
    // inicio de 'boot_sector' (deve ter ao menos bytes_per_sector bytes).
    void WriteBootSector(uint8_t *boot_sector) const;

    uint32_t RootDirSectors() const;
    uint32_t RootDirStartSector() const;
    uint32_t DataStartSector() const;
    uint32_t TotalClusters() const;
    uint32_t BytesPerCluster() const;
    uint32_t ImageSizeBytes() const;
    uint32_t FatSizeBytes() const;

    // Deslocamento (em bytes) do primeiro byte do cluster 'cluster'
    // (numeracao FAT: os dois primeiros clusters de dados sao 2 e 3).
    uint32_t ClusterOffset(uint16_t cluster) const;
};

} // namespace msxdisk
