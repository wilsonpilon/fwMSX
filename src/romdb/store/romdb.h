// fwMSX -- banco de ROMs (SQLite): cadastro, busca e CRUD das ROMs guardadas no
// disco, a tabela de mappers do CARTS.SHA do fMSX e o banco do Vampier (msxromdb).
// Codigo ORIGINAL do fwMSX (BSD-3-Clause), exceto a semantica dos dados de terceiros
// (fMSX e Vampier). Ver doc/romdb-spec.md.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct sqlite3;

namespace romdb {

// Uma ROM guardada no disco (tabela `roms`).
struct RomRecord {
    int64_t id = 0;
    std::string sha1;      // minusculo, 40 caracteres; chave unica
    std::string crc32;     // maiusculo, 8 caracteres
    int64_t size = 0;
    std::string name;      // nome do jogo/programa (preenchido manualmente ou pelo Vampier)
    std::string category;  // bios, interface, cartucho, disco, tabela ou outro
    std::string hardware;  // texto livre: "MSX2 BIOS", "Konami SCC", "Disk ROM 1.8"...
    int mapper = -1;       // numero de mapper do fMSX (0-7), -1 = nao informado
    std::string path;      // caminho do arquivo no disco
    std::string source;    // de onde veio: fmsx, filehunter, vampier, scan, manual
    std::string notes;
};

// Uma linha do banco do Vampier (jogo + ROM).
struct VampierHit {
    std::string sha1;   // maiusculo, como no banco
    std::string game;
    std::string year;
    std::string company;
    std::string rom_type;
    std::string dump;
    std::string remark;
    std::string platform; // "MSX" ou "MSX2" (msxdb_rominfo.Platform) -- 1.26.0
    std::string crc32;    // msxdb_romdetails.CRC32, maiusculo -- 1.26.0
    int64_t file_size = -1; // msxdb_romdetails.FileSize em bytes, -1 = nao informado -- 1.26.0
};

class RomDb {
public:
    RomDb() = default;
    ~RomDb();
    RomDb(const RomDb &) = delete;
    RomDb &operator=(const RomDb &) = delete;

    // Abre (ou cria) o banco no arquivo `path` e garante o esquema.
    bool Open(const std::string &path, std::string &error);
    bool is_open() const { return db_ != nullptr; }

    // CRUD de `roms`. Add() com sha1 ja existente atualiza o caminho e a origem e
    // mantem o nome e as notas. Em ambos, `rec.id` recebe o id da linha.
    bool Add(RomRecord &rec, std::string &error);
    bool Update(const RomRecord &rec, std::string &error);
    bool Delete(int64_t id, std::string &error);
    bool Get(int64_t id, RomRecord &out) const;
    bool FindBySha1(const std::string &sha1, RomRecord &out) const;

    // Busca por texto (nome, sha1, hardware, notas, caminho) e/ou categoria. Vazio = tudo.
    std::vector<RomRecord> Search(const std::string &text, const std::string &category) const;
    int64_t Count() const;

    // Importa o CARTS.SHA do fMSX ("sha1 mapper" por linha). `rows` = linhas gravadas.
    bool ImportCartsSha(const std::string &path, int64_t &rows, std::string &error);
    // Mapper de um SHA-1 do CARTS.SHA, ou -1.
    int CartMapper(const std::string &sha1) const;

    // Executa o dump SQL do Vampier (msxromdb) no mesmo banco. Recria as tabelas msxdb_*.
    bool ImportVampierSql(const std::string &sql_path, int64_t &rows, std::string &error);
    bool HasVampier() const;
    std::vector<VampierHit> VampierSearch(const std::string &text) const;
    // Preenche o nome das ROMs sem nome que o Vampier conhece (pelo SHA-1). Devolve quantas.
    int64_t IdentifyWithVampier(std::string &error);

    // Le um arquivo, calcula SHA-1/CRC32 e grava (Add). `root` e' a pasta de ROMs: o
    // caminho guardado fica relativo a ela quando o arquivo esta dentro.
    bool ScanFile(const std::string &file, const std::string &root, const std::string &source, RomRecord &out,
                  std::string &error);
    // Varre a pasta (recursivo). Conta quantas ROMs novas e quantas atualizadas.
    bool ScanDirectory(const std::string &dir, const std::string &root, const std::string &source, int64_t &added,
                       int64_t &updated, std::string &error);

private:
    bool Exec(const char *sql, std::string &error);

    sqlite3 *db_ = nullptr;
};

// Le um arquivo inteiro para memoria. false se nao abrir.
bool ReadWholeFile(const std::string &path, std::vector<uint8_t> &out);

} // namespace romdb
