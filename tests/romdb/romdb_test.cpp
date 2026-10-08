// Teste do banco de ROMs (src/romdb) -- ver doc/romdb-spec.md. Sem rede: hashes,
// classificacao, leitura de indices, ZIP (ida e volta) e o banco SQLite (CRUD, busca,
// CARTS.SHA e dump do Vampier).
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "romdb/archive/unzip.h"
#include "romdb/cli.h"
#include "romdb/core/hash.h"
#include "romdb/net/listing.h"
#include "romdb/net/web.h"
#include "romdb/store/classify.h"
#include "romdb/store/romdb.h"

namespace fs = std::filesystem;

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

void WriteText(const fs::path &p, const std::string &text) {
    fs::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::binary);
    f << text;
}

const char *kIndexHtml =
    "<html><body><table>\n"
    "<TR><TD><B>Name</B></TD><TD><B>Size</B></TD></TR>\n"
    "<TR><TD><A HREF=\"../\">../</A></TD><TD>-</TD></TR>\n"
    "<TR><TD>\n<A HREF=\"extensions/\">\nextensions/\n</A>\n</TD>\n<TD>  -  </TD>\n<TD>  Directory  </TD>\n</TR>\n"
    "<TR><TD>\n<A HREF=\"Full%20Set%20System%20ROMs%20for%20OpenMSX%20-%2015-08-2026.zip\">\n"
    "Full Set System ROMs for OpenMSX - 15-08-2026.zip\n</A>\n</TD>\n<TD> 32.06 MB </TD>\n<TD> application/zip </TD>\n"
    "</TR>\n"
    "<TR><TD>\n<A HREF=\"Full%20Set%20System%20ROMs%20for%20OpenMSX%20-%2026-08-2025.zip\">\n"
    "Full Set System ROMs for OpenMSX - 26-08-2025.zip\n</A>\n</TD>\n<TD> 27.70 MB </TD>\n<TD> application/zip </TD>\n"
    "</TR>\n"
    "</table></body></html>\n";

const char *kFmsxPage =
    "<a href=\"fMSX59-Windows-bin.zip\">v5.9</a> <a href=\"fMSX60-Windows-bin.zip\">v6.0</a>"
    " <a href=\"fMSX60.zip\">fonte</a> <a href=\"fMSX60-Linux.tar.gz\">linux</a>";

} // namespace

int main() {
    // --- Hashes: vetores padrao --------------------------------------------------
    const std::string abc = "abc";
    check(romdb::Sha1Hex(reinterpret_cast<const uint8_t *>(abc.data()), abc.size()) ==
              "a9993e364706816aba3e25717850c26c9cd0d89d",
          "SHA-1 de 'abc' (vetor padrao)");
    check(romdb::Sha1Hex(nullptr, 0) == "da39a3ee5e6b4b0d3255bfef95601890afd80709", "SHA-1 de arquivo vazio");
    const std::string digits = "123456789";
    check(romdb::Crc32Hex(reinterpret_cast<const uint8_t *>(digits.data()), digits.size()) == "CBF43926",
          "CRC32 de '123456789' (vetor padrao)");

    // --- Classificacao ------------------------------------------------------------
    check(romdb::CategoryFor("MSX2.ROM") == "bios", "MSX2.ROM e' BIOS");
    check(romdb::CategoryFor("MSX2EXT.ROM") == "interface", "MSX2EXT.ROM e' interface (sub-ROM)");
    check(romdb::CategoryFor("DISK.ROM") == "interface", "DISK.ROM e' interface");
    check(romdb::CategoryFor("fmpac.rom") == "interface", "fmpac.rom e' interface");
    check(romdb::CategoryFor("Carnivore2.rom") == "cartucho", "Carnivore2.rom e' cartucho");
    check(romdb::CategoryFor("msxdos1.dsk") == "disco", "msxdos1.dsk e' disco");
    check(romdb::CategoryFor("CARTS.SHA") == "tabela", "CARTS.SHA e' tabela");
    check(romdb::FolderForCategory("cartucho") == "cartuchos", "pasta do tipo cartucho");
    check(romdb::FolderForCategory("bios") == "bios", "pasta do tipo bios");

    // --- URL ----------------------------------------------------------------------
    const std::string name = "Full Set System ROMs for OpenMSX - 15-08-2026.zip";
    const std::string enc = romdb::UrlEncodePath(name);
    check(enc == "Full%20Set%20System%20ROMs%20for%20OpenMSX%20-%2015-08-2026.zip", "codificacao de URL dos espacos e hifens");
    check(romdb::UrlDecode(enc) == name, "decodificacao volta ao nome original");

    // --- Indice de pasta (estilo Abyss) ---------------------------------------------
    const std::string base = "https://download.file-hunter.com/System%20ROMs/";
    const auto entries = romdb::ParseListing(kIndexHtml, base);
    check(entries.size() == 3, "indice: 3 entradas (sem '../' e sem cabecalho): " + std::to_string(entries.size()));
    if (entries.size() == 3) {
        check(entries[0].is_dir && entries[0].name == "extensions", "indice: pasta 'extensions'");
        check(!entries[1].is_dir && entries[1].size == "32.06 MB" && entries[1].type == "application/zip",
              "indice: arquivo com tamanho e tipo");
        check(entries[1].name == name, "indice: nome decodificado");
    }
    const auto quoted = romdb::ParseListing(
        "<TR><TD><A HREF=\"ROM&#39;s%20RepairBas/\">ROM&#39;s RepairBas/</A></TD><TD>-</TD></TR>", base);
    check(quoted.size() == 1 && quoted[0].is_dir && quoted[0].name == "ROM's RepairBas" &&
              quoted[0].url == base + "ROM's%20RepairBas/",
          "indice: entidades HTML (&#39;) viram apostrofo no nome e no link");
    const romdb::ListingEntry *latest = romdb::LatestFullSet(entries);
    check(latest && latest->name.find("15-08-2026") != std::string::npos, "Full Set mais recente: 15-08-2026");
    int y = 0, m = 0, d = 0;
    check(romdb::ParseFullSetDate(name, y, m, d) && y == 2026 && m == 8 && d == 15, "data do nome do Full Set");

    const std::string fmsx = romdb::LatestFmsxWindowsZip(kFmsxPage, "https://fms.komkon.org/fMSX/");
    check(fmsx == "https://fms.komkon.org/fMSX/fMSX60-Windows-bin.zip", "fMSX: pacote Windows de versao mais alta");

    // --- ZIP: escrever, extrair e recusar caminhos perigosos -------------------------
    const fs::path tmp = fs::temp_directory_path() / "fwmsx_romdb_test";
    std::error_code ec;
    fs::remove_all(tmp, ec);
    fs::create_directories(tmp);
    std::string err;
    const std::vector<std::pair<std::string, std::string>> zip_entries = {
        {"bios/MSX.ROM", std::string(32, 'M')},
        {"sub/extra.txt", "texto"},
        {"../escapa.txt", "nao deve sair da pasta"},
    };
    check(romdb::WriteZip((tmp / "pacote.zip").string(), zip_entries, err), "ZIP gravado");
    std::vector<std::string> extracted;
    const fs::path dest = tmp / "saida";
    check(romdb::ExtractZip((tmp / "pacote.zip").string(), dest.string(), extracted, err), "ZIP extraido");
    check(fs::exists(dest / "bios" / "MSX.ROM") && fs::file_size(dest / "bios" / "MSX.ROM") == 32,
          "ZIP: arquivo dentro de pasta extraido com o tamanho certo");
    check(!fs::exists(tmp / "escapa.txt") && !fs::exists(dest.parent_path() / "escapa.txt"),
          "ZIP: caminho com '..' nao escreve fora da pasta de destino");

    // --- Banco SQLite ------------------------------------------------------------------
    const std::string db_path = (tmp / "roms.db").string();
    romdb::RomDb db;
    check(db.Open(db_path, err), "banco aberto (criado)");

    romdb::RomRecord rec;
    rec.sha1 = romdb::Sha1Hex(reinterpret_cast<const uint8_t *>(abc.data()), abc.size());
    rec.crc32 = "352441C2";
    rec.size = 3;
    rec.name = "Teste";
    rec.category = "cartucho";
    rec.path = "cartuchos/abc.rom";
    check(db.Add(rec, err) && rec.id > 0, "Add: ROM gravada com id");
    romdb::RomRecord got;
    check(db.FindBySha1(rec.sha1, got) && got.name == "Teste", "FindBySha1 acha a ROM");
    rec.hardware = "MSX2 teste";
    rec.mapper = 4;
    rec.notes = "anotacao";
    check(db.Update(rec, err), "Update: alteracao gravada");
    romdb::RomRecord again;
    check(db.Get(rec.id, again) && again.hardware == "MSX2 teste" && again.mapper == 4 && again.notes == "anotacao",
          "Get: alteracoes persistidas");
    check(db.Search("MSX2", "").size() == 1, "busca por hardware");
    check(db.Search("", "bios").empty() && db.Search("Teste", "cartucho").size() == 1, "busca com filtro de categoria");
    romdb::RomRecord dup = rec;
    dup.id = 0;
    dup.name = "outro nome";
    check(db.Add(dup, err) && dup.id == rec.id && dup.name == "Teste", "Add de SHA-1 existente mantem o nome e o id");
    check(db.Count() == 1, "contagem: 1 ROM");

    // CARTS.SHA: "sha1 mapper" por linha.
    const fs::path carts = tmp / "CARTS.SHA";
    WriteText(carts, "0733cd627467a866846e15caf1770a5594eaf4cc 4\nda397e783d677d1a78fff222d9d6cb48b915dada 3\nlixo\n");
    int64_t rows = 0;
    check(db.ImportCartsSha(carts.string(), rows, err) && rows == 2, "CARTS.SHA: 2 linhas importadas");
    check(db.CartMapper("0733CD627467A866846E15CAF1770A5594EAF4CC") == 4, "mapper do CARTS.SHA (SHA-1 em maiusculas)");
    check(db.CartMapper("0000") == -1, "SHA-1 desconhecido: mapper -1");

    // Dump do Vampier: tabelas msxdb_* com jogo, empresa e ROM.
    const fs::path vam = tmp / "sql-romdb.sql";
    WriteText(vam,
              "CREATE TABLE msxdb_romdetails (`HashID` INTEGER, `GameID` INTEGER, `RomType` VARCHAR(1020),"
              " `SHA1` VARCHAR(1020), `Dump` VARCHAR(1020), `Remark` VARCHAR(1020));\n"
              "CREATE TABLE msxdb_rominfo (`GameID` INTEGER, `GameName` VARCHAR(1020), `Year` VARCHAR(16),"
              " `CompanyID1` INTEGER);\n"
              "CREATE TABLE msxdb_company (`CompanyID` INTEGER, `ShortName` VARCHAR(400));\n"
              "BEGIN;\n"
              "INSERT INTO msxdb_company VALUES ('276','Konami');\n"
              "INSERT INTO msxdb_rominfo VALUES ('7','Nemesis','1986','276');\n"
              "INSERT INTO msxdb_romdetails VALUES ('1','7','Normal','A9993E364706816ABA3E25717850C26C9CD0D89D','GoodMSX','');\n"
              "COMMIT;\n");
    rows = 0;
    check(db.ImportVampierSql(vam.string(), rows, err) && rows == 1, "Vampier: dump importado (1 ROM)");
    check(db.HasVampier(), "Vampier: presente no banco");
    const auto hits = db.VampierSearch("Nemesis");
    check(hits.size() == 1 && hits[0].company == "Konami" && hits[0].year == "1986", "Vampier: busca pelo nome do jogo");
    rec.name = "";
    check(db.Update(rec, err), "ROM sem nome (para testar a identificacao)");
    check(db.IdentifyWithVampier(err) == 1, "identificar: a ROM sem nome recebe o nome do Vampier");
    romdb::RomRecord named;
    check(db.FindBySha1(rec.sha1, named) && named.name == "Nemesis", "identificar: nome vindo do Vampier");
    check(db.IdentifyWithVampier(err) == 0, "identificar: uma segunda vez nao mexe em quem ja tem nome");

    // Escanear um arquivo local e uma pasta.
    const fs::path scan_dir = tmp / "roms" / "cartuchos";
    WriteText(scan_dir / "jogo.rom", "conteudo do jogo");
    WriteText(scan_dir / "leia.txt", "nao e' ROM");
    int64_t added = 0, updated = 0;
    check(db.ScanDirectory((tmp / "roms").string(), (tmp / "roms").string(), "scan", added, updated, err) && added == 1,
          "scan: 1 ROM nova (o .txt e' ignorado)");
    romdb::RomRecord scanned;
    check(db.Search("jogo", "").size() == 1 && db.Search("jogo", "").front().path == "cartuchos/jogo.rom",
          "scan: caminho relativo a pasta de ROMs");
    (void)scanned;

    db.Delete(rec.id, err);
    check(!db.Get(rec.id, again), "Delete: ROM removida");

    // --- verify (fwmsx --romdb verify): SHA-1 atual do arquivo x SHA-1 no banco ---------
    {
        const fs::path vtmp = tmp / "verify";
        fs::create_directories(vtmp / "cartuchos");
        const std::string content_ok = "conteudo que nao muda";
        const std::string content_original = "conteudo original";
        WriteText(vtmp / "cartuchos" / "ok.rom", content_ok);
        WriteText(vtmp / "cartuchos" / "alterado.rom", content_original);

        romdb::RomDb vdb;
        std::string verr;
        check(vdb.Open((vtmp / "roms.db").string(), verr), "verify: banco aberto");
        int64_t vadded = 0, vupdated = 0;
        check(vdb.ScanDirectory(vtmp.string(), vtmp.string(), "scan", vadded, vupdated, verr) && vadded == 2,
              "verify: 2 ROMs cadastradas (ok.rom, alterado.rom)");

        check(romdb::RunRomDbCommand({"verify", "--roms", vtmp.string()}, "fwmsx") == 0,
              "verify: codigo 0 quando tudo bate ainda");

        // Adultera um arquivo DEPOIS de cadastrado, e apaga o outro -- os dois tem que ser pegos.
        WriteText(vtmp / "cartuchos" / "alterado.rom", "conteudo DIFERENTE do cadastrado");
        fs::remove(vtmp / "cartuchos" / "ok.rom", ec);
        check(romdb::RunRomDbCommand({"verify", "--roms", vtmp.string()}, "fwmsx") == 1,
              "verify: codigo 1 com um arquivo alterado e outro faltando");

        // Restaura os dois certinho: verify volta a dar codigo 0.
        WriteText(vtmp / "cartuchos" / "ok.rom", content_ok);
        WriteText(vtmp / "cartuchos" / "alterado.rom", content_original);
        check(romdb::RunRomDbCommand({"verify", "--roms", vtmp.string()}, "fwmsx") == 0,
              "verify: codigo 0 de novo depois de restaurar os arquivos");
    }

    fs::remove_all(tmp, ec);
    std::printf("\nromdbtest: %s (%d falha(s))\n", g_failures == 0 ? "OK" : "FALHOU", g_failures);
    return g_failures == 0 ? 0 : 1;
}
