//
// msxdisk (fwMSX): formatacao/mensagens compartilhadas -- ver format.h.
//

#include "format.h"

#include <iomanip>
#include <iostream>

#include "../core/dir_entry.h"

namespace msxdisk::cli {

namespace {

void PrintDate(uint16_t date, uint16_t time) {
    const int year = 1980 + (date >> 9);
    const int month = (date >> 5) & 0x0F;
    const int day = date & 0x1F;
    const int hour = time >> 11;
    const int minute = (time >> 5) & 0x3F;

    std::cout << std::setfill('0') << std::setw(4) << year << "-" << std::setw(2) << month << "-" << std::setw(2)
               << day << " " << std::setw(2) << hour << ":" << std::setw(2) << minute << std::setfill(' ');
}

std::string AttrFlags(uint8_t attr) {
    std::string flags;
    flags += (attr & msxdisk::kAttrDirectory) ? 'D' : '-';
    flags += (attr & msxdisk::kAttrReadOnly) ? 'R' : '-';
    flags += (attr & msxdisk::kAttrHidden) ? 'H' : '-';
    flags += (attr & msxdisk::kAttrSystem) ? 'S' : '-';
    flags += (attr & msxdisk::kAttrArchive) ? 'A' : '-';
    return flags;
}

} // namespace

const char *DiskErrorMessage(DiskError error) {
    switch (error) {
        case DiskError::None: return "ok";
        case DiskError::IoError: return "erro de E/S (arquivo/imagem inacessivel)";
        case DiskError::NotAnMsxImage: return "arquivo nao parece ser uma imagem MSX valida (BPB/tamanho incompativel)";
        case DiskError::DirectoryFull: return "diretorio raiz cheio (sem entradas livres)";
        case DiskError::DiskFull: return "disco cheio (sem clusters livres suficientes)";
        case DiskError::FileNotFound: return "arquivo nao encontrado na imagem";
        case DiskError::NameTooLong: return "nome invalido (formato 8.3, sem separadores de diretorio)";
        case DiskError::AlreadyExists: return "ja existe um arquivo/diretorio com esse nome";
        case DiskError::PathNotFound: return "caminho nao encontrado (algum diretorio do caminho nao existe)";
        case DiskError::NotADirectory: return "o caminho aponta para um arquivo, nao um diretorio";
        case DiskError::IsADirectory: return "o caminho aponta para um diretorio, nao um arquivo";
        case DiskError::DirectoryNotEmpty: return "diretorio nao esta vazio";
    }
    return "erro desconhecido";
}

std::optional<DiskImage> LoadOrReport(const std::string &image_path, DiskError *error_out) {
    DiskError error = DiskError::None;
    auto image = DiskImage::Load(image_path, &error);
    if (!image) {
        std::cerr << "msxdisk: nao foi possivel abrir '" << image_path << "': " << DiskErrorMessage(error)
                   << std::endl;
    }
    if (error_out) *error_out = error;
    return image;
}

bool SaveOrReport(const DiskImage &image, const std::string &image_path) {
    DiskError error = DiskError::None;
    if (!image.Save(image_path, &error)) {
        std::cerr << "msxdisk: falha ao salvar '" << image_path << "': " << DiskErrorMessage(error) << std::endl;
        return false;
    }
    return true;
}

bool CheckLocalFileReadable(const std::string &host_path, const char *command_name) {
    std::error_code ec;
    if (!std::filesystem::exists(host_path, ec) || ec) {
        std::cerr << command_name << ": arquivo local nao encontrado: '" << host_path
                   << "' (confira o diretorio atual com 'pwd'/'ls')" << std::endl;
        return false;
    }
    if (std::filesystem::is_directory(host_path, ec)) {
        std::cerr << command_name << ": '" << host_path << "' e um diretorio, nao um arquivo" << std::endl;
        return false;
    }
    return true;
}

void PrintEntry(const FileEntry &e, int indent) {
    const std::string label = (e.attr & msxdisk::kAttrDirectory) ? "[" + e.name + "]" : e.name;
    const int label_width = (15 - indent * 2 > 1) ? (15 - indent * 2) : 1;
    std::cout << std::string(static_cast<size_t>(indent) * 2, ' ') << std::left << std::setw(label_width) << label
               << std::right << AttrFlags(e.attr) << "  " << std::setw(8) << e.size << "  ";
    PrintDate(e.date, e.time);
    std::cout << std::endl;
}

std::string JoinMsxPath(const std::string &dir_path, const std::string &name) {
    if (dir_path.empty()) return name;
    return dir_path + "\\" + name;
}

int PrintTree(const DiskImage &image, const std::string &dir_path, int indent) {
    DiskError error = DiskError::None;
    const auto entries = image.ListDirectory(dir_path, "", &error);
    if (error != DiskError::None && indent == 0) {
        std::cerr << "msxdisk: falha ao listar: " << DiskErrorMessage(error) << std::endl;
        return 0;
    }

    int total = static_cast<int>(entries.size());
    for (const auto &e : entries) {
        PrintEntry(e, indent);
        if (e.attr & msxdisk::kAttrDirectory) {
            total += PrintTree(image, JoinMsxPath(dir_path, e.name), indent + 1);
        }
    }
    return total;
}

} // namespace msxdisk::cli
