// fwMSX -- imagem de disquete (.dsk cru) carregada em memoria, com gravacao
// imediata no arquivo a cada setor escrito. Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver doc/fdc-spec.md.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../core/fdc_state.h"

namespace fdc {

class DiskImage {
public:
    DiskImage() { disk_ = FdcDisk{}; }
    DiskImage(const DiskImage &) = delete;
    DiskImage &operator=(const DiskImage &) = delete;

    // Carrega `path`. Devolve false e preenche `error` se o arquivo nao abre
    // ou o tamanho nao e' de disquete. Se o arquivo nao pode ser aberto para
    // escrita, o disco entra protegido contra gravacao.
    bool Load(const std::string &path, std::string &error);

    // Cria na memoria uma imagem em branco (zeros) de `size` bytes -- sem
    // arquivo associado (as escritas ficam so' na memoria). Para testes.
    bool CreateBlank(size_t size, std::string &error);

    void Eject();

    bool loaded() const { return disk_.data != nullptr; }
    const std::string &path() const { return path_; }
    FdcDisk *disk() { return &disk_; }
    const FdcDisk *disk() const { return &disk_; }
    // Quantos setores ja' foram gravados no arquivo (para diagnostico/testes).
    uint64_t sectors_written() const { return sectors_written_; }

private:
    static void WriteCallback(void *user, size_t offset, size_t length);
    void Persist(size_t offset, size_t length);

    FdcDisk disk_;
    std::vector<uint8_t> data_;
    std::string path_;
    uint64_t sectors_written_ = 0;
};

} // namespace fdc
