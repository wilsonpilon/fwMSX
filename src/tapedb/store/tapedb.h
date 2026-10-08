// fwMSX -- banco de fitas (SQLite): metadados (titulo, empresa, ano, hash) das
// fitas (.cas/.tsx/.tzx) que o usuario tem no disco, SEM download nenhum --
// item (e) de doc/SPEC.md, secao 5.2: "Banco de fitas (SQLite) e download pelo
// site, depois de conferir os termos." O download continua fora de escopo (o
// site nao publica termos de uso, ver doc/SPEC.md, secao 5.2, item 7); este
// banco so' CADASTRA fitas que o proprio usuario ja' tem, igual o `romdb`
// escaneia ROMs locais -- mesma estrutura de CRUD e busca (reaproveitada por
// analogia, nao por codigo compartilhado: sao bancos SQLite separados, ja' que
// fita nao e' uma categoria de ROM). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/tape-spec.md, secao 11.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct sqlite3;

namespace tapedb {

// Uma fita guardada no disco (tabela `fitas`).
struct TapeRecord {
    int64_t id = 0;
    std::string sha1;    // minusculo, 40 caracteres; chave unica (do ARQUIVO, nao do conteudo do bloco #4B)
    int64_t size = 0;
    std::string format;  // "cas", "tsx" ou "tzx" (pela extensao)
    std::string title;   // nome do jogo/programa (TOSEC) -- auto-preenchido do 1o arquivo dentro da fita, se vazio
    std::string company; // TOSEC: empresa/publisher -- so' manual, nao vem da fita
    std::string year;    // TOSEC: ano -- so' manual, nao vem da fita
    std::string path;    // caminho do arquivo no disco (relativo a' pasta de fitas, quando dentro dela)
    std::string source;  // de onde veio: "scan" ou "manual"
    std::string notes;
};

class TapeDb {
public:
    TapeDb() = default;
    ~TapeDb();
    TapeDb(const TapeDb &) = delete;
    TapeDb &operator=(const TapeDb &) = delete;

    // Abre (ou cria) o banco no arquivo `path` e garante o esquema.
    bool Open(const std::string &path, std::string &error);
    bool is_open() const { return db_ != nullptr; }

    // CRUD de `fitas`. Add() com sha1 ja existente atualiza o caminho e a origem e
    // mantem titulo/empresa/ano/notas. Em ambos, `rec.id` recebe o id da linha.
    bool Add(TapeRecord &rec, std::string &error);
    bool Update(const TapeRecord &rec, std::string &error);
    bool Delete(int64_t id, std::string &error);
    bool Get(int64_t id, TapeRecord &out) const;
    bool FindBySha1(const std::string &sha1, TapeRecord &out) const;

    // Busca por texto (titulo, empresa, ano, sha1, caminho, notas). Vazio = tudo.
    std::vector<TapeRecord> Search(const std::string &text) const;
    int64_t Count() const;

    // Le um arquivo de fita, calcula o SHA-1, tenta abrir com o leitor de fita
    // existente (cas_reader/tzx_reader) para auto-preencher `title` com o nome
    // do 1o arquivo encontrado dentro dela (so' se a fita for NOVA no banco --
    // uma fita ja cadastrada mantem o titulo que o usuario deu), e grava (Add).
    // `root` e' a pasta de fitas: o caminho guardado fica relativo a ela quando
    // o arquivo esta dentro.
    bool ScanFile(const std::string &file, const std::string &root, const std::string &source, TapeRecord &out,
                  std::string &error);
    // Varre a pasta (recursivo, so' .cas/.tsx/.tzx). Conta quantas fitas novas e quantas atualizadas.
    bool ScanDirectory(const std::string &dir, const std::string &root, const std::string &source, int64_t &added,
                       int64_t &updated, std::string &error);

private:
    bool Exec(const char *sql, std::string &error);

    sqlite3 *db_ = nullptr;
};

} // namespace tapedb
