//
// msxdisk (fwMSX): implementacao dos comandos compartilhados (modo CLI
// one-shot). Formatacao/mensagens compartilhadas com a sessao do shell
// interativo ficam em format.{h,cpp}.
//

#include "commands.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <optional>

#include "../core/dir_entry.h"
#include "../core/disk_image.h"
#include "format.h"

namespace msxdisk::cli {

int CmdCreate(const CreateOptions &opts) {
    if (opts.with_dos1 && opts.with_dos2) {
        std::cerr << "msxdisk: --dos1 e --dos2 sao mutuamente exclusivos" << std::endl;
        return 1;
    }
    if (opts.with_dos1 && (opts.command_com_path.empty() || opts.msxdos_sys_path.empty())) {
        std::cerr << "msxdisk: --dos1 exige --command <COMMAND.COM> e --sys <MSXDOS.SYS> "
                     "(o msxdisk nao redistribui esses arquivos -- ver doc/msxdisk-spec.md)"
                   << std::endl;
        return 1;
    }
    if (opts.with_dos2 && (opts.msxdos_sys_path.empty() || opts.msxdos2_sys_path.empty())) {
        std::cerr << "msxdisk: --dos2 exige --sys <MSXDOS.SYS> e --sys2 <MSXDOS2.SYS> "
                     "(o msxdisk nao redistribui esses arquivos -- ver doc/msxdisk-spec.md)"
                   << std::endl;
        return 1;
    }

    DiskImage image = DiskImage::CreateBlank();

    if (opts.with_dos1) {
        if (!CheckLocalFileReadable(opts.msxdos_sys_path, "create") ||
            !CheckLocalFileReadable(opts.command_com_path, "create")) {
            return 1;
        }
        DiskError error = DiskError::None;
        // MSXDOS.SYS primeiro (convencao MSX-DOS: arquivo de sistema ocupa
        // a primeira entrada do diretorio raiz).
        if (!image.AddFile(opts.msxdos_sys_path, "MSXDOS.SYS",
                            msxdisk::kAttrHidden | msxdisk::kAttrSystem, &error)) {
            std::cerr << "msxdisk: falha ao incluir MSXDOS.SYS: " << DiskErrorMessage(error) << std::endl;
            return 1;
        }
        if (!image.AddFile(opts.command_com_path, "COMMAND.COM", msxdisk::kAttrArchive, &error)) {
            std::cerr << "msxdisk: falha ao incluir COMMAND.COM: " << DiskErrorMessage(error) << std::endl;
            return 1;
        }
    } else if (opts.with_dos2) {
        if (!CheckLocalFileReadable(opts.msxdos_sys_path, "create") ||
            !CheckLocalFileReadable(opts.msxdos2_sys_path, "create")) {
            return 1;
        }
        DiskError error = DiskError::None;
        if (!image.AddFile(opts.msxdos_sys_path, "MSXDOS.SYS",
                            msxdisk::kAttrHidden | msxdisk::kAttrSystem, &error)) {
            std::cerr << "msxdisk: falha ao incluir MSXDOS.SYS: " << DiskErrorMessage(error) << std::endl;
            return 1;
        }
        if (!image.AddFile(opts.msxdos2_sys_path, "MSXDOS2.SYS",
                            msxdisk::kAttrHidden | msxdisk::kAttrSystem, &error)) {
            std::cerr << "msxdisk: falha ao incluir MSXDOS2.SYS: " << DiskErrorMessage(error) << std::endl;
            return 1;
        }
    }

    if (!SaveOrReport(image, opts.image_path)) return 1;

    std::cout << "Imagem criada: " << opts.image_path << " (" << image.geometry().ImageSizeBytes() << " bytes)"
               << std::endl;
    return 0;
}

int CmdList(const std::string &image_path, const std::string &dir_path, const std::string &pattern, bool tree) {
    auto image = LoadOrReport(image_path);
    if (!image) return 1;

    if (tree) {
        const int total = PrintTree(*image, dir_path, 0);
        std::cout << total << " entrada(s)." << std::endl;
        return 0;
    }

    DiskError error = DiskError::None;
    const auto entries = image->ListDirectory(dir_path, pattern, &error);
    if (error != DiskError::None) {
        std::cerr << "msxdisk: falha ao listar '" << (dir_path.empty() ? "\\" : dir_path)
                   << "': " << DiskErrorMessage(error) << std::endl;
        return 1;
    }
    if (entries.empty()) {
        std::cout << "(nenhuma entrada)" << std::endl;
        return 0;
    }

    for (const auto &e : entries) {
        PrintEntry(e, 0);
    }
    std::cout << entries.size() << " entrada(s)." << std::endl;
    return 0;
}

int CmdAdd(const std::string &image_path, const std::string &host_path, const std::string &msx_path) {
    auto image = LoadOrReport(image_path);
    if (!image) return 1;

    if (!CheckLocalFileReadable(host_path, "add")) return 1;

    const std::string name = msx_path.empty() ? std::filesystem::path(host_path).filename().string() : msx_path;

    DiskError error = DiskError::None;
    if (!image->AddFile(host_path, name, msxdisk::kAttrArchive, &error)) {
        std::cerr << "msxdisk: falha ao adicionar '" << host_path << "': " << DiskErrorMessage(error) << std::endl;
        return 1;
    }
    if (!SaveOrReport(*image, image_path)) return 1;

    std::cout << "Adicionado: " << name << std::endl;
    return 0;
}

int CmdExtract(const std::string &image_path, const std::string &dir_path, const std::string &pattern,
                const std::string &dest_dir) {
    auto image = LoadOrReport(image_path);
    if (!image) return 1;

    DiskError error = DiskError::None;
    const int count = image->ExtractMatching(dir_path, pattern, dest_dir, &error);
    if (count == 0) {
        std::cerr << "msxdisk: falha ao extrair '" << pattern << "': " << DiskErrorMessage(error) << std::endl;
        return 1;
    }

    std::cout << count << " arquivo(s) extraido(s) para " << dest_dir << std::endl;
    return 0;
}

int CmdDelete(const std::string &image_path, const std::string &msx_path) {
    auto image = LoadOrReport(image_path);
    if (!image) return 1;

    DiskError error = DiskError::None;
    if (!image->DeleteFile(msx_path, &error)) {
        std::cerr << "msxdisk: falha ao excluir '" << msx_path << "': " << DiskErrorMessage(error) << std::endl;
        return 1;
    }
    if (!SaveOrReport(*image, image_path)) return 1;

    std::cout << "Excluido: " << msx_path << std::endl;
    return 0;
}

int CmdRename(const std::string &image_path, const std::string &msx_path, const std::string &new_name) {
    auto image = LoadOrReport(image_path);
    if (!image) return 1;

    DiskError error = DiskError::None;
    if (!image->RenameFile(msx_path, new_name, &error)) {
        std::cerr << "msxdisk: falha ao renomear '" << msx_path << "': " << DiskErrorMessage(error) << std::endl;
        return 1;
    }
    if (!SaveOrReport(*image, image_path)) return 1;

    std::cout << "Renomeado: " << msx_path << " -> " << new_name << std::endl;
    return 0;
}

int CmdMkdir(const std::string &image_path, const std::string &msx_path) {
    auto image = LoadOrReport(image_path);
    if (!image) return 1;

    DiskError error = DiskError::None;
    if (!image->MakeDirectory(msx_path, &error)) {
        std::cerr << "msxdisk: falha ao criar diretorio '" << msx_path << "': " << DiskErrorMessage(error)
                   << std::endl;
        return 1;
    }
    if (!SaveOrReport(*image, image_path)) return 1;

    std::cout << "Diretorio criado: " << msx_path << std::endl;
    return 0;
}

int CmdRmdir(const std::string &image_path, const std::string &msx_path) {
    auto image = LoadOrReport(image_path);
    if (!image) return 1;

    DiskError error = DiskError::None;
    if (!image->RemoveDirectory(msx_path, &error)) {
        std::cerr << "msxdisk: falha ao remover diretorio '" << msx_path << "': " << DiskErrorMessage(error)
                   << std::endl;
        return 1;
    }
    if (!SaveOrReport(*image, image_path)) return 1;

    std::cout << "Diretorio removido: " << msx_path << std::endl;
    return 0;
}

int CmdCopy(const std::string &src_image_path, const std::string &dest_image_path) {
    DiskError error = DiskError::None;
    auto image = LoadOrReport(src_image_path, &error); // valida que 'src' e uma imagem MSX de verdade
    if (!image) return 1;

    std::error_code ec;
    std::filesystem::copy_file(src_image_path, dest_image_path, std::filesystem::copy_options::overwrite_existing,
                                ec);
    if (ec) {
        std::cerr << "msxdisk: falha ao copiar para '" << dest_image_path << "': " << ec.message() << std::endl;
        return 1;
    }

    std::cout << "Copiado: " << src_image_path << " -> " << dest_image_path << std::endl;
    return 0;
}

int CmdSaveAs(const std::string &image_path, const std::string &dest_image_path) {
    auto image = LoadOrReport(image_path);
    if (!image) return 1;

    if (!SaveOrReport(*image, dest_image_path)) return 1;

    std::cout << "Salvo como: " << dest_image_path << std::endl;
    return 0;
}

int CmdInfo(const std::string &image_path) {
    auto image = LoadOrReport(image_path);
    if (!image) return 1;

    const auto &g = image->geometry();
    int64_t free_bytes = 0;
    float free_percent = 0.0f;
    image->CapacityStats(&free_bytes, &free_percent);

    std::cout << "Imagem: " << image_path << std::endl;
    std::cout << "  Tamanho total: " << g.ImageSizeBytes() << " bytes" << std::endl;
    std::cout << "  Setores/cluster: " << static_cast<int>(g.sectors_per_cluster) << "  Clusters de dados: "
               << g.TotalClusters() << std::endl;
    std::cout << "  Espaco livre: " << free_bytes << " bytes (" << std::fixed << std::setprecision(1)
               << free_percent << "%)" << std::endl;
    std::cout << "  Entradas no diretorio raiz: " << g.root_dir_entries << std::endl;
    return 0;
}

} // namespace msxdisk::cli
