// fwMSX -- projeto .msxproject: UM arquivo SQLite com tudo do projeto (fontes, sprites, musicas, telas, discos, fitas, documentos, configuracao) e as
// COPIAS SOLTAS numa pasta, mantidas em sincronia. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/project-spec.md.
//
// Nucleo nas 4 linguagens: C++ (banco e sincronizacao), C (o tipo de cada arquivo pela extensao), Fortran (validacao do caminho: nada de "../" ao
// extrair) e Assembly NASM (copia sem CR: o hash de um texto ignora o fim de linha).
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

struct sqlite3;

namespace prj {

// Caminho de arquivo dentro do projeto: relativo, com "/" , sem "..". Devolve false para o que escreveria fora da raiz ou nao existe no Windows.
bool PathOk(const std::string &path);
// "\" -> "/" e tira o "./" do comeco. Nao valida (PathOk valida).
std::string NormalizePath(const std::string &path);
// O tipo do arquivo pela extensao ("basic", "dignified", "asm", "disk", "tape", "screen"...; "file" se nao conhece). `text` = true para texto.
std::string KindOf(const std::string &path, bool *text = nullptr);
// SHA-1 do conteudo; em texto ignora os CR (LF e CRLF dao o mesmo hash).
std::string HashBytes(const std::vector<uint8_t> &data, bool text);

struct FileInfo {
    std::string path, kind, hash;
    int64_t size = 0;
    int64_t mtime = 0; // segundos desde 1970
};

// O estado de um arquivo entre o projeto (banco) e a pasta (disco), comparados com o hash da ULTIMA sincronizacao feita nesta pasta.
enum class State {
    InSync,         // os dois iguais
    DiskChanged,    // so' a pasta mudou desde a ultima sincronizacao
    ProjectChanged, // so' o projeto mudou
    Conflict,       // os dois mudaram (ou nunca foram sincronizados e diferem)
    OnlyDisk,       // existe so' na pasta
    OnlyProject,    // existe so' no projeto (nunca foi extraido para esta pasta)
    DiskDeleted     // foi sincronizado e sumiu da pasta; o projeto ainda tem a versao da ultima sincronizacao
};
enum class Action { None, Push, Pull, Drop, Ask }; // Push = pasta -> projeto; Pull = projeto -> pasta; Drop = tirar do projeto; Ask = o usuario decide
const char *StateName(State s);
const char *ActionName(Action a);

struct SyncItem {
    std::string path;
    State state = State::InSync;
    Action suggest = Action::None; // o que fazer sem perder trabalho; Ask nos conflitos
    Action newer = Action::None;   // nos conflitos: a direcao do lado mais NOVO (Push se a pasta e' mais nova, Pull se o projeto)
    int64_t db_mtime = 0, disk_mtime = 0;
};

struct SyncSummary {
    int same = 0, pushed = 0, pulled = 0, dropped = 0;
    std::vector<SyncItem> conflicts; // ficam como estao: o usuario escolhe (Pull/Push)
    std::vector<SyncItem> deleted;   // sumiram da pasta; so' saem do projeto com `prune`
    std::vector<std::string> errors;
};

class Project {
public:
    Project();
    ~Project();
    Project(const Project &) = delete;
    Project &operator=(const Project &) = delete;

    // Cria um projeto novo (o arquivo nao pode existir). `name` vazio = o nome do arquivo.
    bool Create(const std::string &file, const std::string &name, std::string &err);
    bool Open(const std::string &file, std::string &err);
    void Close();
    bool IsOpen() const { return db_ != nullptr; }
    const std::string &Path() const { return path_; }
    // A pasta das copias soltas quando o usuario nao diz outra: a pasta que contem o .msxproject.
    std::string DefaultRoot() const;
    std::string Name() const;
    std::string Id() const; // identifica o projeto (vai no .msxsync e no manifesto: a pasta sabe de qual projeto e')

    // Configuracao do projeto (modelo do MSX, opcoes do emulador, do Badig...). Valor sem quebra de linha.
    std::string GetConfig(const std::string &key, const std::string &def = "") const;
    bool SetConfig(const std::string &key, const std::string &value, std::string &err);
    bool DelConfig(const std::string &key);
    std::vector<std::pair<std::string, std::string>> Config() const;

    // Arquivos DENTRO do projeto (so' o banco). `kind` vazio = pela extensao (ou o que o arquivo ja' tinha). mtime < 0 = agora.
    bool Put(const std::string &path, const std::vector<uint8_t> &data, std::string &err, const std::string &kind = "", int64_t mtime = -1);
    bool Get(const std::string &path, std::vector<uint8_t> &data) const;
    bool Has(const std::string &path) const;
    bool Remove(const std::string &path, std::string &err);
    std::vector<FileInfo> List(const std::string &prefix = "") const;

    // --- as copias soltas ---
    // Salvar um arquivo do editor: grava no banco E na pasta e marca os dois como iguais.
    bool Save(const std::string &path, const std::vector<uint8_t> &data, const std::string &root, std::string &err);
    // Compara o projeto com a pasta (nao muda nada).
    bool Status(const std::string &root, std::vector<SyncItem> &out, std::string &err);
    bool Push(const std::string &path, const std::string &root, std::string &err); // pasta -> projeto
    bool Pull(const std::string &path, const std::string &root, std::string &err); // projeto -> pasta (sobrescreve)
    bool Drop(const std::string &path, const std::string &root, std::string &err); // tira do projeto (a copia solta nao e' mexida)
    // Aplica o que nao perde trabalho: Push/Pull/InSync; os conflitos ficam em `s.conflicts`. `prune` tambem tira do projeto o que sumiu da pasta.
    // Regrava o manifesto.
    bool Sync(const std::string &root, bool prune, SyncSummary &s, std::string &err);
    // Extrai TUDO para a pasta (o projeto vence), regrava o manifesto. E' o "project export".
    bool Export(const std::string &root, std::string &err);
    // Preenche um projeto NOVO com os arquivos de uma pasta e a configuracao do manifesto, se houver. E' o "project import".
    bool ImportFolder(const std::string &folder, std::string &err);
    bool WriteManifest(const std::string &root, std::string &err) const;

private:
    struct Row {
        std::string hash, kind;
        int64_t mtime = 0;
    };
    bool Exec(const char *sql, std::string &err);
    bool Meta(const std::string &key, std::string &value) const;
    bool SetMeta(const std::string &key, const std::string &value, std::string &err);
    bool Rows(std::map<std::string, Row> &out) const;
    bool PutRaw(const std::string &path, const std::vector<uint8_t> &data, const std::string &kind, int64_t mtime, std::string &err);
    bool PullOne(const std::string &path, const std::string &root, std::map<std::string, std::string> &base, std::string &err);
    bool PushOne(const std::string &path, const std::string &root, std::map<std::string, std::string> &base, std::string &err);
    bool StatusImpl(const std::string &root, const std::map<std::string, std::string> &base, std::vector<SyncItem> &out, std::string &err);
    std::map<std::string, std::string> LoadBase(const std::string &root) const;
    bool SaveBase(const std::string &root, const std::map<std::string, std::string> &base, std::string &err) const;

    sqlite3 *db_ = nullptr;
    std::string path_;
};

} // namespace prj
