// fwMSX -- imagem de disco rigido (HD/SD) em arquivo: acesso por setor de 512 bytes e criacao de imagens particionadas e formatadas
// (MBR + FAT16) que o Nextor enxerga. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/hd-spec.md.
#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "../core/hd_format.h"

namespace hd {

// Um arquivo de imagem aberto para leitura e escrita por setor. As escritas vao direto ao arquivo (nada fica no cache do programa).
class HdImage {
public:
    ~HdImage();
    HdImage() = default;
    HdImage(const HdImage &) = delete;
    HdImage &operator=(const HdImage &) = delete;

    // false + `error` se nao abre, esta' vazio ou nao e' multiplo de 512 bytes. `read_only` abre so' para leitura.
    bool Open(const std::string &path, bool read_only, std::string &error);
    bool is_open() const { return file_ != nullptr; }
    bool read_only() const { return read_only_; }
    uint32_t sectors() const { return sectors_; }
    const std::string &path() const { return path_; }

    bool ReadSector(uint32_t lba, uint8_t *out512);
    bool WriteSector(uint32_t lba, const uint8_t *in512);

private:
    std::FILE *file_ = nullptr;
    std::string path_;
    uint32_t sectors_ = 0;
    bool read_only_ = false;
};

// Tamanhos das particoes (MB) de uma imagem nova: 1 a 4 particoes de 8 a 2048 MB, soma ate' 4096 MB.
bool ValidatePartitionSizes(const std::vector<uint32_t> &mb, std::string &error);
// "512", "512M", "2G" (sem unidade = MB) -> MB; "2G" = 2048. false + `error` se nao e' um tamanho.
bool ParseSize(const std::string &text, uint32_t &mb, std::string &error);

// Cria `path` (sobrescreve): MBR, uma particao FAT16 vazia para cada tamanho e o resto do arquivo vazio (esparso, quando o sistema deixa).
// A primeira particao comeca em 1 MiB. O arquivo tem 1 MiB + soma dos tamanhos.
bool CreateHdImage(const std::string &path, const std::vector<uint32_t> &partition_mb, std::string &error);

// Uma linha por particao (inicio, tamanho, tipo) lida do MBR de uma imagem; false + `error` se nao e' MBR.
struct PartitionInfo {
    uint32_t start = 0;
    uint32_t sectors = 0;
    uint8_t type = 0;
};
bool ReadPartitions(HdImage &image, std::vector<PartitionInfo> &parts, std::string &error);

} // namespace hd
