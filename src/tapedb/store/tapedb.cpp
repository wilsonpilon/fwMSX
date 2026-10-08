// fwMSX -- banco de fitas (SQLite). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "tapedb.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "sqlite3.h"
#include "../../romdb/core/hash.h"
#include "../../tape/cpp/cas_reader.h"
#include "../../tape/cpp/tzx_reader.h"

namespace fs = std::filesystem;

namespace tapedb {

namespace {

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

TapeRecord RowToTape(sqlite3_stmt *s) {
    TapeRecord r;
    r.id = sqlite3_column_int64(s, 0);
    r.sha1 = Text(s, 1);
    r.size = sqlite3_column_int64(s, 2);
    r.format = Text(s, 3);
    r.title = Text(s, 4);
    r.company = Text(s, 5);
    r.year = Text(s, 6);
    r.path = Text(s, 7);
    r.source = Text(s, 8);
    r.notes = Text(s, 9);
    return r;
}

const char *kTapeColumns = "id, sha1, size, format, title, company, year, path, source, notes";

// Caminho relativo a `root` quando o arquivo esta dentro dela; senao o caminho inteiro.
// (mesma logica de romdb::RelativeTo(), duplicada aqui -- bancos deliberadamente
// independentes, ver o comentario no topo de tapedb.h).
std::string RelativeTo(const std::string &file, const std::string &root) {
    std::error_code ec;
    const fs::path f = fs::weakly_canonical(file, ec);
    const fs::path r = fs::weakly_canonical(root, ec);
    const std::string fs_path = f.string();
    const std::string root_path = r.string();
    if (!root_path.empty() && fs_path.size() > root_path.size() && fs_path.compare(0, root_path.size(), root_path) == 0) {
        std::string rel = fs_path.substr(root_path.size());
        while (!rel.empty() && (rel[0] == '/' || rel[0] == '\\')) rel.erase(0, 1);
        for (char &c : rel) {
            if (c == '\\') c = '/';
        }
        return rel;
    }
    return file;
}

bool ReadWholeFile(const std::string &path, std::vector<uint8_t> &out) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return false;
    const std::streamsize size = f.tellg();
    out.resize(static_cast<std::size_t>(size < 0 ? 0 : size));
    f.seekg(0);
    if (size > 0) f.read(reinterpret_cast<char *>(out.data()), size);
    return static_cast<bool>(f) || size == 0;
}

bool EndsWithCi(const std::string &s, const char *suffix) {
    const std::size_t n = std::strlen(suffix);
    if (s.size() < n) return false;
    return std::equal(s.end() - static_cast<std::ptrdiff_t>(n), s.end(), suffix,
                       [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == b; });
}

// Nome do 1o arquivo encontrado dentro da fita (leitor existente -- cas_reader/
// tzx_reader), ou vazio se a fita nao abrir/estiver vazia. So' para auto-preencher
// o titulo de uma fita NOVA no banco; nunca sobrescreve um titulo que o usuario
// ja' tenha dado (ver ScanFile()).
std::string FirstFileNameIn(const std::string &path) {
    tape::TapeImage img;
    std::string error;
    const bool is_tsx = EndsWithCi(path, ".tsx") || EndsWithCi(path, ".tzx");
    const bool ok = is_tsx ? tape::LoadTzxImage(path, img, error) : tape::LoadCasImage(path, img, error);
    if (!ok || img.files.empty()) return std::string();
    return img.files.front().name;
}

std::string FormatFor(const std::string &path) {
    if (EndsWithCi(path, ".tsx")) return "tsx";
    if (EndsWithCi(path, ".tzx")) return "tzx";
    if (EndsWithCi(path, ".cas")) return "cas";
    return "";
}

} // namespace

TapeDb::~TapeDb() {
    if (db_) sqlite3_close(db_);
}

bool TapeDb::Exec(const char *sql, std::string &error) {
    char *msg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &msg) != SQLITE_OK) {
        error = msg ? msg : "erro SQL";
        sqlite3_free(msg);
        return false;
    }
    return true;
}

bool TapeDb::Open(const std::string &path, std::string &error) {
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
        "CREATE TABLE IF NOT EXISTS fitas ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " sha1 TEXT NOT NULL UNIQUE,"
        " size INTEGER NOT NULL DEFAULT 0,"
        " format TEXT NOT NULL DEFAULT '',"
        " title TEXT NOT NULL DEFAULT '',"
        " company TEXT NOT NULL DEFAULT '',"
        " year TEXT NOT NULL DEFAULT '',"
        " path TEXT NOT NULL DEFAULT '',"
        " source TEXT NOT NULL DEFAULT 'manual',"
        " notes TEXT NOT NULL DEFAULT '',"
        " updated TEXT NOT NULL DEFAULT (datetime('now')));"
        "CREATE INDEX IF NOT EXISTS fitas_title ON fitas(title);",
        error);
}

bool TapeDb::Add(TapeRecord &rec, std::string &error) {
    rec.sha1 = Lower(rec.sha1);
    if (rec.source.empty()) rec.source = "manual";
    Stmt st;
    if (!Prepare(db_,
                 "INSERT INTO fitas (sha1, size, format, title, company, year, path, source, notes)"
                 " VALUES (?1,?2,?3,?4,?5,?6,?7,?8,?9)"
                 " ON CONFLICT(sha1) DO UPDATE SET path = excluded.path, source = excluded.source,"
                 " size = excluded.size, format = excluded.format, updated = datetime('now')",
                 st, error)) {
        return false;
    }
    BindText(st.s, 1, rec.sha1);
    sqlite3_bind_int64(st.s, 2, rec.size);
    BindText(st.s, 3, rec.format);
    BindText(st.s, 4, rec.title);
    BindText(st.s, 5, rec.company);
    BindText(st.s, 6, rec.year);
    BindText(st.s, 7, rec.path);
    BindText(st.s, 8, rec.source);
    BindText(st.s, 9, rec.notes);
    if (sqlite3_step(st.s) != SQLITE_DONE) {
        error = sqlite3_errmsg(db_);
        return false;
    }
    TapeRecord saved;
    if (!FindBySha1(rec.sha1, saved)) {
        error = "fita gravada, mas nao encontrada de volta";
        return false;
    }
    rec = saved;
    return true;
}

bool TapeDb::Update(const TapeRecord &rec, std::string &error) {
    Stmt st;
    if (!Prepare(db_,
                 "UPDATE fitas SET title=?1, company=?2, year=?3, path=?4, source=?5, notes=?6,"
                 " updated=datetime('now') WHERE id=?7",
                 st, error)) {
        return false;
    }
    BindText(st.s, 1, rec.title);
    BindText(st.s, 2, rec.company);
    BindText(st.s, 3, rec.year);
    BindText(st.s, 4, rec.path);
    BindText(st.s, 5, rec.source);
    BindText(st.s, 6, rec.notes);
    sqlite3_bind_int64(st.s, 7, rec.id);
    if (sqlite3_step(st.s) != SQLITE_DONE) {
        error = sqlite3_errmsg(db_);
        return false;
    }
    if (sqlite3_changes(db_) == 0) {
        error = "nenhuma fita com esse id";
        return false;
    }
    return true;
}

bool TapeDb::Delete(int64_t id, std::string &error) {
    Stmt st;
    if (!Prepare(db_, "DELETE FROM fitas WHERE id=?1", st, error)) return false;
    sqlite3_bind_int64(st.s, 1, id);
    if (sqlite3_step(st.s) != SQLITE_DONE) {
        error = sqlite3_errmsg(db_);
        return false;
    }
    if (sqlite3_changes(db_) == 0) {
        error = "nenhuma fita com esse id";
        return false;
    }
    return true;
}

bool TapeDb::Get(int64_t id, TapeRecord &out) const {
    Stmt st;
    std::string error;
    const std::string sql = std::string("SELECT ") + kTapeColumns + " FROM fitas WHERE id=?1";
    if (!Prepare(db_, sql.c_str(), st, error)) return false;
    sqlite3_bind_int64(st.s, 1, id);
    if (sqlite3_step(st.s) != SQLITE_ROW) return false;
    out = RowToTape(st.s);
    return true;
}

bool TapeDb::FindBySha1(const std::string &sha1, TapeRecord &out) const {
    Stmt st;
    std::string error;
    const std::string sql = std::string("SELECT ") + kTapeColumns + " FROM fitas WHERE sha1=?1";
    if (!Prepare(db_, sql.c_str(), st, error)) return false;
    BindText(st.s, 1, Lower(sha1));
    if (sqlite3_step(st.s) != SQLITE_ROW) return false;
    out = RowToTape(st.s);
    return true;
}

std::vector<TapeRecord> TapeDb::Search(const std::string &text) const {
    std::vector<TapeRecord> out;
    Stmt st;
    std::string error;
    const std::string sql = std::string("SELECT ") + kTapeColumns +
                            " FROM fitas WHERE (?1 = '' OR title LIKE ?1 OR company LIKE ?1 OR year LIKE ?1"
                            " OR sha1 LIKE ?1 OR notes LIKE ?1 OR path LIKE ?1)"
                            " ORDER BY title, sha1 LIMIT 5000";
    if (!Prepare(db_, sql.c_str(), st, error)) return out;
    BindText(st.s, 1, text.empty() ? std::string() : "%" + text + "%");
    while (sqlite3_step(st.s) == SQLITE_ROW) out.push_back(RowToTape(st.s));
    return out;
}

int64_t TapeDb::Count() const {
    Stmt st;
    std::string error;
    if (!Prepare(db_, "SELECT COUNT(*) FROM fitas", st, error)) return 0;
    return sqlite3_step(st.s) == SQLITE_ROW ? sqlite3_column_int64(st.s, 0) : 0;
}

bool TapeDb::ScanFile(const std::string &file, const std::string &root, const std::string &source, TapeRecord &out,
                      std::string &error) {
    std::vector<uint8_t> bytes;
    if (!ReadWholeFile(file, bytes)) {
        error = "nao foi possivel ler '" + file + "'";
        return false;
    }
    TapeRecord rec;
    rec.sha1 = romdb::Sha1Hex(bytes.data(), bytes.size());
    rec.size = static_cast<int64_t>(bytes.size());
    rec.format = FormatFor(file);
    rec.path = RelativeTo(file, root);
    rec.source = source;
    rec.title = FirstFileNameIn(file); // so' um palpite -- sobrescrito pelo titulo existente abaixo, se houver
    TapeRecord existing;
    if (FindBySha1(rec.sha1, existing)) {
        rec.title = existing.title;
        rec.company = existing.company;
        rec.year = existing.year;
        rec.notes = existing.notes;
    }
    if (!Add(rec, error)) return false;
    out = rec;
    return true;
}

bool TapeDb::ScanDirectory(const std::string &dir, const std::string &root, const std::string &source,
                           int64_t &added, int64_t &updated, std::string &error) {
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
        const std::string path = it->path().string();
        if (FormatFor(path).empty()) continue; // so' .cas/.tsx/.tzx
        std::vector<uint8_t> bytes;
        if (!ReadWholeFile(path, bytes)) continue;
        const std::string sha = romdb::Sha1Hex(bytes.data(), bytes.size());
        TapeRecord existing;
        const bool known = FindBySha1(sha, existing);
        TapeRecord rec;
        if (!ScanFile(path, root, source, rec, error)) {
            error.clear();
            continue;
        }
        if (known) ++updated;
        else ++added;
    }
    Exec("COMMIT", error);
    return true;
}

} // namespace tapedb
