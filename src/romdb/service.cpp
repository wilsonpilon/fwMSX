// fwMSX -- fluxos do banco de ROMs. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "service.h"

#include <cstdio>
#include <filesystem>
#include <system_error>

#include "archive/unzip.h"
#include "core/hash.h"
#include "net/web.h"
#include "sig/sig.h"
#include "store/classify.h"

namespace fs = std::filesystem;

namespace romdb {

const char *const kFmsxDownloadsUrl = "https://fms.komkon.org/fMSX/";
const char *const kFileHunterSystemRoms = "https://download.file-hunter.com/System%20ROMs/";
const char *const kOpenmsxTreeUrl = "https://api.github.com/repos/openMSX/openMSX/git/trees/master?recursive=1";
const char *const kOpenmsxRawBase = "https://raw.githubusercontent.com/openMSX/openMSX/master/";
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

std::string FindMsxDos2Rom(const RomPaths &paths, std::string &error) {
    RomDb db;
    if (!OpenRomDb(paths, db, error)) return "";
    // msxdos22.rom (a ROM generica do MSX-DOS 2.20), as variantes ASCII e a do MK DOS 2.20; o papel cobre as que o openMSX conhece e a gente nao listou
    static const char *const kKnown[] = {"c36c9e0f96738a340381e23b4f97245388801a46", "42f4e3360eb134857b0efa82c1abedf342680124",
                                         "0e253ee076eff37f7ee450c0b10c71763fac990b"};
    std::vector<RomRecord> found;
    for (const char *sha : kKnown) {
        RomRecord r;
        if (db.FindBySha1(sha, r)) found.push_back(r);
    }
    for (const RomRecord &r : db.Search("MSX-DOS 2", "")) {
        if (r.role == "MSX-DOS 2" && r.size == 65536) found.push_back(r);
    }
    for (const RomRecord &r : found) {
        const fs::path p = fs::path(r.path).is_absolute() ? fs::path(r.path) : fs::path(paths.root) / r.path;
        std::error_code ec;
        if (fs::is_regular_file(p, ec)) return p.string();
    }
    error = "nao achei a ROM do MSX-DOS 2 no banco de ROMs: baixe o Full Set System ROMs (janela Downloads ou `romdb filehunter-full`) ou use `dos2 <arquivo.rom>`";
    return "";
}

std::string FindMegaSdRom(const RomPaths &paths, std::string &error) {
    for (const char *name : {"nextsd15.rom", "nextsd14.rom", "n3xtsd15.rom", "n3xtsd14.rom", "megasd1s.rom"}) {
        const fs::path p = fs::path(paths.root) / "ocm" / "roms" / name;
        std::error_code ec;
        if (fs::is_regular_file(p, ec)) return p.string();
    }
    error = "nao achei o Nextor com o driver MegaSD (nextsd15.rom) em " + (fs::path(paths.root) / "ocm" / "roms").string() +
            ": copie para la' as ROMs do pacote SDBIOS do KdL (SDBIOS/make/roms) ou use --megasd-rom <arquivo>";
    return "";
}

std::string FindNextorRom(const RomPaths &paths, std::string &error) {
    RomDb db;
    if (!OpenRomDb(paths, db, error)) return "";
    // Nextor 2.1.1, 2.1.0, 2.1.0 (versao "emulators"), 2.0.1 e 2.0 para a Sunrise IDE, nessa ordem
    static const char *const kKnown[] = {"dca824d7b0ddf25c6e87a8098e97ab7489725f57", "d3a4375ff5f58cf59cc609dd41c90af285f033c2",
                                         "97094d5ed206088c8f455801b7609e614c243562", "61cba1680ac6cb448dc3e8c710a43f4e7ab49457",
                                         "ab1d6dc0cb851c0ca15b260674ad884d0b8d85fb"};
    for (const char *sha : kKnown) {
        RomRecord r;
        if (!db.FindBySha1(sha, r)) continue;
        const fs::path p = fs::path(r.path).is_absolute() ? fs::path(r.path) : fs::path(paths.root) / r.path;
        std::error_code ec;
        if (fs::is_regular_file(p, ec)) return p.string();
    }
    error = "nao achei a ROM do Nextor (Sunrise IDE) no banco de ROMs: baixe o Full Set System ROMs (janela Downloads ou `romdb filehunter-full`) ou use `ide ... <nextor.rom>`";
    return "";
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

// Depois de trazer ROMs de sistema: se as assinaturas do openMSX ainda nao estao no banco, baixa; depois identifica. Uma falha aqui nao desfaz o download.
void IdentifyAfterDownload(const RomPaths &paths, RomDb &db, const Log &log);

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
    IdentifyAfterDownload(paths, db, log);
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
        IdentifyAfterDownload(paths, db, log);
        return true;
    }
    RomRecord rec;
    if (!db.ScanFile(dest.string(), paths.root, "filehunter", rec, error)) return false;
    added = 1;
    IdentifyAfterDownload(paths, db, log);
    return true;
}

// Os caminhos "share/machines/X.xml" e "share/extensions/X.xml" do JSON da arvore do GitHub ("path": "..."), sem biblioteca de JSON: os nomes nao tem aspas nem barras.
std::vector<std::string> OpenmsxPathsFromTree(const std::string &json) {
    std::vector<std::string> out;
    const std::string key = "\"path\"";
    size_t pos = 0;
    while ((pos = json.find(key, pos)) != std::string::npos) {
        pos += key.size();
        const size_t q1 = json.find('"', json.find(':', pos));
        if (q1 == std::string::npos) break;
        const size_t q2 = json.find('"', q1 + 1);
        if (q2 == std::string::npos) break;
        const std::string path = json.substr(q1 + 1, q2 - q1 - 1);
        pos = q2 + 1;
        const bool xml = path.size() > 4 && path.compare(path.size() - 4, 4, ".xml") == 0;
        if (!xml) continue;
        const bool wanted = path.rfind("share/machines/", 0) == 0 || path.rfind("share/extensions/", 0) == 0;
        if (wanted && path.find("..") == std::string::npos && path.find('\\') == std::string::npos) out.push_back(path);
    }
    return out;
}

namespace {

bool StoreSignatures(RomDb &db, const std::vector<SigRow> &rows, int files, const Log &log, int64_t &identified, std::string &error) {
    log("Lidos " + std::to_string(files) + " arquivos, " + std::to_string(rows.size()) + " assinaturas.");
    if (!db.ReplaceSignatures(rows, error)) return false;
    identified = db.IdentifyWithOpenmsx(error);
    if (!error.empty()) return false;
    log(std::to_string(db.SignatureCount()) + " ROMs distintas conhecidas; " + std::to_string(identified) + " do banco identificadas.");
    return true;
}

}  // namespace

bool ImportOpenmsxShare(RomDb &db, const std::string &share_dir, const Log &log, int64_t &identified, std::string &error) {
    identified = 0;
    std::vector<SigRow> rows;
    int files = 0;
    if (!ReadOpenmsxShare(share_dir, rows, files, error)) return false;
    return StoreSignatures(db, rows, files, log, identified, error);
}

bool DownloadOpenmsxSignatures(const RomPaths &paths, RomDb &db, const Log &log, int64_t &identified, std::string &error) {
    identified = 0;
    log("Procurando os XML de maquinas e extensoes no GitHub do openMSX ...");
    std::string json;
    if (!HttpGetText(kOpenmsxTreeUrl, json, error)) return false;
    const std::vector<std::string> wanted = OpenmsxPathsFromTree(json);
    if (wanted.empty()) {
        error = "a arvore do openMSX nao trouxe share/machines nem share/extensions (limite da API do GitHub? tente de novo mais tarde)";
        return false;
    }
    const fs::path share = fs::path(paths.tmp_dir()) / "openmsx" / "share";
    std::error_code ec;
    fs::remove_all(share, ec);
    fs::create_directories(share / "machines", ec);
    fs::create_directories(share / "extensions", ec);
    std::vector<std::pair<std::string, std::string>> jobs;
    for (const std::string &w : wanted) {
        jobs.emplace_back(std::string(kOpenmsxRawBase) + UrlEncodePath(w), (fs::path(paths.tmp_dir()) / "openmsx" / w).string());
    }
    log("Baixando " + std::to_string(jobs.size()) + " arquivos XML ...");
    if (!HttpDownloadMany(jobs, "XML do openMSX", error)) {
        fs::remove_all(fs::path(paths.tmp_dir()) / "openmsx", ec);
        return false;
    }
    const bool ok = ImportOpenmsxShare(db, share.string(), log, identified, error);
    fs::remove_all(fs::path(paths.tmp_dir()) / "openmsx", ec);
    CleanTmp(paths);
    return ok;
}

void IdentifyAfterDownload(const RomPaths &paths, RomDb &db, const Log &log) {
    std::string error;
    int64_t identified = 0;
    if (db.SignatureCount() == 0) {
        log("Assinaturas do openMSX ainda nao baixadas: baixando para identificar as ROMs ...");
        if (!DownloadOpenmsxSignatures(paths, db, log, identified, error)) log("Identificacao pulada: " + error);
        return;
    }
    identified = db.IdentifyWithOpenmsx(error);
    if (error.empty()) log(std::to_string(identified) + " ROMs identificadas pelo openMSX.");
    else log("Identificacao pulada: " + error);
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
