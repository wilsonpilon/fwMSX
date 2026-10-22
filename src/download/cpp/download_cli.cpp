// fwMSX -- downloads: o comando de linha (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver download.h e doc/download-spec.md.
#include <cstdio>

#include "download.h"

namespace dl {

namespace {

void Usage() {
    std::printf("downloads: baixa arquivos item por item (nada em massa)\n"
                "  catalog               lista os utilitarios (Konamiman: nbasic, see, nplayer)\n"
                "  get <id>              baixa os arquivos de um utilitario para downloads/konamiman/<id>/\n"
                "  index [--force]       baixa o indice do File Hunter (allfiles.txt, ~3,5 MB) se ainda nao existe\n"
                "  ls [pasta]            lista uma pasta do File Hunter (pelo indice, sem acessar o site)\n"
                "  search <palavras>     procura no indice (todas as palavras, sem diferenciar maiusculas)\n"
                "  fetch <caminho>       baixa UM arquivo do File Hunter (o caminho como o ls/search mostram)\n"
                "ver: man download\n");
}

} // namespace

int RunDownloadCommand(const std::vector<std::string> &args, const std::string &exe_dir) {
    Paths paths;
    paths.exe_dir = exe_dir;
    const std::string sub = args.empty() ? "" : args[0];
    if (sub.empty() || sub == "-h" || sub == "--help" || sub == "help") {
        Usage();
        return sub.empty() ? 1 : 0;
    }
    if (sub == "catalog") {
        for (const Entry &e : Catalog()) {
            std::printf("%-8s %s\n         %s\n", e.id.c_str(), e.title.c_str(), e.note.c_str());
            for (const Item &it : e.items) std::printf("           - %s\n", it.name.c_str());
        }
        return 0;
    }
    if (sub == "get") {
        if (args.size() < 2) {
            std::fprintf(stderr, "download: use get <id> (veja catalog)\n");
            return 1;
        }
        for (const Entry &e : Catalog()) {
            if (e.id != args[1]) continue;
            int failed = 0;
            for (const Item &it : e.items) {
                std::string out, err;
                bool skipped = false;
                if (Fetch(paths, it.url, it.dest, it.install, false, out, skipped, err)) {
                    std::printf("%s %s\n", skipped ? "ja existe:" : "baixado:   ", out.c_str());
                } else {
                    std::fprintf(stderr, "ERRO %s: %s\n", it.name.c_str(), err.c_str());
                    ++failed;
                }
            }
            return failed == 0 ? 0 : 1;
        }
        std::fprintf(stderr, "download: '%s' nao esta no catalogo (veja catalog)\n", args[1].c_str());
        return 1;
    }
    if (sub == "index") {
        bool fresh = false;
        std::string err;
        if (!EnsureIndex(paths, args.size() > 1 && args[1] == "--force", fresh, err)) {
            std::fprintf(stderr, "download: %s\n", err.c_str());
            return 1;
        }
        Index idx;
        if (!LoadIndex(paths, idx, err)) {
            std::fprintf(stderr, "download: %s\n", err.c_str());
            return 1;
        }
        std::printf("%s: %zu arquivos (%s)\n", paths.IndexCache().c_str(), idx.size(), fresh ? "baixado agora" : "ja existia");
        return 0;
    }
    Index idx;
    std::string err;
    if (sub == "ls" || sub == "search" || sub == "fetch") {
        if (!LoadIndex(paths, idx, err)) {
            std::fprintf(stderr, "download: %s (rode: download index)\n", err.c_str());
            return 1;
        }
    }
    if (sub == "ls") {
        std::string dir = args.size() > 1 ? args[1] : std::string();
        while (!dir.empty() && dir.back() == '/') dir.pop_back();
        std::vector<std::string> folders;
        std::vector<size_t> files;
        idx.List(dir, folders, files);
        for (const std::string &f : folders) std::printf("%s/\n", f.c_str());
        for (const size_t i : files) std::printf("%s\n", idx.path(i).c_str());
        return 0;
    }
    if (sub == "search") {
        std::string q;
        for (size_t i = 1; i < args.size(); ++i) q += (q.empty() ? "" : " ") + args[i];
        bool truncated = false;
        const std::vector<size_t> hit = idx.Search(q, 100, &truncated);
        for (const size_t i : hit) std::printf("%s\n", idx.path(i).c_str());
        std::printf("%zu resultado(s)%s\n", hit.size(), truncated ? " (so' os 100 primeiros: refine a busca)" : "");
        return 0;
    }
    if (sub == "fetch") {
        if (args.size() < 2) {
            std::fprintf(stderr, "download: use fetch <caminho>\n");
            return 1;
        }
        const std::string path = args[1];
        bool known = false;
        for (size_t i = 0; i < idx.size() && !known; ++i) known = idx.path(i) == path;
        if (!known) {
            std::fprintf(stderr, "download: '%s' nao esta no indice (use search / ls)\n", path.c_str());
            return 1;
        }
        const std::string rel = FileHunterRel(path);
        std::string out;
        bool skipped = false;
        if (!Fetch(paths, FileHunterUrl(path), rel, std::string(), false, out, skipped, err)) {
            std::fprintf(stderr, "download: %s\n", err.c_str());
            return 1;
        }
        std::printf("%s %s\n", skipped ? "ja existe:" : "baixado:   ", out.c_str());
        return 0;
    }
    std::fprintf(stderr, "download: comando '%s' desconhecido\n", sub.c_str());
    Usage();
    return 1;
}

} // namespace dl
