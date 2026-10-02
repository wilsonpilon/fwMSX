#include "disk_image.h"

#include <fstream>
#include <iterator>

namespace fdc {

bool DiskImage::Load(const std::string &path, std::string &error, bool read_only) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        error = "nao foi possivel abrir '" + path + "'";
        return false;
    }
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    FdcDisk probe{};
    if (!fdc_disk_detect_geometry(&probe, bytes.data(), bytes.size())) {
        error = "'" + path + "' nao tem tamanho de disquete (" + std::to_string(bytes.size()) + " bytes)";
        return false;
    }

    Eject();
    data_ = std::move(bytes);
    disk_ = probe;
    disk_.data = data_.data();
    disk_.size = data_.size();
    disk_.write_cb = &DiskImage::WriteCallback;
    disk_.write_user = this;
    path_ = path;

    // Somente leitura (pedido) ou sem permissao de escrita no arquivo: entra
    // protegido (o MSX-DOS ve' "Write protect error" em vez de perder dados em
    // silencio). A sonda de escrita so' roda se a escrita nao foi recusada, para
    // nao abrir o arquivo para escrita sem necessidade.
    if (read_only) {
        disk_.write_protected = 1;
    } else {
        std::ofstream probe_write(path, std::ios::binary | std::ios::in | std::ios::out);
        disk_.write_protected = probe_write ? 0 : 1;
    }
    return true;
}

bool DiskImage::CreateBlank(size_t size, std::string &error) {
    FdcDisk probe{};
    std::vector<uint8_t> bytes(size, 0);
    if (!fdc_disk_detect_geometry(&probe, bytes.data(), bytes.size())) {
        error = "tamanho nao e' de disquete: " + std::to_string(size);
        return false;
    }
    Eject();
    data_ = std::move(bytes);
    disk_ = probe;
    disk_.data = data_.data();
    disk_.size = data_.size();
    disk_.write_cb = &DiskImage::WriteCallback;
    disk_.write_user = this;
    return true;
}

void DiskImage::Eject() {
    disk_ = FdcDisk{};
    data_.clear();
    path_.clear();
}

void DiskImage::WriteCallback(void *user, size_t offset, size_t length) { static_cast<DiskImage *>(user)->Persist(offset, length); }

void DiskImage::Persist(size_t offset, size_t length) {
    ++sectors_written_;
    if (path_.empty()) return; // imagem so' em memoria
    std::ofstream out(path_, std::ios::binary | std::ios::in | std::ios::out);
    if (!out) return;
    out.seekp(static_cast<std::streamoff>(offset));
    out.write(reinterpret_cast<const char *>(data_.data() + offset), static_cast<std::streamsize>(length));
}

} // namespace fdc
