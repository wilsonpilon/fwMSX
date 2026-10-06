// fwMSX -- banco de ROMs (SQLite). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "romdb.h"

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "sqlite3.h"
#include "../core/hash.h"
#include "classify.h"

namespace fs = std::filesystem;

namespace romdb {
namespace {

// Statement preparado, liberado no fim do escopo.
struct Stmt {
    sqlite3_stmt *s = nullptr;
    ~Stmt() {
        if (s) sqlite3_finalize(s);
    }
};

bool Prepare(sqlite3 *db, const char *sql, Stmt &st, std::string &error) {
    if (sqlite3_prepare_v2(db, sql, -1, &st.s, nullptr) != SQLITE_OK) {
        error = std::string("SQL: ") + sqlite3_errmsg(db);
        return false;
    }
    return true;
}

void BindText(sqlite3_stmt *s, int idx, const std::string &text) {
    sqlite3_bind_text(s, idx, text.c_str(), static_cast<int>(text.size()), SQLITE_TRANSIENT);
}

std::string Text(sqlite3_stmt *s, int col) {
    const unsigned char *t = sqlite3_column_text(s, col);
    return t ? reinterpret_cast<const char *>(t) : std::string();
}

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string Trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

// Linha de `roms` a partir de um SELECT com as colunas de RomRecord, nesta ordem.
RomRecord RowToRom(sqlite3_stmt *s) {
    RomRecord r;
    r.id = sqlite3_column_int64(s, 0);
    r.sha1 = Text(s, 1);
    r.crc32 = Text(s, 2);
    r.size = sqlite3_column_int64(s, 3);
    r.name = Text(s, 4);
    r.category = Text(s, 5);
    r.hardware = Text(s, 6);
    r.mapper = sqlite3_column_int(s, 7);
    r.path = Text(s, 8);
    r.source = Text(s, 9);
    r.notes = Text(s, 10);
    return r;
}

const char *kRomColumns = "id, sha1, crc32, size, name, category, hardware, mapper, path, source, notes";

// Caminho relativo a `root` quando o arquivo esta dentro dela; senao o caminho inteiro.
std::string RelativeTo(const std::string &file, const std::string &root) {
    std::error_code ec;
    const fs::path f = fs::weakly_canonical(file, ec);
    const fs::path r = fs::weakly_canonical(root, ec);
    const std::string fs_path = f.string();
    const std::string root_path = r.string();
    if (!root_path.empty() && fs_path.size() > root_path.size() && fs_path.compare(0, root_path.size(), root_path) == 0) {
        std::string rel = fs_path.substr(root_path.size());
        while (!rel.empty() && (rel[0] == '/' || rel[0] == '\\')) rel.erase(0, 1);
        // Sempre '/', igual em Windows e Linux, para o banco ser o mesmo nas duas plataformas.
        for (char &c : rel) {
            if (c == '\\') c = '/';
        }
        return rel;
    }
    return file;
}

} // namespace

RomDb::~RomDb() {
    if (db_) sqlite3_close(db_);
}

bool RomDb::Exec(const char *sql, std::string &error) {
    char *msg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &msg) != SQLITE_OK) {
        error = msg ? msg : "erro SQL";
        sqlite3_free(msg);
        return false;
    }
    return true;
}

bool RomDb::Open(const std::string &path, std::string &error) {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        error = "nao foi possivel abrir o banco '" + path + "': " + sqlite3_errmsg(db_);
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }
    return Exec(
        "CREATE TABLE IF NOT EXISTS roms ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " sha1 TEXT NOT NULL UNIQUE,"
        " crc32 TEXT NOT NULL DEFAULT '',"
        " size INTEGER NOT NULL DEFAULT 0,"
        " name TEXT NOT NULL DEFAULT '',"
        " category TEXT NOT NULL DEFAULT 'outro',"
        " hardware TEXT NOT NULL DEFAULT '',"
        " mapper INTEGER NOT NULL DEFAULT -1,"
        " path TEXT NOT NULL DEFAULT '',"
        " source TEXT NOT NULL DEFAULT 'manual',"
        " notes TEXT NOT NULL DEFAULT '',"
        " updated TEXT NOT NULL DEFAULT (datetime('now')));"
        "CREATE INDEX IF NOT EXISTS roms_category ON roms(category);"
        "CREATE TABLE IF NOT EXISTS cart_mappers (sha1 TEXT PRIMARY KEY, mapper INTEGER NOT NULL);"
        "CREATE TABLE IF NOT EXISTS meta (key TEXT PRIMARY KEY, value TEXT);",
        error);
}

bool RomDb::Add(RomRecord &rec, std::string &error) {
    rec.sha1 = Lower(rec.sha1);
    if (rec.category.empty()) rec.category = "outro";
    if (rec.source.empty()) rec.source = "manual";
    Stmt st;
    if (!Prepare(db_,
                 "INSERT INTO roms (sha1, crc32, size, name, category, hardware, mapper, path, source, notes)"
                 " VALUES (?1,?2,?3,?4,?5,?6,?7,?8,?9,?10)"
                 " ON CONFLICT(sha1) DO UPDATE SET path = excluded.path, source = excluded.source,"
                 " size = excluded.size, crc32 = excluded.crc32, updated = datetime('now')",
                 st, error)) {
        return false;
    }
    BindText(st.s, 1, rec.sha1);
    BindText(st.s, 2, rec.crc32);
    sqlite3_bind_int64(st.s, 3, rec.size);
    BindText(st.s, 4, rec.name);
    BindText(st.s, 5, rec.category);
    BindText(st.s, 6, rec.hardware);
    sqlite3_bind_int(st.s, 7, rec.mapper);
    BindText(st.s, 8, rec.path);
    BindText(st.s, 9, rec.source);
    BindText(st.s, 10, rec.notes);
    if (sqlite3_step(st.s) != SQLITE_DONE) {
        error = sqlite3_errmsg(db_);
        return false;
    }
    RomRecord saved;
    if (!FindBySha1(rec.sha1, saved)) {
        error = "ROM gravada, mas nao encontrada de volta";
        return false;
    }
    rec = saved;
    return true;
}

bool RomDb::Update(const RomRecord &rec, std::string &error) {
    Stmt st;
    if (!Prepare(db_,
                 "UPDATE roms SET name=?1, category=?2, hardware=?3, mapper=?4, path=?5, source=?6, notes=?7,"
                 " updated=datetime('now') WHERE id=?8",
                 st, error)) {
        return false;
    }
    BindText(st.s, 1, rec.name);
    BindText(st.s, 2, rec.category);
    BindText(st.s, 3, rec.hardware);
    sqlite3_bind_int(st.s, 4, rec.mapper);
    BindText(st.s, 5, rec.path);
    BindText(st.s, 6, rec.source);
    BindText(st.s, 7, rec.notes);
    sqlite3_bind_int64(st.s, 8, rec.id);
    if (sqlite3_step(st.s) != SQLITE_DONE) {
        error = sqlite3_errmsg(db_);
        return false;
    }
    if (sqlite3_changes(db_) == 0) {
        error = "nenhuma ROM com esse id";
        return false;
    }
    return true;
}

bool RomDb::Delete(int64_t id, std::string &error) {
    Stmt st;
    if (!Prepare(db_, "DELETE FROM roms WHERE id=?1", st, error)) return false;
    sqlite3_bind_int64(st.s, 1, id);
    if (sqlite3_step(st.s) != SQLITE_DONE) {
        error = sqlite3_errmsg(db_);
        return false;
    }
    if (sqlite3_changes(db_) == 0) {
        error = "nenhuma ROM com esse id";
        return false;
    }
    return true;
}

bool RomDb::Get(int64_t id, RomRecord &out) const {
    Stmt st;
    std::string error;
    const std::string sql = std::string("SELECT ") + kRomColumns + " FROM roms WHERE id=?1";
    if (!Prepare(db_, sql.c_str(), st, error)) return false;
    sqlite3_bind_int64(st.s, 1, id);
    if (sqlite3_step(st.s) != SQLITE_ROW) return false;
    out = RowToRom(st.s);
    return true;
}

bool RomDb::FindBySha1(const std::string &sha1, RomRecord &out) const {
    Stmt st;
    std::string error;
    const std::string sql = std::string("SELECT ") + kRomColumns + " FROM roms WHERE sha1=?1";
    if (!Prepare(db_, sql.c_str(), st, error)) return false;
    BindText(st.s, 1, Lower(sha1));
    if (sqlite3_step(st.s) != SQLITE_ROW) return false;
    out = RowToRom(st.s);
    return true;
}

std::vector<RomRecord> RomDb::Search(const std::string &text, const std::string &category) const {
    std::vector<RomRecord> out;
    Stmt st;
    std::string error;
    const std::string sql = std::string("SELECT ") + kRomColumns +
                            " FROM roms WHERE (?1 = '' OR name LIKE ?1 OR sha1 LIKE ?1 OR hardware LIKE ?1"
                            " OR notes LIKE ?1 OR path LIKE ?1) AND (?2 = '' OR category = ?2)"
                            " ORDER BY category, name, sha1 LIMIT 5000";
    if (!Prepare(db_, sql.c_str(), st, error)) return out;
    BindText(st.s, 1, text.empty() ? std::string() : "%" + text + "%");
    BindText(st.s, 2, category);
    while (sqlite3_step(st.s) == SQLITE_ROW) out.push_back(RowToRom(st.s));
    return out;
}

int64_t RomDb::Count() const {
    Stmt st;
    std::string error;
    if (!Prepare(db_, "SELECT COUNT(*) FROM roms", st, error)) return 0;
    return sqlite3_step(st.s) == SQLITE_ROW ? sqlite3_column_int64(st.s, 0) : 0;
}

bool RomDb::ImportCartsSha(const std::string &path, int64_t &rows, std::string &error) {
    std::ifstream f(path);
    if (!f) {
        error = "nao foi possivel abrir '" + path + "'";
        return false;
    }
    Exec("BEGIN", error);
    Stmt st;
    if (!Prepare(db_, "INSERT OR REPLACE INTO cart_mappers (sha1, mapper) VALUES (?1, ?2)", st, error)) {
        Exec("ROLLBACK", error);
        return false;
    }
    rows = 0;
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream in(line);
        std::string sha, mapper_text;
        if (!(in >> sha >> mapper_text)) continue;
        if (sha.size() != 40) continue;
        sqlite3_reset(st.s);
        BindText(st.s, 1, Lower(sha));
        sqlite3_bind_int(st.s, 2, std::atoi(mapper_text.c_str()));
        if (sqlite3_step(st.s) == SQLITE_DONE) ++rows;
    }
    return Exec("COMMIT", error);
}

int RomDb::CartMapper(const std::string &sha1) const {
    Stmt st;
    std::string error;
    if (!Prepare(db_, "SELECT mapper FROM cart_mappers WHERE sha1=?1", st, error)) return -1;
    BindText(st.s, 1, Lower(sha1));
    return sqlite3_step(st.s) == SQLITE_ROW ? sqlite3_column_int(st.s, 0) : -1;
}

bool RomDb::ImportVampierSql(const std::string &sql_path, int64_t &rows, std::string &error) {
    std::vector<uint8_t> bytes;
    if (!ReadWholeFile(sql_path, bytes)) {
        error = "nao foi possivel abrir '" + sql_path + "'";
        return false;
    }
    const std::string sql(bytes.begin(), bytes.end());
    // O dump cria as tabelas sem IF NOT EXISTS: recria do zero.
    if (!Exec("DROP TABLE IF EXISTS msxdb_romdetails; DROP TABLE IF EXISTS msxdb_rominfo;"
              " DROP TABLE IF EXISTS msxdb_company;",
              error)) {
        return false;
    }
    if (!Exec(sql.c_str(), error)) {
        if (sqlite3_get_autocommit(db_) == 0) Exec("ROLLBACK", error);
        return false;
    }
    if (sqlite3_get_autocommit(db_) == 0) Exec("COMMIT", error);
    Stmt st;
    rows = 0;
    if (Prepare(db_, "SELECT COUNT(*) FROM msxdb_romdetails", st, error) && sqlite3_step(st.s) == SQLITE_ROW) {
        rows = sqlite3_column_int64(st.s, 0);
    }
    return true;
}

bool RomDb::HasVampier() const {
    Stmt st;
    std::string error;
    if (!Prepare(db_, "SELECT COUNT(*) FROM sqlite_master WHERE name='msxdb_romdetails'", st, error)) return false;
    return sqlite3_step(st.s) == SQLITE_ROW && sqlite3_column_int(st.s, 0) > 0;
}

std::vector<VampierHit> RomDb::VampierSearch(const std::string &text) const {
    std::vector<VampierHit> out;
    if (!HasVampier()) return out;
    Stmt st;
    std::string error;
    const char *sql =
        "SELECT d.SHA1, IFNULL(i.GameName,''), IFNULL(i.Year,''), IFNULL(c.ShortName,''), IFNULL(d.RomType,''),"
        " IFNULL(d.Dump,''), IFNULL(d.Remark,'')"
        " FROM msxdb_romdetails d"
        " LEFT JOIN msxdb_rominfo i ON i.GameID = d.GameID"
        " LEFT JOIN msxdb_company c ON c.CompanyID = i.CompanyID1"
        " WHERE (?1 = '' OR i.GameName LIKE ?1 OR d.SHA1 LIKE ?1)"
        " ORDER BY i.GameName, d.SHA1 LIMIT 5000";
    if (!Prepare(db_, sql, st, error)) return out;
    BindText(st.s, 1, text.empty() ? std::string() : "%" + text + "%");
    while (sqlite3_step(st.s) == SQLITE_ROW) {
        VampierHit h;
        h.sha1 = Text(st.s, 0);
        h.game = Text(st.s, 1);
        h.year = Text(st.s, 2);
        h.company = Text(st.s, 3);
        h.rom_type = Text(st.s, 4);
        h.dump = Text(st.s, 5);
        h.remark = Text(st.s, 6);
        out.push_back(h);
    }
    return out;
}

int64_t RomDb::IdentifyWithVampier(std::string &error) {
    if (!HasVampier()) {
        error = "banco do Vampier nao importado (fwmsx --romdb vampier)";
        return 0;
    }
    if (!Exec("UPDATE roms SET name = (SELECT i.GameName FROM msxdb_romdetails d"
              " JOIN msxdb_rominfo i ON i.GameID = d.GameID WHERE d.SHA1 = UPPER(roms.sha1) LIMIT 1)"
              " WHERE name = '' AND EXISTS (SELECT 1 FROM msxdb_romdetails d WHERE d.SHA1 = UPPER(roms.sha1))",
              error)) {
        return 0;
    }
    return sqlite3_changes(db_);
}

bool RomDb::ScanFile(const std::string &file, const std::string &root, const std::string &source, RomRecord &out,
                     std::string &error) {
    std::vector<uint8_t> bytes;
    if (!ReadWholeFile(file, bytes)) {
        error = "nao foi possivel ler '" + file + "'";
        return false;
    }
    RomRecord rec;
    rec.sha1 = Sha1Hex(bytes.data(), bytes.size());
    rec.crc32 = Crc32Hex(bytes.data(), bytes.size());
    rec.size = static_cast<int64_t>(bytes.size());
    const fs::path p(file);
    // Nome vazio: o nome do jogo vem do usuario ou do banco do Vampier (identify).
    // Sem nome, a tela mostra o nome do arquivo.
    rec.category = CategoryFor(p.filename().string());
    rec.path = RelativeTo(file, root);
    rec.source = source;
    rec.mapper = CartMapper(rec.sha1);
    // Nome e notas que o usuario ja tiver dado ficam: Add() so' atualiza caminho e origem.
    RomRecord existing;
    if (FindBySha1(rec.sha1, existing)) {
        rec.name = existing.name;
        rec.hardware = existing.hardware;
        rec.notes = existing.notes;
        if (existing.mapper >= 0) rec.mapper = existing.mapper;
        rec.category = existing.category;
    }
    if (!Add(rec, error)) return false;
    out = rec;
    return true;
}

bool RomDb::ScanDirectory(const std::string &dir, const std::string &root, const std::string &source, int64_t &added,
                          int64_t &updated, std::string &error) {
    added = updated = 0;
    std::error_code ec;
    if (!fs::is_directory(dir, ec)) {
        error = "'" + dir + "' nao e' uma pasta";
        return false;
    }
    Exec("BEGIN", error);
    for (auto it = fs::recursive_directory_iterator(dir, ec); it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) break;
        if (!it->is_regular_file(ec)) continue;
        const std::string ext = Lower(it->path().extension().string());
        // Arquivos que nao sao ROM: pagina HTML, texto, o proprio executavel.
        if (ext == ".exe" || ext == ".html" || ext == ".htm" || ext == ".txt" || ext == ".md" || ext == ".zip" ||
            ext == ".db") {
            continue;
        }
        RomRecord existing;
        std::vector<uint8_t> bytes;
        if (!ReadWholeFile(it->path().string(), bytes)) continue;
        const std::string sha = Sha1Hex(bytes.data(), bytes.size());
        const bool known = FindBySha1(sha, existing);
        RomRecord rec;
        if (!ScanFile(it->path().string(), root, source, rec, error)) {
            error.clear();
            continue;
        }
        if (known) ++updated;
        else ++added;
    }
    Exec("COMMIT", error);
    return true;
}

bool ReadWholeFile(const std::string &path, std::vector<uint8_t> &out) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return false;
    const std::streamsize size = f.tellg();
    out.resize(static_cast<size_t>(size < 0 ? 0 : size));
    f.seekg(0);
    if (size > 0) f.read(reinterpret_cast<char *>(out.data()), size);
    return static_cast<bool>(f) || size == 0;
}

} // namespace romdb
