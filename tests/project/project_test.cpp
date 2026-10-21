// fwMSX -- teste do projeto .msxproject (SQLite) e das copias soltas: banco, configuracao, caminhos, sincronizacao com hash de base,
// conflitos, copia do projeto para "outro PC", import de uma pasta (o clone do GitHub) e o que NAO pode acontecer (escrever fora da raiz).
#include <sqlite3.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "../../src/project/cpp/project.h"

namespace fs = std::filesystem;

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, const std::string &what) {
    ++g_checks;
    if (!cond) {
        ++g_failures;
        std::printf("[FAIL] %s\n", what.c_str());
    }
}

std::vector<uint8_t> B(const std::string &s) { return std::vector<uint8_t>(s.begin(), s.end()); }

std::string Slurp(const fs::path &p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

void Spit(const fs::path &p, const std::string &d) {
    std::error_code ec;
    fs::create_directories(p.parent_path(), ec);
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    f.write(d.data(), static_cast<std::streamsize>(d.size()));
}

const prj::SyncItem *Find(const std::vector<prj::SyncItem> &v, const std::string &path) {
    for (const prj::SyncItem &i : v) {
        if (i.path == path) return &i;
    }
    return nullptr;
}

prj::State StateOf(prj::Project &p, const std::string &root, const std::string &path) {
    std::vector<prj::SyncItem> v;
    std::string err;
    p.Status(root, v, err);
    const prj::SyncItem *i = Find(v, path);
    return i ? i->state : static_cast<prj::State>(-1);
}

void TestPure() {
    check(prj::PathOk("src/jogo.bas") && prj::PathOk("a.txt") && prj::PathOk("sprites/hero 1.spr"), "caminhos normais servem");
    check(!prj::PathOk("../x") && !prj::PathOk("a/../b") && !prj::PathOk("/etc/x") && !prj::PathOk("C:/x") && !prj::PathOk("a//b") && !prj::PathOk("") &&
              !prj::PathOk("./a") && !prj::PathOk("a/.") && !prj::PathOk("a\\b") && !prj::PathOk("a/b.") && !prj::PathOk("con?.txt"),
          "caminhos que escapam da raiz ou nao existem no Windows sao recusados");
    check(prj::NormalizePath(".\\src\\a.bas") == "src/a.bas", "normaliza \\ e ./");
    bool text = false;
    check(prj::KindOf("x/JOGO.BAS", &text) == "basic" && text, "BAS = basic, texto");
    check(prj::KindOf("a.dmx", &text) == "dignified" && text, "DMX = dignified");
    check(prj::KindOf("a/b.dsk", &text) == "disk" && !text, "DSK = disk, binario");
    check(prj::KindOf("Obifile", &text) == "recipe" && text, "Obifile = receita");
    check(prj::KindOf("a.xyz", &text) == "file" && !text, "extensao desconhecida = file, binario");
    check(prj::HashBytes(B("a\r\nb\r\n"), true) == prj::HashBytes(B("a\nb\n"), true), "o hash de um texto ignora o CR");
    check(prj::HashBytes(B("a\r\nb"), false) != prj::HashBytes(B("a\nb"), false), "o hash de um binario nao ignora nada");
    check(prj::HashBytes({}, true) == "da39a3ee5e6b4b0d3255bfef95601890afd80709", "SHA-1 do vazio");
}

void TestDatabase(const fs::path &dir) {
    const fs::path file = dir / "jogo.msxproject";
    prj::Project p;
    std::string err;
    check(p.Create(file.string(), "Meu Jogo", err), "cria o projeto: " + err);
    prj::Project q;
    check(!q.Create(file.string(), "x", err) && fs::exists(file), "nao cria por cima de um arquivo existente (e nao o apaga)");
    check(p.Name() == "Meu Jogo" && p.Id().size() == 16, "nome e id");
    check(p.SetConfig("machine", "MSX2", err) && p.SetConfig("emu.disk", "1", err) && p.GetConfig("machine") == "MSX2" && p.GetConfig("nada", "dflt") == "dflt", "configuracao");
    check(!p.SetConfig("a=b", "x", err) && !p.SetConfig("k", "a\nb", err), "chave com = e valor com quebra de linha sao recusados");
    check(p.Put("src/jogo.bas", B("10 PRINT 1\r\n"), err) && p.Put("sprites/hero.spr", B("\x01\x02"), err), "grava arquivos");
    check(!p.Put("../fora.txt", B("x"), err) && !p.Put("/abs.txt", B("x"), err), "Put recusa caminho invalido");
    std::vector<uint8_t> d;
    check(p.Get("src\\jogo.bas", d) && d == B("10 PRINT 1\r\n"), "Get (aceita \\)");
    check(p.Has("sprites/hero.spr") && !p.Has("x"), "Has");
    const auto l = p.List();
    check(l.size() == 2 && l[0].path == "sprites/hero.spr" && l[0].kind == "sprite" && l[1].kind == "basic" && l[1].size == 12, "List: tipos e tamanhos");
    check(p.List("src/").size() == 1, "List com prefixo");
    check(p.Put("src/jogo.bas", B("20 END\r\n"), err) && p.List().size() == 2, "regravar o mesmo caminho troca o conteudo");
    check(p.Remove("sprites/hero.spr", err) && !p.Remove("sprites/hero.spr", err), "Remove");
    p.Close();

    // reabrir: tudo persiste; e nunca ha' arquivo -wal / -journal sobrando (o arquivo unico e' o projeto inteiro)
    prj::Project r;
    check(r.Open(file.string(), err), "reabre: " + err);
    check(r.Name() == "Meu Jogo" && r.GetConfig("machine") == "MSX2" && r.Has("src/jogo.bas"), "o que foi gravado persiste");
    check(!fs::exists(dir / "jogo.msxproject-wal") && !fs::exists(dir / "jogo.msxproject-journal") && !fs::exists(dir / "jogo.msxproject-shm"),
          "sem -wal/-journal/-shm: um arquivo so'");
    r.Close();
    Spit(dir / "lixo.msxproject", "isto nao e' sqlite");
    check(!r.Open((dir / "lixo.msxproject").string(), err), "recusa arquivo que nao e' projeto");
    check(!r.Open((dir / "naoexiste.msxproject").string(), err) && !fs::exists(dir / "naoexiste.msxproject"), "Open nao cria arquivo");
}

void TestSync(const fs::path &dir) {
    const fs::path file = dir / "sync.msxproject";
    const std::string root = (dir / "pcA").string();
    prj::Project p;
    std::string err;
    check(p.Create(file.string(), "", err) && p.Name() == "sync", "nome padrao = o do arquivo");
    p.SetConfig("machine", "MSX2+", err);
    p.Put("src/jogo.bas", B("10 PRINT 1\r\n"), err);
    p.Put("src/main.asm", B("org 0x4000\n"), err);
    p.Put("discos/jogo.dsk", B(std::string("\x00\x01\x02\xFF", 4)), err);

    // 1a vez: tudo so' existe no projeto -> extrai
    prj::SyncSummary s;
    check(p.Sync(root, false, s, err), "primeira sincronizacao: " + err);
    check(s.pulled == 3 && s.pushed == 0 && s.conflicts.empty(), "extrai os 3 arquivos");
    check(Slurp(fs::path(root) / "src/jogo.bas") == "10 PRINT 1\r\n" && Slurp(fs::path(root) / "discos/jogo.dsk").size() == 4, "as copias soltas tem os mesmos bytes");
    check(fs::exists(fs::path(root) / "msxproject.txt") && fs::exists(fs::path(root) / ".msxsync"), "manifesto e .msxsync");
    check(Slurp(fs::path(root) / "msxproject.txt").find("machine=MSX2+") != std::string::npos && Slurp(fs::path(root) / "msxproject.txt").find("basic\tsrc/jogo.bas") != std::string::npos,
          "o manifesto tem a configuracao e os tipos");
    check(p.Sync(root, false, s, err) && s.same == 3 && s.pulled == 0 && s.pushed == 0, "de novo: tudo igual");

    // so' a pasta mudou -> pasta->projeto
    Spit(fs::path(root) / "src/main.asm", "org 0x8000\n");
    check(StateOf(p, root, "src/main.asm") == prj::State::DiskChanged, "pasta mudou");
    check(p.Sync(root, false, s, err) && s.pushed == 1, "sincroniza para dentro do projeto");
    std::vector<uint8_t> d;
    check(p.Get("src/main.asm", d) && d == B("org 0x8000\n"), "o projeto recebeu a edicao da pasta");

    // so' o projeto mudou -> projeto->pasta
    p.Put("src/jogo.bas", B("10 PRINT 2\r\n"), err);
    check(StateOf(p, root, "src/jogo.bas") == prj::State::ProjectChanged, "projeto mudou");
    check(p.Sync(root, false, s, err) && s.pulled == 1 && Slurp(fs::path(root) / "src/jogo.bas") == "10 PRINT 2\r\n", "extrai para a pasta");

    // fim de linha: o git troca LF por CRLF e isso NAO e' alteracao
    Spit(fs::path(root) / "src/main.asm", "org 0x8000\r\n");
    check(StateOf(p, root, "src/main.asm") == prj::State::InSync, "CRLF x LF num texto = igual");
    Spit(fs::path(root) / "discos/jogo.dsk", std::string("\x00\x01\x02\xFE", 4));
    check(StateOf(p, root, "discos/jogo.dsk") == prj::State::DiskChanged, "num binario cada byte conta");
    p.Sync(root, false, s, err);

    // arquivo novo na pasta (ex.: criado fora do editor) volta para o projeto; e nunca se apaga copia solta
    Spit(fs::path(root) / "docs/leia.md", "# oi\n");
    check(StateOf(p, root, "docs/leia.md") == prj::State::OnlyDisk, "so' na pasta");
    check(p.Sync(root, false, s, err) && s.pushed == 1 && p.Has("docs/leia.md") && p.List("docs/")[0].kind == "doc", "entra no projeto com o tipo certo");
    Spit(fs::path(root) / ".git/config", "x");
    Spit(fs::path(root) / "x.msxproject", "x");
    check(p.Sync(root, false, s, err) && !p.Has(".git/config") && !p.Has("x.msxproject"), "pastas com ponto e .msxproject nao entram");

    // conflito: os dois mudaram
    Spit(fs::path(root) / "src/jogo.bas", "10 PRINT 3\r\n");
    p.Put("src/jogo.bas", B("10 PRINT 4\r\n"), err, "", 4000000000LL); // o projeto bem mais novo
    std::vector<prj::SyncItem> items;
    p.Status(root, items, err);
    const prj::SyncItem *c = Find(items, "src/jogo.bas");
    check(c && c->state == prj::State::Conflict && c->suggest == prj::Action::Ask && c->newer == prj::Action::Pull, "conflito: pergunta; o lado mais novo e' o projeto");
    check(p.Sync(root, false, s, err) && s.conflicts.size() == 1 && Slurp(fs::path(root) / "src/jogo.bas") == "10 PRINT 3\r\n", "o conflito nao e' resolvido sozinho");
    p.Get("src/jogo.bas", d);
    check(d == B("10 PRINT 4\r\n"), "e o projeto tambem ficou como estava");
    check(p.Pull("src/jogo.bas", root, err) && Slurp(fs::path(root) / "src/jogo.bas") == "10 PRINT 4\r\n" && StateOf(p, root, "src/jogo.bas") == prj::State::InSync,
          "Pull resolve a favor do projeto");
    Spit(fs::path(root) / "src/jogo.bas", "10 PRINT 5\r\n");
    p.Put("src/jogo.bas", B("10 PRINT 6\r\n"), err, "", 1);
    check(StateOf(p, root, "src/jogo.bas") == prj::State::Conflict, "outro conflito");
    check(p.Push("src/jogo.bas", root, err) && p.Get("src/jogo.bas", d) && d == B("10 PRINT 5\r\n") && StateOf(p, root, "src/jogo.bas") == prj::State::InSync,
          "Push resolve a favor da pasta");

    // apagado na pasta: so' sai do projeto com prune
    fs::remove(fs::path(root) / "docs/leia.md");
    check(StateOf(p, root, "docs/leia.md") == prj::State::DiskDeleted, "sumiu da pasta");
    check(p.Sync(root, false, s, err) && s.deleted.size() == 1 && p.Has("docs/leia.md"), "sem prune o projeto guarda");
    check(p.Sync(root, true, s, err) && s.dropped == 1 && !p.Has("docs/leia.md"), "com prune tira do projeto");

    // Save: banco E pasta numa vez so'
    check(p.Save("src/novo.bas", B("10 END\r\n"), root, err) && p.Has("src/novo.bas") && Slurp(fs::path(root) / "src/novo.bas") == "10 END\r\n" &&
              StateOf(p, root, "src/novo.bas") == prj::State::InSync,
          "Save grava nos dois e marca igual");
    check(!p.Save("../fora.bas", B("x"), root, err) && !fs::exists(dir / "fora.bas"), "Save recusa caminho invalido");

    // OUTRO PC: so' o .msxproject foi copiado; a pasta nao existe -> extrai tudo; e nao perde nada
    const std::string id_a = p.Id();
    p.Close();
    const fs::path file2 = dir / "pcB" / "sync.msxproject";
    fs::create_directories(file2.parent_path());
    fs::copy_file(file, file2);
    prj::Project p2;
    check(p2.Open(file2.string(), err), "abre a copia");
    const std::string rootB = (dir / "pcB" / "pasta").string();
    check(p2.Sync(rootB, true, s, err) && s.pulled == static_cast<int>(p2.List().size()) && s.dropped == 0, "o .msxproject copiado extrai TUDO na pasta escolhida (nada e' apagado)");
    check(Slurp(fs::path(rootB) / "src/jogo.bas") == "10 PRINT 5\r\n" && Slurp(fs::path(rootB) / "src/novo.bas") == "10 END\r\n", "mesmos arquivos");
    check(p2.Id() == id_a && p2.GetConfig("machine") == "MSX2+", "o id e a configuracao viajam no projeto");

    // uma pasta sincronizada com OUTRO projeto: o .msxsync dela nao vale (sem base = conflito, nunca apagar)
    prj::Project other;
    check(other.Create((dir / "outro.msxproject").string(), "outro", err), "outro projeto");
    other.Put("src/jogo.bas", B("OUTRO\n"), err);
    check(other.Sync(rootB, true, s, err) && s.dropped == 0 && s.conflicts.size() == 1 && s.conflicts[0].path == "src/jogo.bas", "pasta de outro projeto: conflitos, nada apagado");
    check(Slurp(fs::path(rootB) / "src/jogo.bas") == "10 PRINT 5\r\n", "e a copia solta continua intacta");
}

void TestImportAndSafety(const fs::path &dir) {
    std::string err;
    // o "clone do GitHub": so' a pasta (copias soltas + manifesto, sem .msxsync) -> recria o .msxproject
    const fs::path src = dir / "clone";
    prj::Project p;
    check(p.Create((dir / "origem.msxproject").string(), "Origem", err), "origem");
    p.SetConfig("machine", "MSX1", err);
    p.Put("a/prog.bas", B("10 END\r\n"), err);
    p.Put("rom/x.bin", B("\x01\x02"), err);
    prj::SyncSummary s;
    check(p.Sync((dir / "pasta_origem").string(), false, s, err), "extrai para a pasta que iria ao git");
    fs::copy(dir / "pasta_origem", src, fs::copy_options::recursive);
    fs::remove(src / ".msxsync"); // o .msxsync e' local: nao vai para o git
    prj::Project n;
    check(n.Create((dir / "novo.msxproject").string(), "", err) && n.ImportFolder(src.string(), err), "import da pasta: " + err);
    check(n.Name() == "Origem" && n.Id() == p.Id() && n.GetConfig("machine") == "MSX1", "nome, id e configuracao vem do manifesto");
    check(n.List().size() == 2 && n.Has("a/prog.bas") && n.Has("rom/x.bin") && !n.Has("msxproject.txt"), "os arquivos entram; o manifesto nao e' arquivo do projeto");
    std::vector<prj::SyncItem> v;
    n.Status(src.string(), v, err);
    check(v.size() == 2 && v[0].state == prj::State::InSync && v[1].state == prj::State::InSync, "a pasta e o projeto novo ja' estao em sincronia");

    // um projeto com caminho malicioso (banco mexido na mao) NUNCA escreve fora da raiz
    p.Close();
    sqlite3 *db = nullptr;
    check(sqlite3_open((dir / "origem.msxproject").string().c_str(), &db) == SQLITE_OK, "abre o banco cru");
    sqlite3_exec(db, "INSERT INTO files(path,kind,content,size,hash,mtime) VALUES('../evil.txt','file',x'41',1,'x',1)", nullptr, nullptr, nullptr);
    sqlite3_close(db);
    prj::Project m;
    check(m.Open((dir / "origem.msxproject").string(), err), "reabre");
    const std::string root = (dir / "raiz_segura").string();
    check(m.Sync(root, false, s, err) && !s.errors.empty(), "o caminho malicioso vira erro");
    check(!fs::exists(dir / "evil.txt") && !fs::exists(fs::path(root).parent_path() / "evil.txt"), "nada foi escrito fora da raiz");
    check(!m.Export(root, err) && err.find("invalido") != std::string::npos, "Export tambem recusa");
}

// Historico (as ultimas versoes de cada arquivo), lapides (a remocao viaja entre PCs) e nomes que so' diferem em maiusculas.
void TestHistoryAndRemoval(const fs::path &dir) {
    std::string err;
    const fs::path pcA = dir / "hA", pcB = dir / "hB";
    fs::create_directories(pcA);
    prj::Project a;
    check(a.Create((pcA / "h.msxproject").string(), "Hist", err), "cria o projeto do historico");

    // --- historico ---
    check(a.Put("src/x.bas", B("v1\n"), err) && a.Put("src/x.bas", B("v2\n"), err) && a.Put("src/x.bas", B("v3\n"), err), "tres versoes");
    auto revs = a.Revisions("src/x.bas");
    check(revs.size() == 2, "o historico guarda as anteriores (v1 e v2), nao a atual");
    std::vector<uint8_t> d;
    check(!revs.empty() && a.GetRevision(revs[0].id, d) && d == B("v2\n"), "a mais nova do historico e' a v2");
    check(a.Put("src/x.bas", B("v3\n"), err) && a.Revisions("src/x.bas").size() == 2, "regravar o mesmo conteudo nao cria versao");
    check(a.Put("src/x.bas", B("v3\r\n"), err) && a.Revisions("src/x.bas").size() == 2, "so' o fim de linha mudou: igual para o hash, nao cria versao");
    for (int i = 4; i <= 20; ++i) a.Put("src/x.bas", B("v" + std::to_string(i) + "\n"), err);
    check(a.Revisions("src/x.bas").size() == 10, "padrao: as ultimas 10");
    check(a.SetConfig("revisions.keep", "3", err) && a.Put("src/x.bas", B("v21\n"), err) && a.Revisions("src/x.bas").size() == 3, "revisions.keep corta o excesso");
    check(a.SetConfig("revisions.keep", "0", err) && a.Put("src/y.txt", B("a"), err) && a.Put("src/y.txt", B("b"), err) && a.Revisions("src/y.txt").empty(),
          "revisions.keep=0 desliga");
    a.SetConfig("revisions.keep", "10", err);
    a.Put("src/big.bin", std::vector<uint8_t>(3 * 1024 * 1024, 1), err);
    a.Put("src/big.bin", std::vector<uint8_t>(3 * 1024 * 1024, 2), err);
    check(a.Revisions("src/big.bin").empty(), "arquivo grande (> 2 MiB) nao entra no historico");

    // restaurar: projeto E pasta; a versao de agora tambem vai para o historico (desfazer o desfazer)
    check(a.Sync(pcA.string(), false, *std::make_unique<prj::SyncSummary>(), err), "sincroniza a pasta");
    revs = a.Revisions("src/x.bas");
    const int64_t old_id = revs.back().id;
    check(a.GetRevision(old_id, d), "versao antiga existe");
    const std::vector<uint8_t> old_data = d;
    check(a.Restore("src/x.bas", old_id, pcA.string(), err), "restaura: " + err);
    check(a.Get("src/x.bas", d) && d == old_data && Slurp(pcA / "src/x.bas") == std::string(old_data.begin(), old_data.end()), "o projeto e a pasta voltaram");
    check(StateOf(a, pcA.string(), "src/x.bas") == prj::State::InSync, "e ficam em sincronia");
    check(!a.Restore("src/y.txt", old_id, pcA.string(), err) && err.find("nao e' de") != std::string::npos, "versao de outro arquivo e' recusada");

    // --- nomes que so' diferem em maiusculas ---
    check(!a.Put("SRC/X.BAS", B("z"), err) && err.find("colide") != std::string::npos, "SRC/X.BAS colide com src/x.bas");
    check(a.Put("src/x2.bas", B("z"), err), "nome diferente de verdade entra");

    // --- remocao propagando ---
    // PC A tira um arquivo; o PC B (que ja' o tinha, em sincronia) recebe o .msxproject e e' avisado
    check(a.Put("docs/old.md", B("# velho\n"), err), "arquivo que sera' tirado");
    a.Sync(pcA.string(), false, *std::make_unique<prj::SyncSummary>(), err);
    a.Close();
    fs::create_directories(pcB);
    fs::copy_file(pcA / "h.msxproject", pcB / "h.msxproject");
    fs::copy(pcA / "docs", pcB / "docs", fs::copy_options::recursive);
    fs::copy(pcA / "src", pcB / "src", fs::copy_options::recursive); // B tem a pasta inteira (senao o --prune tiraria o que 'sumiu' dela)
    fs::copy_file(pcA / ".msxsync", pcB / ".msxsync"); // B ja' estava sincronizado com o projeto
    check(a.Open((pcA / "h.msxproject").string(), err), "reabre A");
    check(a.Drop("docs/old.md", pcA.string(), err), "A tira docs/old.md do projeto");
    check(fs::exists(pcA / "docs/old.md") && StateOf(a, pcA.string(), "docs/old.md") == static_cast<prj::State>(-1),
          "em A a copia da pasta fica e Status nao reclama dela (a pasta decidiu ficar)");
    check(a.Revisions("docs/old.md").size() == 1 && a.Removed().size() == 1, "tirar guarda o conteudo no historico e deixa a lapide");
    a.Close();
    fs::copy_file(pcA / "h.msxproject", pcB / "h.msxproject", fs::copy_options::overwrite_existing); // o .msxproject novo viaja para B
    prj::Project b;
    check(b.Open((pcB / "h.msxproject").string(), err), "abre em B");
    check(StateOf(b, pcB.string(), "docs/old.md") == prj::State::ProjectRemoved, "em B: tirado do projeto, copia igual -> pergunta (nunca apaga sozinho)");
    prj::SyncSummary s;
    check(b.Sync(pcB.string(), false, s, err) && s.removed.size() == 1 && s.removed[0].path == "docs/old.md" && fs::exists(pcB / "docs/old.md"), "Sync lista e NAO apaga");
    Spit(pcB / "docs/old.md", "# velho, mas mexi\n");
    check(StateOf(b, pcB.string(), "docs/old.md") == prj::State::RemovedChanged, "se a copia mudou depois: RemovedChanged");
    check(b.Sync(pcB.string(), true, s, err) && s.deleted_copies == 0 && fs::exists(pcB / "docs/old.md"), "--prune tambem nao apaga o que mudou");
    Spit(pcB / "docs/old.md", "# velho\n");
    check(b.Sync(pcB.string(), true, s, err) && s.deleted_copies == 1 && !fs::exists(pcB / "docs/old.md"), "--prune apaga a copia igual (esta' no historico)");
    check(b.Put("docs/old.md", B("# de volta\n"), err) && b.Removed().empty(), "voltar ao projeto apaga a lapide");
    // o usuario escolhe apagar a copia depois (DeleteLoose) e traz de volta
    check(b.Remove("docs/old.md", err), "tira de novo");
    Spit(pcB / "docs/old.md", "# de volta\n");
    check(!b.DeleteLoose("docs/old.md", pcB.string(), err) || !fs::exists(pcB / "docs/old.md"), "DeleteLoose apaga a copia");
    check(!b.Has("docs/old.md"), "e continua fora do projeto");
    const auto rv = b.Revisions("docs/old.md");
    check(!rv.empty() && b.Restore("docs/old.md", rv[0].id, pcB.string(), err) && b.Has("docs/old.md") && fs::exists(pcB / "docs/old.md"),
          "Restore traz de volta um arquivo tirado (projeto e pasta)");

    // --- o manifesto traz a configuracao que o banco ainda nao tem ---
    b.SetConfig("machine", "MSX2", err);
    b.WriteManifest(pcB.string(), err);
    prj::Project c;
    check(c.Create((dir / "hC.msxproject").string(), "Outro", err), "outro projeto");
    c.Close();
    b.DelConfig("machine");
    check(b.Sync(pcB.string(), false, s, err) && b.GetConfig("machine") == "MSX2", "Sync adota do manifesto a chave que o banco nao tinha");
    b.SetConfig("machine", "MSX1", err);
    check(b.Sync(pcB.string(), false, s, err) && b.GetConfig("machine") == "MSX1", "mas nunca troca a que o banco ja' tem");

    // --- projeto do esquema 1 abre e migra ---
    b.Close();
    sqlite3 *db = nullptr;
    sqlite3_open((pcB / "h.msxproject").string().c_str(), &db);
    sqlite3_exec(db, "DROP TABLE tombstones; DROP TABLE revisions; UPDATE meta SET value='1' WHERE key='schema';", nullptr, nullptr, nullptr);
    sqlite3_close(db);
    check(b.Open((pcB / "h.msxproject").string(), err), "projeto do esquema 1 abre");
    check(b.Put("n.txt", B("1"), err) && b.Put("n.txt", B("2"), err) && b.Revisions("n.txt").size() == 1 && b.Removed().empty(), "e ja' tem o historico e as lapides (migrou)");
}

} // namespace

int main() {
    const fs::path dir = fs::temp_directory_path() / "fwmsx_prjtest";
    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir);

    TestPure();
    TestDatabase(dir);
    TestSync(dir);
    TestImportAndSafety(dir);
    TestHistoryAndRemoval(dir);

    fs::remove_all(dir, ec);
    std::printf("project: %d verificacoes, %d falhas\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
