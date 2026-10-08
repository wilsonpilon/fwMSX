// Teste do banco de fitas (src/tapedb) -- ver doc/tape-spec.md, secao 11. CRUD,
// busca, e escaneamento de pasta (com auto-preenchimento do titulo a partir do
// leitor de fita existente). Sem Z80/mapa de memoria, sem rede (nao ha' download).
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "romdb/core/hash.h"
#include "tape/cpp/cas_format.h"
#include "tapedb/cli.h"
#include "tapedb/service.h"
#include "tapedb/store/tapedb.h"

namespace fs = std::filesystem;

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

std::string TempDir() {
    const char *t = std::getenv("TEMP");
    const fs::path base = t ? fs::path(t) : fs::path("/tmp");
    const fs::path dir = base / "fwmsx_tapedb_test";
    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);
    return dir.string();
}

// Monta um .cas minimo (cabecalho de sincronismo + 10xID + 6 de nome + outro
// cabecalho + dados), com um nome conhecido -- so' para ScanFile() auto-detectar
// o titulo a partir do leitor de fita existente (cas_reader).
void WriteMinimalCas(const std::string &path, const std::string &name6) {
    std::vector<uint8_t> bytes;
    bytes.insert(bytes.end(), tape::kCasHeader.begin(), tape::kCasHeader.end());
    bytes.insert(bytes.end(), tape::kCasFileIdBytes, tape::kCasIdBasic);
    std::string padded = name6.substr(0, tape::kCasFileNameBytes);
    padded.resize(tape::kCasFileNameBytes, ' ');
    bytes.insert(bytes.end(), padded.begin(), padded.end());
    bytes.insert(bytes.end(), tape::kCasHeader.begin(), tape::kCasHeader.end());
    bytes.push_back(0xAA); // 1 byte de dado, so' para nao ser um arquivo vazio
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    f.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

} // namespace

int main() {
    const std::string tmp = TempDir();

    // --- Banco SQLite: CRUD e busca, direto pela API -------------------------
    {
        const std::string db_path = (fs::path(tmp) / "fitas.db").string();
        tapedb::TapeDb db;
        std::string err;
        check(db.Open(db_path, err), "banco aberto (criado)");

        const std::string content = "fita de teste";
        tapedb::TapeRecord rec;
        rec.sha1 = romdb::Sha1Hex(reinterpret_cast<const uint8_t *>(content.data()), content.size());
        rec.size = static_cast<int64_t>(content.size());
        rec.format = "cas";
        rec.title = "Teste";
        rec.path = "teste.cas";
        check(db.Add(rec, err) && rec.id > 0, "Add: fita gravada com id");

        tapedb::TapeRecord got;
        check(db.FindBySha1(rec.sha1, got) && got.title == "Teste", "FindBySha1 acha a fita");

        rec.company = "Konami";
        rec.year = "1987";
        rec.notes = "anotacao";
        check(db.Update(rec, err), "Update: alteracao gravada");
        tapedb::TapeRecord again;
        check(db.Get(rec.id, again) && again.company == "Konami" && again.year == "1987" && again.notes == "anotacao",
              "Get: alteracoes persistidas");

        check(db.Search("Konami").size() == 1, "busca por empresa");
        check(db.Search("inexistente").empty(), "busca sem resultado");

        tapedb::TapeRecord dup = rec;
        dup.id = 0;
        dup.title = "outro titulo";
        check(db.Add(dup, err) && dup.id == rec.id && dup.title == "Teste", "Add de SHA-1 existente mantem o titulo e o id");
        check(db.Count() == 1, "contagem: 1 fita");

        check(!db.Update(tapedb::TapeRecord{}, err), "Update com id inexistente falha");
        check(!db.Delete(99999, err), "Delete com id inexistente falha");
        check(db.Delete(rec.id, err), "Delete: fita removida");
        check(db.Count() == 0, "contagem depois do Delete: 0 fitas");
    }

    // --- ScanFile: auto-preenche o titulo a partir do leitor de fita ---------
    {
        const std::string db_path = (fs::path(tmp) / "fitas2.db").string();
        tapedb::TapeDb db;
        std::string err;
        check(db.Open(db_path, err), "banco 2 aberto");

        const std::string cas_path = (fs::path(tmp) / "jogo.cas").string();
        WriteMinimalCas(cas_path, "JOGO");

        tapedb::TapeRecord rec;
        check(db.ScanFile(cas_path, tmp, "scan", rec, err), "ScanFile: le e cadastra o .cas (" + err + ")");
        check(rec.format == "cas", "ScanFile: formato detectado pela extensao");
        check(rec.title == "JOGO", "ScanFile: titulo auto-preenchido do 1o arquivo dentro da fita");
        check(rec.path == "jogo.cas", "ScanFile: caminho relativo a' pasta de fitas");

        // Editar o titulo manualmente e re-escanear NAO deve perder a edicao.
        rec.title = "Jogo Editado Pelo Usuario";
        check(db.Update(rec, err), "titulo editado manualmente");
        tapedb::TapeRecord rescanned;
        check(db.ScanFile(cas_path, tmp, "scan", rescanned, err), "ScanFile de novo (mesmo arquivo)");
        check(rescanned.title == "Jogo Editado Pelo Usuario",
              "ScanFile de novo: NAO sobrescreve o titulo que o usuario editou");
        check(db.Count() == 1, "re-escanear o MESMO arquivo nao duplica a fita (mesmo SHA-1)");
    }

    // --- ScanDirectory: so' .cas/.tsx/.tzx, pastas aninhadas, contagem -------
    {
        const std::string scan_dir = (fs::path(tmp) / "pasta").string();
        fs::create_directories(fs::path(scan_dir) / "sub");
        WriteMinimalCas((fs::path(scan_dir) / "a.cas").string(), "A");
        WriteMinimalCas((fs::path(scan_dir) / "sub" / "b.cas").string(), "B");
        std::ofstream ignore((fs::path(scan_dir) / "leiame.txt").string());
        ignore << "isto nao e' uma fita";
        ignore.close();

        const std::string db_path = (fs::path(tmp) / "fitas3.db").string();
        tapedb::TapeDb db;
        std::string err;
        check(db.Open(db_path, err), "banco 3 aberto");
        int64_t added = 0, updated = 0;
        check(db.ScanDirectory(scan_dir, scan_dir, "scan", added, updated, err), "ScanDirectory: varre a pasta");
        check(added == 2 && updated == 0, "ScanDirectory: 2 fitas novas (recursivo), .txt ignorado");
        check(db.Count() == 2, "ScanDirectory: 2 fitas no banco");

        int64_t added2 = 0, updated2 = 0;
        check(db.ScanDirectory(scan_dir, scan_dir, "scan", added2, updated2, err), "ScanDirectory de novo");
        check(added2 == 0 && updated2 == 2, "ScanDirectory de novo: as 2 ja' existiam (atualizadas, nao duplicadas)");
    }

    // --- CLI (fwmsx --fitadb) -------------------------------------------------
    {
        const std::string cli_dir = (fs::path(tmp) / "cli").string();
        fs::create_directories(cli_dir);
        const std::string cas_path = (fs::path(cli_dir) / "radio.cas").string();
        WriteMinimalCas(cas_path, "RADIO");
        const std::string fitas_dir = (fs::path(tmp) / "cli_fitas").string();

        check(tapedb::RunTapeDbCommand({"add", cas_path, "--fitas", fitas_dir, "--empresa", "Dinamic", "--ano", "1990"},
                                        "fwmsx") == 0,
              "cli add: sucesso (codigo 0)");
        check(tapedb::RunTapeDbCommand({"list", "--fitas", fitas_dir}, "fwmsx") == 0, "cli list: codigo 0");
        check(tapedb::RunTapeDbCommand({"search", "Dinamic", "--fitas", fitas_dir}, "fwmsx") == 0,
              "cli search: codigo 0");
        check(tapedb::RunTapeDbCommand({"stats", "--fitas", fitas_dir}, "fwmsx") == 0, "cli stats: codigo 0");

        // Confirma pela API direta que o add com --empresa/--ano pegou (CLI nao imprime em formato facil de conferir).
        tapedb::TapeDb db;
        std::string err;
        check(tapedb::OpenTapeDb(tapedb::TapeDbPaths{fitas_dir}, db, err), "abre o banco criado pela CLI");
        tapedb::TapeRecord rec;
        check(db.Get(1, rec) && rec.title == "RADIO" && rec.company == "Dinamic" && rec.year == "1990",
              "cli add: titulo auto-detectado + empresa/ano das opcoes");

        check(tapedb::RunTapeDbCommand({"show", "1", "--fitas", fitas_dir}, "fwmsx") == 0, "cli show por id: codigo 0");
        check(tapedb::RunTapeDbCommand({"show", rec.sha1, "--fitas", fitas_dir}, "fwmsx") == 0,
              "cli show por sha1: codigo 0");
        check(tapedb::RunTapeDbCommand({"show", "99999", "--fitas", fitas_dir}, "fwmsx") == 1,
              "cli show com id inexistente: codigo 1");
        check(tapedb::RunTapeDbCommand({"edit", "1", "--notas", "nota da cli", "--fitas", fitas_dir}, "fwmsx") == 0,
              "cli edit: codigo 0");
        check(db.Get(1, rec) && rec.notes == "nota da cli", "cli edit: notas gravadas de verdade");
        check(tapedb::RunTapeDbCommand({"del", "1", "--fitas", fitas_dir}, "fwmsx") == 0, "cli del: codigo 0");
        check(db.Count() == 0, "cli del: fita removida de verdade");

        check(tapedb::RunTapeDbCommand({"help"}, "fwmsx") == 0, "cli help: codigo 0");
        check(tapedb::RunTapeDbCommand({}, "fwmsx") == 0, "cli sem argumento: mostra ajuda, codigo 0");
        check(tapedb::RunTapeDbCommand({"comando-invalido", "--fitas", fitas_dir}, "fwmsx") == 2,
              "cli comando desconhecido: codigo 2");
        check(tapedb::RunTapeDbCommand({"scan", "--fitas", fitas_dir}, "fwmsx") == 1, "cli scan sem pasta: codigo 1");
        check(tapedb::RunTapeDbCommand({"add", "--fitas", fitas_dir}, "fwmsx") == 1, "cli add sem arquivo: codigo 1");
    }

    if (g_failures == 0) {
        std::printf("\nTodos os testes do banco de fitas passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) do banco de fitas falharam.\n", g_failures);
    return 1;
}
