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
#include "romdb/service.h"
#include "romdb/sig/sig.h"
#include "romdb/store/classify.h"
#include "sqlite3.h"
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

    // Dump do Vampier: tabelas msxdb_* com jogo, empresa e ROM -- colunas (e nomes) iguais ao
    // dump real (conferido baixando sql-msxromdb.zip em 2026-10-08: msxdb_romdetails tem CRC32 e
    // FileSize, msxdb_rominfo tem Platform -- nenhum dos dois estava no fixture antigo deste teste).
    const fs::path vam = tmp / "sql-romdb.sql";
    WriteText(vam,
              "CREATE TABLE msxdb_romdetails (`HashID` INTEGER, `GameID` INTEGER, `RomType` VARCHAR(1020),"
              " `SHA1` VARCHAR(1020), `Dump` VARCHAR(1020), `Remark` VARCHAR(1020), `CRC32` VARCHAR(32),"
              " `FileSize` INTEGER);\n"
              "CREATE TABLE msxdb_rominfo (`GameID` INTEGER, `GameName` VARCHAR(1020), `Year` VARCHAR(16),"
              " `CompanyID1` INTEGER, `Platform` VARCHAR(1020));\n"
              "CREATE TABLE msxdb_company (`CompanyID` INTEGER, `ShortName` VARCHAR(400));\n"
              "BEGIN;\n"
              "INSERT INTO msxdb_company VALUES ('276','Konami');\n"
              "INSERT INTO msxdb_rominfo VALUES ('7','Nemesis','1986','276','MSX');\n"
              "INSERT INTO msxdb_rominfo VALUES ('8','Sem Dados','','276',NULL);\n"
              "INSERT INTO msxdb_romdetails VALUES ('1','7','Normal','A9993E364706816ABA3E25717850C26C9CD0D89D','GoodMSX','','4E20D256','32768');\n"
              "INSERT INTO msxdb_romdetails VALUES ('2','8','Normal','0000000000000000000000000000000000000000','','',NULL,NULL);\n"
              "COMMIT;\n");
    rows = 0;
    check(db.ImportVampierSql(vam.string(), rows, err) && rows == 2, "Vampier: dump importado (2 ROMs)");
    check(db.HasVampier(), "Vampier: presente no banco");
    const auto hits = db.VampierSearch("Nemesis");
    check(hits.size() == 1 && hits[0].company == "Konami" && hits[0].year == "1986", "Vampier: busca pelo nome do jogo");
    check(hits[0].platform == "MSX" && hits[0].crc32 == "4E20D256" && hits[0].file_size == 32768,
          "Vampier: platform/CRC32/FileSize (ja' no dump SQL, so' nao eram lidos) -- 1.26.0");
    const auto hits2 = db.VampierSearch("Sem Dados");
    check(hits2.size() == 1 && hits2[0].platform.empty() && hits2[0].crc32.empty() && hits2[0].file_size == -1,
          "Vampier: platform/CRC32/FileSize NULL no dump viram vazio/-1, sem travar -- 1.26.0");
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

    // --- assinaturas do openMSX (src/romdb/sig) ------------------------------------------------------------------
    {
        const std::string machine_xml =
            "<?xml version=\"1.0\" ?>\n<!DOCTYPE msxconfig SYSTEM 'msxconfig2.dtd'>\n<msxconfig>\n"
            "  <info><manufacturer>Philips</manufacturer><code>NMS 8250</code><type>MSX2</type></info>\n"
            "  <devices><primary slot=\"0\">\n"
            "    <ROM id=\"MSX BIOS with BASIC ROM\"><mem base=\"0x0000\" size=\"0x8000\"/>\n"
            "      <rom><sha1>6103B39F1E38D1AA2D84B1C3219C44F1ABB5436E</sha1><filename>nms8250_basic-bios2.rom</filename></rom></ROM>\n"
            "    <ROM id=\"MSX Sub ROM\"><rom><sha1>5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02</sha1> <!-- verificada --> <filename>nms8250_msx2sub.rom</filename></rom></ROM>\n"
            "    <PSG id=\"PSG\"><type>YM2149</type></PSG>\n"
            "  </primary></devices></msxconfig>\n";
        // "abc" tem o SHA-1 a9993e36...: e' a ROM "que existe no disco" nos testes abaixo
        const std::string ext_xml =
            "<msxconfig><info><name>MSX-DOS 2</name><manufacturer>ASCII</manufacturer><code/><type>OS ROM</type></info>\n"
            "<devices><primary slot=\"any\"><secondary slot=\"any\"><ROM id=\"MSXDOS2 cart\"><mem base=\"0x4000\" size=\"0x04000\"/>\n"
            "  <rom><sha1>c36c9e0f96738a340381e23b4f97245388801a46</sha1><sha1>a9993e364706816aba3e25717850c26c9cd0d89d</sha1>"
            "<filename>msxdos22.rom</filename></rom><mappertype>MSXDOS2</mappertype></ROM></secondary></primary></devices></msxconfig>\n";
        std::vector<romdb::SigRow> rows;
        romdb::ParseOpenmsxXml(machine_xml, "Philips_NMS_8250", false, rows);
        check(rows.size() == 2 && rows[0].sha1 == "6103b39f1e38d1aa2d84b1c3219c44f1abb5436e" && rows[0].filename == "nms8250_basic-bios2.rom" &&
                  rows[0].role == "MSX BIOS with BASIC ROM" && rows[0].family == "MSX2" && rows[0].hardware == "Philips NMS 8250" && rows[0].source == "machine",
              "openMSX XML de maquina: SHA-1 (minusculo), arquivo, papel (id da ROM), familia e hardware (fabricante + codigo)");
        check(rows[1].role == "MSX Sub ROM" && rows[1].filename == "nms8250_msx2sub.rom", "o comentario entre as tags nao atrapalha; segunda ROM");
        std::vector<romdb::SigRow> ext;
        romdb::ParseOpenmsxXml(ext_xml, "msxdos2", true, ext);
        check(ext.size() == 2 && ext[0].role == "MSX-DOS 2" && ext[0].hardware == "MSX-DOS 2" && ext[0].family == "OS ROM" && ext[0].source == "extension" &&
                  ext[0].filename == "msxdos22.rom" && ext[1].filename == "msxdos22.rom",
              "openMSX XML de extensao: o papel e' o nome da extensao; um <rom> com 2 SHA-1 vira 2 linhas; tag vazia <code/> tratada");

        check(romdb::CategoryForSignature("MSX BIOS with BASIC ROM", "MSX2", "machine", "") == "bios" &&
                  romdb::CategoryForSignature("Memory Mapped FDC", "MSX2", "machine", "") == "interface" &&
                  romdb::CategoryForSignature("MSX-DOS 2", "OS ROM", "extension", "") == "interface" &&
                  romdb::CategoryForSignature("x", "debug tool", "extension", "") == "outro",
              "categoria sugerida pela assinatura");

        const std::string tree = "{\"tree\":[{\"path\": \"share/machines/A.xml\"},{\"path\": \"share/extensions/B C.xml\"},{\"path\": \"src/x.xml\"},"
                                 "{\"path\": \"share/machines/note.txt\"},{\"path\": \"share/machines/../evil.xml\"}]}";
        const std::vector<std::string> paths = romdb::OpenmsxPathsFromTree(tree);
        check(paths.size() == 2 && paths[0] == "share/machines/A.xml" && paths[1] == "share/extensions/B C.xml", "arvore do GitHub: so' os XML de machines/extensions, sem '..'");

        // banco: identificacao pelo SHA-1
        const fs::path sdir = tmp / "sig";
        fs::create_directories(sdir / "share" / "machines");
        fs::create_directories(sdir / "share" / "extensions");
        WriteText(sdir / "share" / "machines" / "Philips_NMS_8250.xml", machine_xml);
        WriteText(sdir / "share" / "extensions" / "msxdos2.xml", ext_xml);
        WriteText(sdir / "roms" / "algo.rom", "abc");
        WriteText(sdir / "roms" / "outra.rom", "xyz");
        romdb::RomDb sdb;
        std::string serr;
        check(sdb.Open((sdir / "roms.db").string(), serr), "assinaturas: banco aberto");
        romdb::RomRecord rec;
        check(sdb.ScanFile((sdir / "roms" / "algo.rom").string(), (sdir / "roms").string(), "scan", rec, serr) &&
                  sdb.ScanFile((sdir / "roms" / "outra.rom").string(), (sdir / "roms").string(), "scan", rec, serr),
              "assinaturas: duas ROMs cadastradas");
        check(sdb.IdentifyWithOpenmsx(serr) == 0 && !serr.empty(), "identificar sem assinaturas importadas: erro explicando o que fazer");
        serr.clear();
        std::vector<romdb::SigRow> all;
        int nfiles = 0;
        check(romdb::ReadOpenmsxShare((sdir / "share").string(), all, nfiles, serr) && nfiles == 2 && all.size() == 4, "ReadOpenmsxShare le machines e extensions");
        check(!romdb::ReadOpenmsxShare((sdir / "nada").string(), all, nfiles, serr), "pasta que nao e' share = erro");
        std::vector<romdb::SigRow> all2;
        serr.clear();
        romdb::ReadOpenmsxShare((sdir / "share").string(), all2, nfiles, serr);
        check(sdb.ReplaceSignatures(all2, serr) && sdb.SignatureCount() == 4, "ReplaceSignatures: 4 SHA-1 distintos");
        check(sdb.IdentifyWithOpenmsx(serr) == 1 && serr.empty(), "so' a ROM que o openMSX conhece e' marcada");
        romdb::RomRecord got;
        check(sdb.FindBySha1("a9993e364706816aba3e25717850c26c9cd0d89d", got) && got.official == "msxdos22.rom" && got.role == "MSX-DOS 2" && got.family == "OS ROM" &&
                  got.hardware == "MSX-DOS 2" && got.category == "interface",
              "ROM identificada: nome oficial, papel, familia, hardware e categoria");
        romdb::RomRecord other;
        check(sdb.FindBySha1("66b27417d37e024c46526c2f6d358a754fc552f3", other) && other.official.empty() && other.role.empty(), "a ROM desconhecida fica como estava");
        check(sdb.Search("MSX-DOS 2", "").size() == 1 && sdb.Search("msxdos22", "").size() == 1 && sdb.Search("OS ROM", "").size() == 1, "a busca acha pelo papel, pelo nome oficial e pela familia");
        check(sdb.SignatureHits("A9993E364706816ABA3E25717850C26C9CD0D89D").size() == 1, "SignatureHits aceita SHA-1 maiusculo");
        // se uma ROM e' usada por 8 maquinas o hardware lista 6 e conta o resto
        std::vector<romdb::SigRow> many;
        for (int i = 0; i < 8; ++i) {
            romdb::SigRow r;
            r.sha1 = "a9993e364706816aba3e25717850c26c9cd0d89d";
            r.filename = "generic.rom";
            r.role = "MSX BIOS with BASIC ROM";
            r.family = "MSX";
            r.hardware = "Maquina " + std::to_string(i);
            r.source = "machine";
            many.push_back(r);
        }
        check(sdb.ReplaceSignatures(many, serr) && sdb.IdentifyWithOpenmsx(serr) == 1 && sdb.FindBySha1("a9993e364706816aba3e25717850c26c9cd0d89d", got) &&
                  got.hardware == "Maquina 0, Maquina 1, Maquina 2, Maquina 3, Maquina 4, Maquina 5 (+2)" && got.category == "bios",
              "hardware: no maximo 6 nomes e o resto contado");

        // banco antigo (sem as colunas da identificacao): Open() acrescenta so' o que falta
        const fs::path old_db = sdir / "antigo.db";
        {
            sqlite3 *raw = nullptr;
            sqlite3_open(old_db.string().c_str(), &raw);
            sqlite3_exec(raw,
                         "CREATE TABLE roms (id INTEGER PRIMARY KEY AUTOINCREMENT, sha1 TEXT NOT NULL UNIQUE, crc32 TEXT NOT NULL DEFAULT '', size INTEGER NOT NULL DEFAULT 0,"
                         " name TEXT NOT NULL DEFAULT '', category TEXT NOT NULL DEFAULT 'outro', hardware TEXT NOT NULL DEFAULT '', mapper INTEGER NOT NULL DEFAULT -1,"
                         " path TEXT NOT NULL DEFAULT '', source TEXT NOT NULL DEFAULT 'manual', notes TEXT NOT NULL DEFAULT '', updated TEXT NOT NULL DEFAULT '');"
                         "INSERT INTO roms (sha1, name) VALUES ('0000000000000000000000000000000000000001', 'Jogo antigo');",
                         nullptr, nullptr, nullptr);
            sqlite3_close(raw);
        }
        romdb::RomDb odb;
        check(odb.Open(old_db.string(), serr) && odb.Search("antigo", "").size() == 1 && odb.Search("antigo", "")[0].role.empty(), "banco antigo: abre e ganha as colunas novas");
        check(odb.Open(old_db.string(), serr), "reabrir o banco ja' migrado nao falha");
    }

    fs::remove_all(tmp, ec);
    std::printf("\nromdbtest: %s (%d falha(s))\n", g_failures == 0 ? "OK" : "FALHOU", g_failures);
    return g_failures == 0 ? 0 : 1;
}
