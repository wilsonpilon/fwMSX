//
// Nucleo C++ do msxdisk (fwMSX): imagem de disco FAT12 MSX em memoria.
//
#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "dir_entry.h"
#include "geometry.h"

namespace msxdisk {

enum class DiskError {
    None,
    IoError,
    NotAnMsxImage,
    DirectoryFull,
    DiskFull,
    FileNotFound,
    NameTooLong,
    AlreadyExists,
    PathNotFound,     // um componente de diretorio do caminho nao existe
    NotADirectory,    // um componente do caminho existe mas nao e diretorio
    IsADirectory,      // operacao de arquivo apontada para um diretorio
    DirectoryNotEmpty, // RemoveDirectory numa pasta com arquivos dentro
};

// Metadados de uma entrada de diretorio, ja no formato "de exibicao"
// (ex.: "ARQUIVO.BAS"), para uso pelos comandos de CLI/shell/TUI/GUI.
// 'attr' carrega kAttrDirectory quando a entrada e um subdiretorio
// (MSX-DOS 2).
struct FileEntry {
    std::string name;
    uint32_t size = 0;
    uint16_t first_cluster = 0;
    uint8_t attr = 0;
    uint16_t date = 0;
    uint16_t time = 0;
};

// Representa uma imagem .DSK inteira, carregada em memoria. Suporta o
// diretorio raiz (MSX-DOS 1) e subdiretorios (MSX-DOS 2, Fase 2) -- ver
// doc/msxdisk-spec.md.
class DiskImage {
public:
    // Cria uma imagem em branco (ainda nao gravada em arquivo) com a
    // geometria informada: formata boot sector, as duas copias da FAT e
    // zera o diretorio raiz e a area de dados.
    static DiskImage CreateBlank(const Geometry &geometry = Geometry::Disk720KB());

    // Carrega uma imagem existente. A geometria e lida do proprio BPB do
    // arquivo (Geometry::ParseFromBootSector), nao e fixa em 720KB.
    static std::optional<DiskImage> Load(const std::filesystem::path &path, DiskError *error = nullptr);

    // Grava a imagem atual (em memoria) em 'path'.
    bool Save(const std::filesystem::path &path, DiskError *error = nullptr) const;

    // Lista as entradas do diretorio 'dir_path' ("" = raiz), opcionalmente
    // filtrando por um padrao com coringas MSX-DOS ('*'/'?'); padrao
    // vazio = todas. As entradas especiais "." e ".." nunca aparecem.
    std::vector<FileEntry> ListDirectory(const std::string &dir_path, const std::string &pattern = "",
                                          DiskError *error = nullptr) const;

    // Adiciona o conteudo de 'host_path' na imagem com o caminho MSX
    // 'msx_path' (pode incluir subdiretorios, ex. "JOGOS\ARQ.BAS"); os
    // diretorios do caminho precisam ja existir (ver MakeDirectory).
    // 'attr' default e "arquivo" (kAttrArchive).
    bool AddFile(const std::filesystem::path &host_path, const std::string &msx_path,
                 uint8_t attr, DiskError *error = nullptr);

    // Extrai exatamente o arquivo 'msx_path' para 'host_path'.
    bool ExtractFile(const std::string &msx_path, const std::filesystem::path &host_path,
                      DiskError *error = nullptr) const;

    // Extrai todos os arquivos do diretorio 'dir_path' que combinam com
    // 'pattern' (coringas MSX-DOS) para dentro de 'dest_dir' (criado se
    // preciso), usando o proprio nome MSX como nome do arquivo extraido.
    // Nao desce em subdiretorios. Devolve a quantidade extraida.
    int ExtractMatching(const std::string &dir_path, const std::string &pattern,
                         const std::filesystem::path &dest_dir, DiskError *error = nullptr) const;

    // Remove o arquivo 'msx_path' (libera diretorio e cadeia de clusters).
    // Falha com DiskError::IsADirectory se o caminho for um subdiretorio
    // (use RemoveDirectory para isso).
    bool DeleteFile(const std::string &msx_path, DiskError *error = nullptr);

    // Renomeia um arquivo/diretorio para 'new_name' (sem separadores --
    // rename nao move entre diretorios, igual ao REN do MSX-DOS).
    bool RenameFile(const std::string &msx_path, const std::string &new_name, DiskError *error = nullptr);

    // Cria o subdiretorio 'msx_path' (MSX-DOS 2); o diretorio pai precisa
    // ja existir. Grava as entradas especiais "." e ".." automaticamente.
    bool MakeDirectory(const std::string &msx_path, DiskError *error = nullptr);

    // Remove o subdiretorio 'msx_path'; falha com DiskError::DirectoryNotEmpty
    // se houver qualquer arquivo/subdiretorio dentro (alem de "."/"..").
    bool RemoveDirectory(const std::string &msx_path, DiskError *error = nullptr);

    const Geometry &geometry() const { return geometry_; }

    // Estatisticas de capacidade (usa o modulo Fortran msxdisk_geometry_stats).
    void CapacityStats(int64_t *free_bytes, float *free_percent) const;

private:
    // Localiza um diretorio dentro da imagem: a raiz (area fixa, MSX-DOS 1)
    // ou um subdiretorio (cadeia de clusters comecando em first_cluster,
    // MSX-DOS 2).
    struct DirLocation {
        bool is_root = true;
        uint16_t first_cluster = 0; // valido apenas quando !is_root
    };

    explicit DiskImage(Geometry geometry);

    uint8_t *FatPtr();
    const uint8_t *FatPtr() const;
    void SyncFatCopies();

    RawDirEntry *RootDirBegin();
    const RawDirEntry *RootDirBegin() const;

    void ZeroCluster(uint16_t cluster);

    uint16_t AllocateClusterChain(uint32_t byte_length, DiskError *error);
    void FreeClusterChain(uint16_t first_cluster);
    uint32_t CountUsedClusters() const;

    // Quantidade de slots de 32 bytes atualmente alocados para 'dir' (para
    // a raiz e sempre root_dir_entries; para um subdiretorio, cresce
    // conforme a cadeia de clusters).
    uint32_t DirSlotCount(const DirLocation &dir) const;
    RawDirEntry *DirSlot(const DirLocation &dir, uint32_t index);
    const RawDirEntry *DirSlot(const DirLocation &dir, uint32_t index) const;

    // Acha uma entrada livre/apagada dentro de 'dir'; se 'dir' for um
    // subdiretorio cheio, estende a cadeia com mais um cluster
    // automaticamente (a raiz nao pode crescer -- DiskError::DirectoryFull).
    RawDirEntry *FindFreeSlotIn(const DirLocation &dir, DiskError *error);

    RawDirEntry *FindEntryIn(const DirLocation &dir, const std::string &name);
    const RawDirEntry *FindEntryIn(const DirLocation &dir, const std::string &name) const;

    // Percorre 'dir_components' a partir da raiz, seguindo entradas com
    // atributo de diretorio. dir_components vazio devolve a propria raiz.
    std::optional<DirLocation> ResolveDir(const std::vector<std::string> &dir_components, DiskError *error) const;

    Geometry geometry_;
    std::vector<uint8_t> data_; // imagem inteira, setor 0..N-1, em bytes.
};

} // namespace msxdisk
