// fwMSX -- projeto .msxproject (SQLite) e as copias soltas. Ver project.h e doc/project-spec.md.
#include "project.h"

#include <sqlite3.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>
#include <set>

#include "../../romdb/core/hash.h"

extern "C" {
int prj_classify(const char *path, char *kind, int cap);
int prj_path_ok(const char *path, int n);
int prj_strip_cr(uint8_t *dst, const uint8_t *src, int n);
}

namespace fs = std::filesystem;

namespace prj {

namespace {

constexpr int kSchema = 2; // 2: lapides (tombstones) e historico (revisions)
constexpr int64_t kRevMaxBytes = 8 * 1024 * 1024; // acima disso (um .dsk) nao entra no historico: encheria o banco; 8 MiB cobre as maiores ROMs de MSX
// As tabelas do esquema 2: IF NOT EXISTS serve ao Create e a' migracao de um projeto do esquema 1 (que so' ganha as tabelas, nada muda nos dados).
const char *const kTables =
    "CREATE TABLE IF NOT EXISTS tombstones(path TEXT PRIMARY KEY, hash TEXT NOT NULL, removed INTEGER NOT NULL);"
    "CREATE TABLE IF NOT EXISTS revisions(id INTEGER PRIMARY KEY, path TEXT NOT NULL, hash TEXT NOT NULL, content BLOB NOT NULL,"
    " size INTEGER NOT NULL, mtime INTEGER NOT NULL, saved INTEGER NOT NULL);"
    "CREATE INDEX IF NOT EXISTS revisions_path ON revisions(path,id);";
const char *const kManifest = "msxproject.txt"; // gerado na raiz; nao e' um arquivo do projeto
const char *const kSidecar = ".msxsync";         // o hash da ultima sincronizacao de cada arquivo, DESTA pasta (nao viaja no banco)

int64_t NowSec() {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

// O relogio dos arquivos nao e' o do sistema (epocas diferentes no libstdc++): converte pela diferenca dos dois "agora".
int64_t ToUnix(fs::file_time_type t) {
    using namespace std::chrono;
    const auto sys = time_point_cast<system_clock::duration>(t - fs::file_time_type::clock::now() + system_clock::now());
    return duration_cast<seconds>(sys.time_since_epoch()).count();
}

fs::file_time_type FromUnix(int64_t s) {
    using namespace std::chrono;
    const system_clock::time_point sys{duration_cast<system_clock::duration>(seconds(s))};
    return time_point_cast<fs::file_time_type::duration>(sys - system_clock::now() + fs::file_time_type::clock::now());
}

bool ReadFile(const fs::path &p, std::vector<uint8_t> &out) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return false;
    f.seekg(0, std::ios::end);
    const std::streamoff n = f.tellg();
    f.seekg(0, std::ios::beg);
    out.assign(static_cast<size_t>(n < 0 ? 0 : n), 0);
    if (n > 0) f.read(reinterpret_cast<char *>(out.data()), n);
    return static_cast<bool>(f) || n == 0;
}

bool WriteFile(const fs::path &p, const std::vector<uint8_t> &data, std::string &err) {
    std::error_code ec;
    if (p.has_parent_path()) fs::create_directories(p.parent_path(), ec);
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    if (f && !data.empty()) f.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!f) {
        err = "nao foi possivel gravar " + p.string();
        return false;
    }
    return true;
}

std::string RandomId() {
    std::random_device rd;
    std::mt19937_64 g((static_cast<uint64_t>(rd()) << 32) ^ rd());
    char buf[20];
    std::snprintf(buf, sizeof(buf), "%016llx", static_cast<unsigned long long>(g()));
    return buf;
}

// Arquivos da pasta que nao sao do projeto: o manifesto, o .msxsync, o proprio banco e seu journal, e tudo que comeca com ponto (.git...).
bool Ignored(const std::string &rel) {
    if (rel == kManifest) return true;
    const size_t slash = rel.rfind('/');
    const std::string base = slash == std::string::npos ? rel : rel.substr(slash + 1);
    if (!base.empty() && base[0] == '.') return true;
    const auto ends = [&](const char *suf) {
        const size_t n = std::char_traits<char>::length(suf);
        return base.size() >= n && std::equal(base.end() - static_cast<long>(n), base.end(), suf, [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == b; });
    };
    if (ends(".msxproject") || ends(".msxproject-journal") || ends(".msxproject-wal") || ends(".msxproject-shm")) return true;
    // copia de seguranca do editor do fwMSX (Save refaz READ.ME como READ.{E; o caractere de backup pode ser { ~ @)
    const size_t dot = base.rfind('.');
    return dot != std::string::npos && dot + 1 < base.size() && base.size() - dot >= 3 && (base[dot + 1] == '{' || base[dot + 1] == '~' || base[dot + 1] == '@');
}

struct DiskFile {
    int64_t mtime = 0;
};

// Os arquivos de `root` (recursivo, sem as pastas que comecam com ponto): caminho relativo com "/" -> dados.
std::map<std::string, DiskFile> ScanDisk(const fs::path &root) {
    std::map<std::string, DiskFile> out;
    std::error_code ec;
    if (!fs::is_directory(root, ec)) return out;
    fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec), end;
    for (; !ec && it != end; it.increment(ec)) {
        const fs::path &p = it->path();
        std::error_code e2;
        const std::string rel = fs::relative(p, root, e2).generic_string();
        if (e2) continue;
        if (it->is_directory(e2)) {
            if (!p.filename().empty() && p.filename().string()[0] == '.') it.disable_recursion_pending();
            continue;
        }
        if (!it->is_regular_file(e2) || Ignored(rel) || !PathOk(rel)) continue;
        DiskFile d;
        d.mtime = ToUnix(fs::last_write_time(p, e2));
        out[rel] = d;
    }
    return out;
}

// Instrucao SQL preparada, com RAII.
class Stmt {
public:
    Stmt(sqlite3 *db, const char *sql) { ok_ = sqlite3_prepare_v2(db, sql, -1, &s_, nullptr) == SQLITE_OK; }
    ~Stmt() {
        if (s_) sqlite3_finalize(s_);
    }
    Stmt(const Stmt &) = delete;
    Stmt &operator=(const Stmt &) = delete;
    bool ok() const { return ok_; }
    sqlite3_stmt *get() { return s_; }
    void Text(int i, const std::string &v) { sqlite3_bind_text(s_, i, v.c_str(), static_cast<int>(v.size()), SQLITE_TRANSIENT); }
    void Int(int i, int64_t v) { sqlite3_bind_int64(s_, i, v); }
    void Blob(int i, const std::vector<uint8_t> &v) { sqlite3_bind_blob(s_, i, v.empty() ? "" : reinterpret_cast<const char *>(v.data()), static_cast<int>(v.size()), SQLITE_TRANSIENT); }
    bool Step() { return sqlite3_step(s_) == SQLITE_ROW; }
    bool Done() { return sqlite3_step(s_) == SQLITE_DONE; }
    std::string Col(int i) { return reinterpret_cast<const char *>(sqlite3_column_text(s_, i)) ? reinterpret_cast<const char *>(sqlite3_column_text(s_, i)) : ""; }
    int64_t ColInt(int i) { return sqlite3_column_int64(s_, i); }

private:
    sqlite3_stmt *s_ = nullptr;
    bool ok_ = false;
};

} // namespace

// ---------------------------------------------------------------------------------------------------------------
// funcoes soltas
// ---------------------------------------------------------------------------------------------------------------

bool IsIgnored(const std::string &rel) {
    if (Ignored(rel)) return true;
    for (size_t i = 0; i < rel.size(); ++i) { // qualquer pasta que comece com ponto (.git, .vscode)
        if (rel[i] == '.' && (i == 0 || rel[i - 1] == '/')) return true;
    }
    return false;
}

std::string NormalizePath(const std::string &path) {
    std::string p = path;
    std::replace(p.begin(), p.end(), '\\', '/');
    while (p.rfind("./", 0) == 0) p.erase(0, 2);
    return p;
}

bool PathOk(const std::string &path) {
    if (path.find('\\') != std::string::npos) return false; // quem chama normaliza antes
    return prj_path_ok(path.data(), static_cast<int>(path.size())) == 1;
}

std::string KindOf(const std::string &path, bool *text) {
    char kind[32];
    const int t = prj_classify(path.c_str(), kind, sizeof(kind));
    if (text) *text = t != 0;
    return kind;
}

std::string HashBytes(const std::vector<uint8_t> &data, bool text) {
    if (!text || data.empty()) return romdb::Sha1Hex(data.data(), data.size());
    std::vector<uint8_t> t(data.size());
    const int n = prj_strip_cr(t.data(), data.data(), static_cast<int>(data.size()));
    return romdb::Sha1Hex(t.data(), static_cast<size_t>(n));
}

const char *StateName(State s) {
    switch (s) {
    case State::InSync: return "igual";
    case State::DiskChanged: return "pasta mudou";
    case State::ProjectChanged: return "projeto mudou";
    case State::Conflict: return "CONFLITO";
    case State::OnlyDisk: return "so na pasta";
    case State::OnlyProject: return "so no projeto";
    case State::DiskDeleted: return "sumiu da pasta";
    case State::ProjectRemoved: return "tirado do projeto";
    case State::RemovedChanged: return "tirado do projeto, pasta mudou";
    }
    return "?";
}

const char *ActionName(Action a) {
    switch (a) {
    case Action::None: return "-";
    case Action::Push: return "pasta->projeto";
    case Action::Pull: return "projeto->pasta";
    case Action::Drop: return "tirar do projeto";
    case Action::Ask: return "escolher";
    }
    return "?";
}

// ---------------------------------------------------------------------------------------------------------------
// o banco
// ---------------------------------------------------------------------------------------------------------------

Project::Project() = default;
Project::~Project() { Close(); }

void Project::Close() {
    if (db_) sqlite3_close(db_);
    db_ = nullptr;
    path_.clear();
}

bool Project::Exec(const char *sql, std::string &err) {
    char *msg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &msg) != SQLITE_OK) {
        err = msg ? msg : "erro do SQLite";
        sqlite3_free(msg);
        return false;
    }
    return true;
}

bool Project::Create(const std::string &file, const std::string &name, std::string &err) {
    Close();
    std::error_code ec;
    if (fs::exists(file, ec)) {
        err = "ja' existe: " + file;
        return false;
    }
    if (sqlite3_open_v2(file.c_str(), &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
        err = std::string("nao foi possivel criar o projeto: ") + (db_ ? sqlite3_errmsg(db_) : "?");
        Close();
        return false;
    }
    path_ = file;
    // Journal tradicional (nada de -wal/-shm): o .msxproject fechado ou entre gravacoes e' SEMPRE o projeto inteiro, e e' isso que se copia entre PCs.
    if (!Exec("PRAGMA journal_mode=DELETE;PRAGMA synchronous=FULL;", err) ||
        !Exec("CREATE TABLE meta(key TEXT PRIMARY KEY, value TEXT NOT NULL);"
              "CREATE TABLE config(key TEXT PRIMARY KEY, value TEXT NOT NULL);"
              "CREATE TABLE files(id INTEGER PRIMARY KEY, path TEXT UNIQUE NOT NULL, kind TEXT NOT NULL, content BLOB NOT NULL,"
              " size INTEGER NOT NULL, hash TEXT NOT NULL, mtime INTEGER NOT NULL);",
              err) ||
        !Exec(kTables, err)) {
        Close();
        fs::remove(file, ec);
        return false;
    }
    const std::string nm = name.empty() ? fs::path(file).stem().string() : name;
    if (!SetMeta("schema", std::to_string(kSchema), err) || !SetMeta("name", nm, err) || !SetMeta("id", RandomId(), err) ||
        !SetMeta("created", std::to_string(NowSec()), err)) {
        Close();
        fs::remove(file, ec);
        return false;
    }
    return true;
}

bool Project::Open(const std::string &file, std::string &err) {
    Close();
    std::error_code ec;
    if (!fs::is_regular_file(file, ec)) {
        err = "nao existe: " + file;
        return false;
    }
    if (sqlite3_open_v2(file.c_str(), &db_, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK) {
        err = std::string("nao foi possivel abrir o projeto: ") + (db_ ? sqlite3_errmsg(db_) : "?");
        Close();
        return false;
    }
    path_ = file;
    {
        Stmt q(db_, "PRAGMA quick_check;");
        if (!q.ok() || !q.Step() || q.Col(0) != "ok") {
            err = "o arquivo nao e' um projeto valido (ou esta' corrompido): " + file;
            Close();
            return false;
        }
    }
    std::string schema;
    if (!Meta("schema", schema)) {
        err = "o arquivo nao e' um .msxproject do fwMSX: " + file;
        Close();
        return false;
    }
    if (std::atoi(schema.c_str()) > kSchema) {
        err = "projeto de uma versao mais nova do fwMSX (esquema " + schema + ")";
        Close();
        return false;
    }
    Exec("PRAGMA journal_mode=DELETE;PRAGMA synchronous=FULL;", err);
    err.clear();
    if (std::atoi(schema.c_str()) < kSchema) { // projeto antigo: ganha as tabelas novas e passa a ser do esquema atual
        if (!Exec(kTables, err) || !SetMeta("schema", std::to_string(kSchema), err)) {
            Close();
            return false;
        }
    }
    return true;
}

std::string Project::DefaultRoot() const {
    const fs::path p = fs::absolute(path_);
    return p.has_parent_path() ? p.parent_path().string() : std::string(".");
}

bool Project::Meta(const std::string &key, std::string &value) const {
    Stmt q(db_, "SELECT value FROM meta WHERE key=?1");
    if (!q.ok()) return false;
    q.Text(1, key);
    if (!q.Step()) return false;
    value = q.Col(0);
    return true;
}

bool Project::SetMeta(const std::string &key, const std::string &value, std::string &err) {
    Stmt q(db_, "INSERT INTO meta(key,value) VALUES(?1,?2) ON CONFLICT(key) DO UPDATE SET value=?2");
    if (!q.ok()) return err = sqlite3_errmsg(db_), false;
    q.Text(1, key);
    q.Text(2, value);
    if (!q.Done()) return err = sqlite3_errmsg(db_), false;
    return true;
}

std::string Project::Name() const {
    std::string v;
    return Meta("name", v) ? v : std::string();
}

std::string Project::Id() const {
    std::string v;
    return Meta("id", v) ? v : std::string();
}

std::string Project::GetConfig(const std::string &key, const std::string &def) const {
    Stmt q(db_, "SELECT value FROM config WHERE key=?1");
    if (!q.ok()) return def;
    q.Text(1, key);
    return q.Step() ? q.Col(0) : def;
}

bool Project::SetConfig(const std::string &key, const std::string &value, std::string &err) {
    if (key.empty() || key.find_first_of("=\r\n\t[]") != std::string::npos) return err = "chave de configuracao invalida: " + key, false;
    if (value.find_first_of("\r\n") != std::string::npos) return err = "o valor da configuracao nao pode ter quebra de linha", false;
    Stmt q(db_, "INSERT INTO config(key,value) VALUES(?1,?2) ON CONFLICT(key) DO UPDATE SET value=?2");
    if (!q.ok()) return err = sqlite3_errmsg(db_), false;
    q.Text(1, key);
    q.Text(2, value);
    if (!q.Done()) return err = sqlite3_errmsg(db_), false;
    return true;
}

bool Project::DelConfig(const std::string &key) {
    Stmt q(db_, "DELETE FROM config WHERE key=?1");
    if (!q.ok()) return false;
    q.Text(1, key);
    return q.Done() && sqlite3_changes(db_) > 0;
}

std::vector<std::pair<std::string, std::string>> Project::Config() const {
    std::vector<std::pair<std::string, std::string>> v;
    Stmt q(db_, "SELECT key,value FROM config ORDER BY key");
    while (q.ok() && q.Step()) v.emplace_back(q.Col(0), q.Col(1));
    return v;
}

// ---------------------------------------------------------------------------------------------------------------
// arquivos dentro do projeto
// ---------------------------------------------------------------------------------------------------------------

bool Project::PutRaw(const std::string &path, const std::vector<uint8_t> &data, const std::string &kind, int64_t mtime, std::string &err) {
    bool text = false;
    const std::string by_ext = KindOf(path, &text);
    const std::string k = kind.empty() ? by_ext : kind;
    {
        // dois nomes que so' diferem em maiusculas viram um so' no Windows (e no macOS): recusa em vez de um sobrescrever o outro na pasta
        Stmt c(db_, "SELECT path FROM files WHERE lower(path)=lower(?1) AND path<>?1");
        if (c.ok()) {
            c.Text(1, path);
            if (c.Step()) return err = "o nome colide com '" + c.Col(0) + "' (so' difere em maiusculas): " + path, false;
        }
    }
    const std::string new_hash = HashBytes(data, text);
    {
        Stmt h(db_, "SELECT hash FROM files WHERE path=?1");
        if (h.ok()) {
            h.Text(1, path);
            if (h.Step() && h.Col(0) != new_hash) Archive(path); // sobrescrita com conteudo diferente: a versao de agora vai para o historico
        }
    }
    Stmt q(db_,
           "INSERT INTO files(path,kind,content,size,hash,mtime) VALUES(?1,?2,?3,?4,?5,?6) "
           "ON CONFLICT(path) DO UPDATE SET kind=?2,content=?3,size=?4,hash=?5,mtime=?6");
    if (!q.ok()) return err = sqlite3_errmsg(db_), false;
    q.Text(1, path);
    q.Text(2, k);
    q.Blob(3, data);
    q.Int(4, static_cast<int64_t>(data.size()));
    q.Text(5, new_hash);
    q.Int(6, mtime < 0 ? NowSec() : mtime);
    if (!q.Done()) return err = sqlite3_errmsg(db_), false;
    Stmt t(db_, "DELETE FROM tombstones WHERE path=?1"); // voltou ao projeto: a lapide nao vale mais
    if (t.ok()) {
        t.Text(1, path);
        t.Done();
    }
    return true;
}

bool Project::Put(const std::string &path_in, const std::vector<uint8_t> &data, std::string &err, const std::string &kind, int64_t mtime) {
    const std::string path = NormalizePath(path_in);
    if (!PathOk(path)) return err = "caminho invalido dentro do projeto: " + path_in, false;
    std::string k = kind;
    if (k.empty()) { // arquivo que ja' existe mantem o tipo que tinha
        Stmt q(db_, "SELECT kind FROM files WHERE path=?1");
        if (q.ok()) {
            q.Text(1, path);
            if (q.Step()) k = q.Col(0);
        }
    }
    return PutRaw(path, data, k, mtime, err);
}

bool Project::Get(const std::string &path_in, std::vector<uint8_t> &data) const {
    Stmt q(db_, "SELECT content FROM files WHERE path=?1");
    if (!q.ok()) return false;
    q.Text(1, NormalizePath(path_in));
    if (!q.Step()) return false;
    const int n = sqlite3_column_bytes(q.get(), 0);
    const uint8_t *p = static_cast<const uint8_t *>(sqlite3_column_blob(q.get(), 0));
    data.assign(p, p + (p ? n : 0));
    return true;
}

bool Project::Has(const std::string &path_in) const {
    Stmt q(db_, "SELECT 1 FROM files WHERE path=?1");
    if (!q.ok()) return false;
    q.Text(1, NormalizePath(path_in));
    return q.Step();
}

bool Project::Remove(const std::string &path_in, std::string &err) {
    const std::string path = NormalizePath(path_in);
    std::string hash;
    {
        Stmt h(db_, "SELECT hash FROM files WHERE path=?1");
        if (!h.ok()) return err = sqlite3_errmsg(db_), false;
        h.Text(1, path);
        if (!h.Step()) return err = "nao esta' no projeto: " + path_in, false;
        hash = h.Col(0);
    }
    Archive(path); // tirar nao perde o conteudo: fica no historico
    {
        // a lapide leva a remocao para os outros PCs: la' a copia da pasta, se ainda for igual a este hash, sera' oferecida para apagar
        Stmt t(db_, "INSERT OR REPLACE INTO tombstones(path,hash,removed) VALUES(?1,?2,?3)");
        if (!t.ok()) return err = sqlite3_errmsg(db_), false;
        t.Text(1, path);
        t.Text(2, hash);
        t.Int(3, NowSec());
        if (!t.Done()) return err = sqlite3_errmsg(db_), false;
    }
    Stmt q(db_, "DELETE FROM files WHERE path=?1");
    if (!q.ok()) return err = sqlite3_errmsg(db_), false;
    q.Text(1, path);
    if (!q.Done()) return err = sqlite3_errmsg(db_), false;
    return true;
}

// O historico guarda as ultimas `revisions.keep` versoes de cada arquivo (padrao 10; 0 desliga). So' guarda se o conteudo mudou em relacao a' ultima.
bool Project::Archive(const std::string &path) {
    const int keep = std::atoi(GetConfig("revisions.keep", "10").c_str());
    if (keep <= 0) return true;
    std::string hash;
    {
        Stmt q(db_, "SELECT hash,size FROM files WHERE path=?1");
        if (!q.ok()) return false;
        q.Text(1, path);
        if (!q.Step() || q.ColInt(1) > kRevMaxBytes) return true;
        hash = q.Col(0);
    }
    {
        Stmt l(db_, "SELECT hash FROM revisions WHERE path=?1 ORDER BY id DESC LIMIT 1");
        if (l.ok()) {
            l.Text(1, path);
            if (l.Step() && l.Col(0) == hash) return true;
        }
    }
    {
        Stmt ins(db_, "INSERT INTO revisions(path,hash,content,size,mtime,saved) SELECT path,hash,content,size,mtime,?2 FROM files WHERE path=?1");
        if (!ins.ok()) return false;
        ins.Text(1, path);
        ins.Int(2, NowSec());
        if (!ins.Done()) return false;
    }
    Stmt pr(db_, "DELETE FROM revisions WHERE path=?1 AND id NOT IN (SELECT id FROM revisions WHERE path=?1 ORDER BY id DESC LIMIT ?2)");
    if (!pr.ok()) return false;
    pr.Text(1, path);
    pr.Int(2, keep);
    return pr.Done();
}

std::vector<Revision> Project::Revisions(const std::string &path_in) const {
    std::vector<Revision> v;
    Stmt q(db_, "SELECT id,hash,size,mtime,saved FROM revisions WHERE path=?1 ORDER BY id DESC");
    if (!q.ok()) return v;
    q.Text(1, NormalizePath(path_in));
    while (q.Step()) {
        Revision r;
        r.id = q.ColInt(0);
        r.hash = q.Col(1);
        r.size = q.ColInt(2);
        r.mtime = q.ColInt(3);
        r.saved = q.ColInt(4);
        v.push_back(r);
    }
    return v;
}

bool Project::GetRevision(int64_t id, std::vector<uint8_t> &data) const {
    Stmt q(db_, "SELECT content FROM revisions WHERE id=?1");
    if (!q.ok()) return false;
    q.Int(1, id);
    if (!q.Step()) return false;
    const int n = sqlite3_column_bytes(q.get(), 0);
    const uint8_t *p = static_cast<const uint8_t *>(sqlite3_column_blob(q.get(), 0));
    data.assign(p, p + (p ? n : 0));
    return true;
}

bool Project::Restore(const std::string &path_in, int64_t id, const std::string &root, std::string &err) {
    const std::string path = NormalizePath(path_in);
    if (!PathOk(path)) return err = "caminho invalido dentro do projeto: " + path_in, false;
    {
        Stmt q(db_, "SELECT path FROM revisions WHERE id=?1");
        if (!q.ok()) return err = sqlite3_errmsg(db_), false;
        q.Int(1, id);
        if (!q.Step() || q.Col(0) != path) return err = "essa versao nao e' de " + path_in, false;
    }
    std::vector<uint8_t> data;
    if (!GetRevision(id, data)) return err = "versao nao encontrada", false;
    if (!Put(path, data, err)) return false; // a versao de agora (se for diferente) entra no historico antes de ser trocada
    return Pull(path, root, err);
}

std::vector<std::string> Project::Removed() const {
    std::vector<std::string> v;
    Stmt q(db_, "SELECT path FROM tombstones ORDER BY path");
    while (q.ok() && q.Step()) v.push_back(q.Col(0));
    return v;
}

bool Project::Tombs(std::map<std::string, std::string> &out) const {
    Stmt q(db_, "SELECT path,hash FROM tombstones");
    if (!q.ok()) return false;
    while (q.Step()) out[q.Col(0)] = q.Col(1);
    return true;
}

bool Project::DeleteLoose(const std::string &path_in, const std::string &root, std::string &err) {
    const std::string path = NormalizePath(path_in);
    if (!PathOk(path)) return err = "caminho invalido: " + path_in, false;
    if (Has(path)) return err = path + " ainda esta' no projeto", false;
    auto base = LoadBase(root);
    std::error_code ec;
    fs::remove(fs::path(root) / path, ec);
    if (ec) return err = "nao foi possivel apagar " + path + ": " + ec.message(), false;
    base.erase(path);
    return SaveBase(root, base, err);
}

std::vector<FileInfo> Project::List(const std::string &prefix) const {
    std::vector<FileInfo> v;
    Stmt q(db_, "SELECT path,kind,hash,size,mtime FROM files ORDER BY path");
    const std::string pre = NormalizePath(prefix);
    while (q.ok() && q.Step()) {
        FileInfo f;
        f.path = q.Col(0);
        if (!pre.empty() && f.path.rfind(pre, 0) != 0) continue;
        f.kind = q.Col(1);
        f.hash = q.Col(2);
        f.size = q.ColInt(3);
        f.mtime = q.ColInt(4);
        v.push_back(f);
    }
    return v;
}

bool Project::Rows(std::map<std::string, Row> &out) const {
    Stmt q(db_, "SELECT path,kind,hash,mtime FROM files");
    if (!q.ok()) return false;
    while (q.Step()) {
        Row r;
        r.kind = q.Col(1);
        r.hash = q.Col(2);
        r.mtime = q.ColInt(3);
        out[q.Col(0)] = r;
    }
    return true;
}

// ---------------------------------------------------------------------------------------------------------------
// o .msxsync: o hash da ultima sincronizacao de cada arquivo, nesta pasta
// ---------------------------------------------------------------------------------------------------------------

std::map<std::string, std::string> Project::LoadBase(const std::string &root) const {
    std::map<std::string, std::string> base;
    std::ifstream f(fs::path(root) / kSidecar, std::ios::binary);
    if (!f) return base;
    std::string line, id;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.rfind("project=", 0) == 0) {
            id = line.substr(8);
            continue;
        }
        const size_t tab = line.find('\t');
        if (line.empty() || line[0] == '#' || tab == std::string::npos) continue;
        base[line.substr(tab + 1)] = line.substr(0, tab);
    }
    if (id != Id()) base.clear(); // a pasta foi sincronizada com OUTRO projeto: sem base, tudo que difere vira conflito (nada e' apagado)
    return base;
}

bool Project::SaveBase(const std::string &root, const std::map<std::string, std::string> &base, std::string &err) const {
    std::string s = "# fwMSX: hash da ultima sincronizacao de cada arquivo desta pasta (nao edite; nao vai para o git)\nproject=" + Id() + "\n";
    for (const auto &kv : base) s += kv.second + "\t" + kv.first + "\n";
    return WriteFile(fs::path(root) / kSidecar, std::vector<uint8_t>(s.begin(), s.end()), err);
}

// ---------------------------------------------------------------------------------------------------------------
// sincronizacao
// ---------------------------------------------------------------------------------------------------------------

bool Project::StatusImpl(const std::string &root, const std::map<std::string, std::string> &base, std::vector<SyncItem> &out, std::string &err) {
    std::map<std::string, Row> rows;
    if (!Rows(rows)) return err = sqlite3_errmsg(db_), false;
    std::map<std::string, std::string> tombs;
    Tombs(tombs);
    const std::map<std::string, DiskFile> disk = ScanDisk(root);
    std::set<std::string> all;
    for (const auto &kv : rows) all.insert(kv.first);
    for (const auto &kv : disk) all.insert(kv.first);
    for (const std::string &p : all) {
        SyncItem it;
        it.path = p;
        const auto r = rows.find(p);
        const auto d = disk.find(p);
        const auto b = base.find(p);
        const std::string bh = b == base.end() ? std::string() : b->second;
        if (r != rows.end()) it.db_mtime = r->second.mtime;
        if (d != disk.end()) it.disk_mtime = d->second.mtime;
        if (r != rows.end() && d != disk.end()) {
            std::vector<uint8_t> bytes;
            bool text = false;
            KindOf(p, &text);
            if (!ReadFile(fs::path(root) / p, bytes)) {
                err = "nao foi possivel ler " + p;
                return false;
            }
            const std::string dh = HashBytes(bytes, text);
            if (dh == r->second.hash) {
                it.state = State::InSync;
            } else if (bh == r->second.hash) {
                it.state = State::DiskChanged;
                it.suggest = Action::Push;
            } else if (bh == dh) {
                it.state = State::ProjectChanged;
                it.suggest = Action::Pull;
            } else {
                it.state = State::Conflict;
                it.suggest = Action::Ask;
                it.newer = it.disk_mtime > it.db_mtime ? Action::Push : Action::Pull;
            }
        } else if (r != rows.end()) {
            if (bh.empty()) {
                it.state = State::OnlyProject;
                it.suggest = Action::Pull;
            } else if (bh == r->second.hash) {
                it.state = State::DiskDeleted;
                it.suggest = Action::Drop;
            } else { // apagado na pasta E alterado no projeto
                it.state = State::Conflict;
                it.suggest = Action::Ask;
                it.newer = Action::Pull;
            }
        } else if (tombs.count(p)) {
            // tirado do projeto (aqui ou em outro PC) e a pasta ainda tem a copia: apagar e' decisao do usuario (Ask), nunca automatica
            std::vector<uint8_t> bytes;
            bool text = false;
            KindOf(p, &text);
            if (!ReadFile(fs::path(root) / p, bytes)) {
                err = "nao foi possivel ler " + p;
                return false;
            }
            const std::string dh = HashBytes(bytes, text);
            if (bh == "!" + tombs[p] && dh == tombs[p]) continue; // esta pasta ja' decidiu ficar com a copia solta fora do projeto
            it.state = dh == tombs[p] ? State::ProjectRemoved : State::RemovedChanged;
            it.suggest = Action::Ask;
        } else {
            it.state = State::OnlyDisk; // nunca se apaga uma copia solta: o que so' existe na pasta volta para o projeto
            it.suggest = Action::Push;
        }
        out.push_back(it);
    }
    return true;
}

bool Project::Status(const std::string &root, std::vector<SyncItem> &out, std::string &err) {
    return StatusImpl(root, LoadBase(root), out, err);
}

bool Project::PullOne(const std::string &path, const std::string &root, std::map<std::string, std::string> &base, std::string &err) {
    Stmt q(db_, "SELECT content,hash,mtime FROM files WHERE path=?1");
    if (!q.ok()) return err = sqlite3_errmsg(db_), false;
    q.Text(1, path);
    if (!q.Step()) return err = "nao esta' no projeto: " + path, false;
    if (!PathOk(path)) return err = "caminho invalido dentro do projeto: " + path, false;
    const int n = sqlite3_column_bytes(q.get(), 0);
    const uint8_t *p = static_cast<const uint8_t *>(sqlite3_column_blob(q.get(), 0));
    const std::vector<uint8_t> data(p, p + (p ? n : 0));
    const fs::path target = fs::path(root) / path;
    if (!WriteFile(target, data, err)) return false;
    std::error_code ec;
    fs::last_write_time(target, FromUnix(q.ColInt(2)), ec); // a copia solta ganha a data do projeto: as duas "idades" passam a coincidir
    base[path] = q.Col(1);
    return true;
}

bool Project::PushOne(const std::string &path, const std::string &root, std::map<std::string, std::string> &base, std::string &err) {
    if (!PathOk(path)) return err = "caminho invalido: " + path, false;
    const fs::path src = fs::path(root) / path;
    std::vector<uint8_t> bytes;
    if (!ReadFile(src, bytes)) return err = "nao foi possivel ler " + src.string(), false;
    std::error_code ec;
    const int64_t mt = ToUnix(fs::last_write_time(src, ec));
    if (!Put(path, bytes, err, "", mt)) return false;
    bool text = false;
    KindOf(path, &text);
    base[path] = HashBytes(bytes, text);
    return true;
}

bool Project::Push(const std::string &path, const std::string &root, std::string &err) {
    auto base = LoadBase(root);
    const std::string p = NormalizePath(path);
    return PushOne(p, root, base, err) && SaveBase(root, base, err);
}

bool Project::Pull(const std::string &path, const std::string &root, std::string &err) {
    auto base = LoadBase(root);
    const std::string p = NormalizePath(path);
    return PullOne(p, root, base, err) && SaveBase(root, base, err);
}

bool Project::Drop(const std::string &path, const std::string &root, std::string &err) {
    auto base = LoadBase(root);
    const std::string p = NormalizePath(path);
    if (!Remove(p, err)) return false;
    // A copia solta fica (Drop nunca a apaga). Para esta pasta nao reclamar dela a cada Sync ("tirado do projeto, apagar?"), a base guarda "!" + o
    // hash tirado: enquanto a copia continuar igual a ele, o Status a ignora. Outros PCs nao tem esse "!" e continuam sendo avisados.
    std::map<std::string, std::string> t;
    Tombs(t);
    std::error_code ec;
    const auto it = t.find(p);
    if (it != t.end() && fs::exists(fs::path(root) / p, ec)) base[p] = "!" + it->second;
    else base.erase(p);
    return SaveBase(root, base, err);
}

bool Project::Save(const std::string &path_in, const std::vector<uint8_t> &data, const std::string &root, std::string &err) {
    const std::string path = NormalizePath(path_in);
    if (!PathOk(path)) return err = "caminho invalido dentro do projeto: " + path_in, false;
    const int64_t now = NowSec();
    if (!Put(path, data, err, "", now)) return false;
    auto base = LoadBase(root);
    if (!PullOne(path, root, base, err)) return false; // grava a copia solta com a data do projeto e marca os dois como iguais
    return SaveBase(root, base, err);
}

bool Project::Sync(const std::string &root, bool prune, SyncSummary &s, std::string &err) {
    s = SyncSummary();
    std::error_code ec;
    fs::create_directories(root, ec);
    auto base = LoadBase(root);
    AdoptManifestConfig(root);
    std::vector<SyncItem> items;
    if (!StatusImpl(root, base, items, err)) return false;
    if (!Exec("BEGIN;", err)) return false;
    for (const SyncItem &it : items) {
        std::string e;
        switch (it.state) {
        case State::InSync: {
            ++s.same;
            Stmt q(db_, "SELECT hash FROM files WHERE path=?1");
            if (q.ok()) {
                q.Text(1, it.path);
                if (q.Step()) base[it.path] = q.Col(0);
            }
            break;
        }
        case State::DiskChanged:
        case State::OnlyDisk:
            if (PushOne(it.path, root, base, e)) ++s.pushed;
            else s.errors.push_back(it.path + ": " + e);
            break;
        case State::ProjectChanged:
        case State::OnlyProject:
            if (PullOne(it.path, root, base, e)) ++s.pulled;
            else s.errors.push_back(it.path + ": " + e);
            break;
        case State::DiskDeleted:
            if (!prune) {
                s.deleted.push_back(it);
            } else if (Remove(it.path, e)) {
                base.erase(it.path);
                ++s.dropped;
            } else {
                s.errors.push_back(it.path + ": " + e);
            }
            break;
        case State::Conflict:
            s.conflicts.push_back(it);
            break;
        case State::ProjectRemoved:
            if (prune) { // a copia e' IGUAL ao que foi tirado e continua no historico: pode ir
                std::error_code rec;
                fs::remove(fs::path(root) / it.path, rec);
                if (rec) {
                    s.errors.push_back(it.path + ": " + rec.message());
                } else {
                    base.erase(it.path);
                    ++s.deleted_copies;
                }
            } else {
                s.removed.push_back(it);
            }
            break;
        case State::RemovedChanged:
            s.removed.push_back(it); // mudou depois de tirado: nunca apaga sozinho
            break;
        }
    }
    std::string e2;
    if (!Exec("COMMIT;", e2)) {
        Exec("ROLLBACK;", e2);
        return err = "COMMIT falhou: " + e2, false;
    }
    if (!SaveBase(root, base, err)) return false;
    return WriteManifest(root, err);
}

bool Project::Export(const std::string &root, std::string &err) {
    std::error_code ec;
    fs::create_directories(root, ec);
    auto base = LoadBase(root);
    std::map<std::string, Row> rows;
    if (!Rows(rows)) return err = sqlite3_errmsg(db_), false;
    for (const auto &kv : rows) {
        if (!PullOne(kv.first, root, base, err)) return false;
    }
    return SaveBase(root, base, err) && WriteManifest(root, err);
}

// O manifesto (msxproject.txt) vai para o git: quem clona e sincroniza traz as chaves de [config] que o banco ainda nao conhece. As que o banco ja' tem
// NAO sao trocadas (o banco e' a fonte; o manifesto e' uma copia legivel). Manifesto de outro projeto (id diferente) e' ignorado.
void Project::AdoptManifestConfig(const std::string &root) {
    std::ifstream f(fs::path(root) / kManifest, std::ios::binary);
    if (!f) return;
    std::string line, section, id;
    std::vector<std::pair<std::string, std::string>> cfg;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        if (line[0] == '[') {
            section = line;
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        if (section.empty() && line.compare(0, eq, "id") == 0) id = line.substr(eq + 1);
        else if (section == "[config]") cfg.emplace_back(line.substr(0, eq), line.substr(eq + 1));
    }
    if (id != Id()) return;
    std::string e;
    for (const auto &kv : cfg) {
        if (!kv.first.empty() && GetConfig(kv.first, "\x01") == "\x01") SetConfig(kv.first, kv.second, e);
    }
}

bool Project::WriteManifest(const std::string &root, std::string &err) const {
    std::string s = "# fwMSX: manifesto do projeto (gerado; os arquivos do projeto sao os desta pasta). Para recriar o .msxproject depois de um clone:\n"
                    "#   project import <esta pasta> <novo.msxproject>\n";
    s += "project=" + Name() + "\nid=" + Id() + "\nschema=" + std::to_string(kSchema) + "\n[config]\n";
    for (const auto &kv : Config()) s += kv.first + "=" + kv.second + "\n";
    s += "[files]\n";
    for (const FileInfo &f : List()) s += f.kind + "\t" + f.path + "\n";
    return WriteFile(fs::path(root) / kManifest, std::vector<uint8_t>(s.begin(), s.end()), err);
}

bool Project::ImportFolder(const std::string &folder, std::string &err) {
    std::error_code ec;
    if (!fs::is_directory(folder, ec)) return err = "nao e' uma pasta: " + folder, false;
    std::map<std::string, std::string> kinds;
    {
        std::ifstream f(fs::path(folder) / kManifest, std::ios::binary);
        std::string line, section;
        while (f && std::getline(f, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || line[0] == '#') continue;
            if (line[0] == '[') {
                section = line;
                continue;
            }
            if (section.empty()) {
                const size_t eq = line.find('=');
                if (eq == std::string::npos) continue;
                const std::string k = line.substr(0, eq), v = line.substr(eq + 1);
                if (k == "project" && !v.empty() && !SetMeta("name", v, err)) return false;
                if (k == "id" && !v.empty() && !SetMeta("id", v, err)) return false; // a pasta e o projeto novo continuam "a mesma coisa"
            } else if (section == "[config]") {
                const size_t eq = line.find('=');
                if (eq != std::string::npos && !SetConfig(line.substr(0, eq), line.substr(eq + 1), err)) return false;
            } else if (section == "[files]") {
                const size_t tab = line.find('\t');
                if (tab != std::string::npos) kinds[line.substr(tab + 1)] = line.substr(0, tab);
            }
        }
    }
    std::map<std::string, std::string> base;
    if (!Exec("BEGIN;", err)) return false;
    for (const auto &kv : ScanDisk(folder)) {
        std::vector<uint8_t> bytes;
        if (!ReadFile(fs::path(folder) / kv.first, bytes)) {
            Exec("ROLLBACK;", err);
            return err = "nao foi possivel ler " + kv.first, false;
        }
        const auto k = kinds.find(kv.first);
        if (!PutRaw(kv.first, bytes, k == kinds.end() ? KindOf(kv.first) : k->second, kv.second.mtime, err)) {
            Exec("ROLLBACK;", err);
            return false;
        }
        bool text = false;
        KindOf(kv.first, &text);
        base[kv.first] = HashBytes(bytes, text);
    }
    std::string e2;
    if (!Exec("COMMIT;", e2)) return err = "COMMIT falhou: " + e2, false;
    return SaveBase(folder, base, err);
}

} // namespace prj
