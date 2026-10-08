// fwMSX -- linha de comando do banco de fitas. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "cli.h"

#include <cstdlib>
#include <iostream>

#include "service.h"
#include "store/tapedb.h"

namespace tapedb {
namespace {

void PrintTape(const TapeRecord &t) {
    std::cout << t.id << "\t" << t.format << "\t" << t.sha1.substr(0, 12) << "\t"
              << (t.title.empty() ? "(sem titulo)" : t.title) << "\t" << t.company << "\t" << t.year << "\t"
              << t.path << std::endl;
}

void PrintTapeDetail(const TapeRecord &t) {
    std::cout << "id:      " << t.id << "\n"
              << "sha1:    " << t.sha1 << "\n"
              << "tamanho: " << t.size << " bytes\n"
              << "formato: " << t.format << "\n"
              << "titulo:  " << t.title << "\n"
              << "empresa: " << t.company << "\n"
              << "ano:     " << t.year << "\n"
              << "arquivo: " << t.path << "\n"
              << "origem:  " << t.source << "\n"
              << "notas:   " << t.notes << std::endl;
}

std::string Option(const std::vector<std::string> &args, size_t &i) {
    if (i + 1 >= args.size()) return "";
    return args[++i];
}

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
    std::cerr << "fwmsx --fitadb: " << message << std::endl;
    return 1;
}

void PrintHelp() {
    std::cout << "fwmsx --fitadb <comando> [opcoes]\n"
                 "\n"
                 "Pasta de fitas: fitas/ ao lado do executavel (ou --fitas <pasta>). Banco:\n"
                 "fitas/fitas.db. So' CADASTRA metadados de fitas que voce ja' tem no disco --\n"
                 "sem download nenhum (o site de referencia nao publica termos de uso ainda,\n"
                 "ver doc/SPEC.md, secao 5.2, item 7).\n"
                 "\n"
                 "  scan <pasta> [--origem x]   le e cadastra as fitas de uma pasta (.cas/.tsx/.tzx, pelo SHA-1)\n"
                 "  add <arquivo> [--titulo t] [--empresa e] [--ano a] [--notas t]\n"
                 "  list                        lista as fitas cadastradas\n"
                 "  search <texto>              busca por titulo, empresa, ano, SHA-1, caminho ou notas\n"
                 "  show <id|sha1>               mostra uma fita\n"
                 "  edit <id> [--titulo t] [--empresa e] [--ano a] [--notas t]\n"
                 "  del <id>                     remove a fita do banco (o arquivo nao e' apagado)\n"
                 "  stats                        contagem de fitas cadastradas\n"
              << std::endl;
}

} // namespace

int RunTapeDbCommand(const std::vector<std::string> &args_in, const std::string &argv0) {
    // --fitas <pasta> pode vir em qualquer lugar.
    std::vector<std::string> args;
    TapeDbPaths paths = DefaultTapeDbPaths(argv0);
    for (size_t i = 0; i < args_in.size(); ++i) {
        if (args_in[i] == "--fitas" && i + 1 < args_in.size()) {
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
    TapeDb db;
    std::string error;

    if (cmd == "scan" || cmd == "add" || cmd == "list" || cmd == "search" || cmd == "show" || cmd == "edit" ||
        cmd == "del" || cmd == "stats") {
        if (!OpenTapeDb(paths, db, error)) return Fail(error);
    }

    if (cmd == "scan") {
        const std::string dir = Positional(args, 1);
        if (dir.empty()) return Fail("informe a pasta");
        std::string origem = "scan";
        for (size_t i = 1; i < args.size(); ++i)
            if (args[i] == "--origem") origem = Option(args, i);
        int64_t added = 0, updated = 0;
        if (!db.ScanDirectory(dir, paths.root, origem, added, updated, error)) return Fail(error);
        std::cout << added << " nova(s), " << updated << " ja' cadastrada(s)." << std::endl;
        return 0;
    }

    if (cmd == "add") {
        const std::string file = Positional(args, 1);
        if (file.empty()) return Fail("informe o arquivo");
        TapeRecord rec;
        if (!db.ScanFile(file, paths.root, "manual", rec, error)) return Fail(error);
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i] == "--titulo") rec.title = Option(args, i);
            else if (args[i] == "--empresa") rec.company = Option(args, i);
            else if (args[i] == "--ano") rec.year = Option(args, i);
            else if (args[i] == "--notas") rec.notes = Option(args, i);
        }
        if (!db.Update(rec, error)) return Fail(error);
        PrintTapeDetail(rec);
        return 0;
    }

    if (cmd == "list" || cmd == "search") {
        std::string text;
        if (cmd == "search") text = Positional(args, 1);
        const std::vector<TapeRecord> tapes = db.Search(text);
        for (const TapeRecord &t : tapes) PrintTape(t);
        std::cout << tapes.size() << " fita(s)." << std::endl;
        return 0;
    }

    if (cmd == "show") {
        const std::string key = Positional(args, 1);
        TapeRecord rec;
        const bool found = !key.empty() && key.find_first_not_of("0123456789") == std::string::npos
                               ? db.Get(std::atoll(key.c_str()), rec)
                               : db.FindBySha1(key, rec);
        if (!found) return Fail("fita nao encontrada");
        PrintTapeDetail(rec);
        return 0;
    }

    if (cmd == "edit") {
        const std::string key = Positional(args, 1);
        if (key.empty()) return Fail("informe o id");
        TapeRecord rec;
        if (!db.Get(std::atoll(key.c_str()), rec)) return Fail("fita nao encontrada");
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i] == "--titulo") rec.title = Option(args, i);
            else if (args[i] == "--empresa") rec.company = Option(args, i);
            else if (args[i] == "--ano") rec.year = Option(args, i);
            else if (args[i] == "--notas") rec.notes = Option(args, i);
        }
        if (!db.Update(rec, error)) return Fail(error);
        PrintTapeDetail(rec);
        return 0;
    }

    if (cmd == "del") {
        const std::string key = Positional(args, 1);
        if (key.empty()) return Fail("informe o id");
        if (!db.Delete(std::atoll(key.c_str()), error)) return Fail(error);
        std::cout << "fita " << key << " removida do banco (o arquivo nao foi apagado)." << std::endl;
        return 0;
    }

    if (cmd == "stats") {
        std::cout << "Fitas cadastradas: " << db.Count() << std::endl;
        return 0;
    }

    std::cerr << "fwmsx --fitadb: comando desconhecido '" << cmd << "'\n";
    PrintHelp();
    return 2;
}

} // namespace tapedb
