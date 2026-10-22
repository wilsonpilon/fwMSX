// fwMSX -- teste dos downloads (sem internet: o indice vem de texto, o Fetch baixa um arquivo local por file://). Ver doc/download-spec.md.
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <set>

#include "../../src/download/c/dl_match.h"
#include "../../src/download/cpp/download.h"
#include "../../src/romdb/net/web.h"

namespace fs = std::filesystem;

static int g_failures = 0;

static void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

static void spit(const fs::path &p, const std::string &d) {
    fs::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    f.write(d.data(), static_cast<std::streamsize>(d.size()));
}

static std::string slurp(const fs::path &p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

static std::string FileUrl(const fs::path &p) {
    std::string g = fs::absolute(p).generic_string();
    return "file://" + std::string(g.front() == '/' ? "" : "/") + g;
}

int main() {
    // ---- C: busca sem diferenciar caixa ----
    check(dl_contains_ci("Games/Konami/Nemesis 2.rom", 26, "NEMESIS", 7) == 1 && dl_contains_ci("abc", 3, "abcd", 4) == 0 && dl_contains_ci("abc", 3, "", 0) == 1 &&
              dl_contains_ci("abcabd", 6, "abd", 3) == 1 && dl_contains_ci("ABC", 3, "b", 1) == 1 && dl_contains_ci("abc", 3, "d", 1) == 0,
          "dl_contains_ci");

    // ---- indice: BOM, CRLF, barra invertida ----
    const std::string txt = std::string("\xEF\xBB\xBF") + "download.file-hunter.com - 24-06-2026.torrent\r\nBooks\\NL\\Terminal Graphics.pdf\r\nGames\\Konami\\Nemesis.zip\r\n"
                                                         "Games\\Konami\\Nemesis 2.zip\r\nGames\\Compile\\Zanac.zip\r\n\r\nGames\\Konami\\Sub\\Deep.rom\r\nManuals\\VDP.pdf\r\n";
    dl::Index idx;
    check(idx.Parse(txt) == 7 && idx.path(0) == "download.file-hunter.com - 24-06-2026.torrent" && idx.path(1) == "Books/NL/Terminal Graphics.pdf", "Index::Parse: tira o BOM e o CR, '\\' vira '/', pula linhas vazias");
    std::vector<std::string> folders;
    std::vector<size_t> files;
    idx.List("", folders, files);
    check(folders == std::vector<std::string>({"Books", "Games", "Manuals"}) && files.size() == 1 && idx.path(files[0]).rfind("download.file-hunter.com", 0) == 0, "List(raiz): subpastas ordenadas e os arquivos soltos");
    idx.List("Games/Konami", folders, files);
    check(folders == std::vector<std::string>({"Sub"}) && files.size() == 2 && idx.path(files[0]) == "Games/Konami/Nemesis 2.zip" && idx.path(files[1]) == "Games/Konami/Nemesis.zip",
          "List(pasta): so' os filhos imediatos, arquivos ordenados pelo nome");
    idx.List("Games/Kon", folders, files);
    check(folders.empty() && files.empty(), "List: 'Games/Kon' nao e' a pasta 'Games/Konami'");
    bool trunc = true;
    std::vector<size_t> hit = idx.Search("nemesis zip", 10, &trunc);
    check(hit.size() == 2 && !trunc, "Search: todas as palavras, sem diferenciar caixa");
    check(idx.Search("NEMESIS", 10).size() == 2 && idx.Search("nemesis 2", 10).size() == 1 && idx.Search("", 10).empty() && idx.Search("xyzzy", 10).empty(), "Search: caixa, duas palavras, vazio e sem resultado");
    hit = idx.Search("games", 2, &trunc);
    check(hit.size() == 2 && trunc, "Search: o limite corta e avisa");

    // ---- URL e caminhos ----
    check(dl::FileHunterUrl("Games/Konami/Nemesis 2.zip") == "https://download.file-hunter.com/Games/Konami/Nemesis%202.zip", "FileHunterUrl codifica o nome");
    check(dl::FileHunterUrl("a/b\";rm -rf $HOME`x`&.zip").find_first_of("\";$`& ") == std::string::npos, "FileHunterUrl: nenhum caractere de shell passa cru");
    check(dl::SafeRelative("konamiman/nbasic/NBASIC.BIN") && !dl::SafeRelative("../x") && !dl::SafeRelative("a/../x") && !dl::SafeRelative("/abs") && !dl::SafeRelative("C:/x") &&
              !dl::SafeRelative("a\\b") && !dl::SafeRelative("a//b") && !dl::SafeRelative("") && !dl::SafeRelative(".part/x") && !dl::SafeRelative("a/b/"),
          "SafeRelative recusa .., absoluto, unidade, barra invertida, vazio e .part");
    check(dl::LocalComponent("a:b*c?.zip") == "a_b_c_.zip" && dl::LocalComponent("x \"y\"|z") == "x _y__z" && dl::LocalComponent("...") == "_" && dl::LocalComponent("ok.rom") == "ok.rom", "LocalComponent troca o que o Windows nao aceita");

    // ---- catalogo ----
    bool cat_ok = !dl::Catalog().empty();
    bool nbasic_install = false;
    std::set<std::string> ids;
    for (const dl::Entry &e : dl::Catalog()) {
        cat_ok = cat_ok && !e.id.empty() && ids.insert(e.id).second && !e.items.empty();
        for (const dl::Item &it : e.items) {
            cat_ok = cat_ok && dl::SafeRelative(it.dest) && it.url.rfind("https://", 0) == 0 && (it.install.empty() || dl::SafeRelative(it.install));
            if (it.name == "NBASIC.BIN" && it.install == "badig/NBASIC.BIN") nbasic_install = true;
        }
    }
    check(cat_ok && ids.count("nbasic") && ids.count("see") && ids.count("nplayer"), "Catalog: ids unicos, URLs https e destinos seguros");
    check(nbasic_install, "o NBASIC.BIN do catalogo tambem e' instalado em badig/");

    // ---- Fetch, com um arquivo local no lugar da rede (precisa do curl; sem ele, pula) ----
    const fs::path dir = fs::temp_directory_path() / "fwmsx_download_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    spit(dir / "origem" / "arq.bin", "CONTEUDO");
    std::string probe, perr;
    if (!romdb::HttpGetText(FileUrl(dir / "origem" / "arq.bin"), probe, perr) || probe != "CONTEUDO") {
        std::printf("[SKIP] sem curl (ou sem file://): Fetch nao testado\n");
    } else {
        dl::Paths paths;
        paths.exe_dir = (dir / "exe").generic_string();
        std::string out, err;
        bool skipped = true;
        check(dl::Fetch(paths, FileUrl(dir / "origem" / "arq.bin"), "konamiman/x/arq.bin", "badig/ARQ.BIN", false, out, skipped, err) && !skipped &&
                  slurp(dir / "exe" / "downloads" / "konamiman" / "x" / "arq.bin") == "CONTEUDO" && slurp(dir / "exe" / "badig" / "ARQ.BIN") == "CONTEUDO",
              "Fetch baixa para downloads/ e copia para o 'install'");
        check(!fs::exists(dir / "exe" / "downloads" / ".part" / "1.part"), "o temporario .part some");
        spit(dir / "exe" / "downloads" / "konamiman" / "x" / "arq.bin", "ANTIGO");
        check(dl::Fetch(paths, FileUrl(dir / "origem" / "arq.bin"), "konamiman/x/arq.bin", "", false, out, skipped, err) && skipped &&
                  slurp(dir / "exe" / "downloads" / "konamiman" / "x" / "arq.bin") == "ANTIGO",
              "Fetch nao baixa de novo o que ja' existe (skipped)");
        check(dl::Fetch(paths, FileUrl(dir / "origem" / "arq.bin"), "konamiman/x/arq.bin", "", true, out, skipped, err) && !skipped &&
                  slurp(dir / "exe" / "downloads" / "konamiman" / "x" / "arq.bin") == "CONTEUDO",
              "Fetch com overwrite troca o arquivo");
        check(!dl::Fetch(paths, FileUrl(dir / "origem" / "nao_existe.bin"), "konamiman/x/falha.bin", "", false, out, skipped, err) && !err.empty() &&
                  !fs::exists(dir / "exe" / "downloads" / "konamiman" / "x" / "falha.bin"),
              "Fetch de uma origem que nao existe falha sem deixar arquivo");
        check(!dl::Fetch(paths, FileUrl(dir / "origem" / "arq.bin"), "../fora.bin", "", false, out, skipped, err) && !fs::exists(dir / "exe" / "fora.bin"), "Fetch recusa um destino com '..'");
        // o indice em disco
        spit(paths.IndexCache(), std::string(1500, '#') + "\r\nGames\\A.zip\r\n");
        bool fresh = true;
        dl::Index loaded;
        check(dl::EnsureIndex(paths, false, fresh, err) && !fresh && dl::LoadIndex(paths, loaded, err) && loaded.size() == 2, "EnsureIndex nao baixa de novo um indice que ja' existe; LoadIndex le");
        dl::Index none;
        dl::Paths empty;
        empty.exe_dir = (dir / "vazio").generic_string();
        check(!dl::LoadIndex(empty, none, err) && err.find("ainda nao") != std::string::npos, "LoadIndex sem o arquivo explica");
    }
    check(dl::RunDownloadCommand({}, dir.generic_string()) != 0 && dl::RunDownloadCommand({"catalog"}, dir.generic_string()) == 0 && dl::RunDownloadCommand({"get", "nao_existe"}, dir.generic_string()) != 0 &&
              dl::RunDownloadCommand({"search", "x"}, (dir / "vazio").generic_string()) != 0,
          "RunDownloadCommand: sem argumento e id desconhecido falham, catalog funciona, search sem indice explica");
    fs::remove_all(dir);
    std::printf("\n%s (%d falha%s)\n", g_failures == 0 ? "TODOS OS TESTES PASSARAM" : "HA' FALHAS", g_failures, g_failures == 1 ? "" : "s");
    return g_failures == 0 ? 0 : 1;
}
