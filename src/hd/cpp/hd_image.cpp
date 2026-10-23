// fwMSX -- imagem de disco rigido. Ver hd_image.h e doc/hd-spec.md.
#include "hd_image.h"

#include <cctype>
#include <cstdlib>
#include <filesystem>

namespace hd {

namespace {

bool SeekSector(std::FILE *f, uint64_t lba) {
#ifdef _WIN32
    return _fseeki64(f, static_cast<__int64>(lba * HD_SECTOR_SIZE), SEEK_SET) == 0;
#else
    return fseeko(f, static_cast<off_t>(lba * HD_SECTOR_SIZE), SEEK_SET) == 0;
#endif
}

} // namespace

HdImage::~HdImage() {
    if (file_) std::fclose(file_);
}

bool HdImage::Open(const std::string &path, bool read_only, std::string &error) {
    if (file_) std::fclose(file_);
    file_ = nullptr;
    sectors_ = 0;
    std::error_code ec;
    const std::uintmax_t size = std::filesystem::file_size(path, ec);
    if (ec) {
        error = "imagem de disco rigido '" + path + "': nao abre (" + ec.message() + ")";
        return false;
    }
    if (size == 0 || size % HD_SECTOR_SIZE != 0 || size / HD_SECTOR_SIZE > 0xFFFFFFFFull) {
        error = "imagem de disco rigido '" + path + "': o tamanho (" + std::to_string(size) + " bytes) nao e' um numero inteiro de setores de 512 bytes";
        return false;
    }
    file_ = std::fopen(path.c_str(), read_only ? "rb" : "r+b");
    if (!file_) {
        error = "imagem de disco rigido '" + path + "': nao abre para " + (read_only ? "leitura" : "leitura e escrita");
        return false;
    }
    path_ = path;
    sectors_ = static_cast<uint32_t>(size / HD_SECTOR_SIZE);
    read_only_ = read_only;
    return true;
}

bool HdImage::ReadSector(uint32_t lba, uint8_t *out) {
    if (!file_ || lba >= sectors_ || !SeekSector(file_, lba)) return false;
    return std::fread(out, 1, HD_SECTOR_SIZE, file_) == HD_SECTOR_SIZE;
}

bool HdImage::WriteSector(uint32_t lba, const uint8_t *in) {
    if (!file_ || read_only_ || lba >= sectors_ || !SeekSector(file_, lba)) return false;
    if (std::fwrite(in, 1, HD_SECTOR_SIZE, file_) != HD_SECTOR_SIZE) return false;
    return std::fflush(file_) == 0;
}

bool ValidatePartitionSizes(const std::vector<uint32_t> &mb, std::string &error) {
    if (mb.empty() || mb.size() > HD_MAX_PARTITIONS) {
        error = "use de 1 a 4 particoes";
        return false;
    }
    uint32_t total = 0;
    for (const uint32_t m : mb) {
        if (m < HD_MIN_PARTITION_MB || m > HD_MAX_PARTITION_MB) {
            error = "cada particao tem de 8 MB a 2048 MB (2 GB): " + std::to_string(m) + " MB nao serve (o Nextor so' le particoes FAT16 de ate' 2 GB)";
            return false;
        }
        total += m;
    }
    if (total > HD_MAX_TOTAL_MB) {
        error = "a soma das particoes passa de 4096 MB (4 GB): " + std::to_string(total) + " MB";
        return false;
    }
    return true;
}

bool ParseSize(const std::string &text, uint32_t &mb, std::string &error) {
    if (text.empty() || !std::isdigit(static_cast<unsigned char>(text[0]))) {
        error = "tamanho invalido '" + text + "' (ex.: 512, 512M, 2G)";
        return false;
    }
    char *end = nullptr;
    const unsigned long long n = std::strtoull(text.c_str(), &end, 10);
    std::string unit = end ? std::string(end) : std::string();
    for (char &c : unit) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    unsigned long long result = n;
    if (unit.empty() || unit == "M" || unit == "MB") {
        result = n;
    } else if (unit == "G" || unit == "GB") {
        result = n * 1024ull;
    } else {
        error = "unidade invalida em '" + text + "' (use M ou G)";
        return false;
    }
    if (result == 0 || result > 1048576ull) {
        error = "tamanho invalido '" + text + "'";
        return false;
    }
    mb = static_cast<uint32_t>(result);
    return true;
}

bool CreateHdImage(const std::string &path, const std::vector<uint32_t> &partition_mb, std::string &error) {
    if (!ValidatePartitionSizes(partition_mb, error)) return false;
    uint64_t total_sectors = HD_MBR_ALIGN_SECTORS;
    std::vector<HdPartition> parts;
    std::vector<HdFat16Plan> plans;
    for (const uint32_t mb : partition_mb) {
        HdPartition p;
        p.start = static_cast<uint32_t>(total_sectors);
        p.sectors = mb * 2048u;
        p.type = 0x06; // FAT16 (> 32 MB, CHS); o Nextor identifica pelo BPB
        HdFat16Plan plan;
        if (hd_fat16_plan(p.sectors, &plan) != 0) {
            error = "nao foi possivel planejar o FAT16 de " + std::to_string(mb) + " MB";
            return false;
        }
        parts.push_back(p);
        plans.push_back(plan);
        total_sectors += p.sectors;
    }
    std::FILE *f = std::fopen(path.c_str(), "wb");
    if (!f) {
        error = "nao foi possivel criar '" + path + "'";
        return false;
    }
    bool ok = true;
    auto write_at = [&](uint64_t lba, const uint8_t *data, size_t bytes) {
        if (!ok) return;
        ok = SeekSector(f, lba) && std::fwrite(data, 1, bytes, f) == bytes;
    };
    uint8_t mbr[HD_SECTOR_SIZE];
    hd_mbr(mbr, parts.data(), static_cast<int>(parts.size()));
    write_at(0, mbr, sizeof mbr);
    for (size_t i = 0; i < parts.size() && ok; ++i) {
        std::vector<uint8_t> meta(hd_fat16_meta_bytes(&plans[i]));
        hd_fat16_meta(meta.data(), &plans[i]);
        hd_fat16_boot_sector(meta.data(), &plans[i], parts[i].start);
        write_at(parts[i].start, meta.data(), meta.size());
    }
    // o ultimo setor da imagem fixa o tamanho do arquivo (esparso onde o sistema deixa)
    if (ok) {
        uint8_t zero[HD_SECTOR_SIZE];
        hd_fill(zero, 0, sizeof zero);
        write_at(total_sectors - 1, zero, sizeof zero);
    }
    ok = (std::fclose(f) == 0) && ok;
    if (!ok) {
        error = "falha ao gravar '" + path + "' (disco cheio?)";
        std::error_code ec;
        std::filesystem::remove(path, ec);
        return false;
    }
    return true;
}

bool ReadPartitions(HdImage &image, std::vector<PartitionInfo> &parts, std::string &error) {
    parts.clear();
    uint8_t mbr[HD_SECTOR_SIZE];
    if (!image.ReadSector(0, mbr)) {
        error = "nao foi possivel ler o setor 0";
        return false;
    }
    if (mbr[510] != 0x55 || mbr[511] != 0xAA) {
        error = "sem tabela de particoes (o setor 0 nao termina em 55AAh)";
        return false;
    }
    for (int i = 0; i < HD_MAX_PARTITIONS; ++i) {
        const uint8_t *e = mbr + 446 + i * 16;
        PartitionInfo p;
        p.type = e[4];
        p.start = static_cast<uint32_t>(e[8]) | (static_cast<uint32_t>(e[9]) << 8) | (static_cast<uint32_t>(e[10]) << 16) | (static_cast<uint32_t>(e[11]) << 24);
        p.sectors = static_cast<uint32_t>(e[12]) | (static_cast<uint32_t>(e[13]) << 8) | (static_cast<uint32_t>(e[14]) << 16) | (static_cast<uint32_t>(e[15]) << 24);
        if (p.type != 0 && p.sectors != 0) parts.push_back(p);
    }
    return true;
}

} // namespace hd
