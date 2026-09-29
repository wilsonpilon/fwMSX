//
// Nucleo C++ do msxdisk (fwMSX): imagem de disco FAT12 MSX em memoria.
//

#include "disk_image.h"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <fstream>
#include <system_error>

#include "../asm/name_match.h"
#include "../fat12_c/fat12.h"
#include "../fortran/geometry_calc.h"
#include "msx_path.h"
#include "msxdos1_boot.h"

namespace msxdisk {

namespace {

constexpr uint16_t kFatFreeCluster = 0x000;
constexpr uint16_t kFatMinEof = 0xFF8; // valor de FAT >= isso marca fim de cadeia
constexpr uint16_t kFirstDataCluster = 2;

bool HasWildcard(const std::string &pattern) {
    return pattern.find('*') != std::string::npos || pattern.find('?') != std::string::npos;
}

void PackNow(uint16_t *date, uint16_t *time) {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    const int year = local.tm_year + 1900;
    const int fat_year = (year >= 1980) ? (year - 1980) : 0;
    *date = static_cast<uint16_t>((fat_year << 9) | ((local.tm_mon + 1) << 5) | local.tm_mday);
    *time = static_cast<uint16_t>((local.tm_hour << 11) | (local.tm_min << 5) | (local.tm_sec / 2));
}

void SplitEntryName(const RawDirEntry &entry, char name11[11]) {
    std::memcpy(name11, entry.name, 8);
    std::memcpy(name11 + 8, entry.ext, 3);
}

// Grava a entrada especial "." ou ".." (usadas dentro de um subdiretorio
// recem-criado) apontando para 'cluster'.
void WriteSpecialEntry(RawDirEntry *entry, const char *dots, uint16_t cluster) {
    std::memset(entry, 0, sizeof(RawDirEntry));
    std::memset(entry->name, ' ', sizeof(entry->name));
    std::memset(entry->ext, ' ', sizeof(entry->ext));
    std::memcpy(entry->name, dots, std::strlen(dots));
    entry->attr = kAttrDirectory;
    entry->first_cluster = cluster;
    PackNow(&entry->date, &entry->time);
}

std::string JoinMsxPath(const std::string &dir_path, const std::string &name) {
    if (dir_path.empty()) return name;
    return dir_path + "\\" + name;
}

std::vector<std::string> AllComponents(const MsxPath &parsed) {
    std::vector<std::string> all = parsed.dir_components;
    if (!parsed.leaf_name.empty()) all.push_back(parsed.leaf_name);
    return all;
}

} // namespace

DiskImage::DiskImage(Geometry geometry)
    : geometry_(geometry), data_(geometry.ImageSizeBytes(), 0) {}

uint8_t *DiskImage::FatPtr() {
    return data_.data() + static_cast<size_t>(geometry_.reserved_sectors) * geometry_.bytes_per_sector;
}

const uint8_t *DiskImage::FatPtr() const {
    return data_.data() + static_cast<size_t>(geometry_.reserved_sectors) * geometry_.bytes_per_sector;
}

void DiskImage::SyncFatCopies() {
    const uint8_t *primary = FatPtr();
    const uint32_t fat_bytes = geometry_.FatSizeBytes();
    for (uint8_t i = 1; i < geometry_.fat_count; ++i) {
        uint8_t *copy = data_.data() +
                         (static_cast<size_t>(geometry_.reserved_sectors) +
                          static_cast<size_t>(i) * geometry_.sectors_per_fat) *
                             geometry_.bytes_per_sector;
        std::memcpy(copy, primary, fat_bytes);
    }
}

RawDirEntry *DiskImage::RootDirBegin() {
    return reinterpret_cast<RawDirEntry *>(
        data_.data() + static_cast<size_t>(geometry_.RootDirStartSector()) * geometry_.bytes_per_sector);
}

const RawDirEntry *DiskImage::RootDirBegin() const {
    return reinterpret_cast<const RawDirEntry *>(
        data_.data() + static_cast<size_t>(geometry_.RootDirStartSector()) * geometry_.bytes_per_sector);
}

void DiskImage::ZeroCluster(uint16_t cluster) {
    std::memset(data_.data() + geometry_.ClusterOffset(cluster), 0, geometry_.BytesPerCluster());
}

DiskImage DiskImage::CreateBlank(const Geometry &geometry) {
    DiskImage img(geometry);

    const Geometry &ref = Geometry::Disk720KB();
    const bool is_720kb_layout = geometry.bytes_per_sector == ref.bytes_per_sector &&
                                  geometry.sectors_per_cluster == ref.sectors_per_cluster &&
                                  geometry.total_sectors == ref.total_sectors;
    if (is_720kb_layout) {
        // Setor de boot REAL (bootstrap Z80 que procura e carrega
        // MSXDOS.SYS, com fallback gracioso de "Boot error" se nao
        // encontrar) -- serve tanto para MSX-DOS 1 quanto para MSX-DOS 2
        // (o bootstrap so sabe carregar o arquivo chamado MSXDOS.SYS; a
        // distincao de versao fica por conta do proprio MSXDOS.SYS/
        // MSXDOS2.SYS). Identico ao usado por resource/msxDiskUtil e
        // resource/DiskUtilities/Boot.h -- ver msxdos1_boot.h.
        std::memcpy(img.data_.data(), kMsxDos1BootSector720KB, sizeof(kMsxDos1BootSector720KB));
    } else {
        // Geometria ainda sem um setor de boot real conhecido: grava so o
        // BPB (disco reconhecido como FAT12, mas sem bootstrap MSX-DOS).
        img.geometry_.WriteBootSector(img.data_.data());
    }

    // As duas copias da FAT comecam identicas: entrada 0 guarda o media
    // descriptor (convencao FAT12), entrada 1 e reservada como EOF; o
    // restante nasce livre (0) porque data_ ja e zero-inicializado.
    uint8_t *fat = img.FatPtr();
    msxdisk_fat12_set(fat, 0, static_cast<uint16_t>(0x0F00u | geometry.media_descriptor));
    msxdisk_fat12_set(fat, 1, 0x0FFFu);
    img.SyncFatCopies();

    return img;
}

std::optional<DiskImage> DiskImage::Load(const std::filesystem::path &path, DiskError *error) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) {
        if (error) *error = DiskError::IoError;
        return std::nullopt;
    }

    const std::streamoff size = in.tellg();
    if (size < 512) {
        if (error) *error = DiskError::NotAnMsxImage;
        return std::nullopt;
    }
    in.seekg(0);

    uint8_t boot[512];
    in.read(reinterpret_cast<char *>(boot), sizeof(boot));

    const Geometry geometry = Geometry::ParseFromBootSector(boot);
    if (geometry.bytes_per_sector == 0 || geometry.total_sectors == 0 ||
        static_cast<std::streamoff>(geometry.ImageSizeBytes()) != size) {
        if (error) *error = DiskError::NotAnMsxImage;
        return std::nullopt;
    }

    DiskImage img(geometry);
    in.seekg(0);
    in.read(reinterpret_cast<char *>(img.data_.data()), static_cast<std::streamsize>(img.data_.size()));
    if (!in) {
        if (error) *error = DiskError::IoError;
        return std::nullopt;
    }

    if (error) *error = DiskError::None;
    return img;
}

bool DiskImage::Save(const std::filesystem::path &path, DiskError *error) const {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        if (error) *error = DiskError::IoError;
        return false;
    }
    out.write(reinterpret_cast<const char *>(data_.data()), static_cast<std::streamsize>(data_.size()));
    if (!out) {
        if (error) *error = DiskError::IoError;
        return false;
    }
    if (error) *error = DiskError::None;
    return true;
}

uint32_t DiskImage::DirSlotCount(const DirLocation &dir) const {
    if (dir.is_root) {
        return geometry_.root_dir_entries;
    }

    const uint32_t entries_per_cluster = geometry_.BytesPerCluster() / sizeof(RawDirEntry);
    const uint8_t *fat = FatPtr();
    uint32_t count = 0;
    uint16_t cluster = dir.first_cluster;
    const uint32_t max_iters = geometry_.TotalClusters() + 2;
    for (uint32_t i = 0; i < max_iters && cluster >= kFirstDataCluster; ++i) {
        count += entries_per_cluster;
        const uint16_t next = msxdisk_fat12_get(fat, cluster);
        if (next >= kFatMinEof) break;
        cluster = next;
    }
    return count;
}

RawDirEntry *DiskImage::DirSlot(const DirLocation &dir, uint32_t index) {
    if (dir.is_root) {
        return &RootDirBegin()[index];
    }

    const uint32_t entries_per_cluster = geometry_.BytesPerCluster() / sizeof(RawDirEntry);
    const uint32_t cluster_steps = index / entries_per_cluster;
    const uint32_t offset_in_cluster = index % entries_per_cluster;

    const uint8_t *fat = FatPtr();
    uint16_t cluster = dir.first_cluster;
    for (uint32_t i = 0; i < cluster_steps; ++i) {
        cluster = msxdisk_fat12_get(fat, cluster);
    }

    RawDirEntry *base = reinterpret_cast<RawDirEntry *>(data_.data() + geometry_.ClusterOffset(cluster));
    return base + offset_in_cluster;
}

const RawDirEntry *DiskImage::DirSlot(const DirLocation &dir, uint32_t index) const {
    return const_cast<DiskImage *>(this)->DirSlot(dir, index);
}

RawDirEntry *DiskImage::FindFreeSlotIn(const DirLocation &dir, DiskError *error) {
    const uint32_t count = DirSlotCount(dir);
    for (uint32_t i = 0; i < count; ++i) {
        RawDirEntry *e = DirSlot(dir, i);
        const auto first_byte = static_cast<uint8_t>(e->name[0]);
        if (first_byte == kEntryFree || first_byte == kEntryDeleted) {
            if (error) *error = DiskError::None;
            return e;
        }
    }

    if (dir.is_root) {
        if (error) *error = DiskError::DirectoryFull;
        return nullptr;
    }

    // Subdiretorio cheio: estende a cadeia com mais um cluster. Acha o
    // ultimo cluster da cadeia atual e encadeia um novo cluster livre
    // manualmente (nao usa AllocateClusterChain, que comecaria uma cadeia
    // nova do zero em vez de continuar esta).
    const uint8_t *fat_read = FatPtr();
    uint16_t last = dir.first_cluster;
    const uint32_t max_iters = geometry_.TotalClusters() + 2;
    for (uint32_t i = 0; i < max_iters; ++i) {
        const uint16_t next = msxdisk_fat12_get(fat_read, last);
        if (next >= kFatMinEof) break;
        last = next;
    }

    uint8_t *fat = FatPtr();
    uint16_t new_cluster = 0;
    const uint32_t total = geometry_.TotalClusters();
    for (uint32_t offset = 0; offset < total; ++offset) {
        const auto c = static_cast<uint16_t>(kFirstDataCluster + offset);
        if (msxdisk_fat12_get(fat, c) == kFatFreeCluster) {
            new_cluster = c;
            break;
        }
    }
    if (new_cluster == 0) {
        if (error) *error = DiskError::DiskFull;
        return nullptr;
    }

    msxdisk_fat12_set(fat, last, new_cluster);
    msxdisk_fat12_set(fat, new_cluster, 0x0FFFu);
    SyncFatCopies();
    ZeroCluster(new_cluster);

    if (error) *error = DiskError::None;
    return reinterpret_cast<RawDirEntry *>(data_.data() + geometry_.ClusterOffset(new_cluster));
}

RawDirEntry *DiskImage::FindEntryIn(const DirLocation &dir, const std::string &name) {
    const auto pattern = ToFat83Name(name);
    const uint32_t count = DirSlotCount(dir);
    for (uint32_t i = 0; i < count; ++i) {
        RawDirEntry *e = DirSlot(dir, i);
        const auto first_byte = static_cast<uint8_t>(e->name[0]);
        if (first_byte == kEntryFree || first_byte == kEntryDeleted) continue;
        if (e->attr & kAttrVolumeLabel) continue;

        char name11[11];
        SplitEntryName(*e, name11);
        if (msxdisk_name_match(pattern.data(), name11)) {
            return e;
        }
    }
    return nullptr;
}

const RawDirEntry *DiskImage::FindEntryIn(const DirLocation &dir, const std::string &name) const {
    return const_cast<DiskImage *>(this)->FindEntryIn(dir, name);
}

std::optional<DiskImage::DirLocation> DiskImage::ResolveDir(const std::vector<std::string> &dir_components,
                                                             DiskError *error) const {
    DirLocation current;
    current.is_root = true;
    current.first_cluster = 0;

    for (const auto &component : dir_components) {
        const RawDirEntry *entry = FindEntryIn(current, component);
        if (!entry) {
            if (error) *error = DiskError::PathNotFound;
            return std::nullopt;
        }
        if (!(entry->attr & kAttrDirectory)) {
            if (error) *error = DiskError::NotADirectory;
            return std::nullopt;
        }
        current.is_root = false;
        current.first_cluster = entry->first_cluster;
    }

    if (error) *error = DiskError::None;
    return current;
}

uint16_t DiskImage::AllocateClusterChain(uint32_t byte_length, DiskError *error) {
    if (byte_length == 0) {
        return 0; // arquivo de tamanho zero nao ocupa cluster (convencao FAT).
    }

    const uint32_t bpc = geometry_.BytesPerCluster();
    const uint32_t needed = (byte_length + bpc - 1) / bpc;
    const uint32_t total = geometry_.TotalClusters();

    std::vector<uint16_t> free_list;
    free_list.reserve(needed);
    uint8_t *fat = FatPtr();
    for (uint32_t offset = 0; offset < total && free_list.size() < needed; ++offset) {
        const auto cluster = static_cast<uint16_t>(kFirstDataCluster + offset);
        if (msxdisk_fat12_get(fat, cluster) == kFatFreeCluster) {
            free_list.push_back(cluster);
        }
    }

    if (free_list.size() < needed) {
        if (error) *error = DiskError::DiskFull;
        return 0;
    }

    for (size_t i = 0; i + 1 < free_list.size(); ++i) {
        msxdisk_fat12_set(fat, free_list[i], free_list[i + 1]);
    }
    msxdisk_fat12_set(fat, free_list.back(), 0x0FFFu);
    SyncFatCopies();

    if (error) *error = DiskError::None;
    return free_list.front();
}

void DiskImage::FreeClusterChain(uint16_t first_cluster) {
    if (first_cluster < kFirstDataCluster) return;

    uint8_t *fat = FatPtr();
    uint16_t cluster = first_cluster;
    const uint32_t max_iters = geometry_.TotalClusters() + 2;

    for (uint32_t i = 0; i < max_iters && cluster >= kFirstDataCluster; ++i) {
        const uint16_t next = msxdisk_fat12_get(fat, cluster);
        msxdisk_fat12_set(fat, cluster, kFatFreeCluster);
        if (next >= kFatMinEof || next == kFatFreeCluster) break;
        cluster = next;
    }
    SyncFatCopies();
}

uint32_t DiskImage::CountUsedClusters() const {
    const uint8_t *fat = FatPtr();
    uint32_t used = 0;
    const uint32_t total = geometry_.TotalClusters();
    for (uint32_t offset = 0; offset < total; ++offset) {
        const auto cluster = static_cast<uint16_t>(kFirstDataCluster + offset);
        if (msxdisk_fat12_get(fat, cluster) != kFatFreeCluster) ++used;
    }
    return used;
}

std::vector<FileEntry> DiskImage::ListDirectory(const std::string &dir_path, const std::string &pattern,
                                                 DiskError *error) const {
    std::vector<FileEntry> out;

    const auto components = AllComponents(ParseMsxPath(dir_path));
    DiskError local_error = DiskError::None;
    const auto dir = ResolveDir(components, &local_error);
    if (!dir) {
        if (error) *error = local_error;
        return out;
    }

    const bool has_pattern = !pattern.empty();
    const auto pattern11 = has_pattern ? ToFat83Pattern(pattern) : std::array<char, 11>{};

    const uint32_t count = DirSlotCount(*dir);
    for (uint32_t i = 0; i < count; ++i) {
        const RawDirEntry *e = DirSlot(*dir, i);
        const auto first_byte = static_cast<uint8_t>(e->name[0]);
        if (first_byte == kEntryFree || first_byte == kEntryDeleted) continue;
        if (e->attr & kAttrVolumeLabel) continue;
        if (e->name[0] == '.') continue; // esconde "." e ".." (igual ao DIR do MSX-DOS 2 real)

        if (has_pattern) {
            char name11[11];
            SplitEntryName(*e, name11);
            if (!msxdisk_name_match(pattern11.data(), name11)) continue;
        }

        FileEntry fe;
        fe.name = FromFat83Name(e->name, e->ext);
        fe.size = e->size;
        fe.first_cluster = e->first_cluster;
        fe.attr = e->attr;
        fe.date = e->date;
        fe.time = e->time;
        out.push_back(std::move(fe));
    }

    if (error) *error = DiskError::None;
    return out;
}

bool DiskImage::AddFile(const std::filesystem::path &host_path, const std::string &msx_path, uint8_t attr,
                         DiskError *error) {
    const auto parsed = ParseMsxPath(msx_path);

    DiskError local_error = DiskError::None;
    const auto parent = ResolveDir(parsed.dir_components, &local_error);
    if (!parent) {
        if (error) *error = local_error;
        return false;
    }

    if (FindEntryIn(*parent, parsed.leaf_name) != nullptr) {
        if (error) *error = DiskError::AlreadyExists;
        return false;
    }

    std::ifstream in(host_path, std::ios::binary | std::ios::ate);
    if (!in) {
        if (error) *error = DiskError::IoError;
        return false;
    }
    const std::streamoff size = in.tellg();
    in.seekg(0);

    std::vector<uint8_t> content(static_cast<size_t>(size));
    if (size > 0 && !in.read(reinterpret_cast<char *>(content.data()), size)) {
        if (error) *error = DiskError::IoError;
        return false;
    }

    DiskError alloc_error = DiskError::None;
    const uint16_t first_cluster =
        content.empty() ? 0 : AllocateClusterChain(static_cast<uint32_t>(content.size()), &alloc_error);
    if (!content.empty() && first_cluster == 0) {
        if (error) *error = alloc_error;
        return false;
    }

    if (!content.empty()) {
        const uint8_t *fat = FatPtr();
        const uint32_t bpc = geometry_.BytesPerCluster();
        uint16_t cluster = first_cluster;
        size_t written = 0;
        while (true) {
            const uint32_t offset = geometry_.ClusterOffset(cluster);
            const size_t chunk = std::min<size_t>(bpc, content.size() - written);
            std::memcpy(data_.data() + offset, content.data() + written, chunk);
            written += chunk;
            if (written >= content.size()) break;
            cluster = msxdisk_fat12_get(fat, cluster);
        }
    }

    DiskError slot_error = DiskError::None;
    RawDirEntry *entry = FindFreeSlotIn(*parent, &slot_error);
    if (!entry) {
        if (!content.empty()) FreeClusterChain(first_cluster); // rollback
        if (error) *error = slot_error;
        return false;
    }

    const auto name11 = ToFat83Name(parsed.leaf_name);
    std::memcpy(entry->name, name11.data(), 8);
    std::memcpy(entry->ext, name11.data() + 8, 3);
    entry->attr = attr;
    std::memset(entry->reserved, 0, sizeof(entry->reserved));
    PackNow(&entry->date, &entry->time);
    entry->first_cluster = first_cluster;
    entry->size = static_cast<uint32_t>(content.size());

    if (error) *error = DiskError::None;
    return true;
}

bool DiskImage::ExtractFile(const std::string &msx_path, const std::filesystem::path &host_path,
                             DiskError *error) const {
    const auto parsed = ParseMsxPath(msx_path);

    DiskError local_error = DiskError::None;
    const auto parent = ResolveDir(parsed.dir_components, &local_error);
    if (!parent) {
        if (error) *error = local_error;
        return false;
    }

    const RawDirEntry *entry = FindEntryIn(*parent, parsed.leaf_name);
    if (!entry) {
        if (error) *error = DiskError::FileNotFound;
        return false;
    }
    if (entry->attr & kAttrDirectory) {
        if (error) *error = DiskError::IsADirectory;
        return false;
    }

    std::ofstream out(host_path, std::ios::binary | std::ios::trunc);
    if (!out) {
        if (error) *error = DiskError::IoError;
        return false;
    }

    uint32_t remaining = entry->size;
    uint16_t cluster = entry->first_cluster;
    const uint8_t *fat = FatPtr();
    const uint32_t bpc = geometry_.BytesPerCluster();

    while (remaining > 0 && cluster >= kFirstDataCluster) {
        const uint32_t offset = geometry_.ClusterOffset(cluster);
        const uint32_t chunk = std::min<uint32_t>(bpc, remaining);
        out.write(reinterpret_cast<const char *>(data_.data() + offset), chunk);
        remaining -= chunk;
        if (remaining == 0) break;
        cluster = msxdisk_fat12_get(fat, cluster);
    }

    if (!out) {
        if (error) *error = DiskError::IoError;
        return false;
    }
    if (error) *error = DiskError::None;
    return true;
}

int DiskImage::ExtractMatching(const std::string &dir_path, const std::string &pattern,
                                const std::filesystem::path &dest_dir, DiskError *error) const {
    std::error_code ec;
    std::filesystem::create_directories(dest_dir, ec);

    if (!HasWildcard(pattern)) {
        DiskError file_error = DiskError::None;
        const bool ok = ExtractFile(JoinMsxPath(dir_path, pattern), dest_dir / pattern, &file_error);
        if (error) *error = file_error;
        return ok ? 1 : 0;
    }

    DiskError list_error = DiskError::None;
    const auto matches = ListDirectory(dir_path, pattern, &list_error);
    if (matches.empty()) {
        if (error) *error = list_error;
        return 0;
    }

    int count = 0;
    DiskError last_error = DiskError::None;
    for (const auto &fe : matches) {
        if (fe.attr & kAttrDirectory) continue; // extract nao desce em subdiretorios
        DiskError file_error = DiskError::None;
        if (ExtractFile(JoinMsxPath(dir_path, fe.name), dest_dir / fe.name, &file_error)) {
            ++count;
        } else {
            last_error = file_error;
        }
    }
    if (error) *error = (count > 0) ? DiskError::None : last_error;
    return count;
}

bool DiskImage::DeleteFile(const std::string &msx_path, DiskError *error) {
    const auto parsed = ParseMsxPath(msx_path);

    DiskError local_error = DiskError::None;
    const auto parent = ResolveDir(parsed.dir_components, &local_error);
    if (!parent) {
        if (error) *error = local_error;
        return false;
    }

    RawDirEntry *entry = FindEntryIn(*parent, parsed.leaf_name);
    if (!entry) {
        if (error) *error = DiskError::FileNotFound;
        return false;
    }
    if (entry->attr & kAttrDirectory) {
        if (error) *error = DiskError::IsADirectory;
        return false;
    }

    if (entry->first_cluster != 0) {
        FreeClusterChain(entry->first_cluster);
    }
    entry->name[0] = static_cast<char>(kEntryDeleted);

    if (error) *error = DiskError::None;
    return true;
}

bool DiskImage::RenameFile(const std::string &msx_path, const std::string &new_name, DiskError *error) {
    if (new_name.find('\\') != std::string::npos || new_name.find('/') != std::string::npos) {
        // REN nao move entre diretorios, igual ao MSX-DOS real.
        if (error) *error = DiskError::NameTooLong;
        return false;
    }

    const auto parsed = ParseMsxPath(msx_path);

    DiskError local_error = DiskError::None;
    const auto parent = ResolveDir(parsed.dir_components, &local_error);
    if (!parent) {
        if (error) *error = local_error;
        return false;
    }

    RawDirEntry *entry = FindEntryIn(*parent, parsed.leaf_name);
    if (!entry) {
        if (error) *error = DiskError::FileNotFound;
        return false;
    }
    RawDirEntry *conflict = FindEntryIn(*parent, new_name);
    if (conflict != nullptr && conflict != entry) {
        if (error) *error = DiskError::AlreadyExists;
        return false;
    }

    const auto name11 = ToFat83Name(new_name);
    std::memcpy(entry->name, name11.data(), 8);
    std::memcpy(entry->ext, name11.data() + 8, 3);

    if (error) *error = DiskError::None;
    return true;
}

bool DiskImage::MakeDirectory(const std::string &msx_path, DiskError *error) {
    const auto parsed = ParseMsxPath(msx_path);
    if (parsed.leaf_name.empty()) {
        if (error) *error = DiskError::AlreadyExists; // "criar a raiz" nao faz sentido
        return false;
    }

    DiskError local_error = DiskError::None;
    const auto parent = ResolveDir(parsed.dir_components, &local_error);
    if (!parent) {
        if (error) *error = local_error;
        return false;
    }
    if (FindEntryIn(*parent, parsed.leaf_name) != nullptr) {
        if (error) *error = DiskError::AlreadyExists;
        return false;
    }

    DiskError alloc_error = DiskError::None;
    const uint16_t new_cluster = AllocateClusterChain(1, &alloc_error);
    if (new_cluster == 0) {
        if (error) *error = alloc_error;
        return false;
    }
    ZeroCluster(new_cluster);

    auto *entries = reinterpret_cast<RawDirEntry *>(data_.data() + geometry_.ClusterOffset(new_cluster));
    WriteSpecialEntry(&entries[0], ".", new_cluster);
    WriteSpecialEntry(&entries[1], "..", parent->is_root ? 0 : parent->first_cluster);

    DiskError slot_error = DiskError::None;
    RawDirEntry *entry = FindFreeSlotIn(*parent, &slot_error);
    if (!entry) {
        FreeClusterChain(new_cluster); // rollback
        if (error) *error = slot_error;
        return false;
    }

    const auto name11 = ToFat83Name(parsed.leaf_name);
    std::memcpy(entry->name, name11.data(), 8);
    std::memcpy(entry->ext, name11.data() + 8, 3);
    entry->attr = kAttrDirectory;
    std::memset(entry->reserved, 0, sizeof(entry->reserved));
    PackNow(&entry->date, &entry->time);
    entry->first_cluster = new_cluster;
    entry->size = 0;

    if (error) *error = DiskError::None;
    return true;
}

bool DiskImage::RemoveDirectory(const std::string &msx_path, DiskError *error) {
    const auto parsed = ParseMsxPath(msx_path);
    if (parsed.leaf_name.empty()) {
        if (error) *error = DiskError::NotADirectory; // nao remove a raiz
        return false;
    }

    DiskError local_error = DiskError::None;
    const auto parent = ResolveDir(parsed.dir_components, &local_error);
    if (!parent) {
        if (error) *error = local_error;
        return false;
    }

    RawDirEntry *entry = FindEntryIn(*parent, parsed.leaf_name);
    if (!entry) {
        if (error) *error = DiskError::FileNotFound;
        return false;
    }
    if (!(entry->attr & kAttrDirectory)) {
        if (error) *error = DiskError::NotADirectory;
        return false;
    }

    const DirLocation target{false, entry->first_cluster};
    const uint32_t slot_count = DirSlotCount(target);
    for (uint32_t i = 2; i < slot_count; ++i) { // pula "." e ".." (sempre as 2 primeiras)
        const RawDirEntry *e = DirSlot(target, i);
        const auto first_byte = static_cast<uint8_t>(e->name[0]);
        if (first_byte != kEntryFree && first_byte != kEntryDeleted) {
            if (error) *error = DiskError::DirectoryNotEmpty;
            return false;
        }
    }

    FreeClusterChain(target.first_cluster);
    entry->name[0] = static_cast<char>(kEntryDeleted);

    if (error) *error = DiskError::None;
    return true;
}

void DiskImage::CapacityStats(int64_t *free_bytes, float *free_percent) const {
    const uint32_t total = geometry_.TotalClusters();
    const uint32_t used = CountUsedClusters();
    msxdisk_geometry_stats(static_cast<int32_t>(total), static_cast<int32_t>(used),
                            static_cast<int32_t>(geometry_.BytesPerCluster()), free_bytes, free_percent);
}

} // namespace msxdisk
