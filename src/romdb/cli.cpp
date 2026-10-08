// fwMSX -- linha de comando do banco de ROMs. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "cli.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>

#include "core/hash.h"
#include "service.h"
#include "store/romdb.h"

namespace romdb {
namespace {

namespace fs = std::filesystem;

// Nome para mostrar: o nome do jogo, ou o nome do arquivo quando nao ha nome.
std::string DisplayName(const RomRecord &r) {
    if (!r.name.empty()) return r.name;
    const size_t slash = r.path.find_last_of("/\\");
    return slash == std::string::npos ? r.path : r.path.substr(slash + 1);
}

// `rec.path` fica RELATIVO a' pasta de ROMs quando o arquivo esta' dentro
// dela (ver RomDb::ScanFile()/RelativeTo() em store/romdb.cpp); senao e' o
// caminho completo original. Reconstroi o caminho de verdade para abrir o
// arquivo (usado por `verify`).
std::string ResolveRomPath(const RomPaths &paths, const RomRecord &rec) {
    const fs::path p(rec.path);
    return p.is_absolute() ? rec.path : (fs::path(paths.root) / p).string();
}

void PrintRom(const RomRecord &r) {
    std::cout << r.id << "\t" << r.category << "\t" << (r.mapper >= 0 ? std::to_string(r.mapper) : "-") << "\t"
              << r.sha1.substr(0, 12) << "\t" << DisplayName(r) << "\t" << r.hardware << "\t" << r.path << std::endl;
}

void PrintRomDetail(const RomRecord &r) {
    std::cout << "id:        " << r.id << "\n"
              << "sha1:      " << r.sha1 << "\n"
              << "crc32:     " << r.crc32 << "\n"
              << "tamanho:   " << r.size << " bytes\n"
              << "nome:      " << r.name << "\n"
              << "categoria: " << r.category << "\n"
              << "hardware:  " << r.hardware << "\n"
              << "mapper:    " << (r.mapper >= 0 ? std::to_string(r.mapper) : std::string("-")) << "\n"
              << "arquivo:   " << r.path << "\n"
              << "origem:    " << r.source << "\n"
              << "notas:     " << r.notes << std::endl;
}

// Valor de uma opcao "--nome valor" a partir de `i`; avanca `i`. Vazio se faltar.
std::string Option(const std::vector<std::string> &args, size_t &i) {
    if (i + 1 >= args.size()) return "";
    return args[++i];
}

// Coleta o texto sem opcoes ("--x valor") e devolve o primeiro argumento posicional.
std::string Positional(const std::vector<std::string> &args, size_t from) {
    for (size_t i = from; i < args.size(); ++i) {
        if (args[i].rfind("--", 0) == 0) {
            ++i;
            continue;
        }
        return args[i];
    }
    return "";
}

int Fail(const std::string &message) {
    std::cerr << "fwmsx --romdb: " << message << std::endl;
    return 1;
}

void PrintHelp() {
    std::cout << "fwmsx --romdb <comando> [opcoes]\n"
                 "\n"
                 "Pasta de ROMs: roms/ ao lado do executavel (ou --roms <pasta>). Banco: roms/roms.db.\n"
                 "\n"
                 "Downloads:\n"
                 "  fmsx                        baixa o fMSX 6.0 para Windows e separa as ROMs por tipo\n"
                 "  filehunter [caminho]        lista a pasta System ROMs do file-hunter (ou uma subpasta)\n"
                 "  filehunter-full [--data DD-MM-AAAA]   baixa o Full Set System ROMs (o mais recente se omitido)\n"
                 "  filehunter-get <url>        baixa um arquivo ou ZIP do file-hunter (URL da listagem)\n"
                 "  vampier                     baixa e importa o banco do Vampier (msxromdb)\n"
                 "\n"
                 "Banco:\n"
                 "  scan <pasta> [--origem x]   le e cadastra as ROMs de uma pasta (pelo SHA-1)\n"
                 "  cartsha <arquivo>           importa o CARTS.SHA do fMSX (mapper por SHA-1)\n"
                 "  add <arquivo> [--cat c] [--hw h] [--nome n] [--notas t]\n"
                 "  list [--cat c]              lista as ROMs\n"
                 "  search <texto> [--cat c]    busca por nome, SHA-1, hardware, notas ou caminho\n"
                 "  show <id|sha1>              mostra uma ROM\n"
                 "  edit <id> [--cat c] [--hw h] [--nome n] [--mapper n] [--notas t]\n"
                 "  del <id>                    remove a ROM do banco (o arquivo nao e' apagado)\n"
                 "  vsearch <texto>             busca no banco do Vampier (jogo, ROM, SHA-1)\n"
                 "  identify                    preenche os nomes que o Vampier conhece\n"
                 "  verify [--cat c]            recalcula o SHA-1 de cada ROM e confere contra o banco\n"
                 "                              (achou faltando/alterada: codigo de saida 1)\n"
                 "  stats                       contagem por categoria e estado dos bancos\n"
                 "\n"
                 "Categorias: bios, interface, cartucho, disco, tabela, outro.\n"
              << std::endl;
}

} // namespace

int RunRomDbCommand(const std::vector<std::string> &args_in, const std::string &argv0) {
    // --roms <pasta> pode vir em qualquer lugar.
    std::vector<std::string> args;
    RomPaths paths = DefaultRomPaths(argv0);
    for (size_t i = 0; i < args_in.size(); ++i) {
        if (args_in[i] == "--roms" && i + 1 < args_in.size()) {
            paths.root = args_in[++i];
        } else {
            args.push_back(args_in[i]);
        }
    }
    if (args.empty() || args[0] == "help" || args[0] == "--help") {
        PrintHelp();
        return 0;
    }

    const std::string cmd = args[0];
    RomDb db;
    std::string error;
    auto log = [](const std::string &m) { std::cout << m << std::endl; };

    // Comandos que nao precisam do banco aberto ainda.
    if (cmd == "fmsx" || cmd == "filehunter" || cmd == "filehunter-full" || cmd == "filehunter-get" ||
        cmd == "vampier" || cmd == "scan" || cmd == "cartsha" || cmd == "add" || cmd == "list" || cmd == "search" ||
        cmd == "show" || cmd == "edit" || cmd == "del" || cmd == "vsearch" || cmd == "identify" || cmd == "stats" ||
        cmd == "verify") {
        if (!OpenRomDb(paths, db, error)) return Fail(error);
    }

    if (cmd == "fmsx") {
        int64_t added = 0;
        if (!DownloadFmsx(paths, db, log, added, error)) return Fail(error);
        std::cout << "fMSX: " << added << " arquivo(s) guardado(s) em " << paths.root << std::endl;
        return 0;
    }

    if (cmd == "filehunter") {
        const std::string sub = Positional(args, 1);
        const std::string url = sub.empty() ? kFileHunterSystemRoms : std::string(kFileHunterSystemRoms) + sub;
        std::vector<ListingEntry> entries;
        if (!ListFileHunter(url, entries, error)) return Fail(error);
        for (const ListingEntry &e : entries) {
            std::cout << (e.is_dir ? "[pasta] " : "        ") << e.name;
            if (!e.size.empty()) std::cout << "  (" << e.size << ")";
            std::cout << "\n    " << e.url << "\n";
        }
        return 0;
    }

    if (cmd == "filehunter-full") {
        std::string date;
        for (size_t i = 1; i < args.size(); ++i)
            if (args[i] == "--data") date = Option(args, i);
        int64_t added = 0;
        if (!DownloadFileHunterFullSet(paths, db, date, log, added, error)) return Fail(error);
        std::cout << "file-hunter: " << added << " ROM(s) registrada(s)." << std::endl;
        return 0;
    }

    if (cmd == "filehunter-get") {
        const std::string url = Positional(args, 1);
        if (url.empty()) return Fail("informe a URL do arquivo (veja 'filehunter')");
        int64_t added = 0;
        if (!DownloadFileHunterEntry(paths, db, url, log, added, error)) return Fail(error);
        std::cout << "file-hunter: " << added << " ROM(s) registrada(s)." << std::endl;
        return 0;
    }

    if (cmd == "vampier") {
        int64_t rows = 0;
        if (!ImportVampier(paths, db, log, rows, error)) return Fail(error);
        std::cout << "Vampier: " << rows << " ROM(s) no banco de referencia." << std::endl;
        return 0;
    }

    if (cmd == "scan") {
        const std::string dir = Positional(args, 1);
        if (dir.empty()) return Fail("informe a pasta");
        std::string origem = "scan";
        for (size_t i = 1; i < args.size(); ++i)
            if (args[i] == "--origem") origem = Option(args, i);
        int64_t added = 0, updated = 0;
        if (!db.ScanDirectory(dir, paths.root, origem, added, updated, error)) return Fail(error);
        std::cout << added << " nova(s), " << updated << " ja cadastrada(s)." << std::endl;
        return 0;
    }

    if (cmd == "cartsha") {
        const std::string file = Positional(args, 1);
        if (file.empty()) return Fail("informe o arquivo CARTS.SHA");
        int64_t rows = 0;
        if (!db.ImportCartsSha(file, rows, error)) return Fail(error);
        std::cout << rows << " linha(s) importada(s)." << std::endl;
        return 0;
    }

    if (cmd == "add") {
        const std::string file = Positional(args, 1);
        if (file.empty()) return Fail("informe o arquivo");
        RomRecord rec;
        if (!db.ScanFile(file, paths.root, "manual", rec, error)) return Fail(error);
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i] == "--cat") rec.category = Option(args, i);
            else if (args[i] == "--hw") rec.hardware = Option(args, i);
            else if (args[i] == "--nome") rec.name = Option(args, i);
            else if (args[i] == "--notas") rec.notes = Option(args, i);
        }
        if (!db.Update(rec, error)) return Fail(error);
        PrintRomDetail(rec);
        return 0;
    }

    if (cmd == "list" || cmd == "search") {
        std::string text, category;
        if (cmd == "search") text = Positional(args, 1);
        for (size_t i = 1; i < args.size(); ++i)
            if (args[i] == "--cat") category = Option(args, i);
        const std::vector<RomRecord> roms = db.Search(text, category);
        for (const RomRecord &r : roms) PrintRom(r);
        std::cout << roms.size() << " ROM(s)." << std::endl;
        return 0;
    }

    if (cmd == "show") {
        const std::string key = Positional(args, 1);
        RomRecord rec;
        const bool found = !key.empty() && key.find_first_not_of("0123456789") == std::string::npos
                               ? db.Get(std::atoll(key.c_str()), rec)
                               : db.FindBySha1(key, rec);
        if (!found) return Fail("ROM nao encontrada");
        PrintRomDetail(rec);
        return 0;
    }

    if (cmd == "edit") {
        const std::string key = Positional(args, 1);
        if (key.empty()) return Fail("informe o id");
        RomRecord rec;
        if (!db.Get(std::atoll(key.c_str()), rec)) return Fail("ROM nao encontrada");
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i] == "--cat") rec.category = Option(args, i);
            else if (args[i] == "--hw") rec.hardware = Option(args, i);
            else if (args[i] == "--nome") rec.name = Option(args, i);
            else if (args[i] == "--notas") rec.notes = Option(args, i);
            else if (args[i] == "--mapper") rec.mapper = std::atoi(Option(args, i).c_str());
        }
        if (!db.Update(rec, error)) return Fail(error);
        PrintRomDetail(rec);
        return 0;
    }

    if (cmd == "del") {
        const std::string key = Positional(args, 1);
        if (key.empty()) return Fail("informe o id");
        if (!db.Delete(std::atoll(key.c_str()), error)) return Fail(error);
        std::cout << "ROM " << key << " removida do banco (o arquivo nao foi apagado)." << std::endl;
        return 0;
    }

    if (cmd == "vsearch") {
        const std::string text = Positional(args, 1);
        if (!db.HasVampier()) return Fail("banco do Vampier nao importado (fwmsx --romdb vampier)");
        const std::vector<VampierHit> hits = db.VampierSearch(text);
        for (const VampierHit &h : hits) {
            std::cout << h.sha1.substr(0, 12) << "\t" << h.game << "\t" << h.year << "\t" << h.company << "\t"
                      << h.rom_type << "\t" << h.dump << "\t" << h.platform << "\t" << h.crc32 << "\t"
                      << h.file_size << std::endl;
        }
        std::cout << hits.size() << " resultado(s)." << std::endl;
        return 0;
    }

    if (cmd == "identify") {
        const int64_t named = db.IdentifyWithVampier(error);
        if (!error.empty()) return Fail(error);
        std::cout << named << " ROM(s) receberam nome pelo banco do Vampier." << std::endl;
        return 0;
    }

    if (cmd == "stats") {
        std::cout << "ROMs cadastradas: " << db.Count() << "\n";
        for (const char *c : {"bios", "interface", "cartucho", "disco", "tabela", "outro"}) {
            std::cout << "  " << c << ": " << db.Search("", c).size() << "\n";
        }
        std::cout << "Vampier importado: " << (db.HasVampier() ? "sim" : "nao") << "\n";
        return 0;
    }

    if (cmd == "verify") {
        // Recalcula o SHA-1 de cada ROM cadastrada e confere contra o que
        // esta' gravado no banco -- detecta arquivo faltando ou alterado
        // desde que foi cadastrado (ver doc/romdb-spec.md, secao 8:
        // "Verificar as ROMs baixadas contra o SHA-1 conhecido ... e marcar
        // as que batem").
        std::string category;
        for (size_t i = 1; i < args.size(); ++i)
            if (args[i] == "--cat") category = Option(args, i);
        const std::vector<RomRecord> roms = db.Search("", category);
        int64_t ok = 0, divergente = 0, faltando = 0;
        for (const RomRecord &r : roms) {
            const std::string full_path = ResolveRomPath(paths, r);
            std::vector<uint8_t> bytes;
            if (!ReadWholeFile(full_path, bytes)) {
                std::cout << "FALTANDO    " << r.id << "\t" << DisplayName(r) << "\t" << full_path << std::endl;
                ++faltando;
                continue;
            }
            const std::string sha1 = Sha1Hex(bytes.data(), bytes.size());
            if (sha1 == r.sha1) {
                ++ok;
            } else {
                std::cout << "DIVERGENTE  " << r.id << "\t" << DisplayName(r) << "\t" << full_path << std::endl;
                ++divergente;
            }
        }
        std::cout << ok << " ok, " << divergente << " divergente(s), " << faltando << " faltando, de " << roms.size()
                   << " no total." << std::endl;
        return (divergente == 0 && faltando == 0) ? 0 : 1;
    }

    std::cerr << "fwmsx --romdb: comando desconhecido '" << cmd << "'\n";
    PrintHelp();
    return 2;
}

} // namespace romdb
