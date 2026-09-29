//
// Nucleo C++ do msxdisk (fwMSX): geometria/BPB de um disco FAT12 MSX.
//

#include "geometry.h"

#include <cstring>

namespace msxdisk {

namespace {

void WriteLE16(uint8_t *dest, uint16_t value) {
    dest[0] = static_cast<uint8_t>(value & 0xFF);
    dest[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

uint16_t ReadLE16(const uint8_t *src) {
    return static_cast<uint16_t>(src[0] | (static_cast<uint16_t>(src[1]) << 8));
}

} // namespace

Geometry Geometry::Disk720KB() {
    return Geometry{};
}

Geometry Geometry::ParseFromBootSector(const uint8_t *boot_sector) {
    Geometry g;
    g.bytes_per_sector = ReadLE16(boot_sector + 0x0B);
    g.sectors_per_cluster = boot_sector[0x0D];
    g.reserved_sectors = ReadLE16(boot_sector + 0x0E);
    g.fat_count = boot_sector[0x10];
    g.root_dir_entries = ReadLE16(boot_sector + 0x11);
    g.total_sectors = ReadLE16(boot_sector + 0x13);
    g.media_descriptor = boot_sector[0x15];
    g.sectors_per_fat = ReadLE16(boot_sector + 0x16);
    g.sectors_per_track = ReadLE16(boot_sector + 0x18);
    g.head_count = ReadLE16(boot_sector + 0x1A);
    return g;
}

void Geometry::WriteBootSector(uint8_t *boot_sector) const {
    std::memset(boot_sector, 0, bytes_per_sector);

    // Fallback generico para geometrias sem um setor de boot real
    // conhecido ainda (ver DiskImage::CreateBlank, que usa o bootstrap
    // real de msxdos1_boot.h para o layout de 720KB). Aqui so gravamos um
    // JMP valido + o BPB, sem bootstrap -- disco reconhecido como FAT12,
    // mas sem boot funcional.
    boot_sector[0] = 0xEB;
    boot_sector[1] = 0xFE;
    boot_sector[2] = 0x90;
    std::memcpy(boot_sector + 3, "fwMSX   ", 8);

    WriteLE16(boot_sector + 0x0B, bytes_per_sector);
    boot_sector[0x0D] = sectors_per_cluster;
    WriteLE16(boot_sector + 0x0E, reserved_sectors);
    boot_sector[0x10] = fat_count;
    WriteLE16(boot_sector + 0x11, root_dir_entries);
    WriteLE16(boot_sector + 0x13, total_sectors);
    boot_sector[0x15] = media_descriptor;
    WriteLE16(boot_sector + 0x16, sectors_per_fat);
    WriteLE16(boot_sector + 0x18, sectors_per_track);
    WriteLE16(boot_sector + 0x1A, head_count);
}

uint32_t Geometry::RootDirSectors() const {
    const uint32_t entry_bytes = static_cast<uint32_t>(root_dir_entries) * 32u;
    return (entry_bytes + bytes_per_sector - 1u) / bytes_per_sector;
}

uint32_t Geometry::RootDirStartSector() const {
    return reserved_sectors + static_cast<uint32_t>(fat_count) * sectors_per_fat;
}

uint32_t Geometry::DataStartSector() const {
    return RootDirStartSector() + RootDirSectors();
}

uint32_t Geometry::TotalClusters() const {
    const uint32_t data_sectors = total_sectors - DataStartSector();
    return data_sectors / sectors_per_cluster;
}

uint32_t Geometry::BytesPerCluster() const {
    return static_cast<uint32_t>(sectors_per_cluster) * bytes_per_sector;
}

uint32_t Geometry::ImageSizeBytes() const {
    return static_cast<uint32_t>(total_sectors) * bytes_per_sector;
}

uint32_t Geometry::FatSizeBytes() const {
    return static_cast<uint32_t>(sectors_per_fat) * bytes_per_sector;
}

uint32_t Geometry::ClusterOffset(uint16_t cluster) const {
    // Clusters de dados comecam em 2 na numeracao da FAT.
    const uint32_t sector = DataStartSector() + (static_cast<uint32_t>(cluster) - 2u) * sectors_per_cluster;
    return sector * bytes_per_sector;
}

} // namespace msxdisk
