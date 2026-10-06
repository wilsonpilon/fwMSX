// fwMSX -- fluxos do banco de ROMs. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "service.h"

#include <cstdio>
#include <filesystem>
#include <system_error>

#include "archive/unzip.h"
#include "core/hash.h"
#include "net/web.h"
#include "store/classify.h"

namespace fs = std::filesystem;

namespace romdb {

const char *const kFmsxDownloadsUrl = "https://fms.komkon.org/fMSX/";
const char *const kFileHunterSystemRoms = "https://download.file-hunter.com/System%20ROMs/";
const char *const kVampierSqlUrl = "https://romdb.vampier.net/Archive/sql-msxromdb.zip";

std::string RomPaths::db_file() const { return (fs::path(root) / "roms.db").string(); }
std::string RomPaths::tmp_dir() const { return (fs::path(root) / "_tmp").string(); }

RomPaths DefaultRomPaths(const std::string &argv0) {
    std::error_code ec;
    const fs::path exe_dir = fs::absolute(fs::path(argv0), ec).parent_path();
    RomPaths p;
    p.root = (exe_dir.empty() ? fs::path(".") : exe_dir / "roms").string();
    return p;
}

bool OpenRomDb(const RomPaths &paths, RomDb &db, std::string &error) {
    std::error_code ec;
    fs::create_directories(paths.root, ec);
    if (ec) {
        error = "nao foi possivel criar a pasta de ROMs '" + paths.root + "': " + ec.message();
        return false;
    }
    return db.Open(paths.db_file(), error);
}

namespace {

// Copia para a pasta do tipo e registra no banco.
bool StoreFile(const RomPaths &paths, RomDb &db, const std::string &file, const std::string &source,
               int64_t &added, std::string &error) {
    const std::string name = fs::path(file).filename().string();
    const fs::path dest_dir = fs::path(paths.root) / FolderForCategory(CategoryFor(name));
    std::error_code ec;
    fs::create_directories(dest_dir, ec);
    const fs::path dest = dest_dir / name;
    if (fs::path(file) != dest) fs::copy_file(file, dest, fs::copy_options::overwrite_existing, ec);
    if (ec) {
        error = "nao foi possivel copiar '" + name + "': " + ec.message();
        return false;
    }
    RomRecord rec;
    if (!db.ScanFile(dest.string(), paths.root, source, rec, error)) return false;
    ++added;
    return true;
}

// Baixa `url` para o diretorio temporario e devolve o caminho do arquivo.
bool FetchToTmp(const RomPaths &paths, const std::string &url, std::string &local, std::string &error) {
    std::error_code ec;
    fs::create_directories(paths.tmp_dir(), ec);
    const std::string name = fs::path(url).filename().string();
    // A URL pode ter %20: nome local sem codificacao.
    const std::string decoded = UrlDecode(name);
    local = (fs::path(paths.tmp_dir()) / decoded).string();
    return HttpDownload(url, local, error);
}

void CleanTmp(const RomPaths &paths) {
    std::error_code ec;
    fs::remove_all(paths.tmp_dir(), ec);
}

} // namespace

bool DownloadFmsx(const RomPaths &paths, RomDb &db, const Log &log, int64_t &added, std::string &error) {
    added = 0;
    log("Consultando " + std::string(kFmsxDownloadsUrl) + " ...");
    std::string html;
    if (!HttpGetText(kFmsxDownloadsUrl, html, error)) return false;
    const std::string url = LatestFmsxWindowsZip(html, kFmsxDownloadsUrl);
    if (url.empty()) {
        error = "pacote do fMSX para Windows nao encontrado na pagina de downloads";
        return false;
    }
    log("Baixando " + url + " ...");
    std::string zip;
    if (!FetchToTmp(paths, url, zip, error)) return false;

    const std::string extract_dir = (fs::path(paths.tmp_dir()) / "fmsx").string();
    std::vector<std::string> files;
    log("Extraindo...");
    if (!ExtractZip(zip, extract_dir, files, error)) {
        CleanTmp(paths);
        return false;
    }
    for (const std::string &f : files) {
        const std::string ext = fs::path(f).extension().string();
        // So' o que e' dado de ROM/tabela: sem o executavel nem a pagina de ajuda.
        if (ext == ".exe" || ext == ".html" || ext == ".htm") continue;
        log("Guardando " + fs::path(f).filename().string());
        if (!StoreFile(paths, db, f, "fmsx", added, error)) {
            CleanTmp(paths);
            return false;
        }
    }
    CleanTmp(paths);
    return true;
}

bool ListFileHunter(const std::string &url, std::vector<ListingEntry> &entries, std::string &error) {
    std::string html;
    if (!HttpGetText(url, html, error)) return false;
    entries = ParseListing(html, url);
    return true;
}

bool DownloadFileHunterFullSet(const RomPaths &paths, RomDb &db, const std::string &date, const Log &log,
                               int64_t &added, std::string &error) {
    added = 0;
    std::vector<ListingEntry> entries;
    if (!ListFileHunter(kFileHunterSystemRoms, entries, error)) return false;

    const ListingEntry *pick = nullptr;
    if (date.empty()) {
        pick = LatestFullSet(entries);
    } else {
        for (const ListingEntry &e : entries) {
            int y = 0, m = 0, d = 0;
            if (!e.is_dir && e.name.find("Full Set System ROMs") != std::string::npos &&
                ParseFullSetDate(e.name, y, m, d)) {
                char wanted[11];
                std::snprintf(wanted, sizeof wanted, "%02d-%02d-%04d", d, m, y);
                if (date == wanted) pick = &e;
            }
        }
    }
    if (!pick) {
        error = date.empty() ? "nenhum 'Full Set System ROMs' encontrado" : "nenhum Full Set com a data " + date;
        return false;
    }

    // Data do nome, para a pasta: "15-08-2026".
    int y = 0, m = 0, d = 0;
    ParseFullSetDate(pick->name, y, m, d);
    char stamp[16];
    std::snprintf(stamp, sizeof stamp, "%02d-%02d-%04d", d, m, y);
    log("Baixando " + pick->name + " (" + pick->size + ") ...");
    std::string zip;
    if (!FetchToTmp(paths, pick->url, zip, error)) return false;

    const fs::path target = fs::path(paths.root) / "filehunter" / stamp;
    std::error_code ec;
    fs::create_directories(target, ec);
    std::vector<std::string> files;
    log("Extraindo em " + target.string() + " ...");
    if (!ExtractZip(zip, target.string(), files, error)) {
        CleanTmp(paths);
        return false;
    }
    CleanTmp(paths);
    log("Registrando ROMs no banco...");
    int64_t updated = 0;
    if (!db.ScanDirectory(target.string(), paths.root, "filehunter", added, updated, error)) return false;
    added += updated;
    return true;
}

bool DownloadFileHunterEntry(const RomPaths &paths, RomDb &db, const std::string &entry_url, const Log &log,
                             int64_t &added, std::string &error) {
    added = 0;
    // Caminho relativo ao indice: "extensions/Carnivore2.rom" -> pasta filehunter/extensions.
    std::string rel = entry_url;
    const std::string base = kFileHunterSystemRoms;
    if (rel.rfind(base, 0) == 0) rel = rel.substr(base.size());
    rel = UrlDecode(rel);
    const fs::path dest = fs::path(paths.root) / "filehunter" / rel;
    std::error_code ec;
    fs::create_directories(dest.parent_path(), ec);
    log("Baixando " + rel + " ...");
    if (!HttpDownload(entry_url, dest.string(), error)) return false;
    if (dest.extension() == ".zip") {
        const fs::path out = dest.parent_path() / dest.stem();
        std::vector<std::string> files;
        if (!ExtractZip(dest.string(), out.string(), files, error)) return false;
        int64_t updated = 0;
        if (!db.ScanDirectory(out.string(), paths.root, "filehunter", added, updated, error)) return false;
        added += updated;
        return true;
    }
    RomRecord rec;
    if (!db.ScanFile(dest.string(), paths.root, "filehunter", rec, error)) return false;
    added = 1;
    return true;
}

bool ImportVampier(const RomPaths &paths, RomDb &db, const Log &log, int64_t &rows, std::string &error) {
    rows = 0;
    log("Baixando " + std::string(kVampierSqlUrl) + " ...");
    std::string zip;
    if (!FetchToTmp(paths, kVampierSqlUrl, zip, error)) return false;
    const std::string extract_dir = (fs::path(paths.tmp_dir()) / "vampier").string();
    std::vector<std::string> files;
    if (!ExtractZip(zip, extract_dir, files, error)) {
        CleanTmp(paths);
        return false;
    }
    std::string sql;
    for (const std::string &f : files) {
        if (fs::path(f).extension() == ".sql") sql = f;
    }
    if (sql.empty()) {
        CleanTmp(paths);
        error = "o pacote do Vampier nao tem arquivo .sql";
        return false;
    }
    log("Importando " + fs::path(sql).filename().string() + " (pode levar alguns segundos)...");
    const bool ok = db.ImportVampierSql(sql, rows, error);
    CleanTmp(paths);
    return ok;
}

} // namespace romdb
