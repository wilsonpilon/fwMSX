// fwMSX -- criacao de arquivos .dsk formatados. Ver disk_creator.h.
#include "disk_creator.h"

#include <fstream>
#include <vector>

#include "../../msxdisk/core/msxdos1_boot.h"

namespace diskfmt {

bool CreateBlankDisk(const std::string &path, const DiskFmtSpec *spec, std::string &error) {
    if (!spec) {
        error = "formato de disco desconhecido";
        return false;
    }
    std::vector<uint8_t> image(diskfmt_image_size(spec));
    if (diskfmt_build(spec, image.data(), image.size(), msxdisk::kMsxDos1BootSector720KB) != 0) {
        error = "falha ao montar o disco formatado";
        return false;
    }
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f || !f.write(reinterpret_cast<const char *>(image.data()), static_cast<std::streamsize>(image.size()))) {
        error = "nao foi possivel gravar '" + path + "'";
        return false;
    }
    return true;
}

} // namespace diskfmt
