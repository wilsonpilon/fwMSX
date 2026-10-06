// fwMSX -- fluxos do banco de ROMs: baixar o fMSX, o System ROMs do file-hunter, o
// banco do Vampier e escanear pastas. Usado pela CLI (`fwmsx --romdb`) e pela GUI
// (menu ROMs). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/romdb-spec.md.
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "net/listing.h"
#include "store/romdb.h"

namespace romdb {

using Log = std::function<void(const std::string &)>;

// Pasta de ROMs e banco. Padrao: `roms/` ao lado do executavel.
struct RomPaths {
    std::string root;
    std::string db_file() const;  // root/roms.db
    std::string tmp_dir() const;  // root/_tmp
};

// Padrao da pasta de ROMs: `roms/` ao lado do executavel (argv0), ou no diretorio atual.
RomPaths DefaultRomPaths(const std::string &argv0);

// Cria a pasta e abre o banco.
bool OpenRomDb(const RomPaths &paths, RomDb &db, std::string &error);

// Pagina de downloads do fMSX: baixa o ZIP de Windows mais recente, extrai e grava
// cada arquivo na pasta do tipo (bios, interfaces, cartuchos...), registrando no banco.
bool DownloadFmsx(const RomPaths &paths, RomDb &db, const Log &log, int64_t &added, std::string &error);

// Indice da pasta de ROMs do file-hunter (URL de pasta, termina em '/').
bool ListFileHunter(const std::string &url, std::vector<ListingEntry> &entries, std::string &error);

// Baixa o "Full Set System ROMs for OpenMSX" de uma data (DD-MM-AAAA) ou o mais recente
// (`date` vazio). Extrai em root/filehunter/<data>/ e registra as ROMs.
bool DownloadFileHunterFullSet(const RomPaths &paths, RomDb &db, const std::string &date, const Log &log,
                               int64_t &added, std::string &error);

// Baixa um arquivo (ou pacote ZIP) do file-hunter para root/filehunter/<caminho>, extrai
// e registra. `entry_url` e' a URL completa do arquivo. Pastas nao sao baixadas por aqui.
bool DownloadFileHunterEntry(const RomPaths &paths, RomDb &db, const std::string &entry_url, const Log &log,
                             int64_t &added, std::string &error);

// Baixa o pacote SQL do banco do Vampier (msxromdb), extrai e importa.
bool ImportVampier(const RomPaths &paths, RomDb &db, const Log &log, int64_t &rows, std::string &error);

// URLs usadas pelos downloads (publicas, sem login).
extern const char *const kFmsxDownloadsUrl;      // https://fms.komkon.org/fMSX/
extern const char *const kFileHunterSystemRoms;  // https://download.file-hunter.com/System%20ROMs/
extern const char *const kVampierSqlUrl;         // https://romdb.vampier.net/Archive/sql-msxromdb.zip

} // namespace romdb
