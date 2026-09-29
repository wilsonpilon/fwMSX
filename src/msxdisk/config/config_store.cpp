//
// msxdisk (fwMSX): armazenamento de configuracao/temas em SQLite -- ver
// config_store.h.
//

#include "config_store.h"

#include <cstdio>
#include <cstdlib>

#include <sqlite3.h>

namespace msxdisk::config {

namespace {

std::string ColorToHex(const RgbColor &c) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", c.r, c.g, c.b);
    return buf;
}

RgbColor HexToColor(const std::string &hex) {
    RgbColor c;
    if (hex.size() == 7 && hex[0] == '#') {
        c.r = static_cast<uint8_t>(std::stoi(hex.substr(1, 2), nullptr, 16));
        c.g = static_cast<uint8_t>(std::stoi(hex.substr(3, 2), nullptr, 16));
        c.b = static_cast<uint8_t>(std::stoi(hex.substr(5, 2), nullptr, 16));
    }
    return c;
}

} // namespace

std::filesystem::path ConfigStore::DefaultPath() {
    const char *home =
#if defined(_WIN32)
        std::getenv("USERPROFILE");
#else
        std::getenv("HOME");
#endif
    const std::filesystem::path base =
        (home != nullptr && *home != '\0') ? std::filesystem::path(home) : std::filesystem::current_path();
    return base / ".msxdisk" / "config.sqlite3";
}

ConfigStore::ConfigStore(sqlite3 *db) : db_(db) {}

ConfigStore::~ConfigStore() {
    if (db_ != nullptr) sqlite3_close(db_);
}

ConfigStore::ConfigStore(ConfigStore &&other) noexcept : db_(other.db_) { other.db_ = nullptr; }

ConfigStore &ConfigStore::operator=(ConfigStore &&other) noexcept {
    if (this != &other) {
        if (db_ != nullptr) sqlite3_close(db_);
        db_ = other.db_;
        other.db_ = nullptr;
    }
    return *this;
}

std::optional<ConfigStore> ConfigStore::Open(const std::filesystem::path &db_path) {
    std::error_code ec;
    std::filesystem::create_directories(db_path.parent_path(), ec);

    sqlite3 *db = nullptr;
    if (sqlite3_open(db_path.string().c_str(), &db) != SQLITE_OK) {
        if (db != nullptr) sqlite3_close(db);
        return std::nullopt;
    }

    ConfigStore store(db);
    store.EnsureSchema();
    store.SeedBuiltinThemesIfEmpty();
    return store;
}

void ConfigStore::EnsureSchema() {
    static const char *kSchema = R"SQL(
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL,
            description TEXT
        );
        CREATE TABLE IF NOT EXISTS themes (
            name TEXT PRIMARY KEY,
            description TEXT,
            fg_normal TEXT, bg_normal TEXT,
            fg_selected TEXT, bg_selected TEXT,
            fg_marked TEXT, bg_marked TEXT,
            fg_border TEXT,
            fg_titlebar TEXT, bg_titlebar TEXT,
            fg_statusbar TEXT, bg_statusbar TEXT
        );
        CREATE TABLE IF NOT EXISTS recent_images (
            path TEXT PRIMARY KEY,
            last_opened_at TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS image_metadata (
            path TEXT PRIMARY KEY,
            description TEXT NOT NULL DEFAULT '',
            notes TEXT NOT NULL DEFAULT '',
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL
        );
    )SQL";
    sqlite3_exec(db_, kSchema, nullptr, nullptr, nullptr);
}

void ConfigStore::SeedBuiltinThemesIfEmpty() {
    if (!ThemeNames().empty()) return;

    SaveTheme(ClassicTheme());
    SaveTheme(DarkTheme());
    if (!GetSetting("active_theme")) {
        SetSetting("active_theme", "classic", "Tema ativo da TUI (nome de uma linha da tabela themes)");
    }
}

std::optional<std::string> ConfigStore::GetSetting(const std::string &key) const {
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, "SELECT value FROM settings WHERE key = ?;", -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<std::string> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        result = text != nullptr ? std::string(reinterpret_cast<const char *>(text)) : std::string();
    }
    sqlite3_finalize(stmt);
    return result;
}

void ConfigStore::SetSetting(const std::string &key, const std::string &value, const std::string &description) {
    sqlite3_stmt *stmt = nullptr;
    const char *sql = "INSERT INTO settings(key, value, description) VALUES(?, ?, ?) "
                       "ON CONFLICT(key) DO UPDATE SET value = excluded.value, "
                       "description = CASE WHEN excluded.description <> '' THEN excluded.description "
                       "ELSE settings.description END;";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void ConfigStore::AddRecentImage(const std::string &path) {
    sqlite3_stmt *stmt = nullptr;
    const char *sql = "INSERT INTO recent_images(path, last_opened_at) VALUES(?, datetime('now')) "
                       "ON CONFLICT(path) DO UPDATE SET last_opened_at = excluded.last_opened_at;";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<std::string> ConfigStore::RecentImages(int limit) const {
    std::vector<std::string> result;
    sqlite3_stmt *stmt = nullptr;
    const char *sql = "SELECT path FROM recent_images ORDER BY last_opened_at DESC LIMIT ?;";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return result;

    sqlite3_bind_int(stmt, 1, limit);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        if (text != nullptr) result.emplace_back(reinterpret_cast<const char *>(text));
    }
    sqlite3_finalize(stmt);
    return result;
}

void ConfigStore::RecordImageSeen(const std::string &path) {
    sqlite3_stmt *stmt = nullptr;
    const char *sql = "INSERT INTO image_metadata(path, created_at, updated_at) "
                       "VALUES(?, datetime('now'), datetime('now')) "
                       "ON CONFLICT(path) DO NOTHING;";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void ConfigStore::SetImageMetadata(const std::string &path, const std::string &description,
                                    const std::string &notes) {
    sqlite3_stmt *stmt = nullptr;
    const char *sql = "INSERT INTO image_metadata(path, description, notes, created_at, updated_at) "
                       "VALUES(?, ?, ?, datetime('now'), datetime('now')) "
                       "ON CONFLICT(path) DO UPDATE SET description = excluded.description, "
                       "notes = excluded.notes, updated_at = excluded.updated_at;";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, notes.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::optional<ConfigStore::ImageMetadata> ConfigStore::GetImageMetadata(const std::string &path) const {
    sqlite3_stmt *stmt = nullptr;
    const char *sql = "SELECT description, notes, created_at, updated_at FROM image_metadata WHERE path = ?;";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<ImageMetadata> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const auto col_str = [&](int idx) {
            const unsigned char *text = sqlite3_column_text(stmt, idx);
            return text != nullptr ? std::string(reinterpret_cast<const char *>(text)) : std::string();
        };
        ImageMetadata meta;
        meta.path = path;
        meta.description = col_str(0);
        meta.notes = col_str(1);
        meta.created_at = col_str(2);
        meta.updated_at = col_str(3);
        result = meta;
    }
    sqlite3_finalize(stmt);
    return result;
}

bool ConfigStore::SaveTheme(const Theme &theme) {
    sqlite3_stmt *stmt = nullptr;
    const char *sql =
        "INSERT INTO themes(name, description, fg_normal, bg_normal, fg_selected, bg_selected, "
        "fg_marked, bg_marked, fg_border, fg_titlebar, bg_titlebar, fg_statusbar, bg_statusbar) "
        "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?) "
        "ON CONFLICT(name) DO UPDATE SET description=excluded.description, fg_normal=excluded.fg_normal, "
        "bg_normal=excluded.bg_normal, fg_selected=excluded.fg_selected, bg_selected=excluded.bg_selected, "
        "fg_marked=excluded.fg_marked, bg_marked=excluded.bg_marked, fg_border=excluded.fg_border, "
        "fg_titlebar=excluded.fg_titlebar, bg_titlebar=excluded.bg_titlebar, "
        "fg_statusbar=excluded.fg_statusbar, bg_statusbar=excluded.bg_statusbar;";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    int i = 1;
    sqlite3_bind_text(stmt, i++, theme.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, i++, theme.description.c_str(), -1, SQLITE_TRANSIENT);

    const auto bind_color = [&](const RgbColor &c) {
        const std::string hex = ColorToHex(c);
        sqlite3_bind_text(stmt, i++, hex.c_str(), -1, SQLITE_TRANSIENT);
    };
    bind_color(theme.fg_normal);
    bind_color(theme.bg_normal);
    bind_color(theme.fg_selected);
    bind_color(theme.bg_selected);
    bind_color(theme.fg_marked);
    bind_color(theme.bg_marked);
    bind_color(theme.fg_border);
    bind_color(theme.fg_titlebar);
    bind_color(theme.bg_titlebar);
    bind_color(theme.fg_statusbar);
    bind_color(theme.bg_statusbar);

    const bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

std::optional<Theme> ConfigStore::LoadTheme(const std::string &name) const {
    sqlite3_stmt *stmt = nullptr;
    const char *sql = "SELECT description, fg_normal, bg_normal, fg_selected, bg_selected, fg_marked, "
                       "bg_marked, fg_border, fg_titlebar, bg_titlebar, fg_statusbar, bg_statusbar "
                       "FROM themes WHERE name = ?;";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<Theme> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const auto col_str = [&](int idx) {
            const unsigned char *text = sqlite3_column_text(stmt, idx);
            return text != nullptr ? std::string(reinterpret_cast<const char *>(text)) : std::string();
        };

        Theme t;
        t.name = name;
        t.description = col_str(0);
        t.fg_normal = HexToColor(col_str(1));
        t.bg_normal = HexToColor(col_str(2));
        t.fg_selected = HexToColor(col_str(3));
        t.bg_selected = HexToColor(col_str(4));
        t.fg_marked = HexToColor(col_str(5));
        t.bg_marked = HexToColor(col_str(6));
        t.fg_border = HexToColor(col_str(7));
        t.fg_titlebar = HexToColor(col_str(8));
        t.bg_titlebar = HexToColor(col_str(9));
        t.fg_statusbar = HexToColor(col_str(10));
        t.bg_statusbar = HexToColor(col_str(11));
        result = t;
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<std::string> ConfigStore::ThemeNames() const {
    std::vector<std::string> names;
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, "SELECT name FROM themes ORDER BY name;", -1, &stmt, nullptr) != SQLITE_OK) {
        return names;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        if (text != nullptr) names.emplace_back(reinterpret_cast<const char *>(text));
    }
    sqlite3_finalize(stmt);
    return names;
}

} // namespace msxdisk::config
