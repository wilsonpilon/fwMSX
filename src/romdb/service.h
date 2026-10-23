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

// A ROM do kernel do MSX-DOS 2 (cartucho de 64KB, mapper msxdos2) que o banco ja' tem no disco: o msxdos22.rom do openMSX (pelo SHA-1, mesmo antes do
// `identify`), depois qualquer ROM de 64KB com o papel "MSX-DOS 2", depois as variantes conhecidas. Devolve o caminho completo ou "" (e `error` diz
// o que fazer). Nao baixa nada.
std::string FindMsxDos2Rom(const RomPaths &paths, std::string &error);

// Assinaturas das ROMs de sistema do openMSX (nome oficial, papel, hardware): baixa os XML de share/machines e share/extensions do GitHub (um unico curl,
// ~350 arquivos pequenos), le, grava na tabela `signatures` e identifica as ROMs do banco. `sigs` = quantas ROMs distintas o openMSX reconhece.
bool DownloadOpenmsxSignatures(const RomPaths &paths, RomDb &db, const Log &log, int64_t &identified, std::string &error);
// Os caminhos share/machines/*.xml e share/extensions/*.xml do JSON da arvore do GitHub (exposto para o teste).
std::vector<std::string> OpenmsxPathsFromTree(const std::string &json);
// O mesmo a partir de uma pasta share do openMSX que o usuario ja' tem (sem rede).
bool ImportOpenmsxShare(RomDb &db, const std::string &share_dir, const Log &log, int64_t &identified, std::string &error);

// Baixa o pacote SQL do banco do Vampier (msxromdb), extrai e importa.
bool ImportVampier(const RomPaths &paths, RomDb &db, const Log &log, int64_t &rows, std::string &error);

// URLs usadas pelos downloads (publicas, sem login).
extern const char *const kFmsxDownloadsUrl;      // https://fms.komkon.org/fMSX/
extern const char *const kFileHunterSystemRoms;  // https://download.file-hunter.com/System%20ROMs/
extern const char *const kOpenmsxTreeUrl;    // a arvore do openMSX no GitHub (API), para achar os XML de share/
extern const char *const kOpenmsxRawBase;    // https://raw.githubusercontent.com/openMSX/openMSX/master/
extern const char *const kVampierSqlUrl;         // https://romdb.vampier.net/Archive/sql-msxromdb.zip

} // namespace romdb
