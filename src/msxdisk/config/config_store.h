//
// msxdisk (fwMSX): armazenamento de configuracao/temas/estado da TUI em
// SQLite -- Fase 4. Guarda tudo o que e configuravel pela interface
// (tema ativo, imagens recentes, outras opcoes) num unico arquivo de
// banco, em vez de espalhar em INI/JSON.
//
#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "theme.h"

struct sqlite3;

namespace msxdisk::config {

class ConfigStore {
public:
    // ~/.msxdisk/config.sqlite3 (USERPROFILE no Windows, HOME no resto).
    static std::filesystem::path DefaultPath();

    // Abre (criando se preciso) o banco em 'db_path', garante o schema e
    // semeia os temas embutidos na primeira vez.
    static std::optional<ConfigStore> Open(const std::filesystem::path &db_path = DefaultPath());

    ~ConfigStore();
    ConfigStore(const ConfigStore &) = delete;
    ConfigStore &operator=(const ConfigStore &) = delete;
    ConfigStore(ConfigStore &&other) noexcept;
    ConfigStore &operator=(ConfigStore &&other) noexcept;

    std::optional<std::string> GetSetting(const std::string &key) const;
    void SetSetting(const std::string &key, const std::string &value, const std::string &description = "");

    void AddRecentImage(const std::string &path);
    std::vector<std::string> RecentImages(int limit = 10) const;

    struct ImageMetadata {
        std::string path;         // caminho absoluto da imagem .dsk (chave)
        std::string description;  // descricao curta definida pelo usuario
        std::string notes;        // anotacoes livres
        std::string created_at;   // ISO 8601 (texto do SQLite), vazio se desconhecido
        std::string updated_at;
    };

    // Garante que existe uma linha de metadados para 'path', gravando
    // created_at=agora se for a primeira vez (chamar no 'create'/'load'
    // para nao perder a data de criacao mesmo antes do usuario escrever
    // uma descricao/nota).
    void RecordImageSeen(const std::string &path);

    // Grava descricao/notas (upsert); preserva created_at existente.
    void SetImageMetadata(const std::string &path, const std::string &description, const std::string &notes);
    std::optional<ImageMetadata> GetImageMetadata(const std::string &path) const;

    bool SaveTheme(const Theme &theme);
    std::optional<Theme> LoadTheme(const std::string &name) const;
    std::vector<std::string> ThemeNames() const;

private:
    explicit ConfigStore(sqlite3 *db);
    void EnsureSchema();
    void SeedBuiltinThemesIfEmpty();

    sqlite3 *db_ = nullptr;
};

} // namespace msxdisk::config
