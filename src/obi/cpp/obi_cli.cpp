// fwMSX -- obi: o comando de linha (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/obi-spec.md.
#include <cstdio>
#include <filesystem>

#include "obi.h"

namespace obi {

int RunObiCommand(const std::vector<std::string> &args) {
    namespace fs = std::filesystem;
    std::string sub = args.empty() ? "" : args[0];
    if (sub == "-h" || sub == "--help" || sub == "help" || sub.empty()) {
        std::printf("obi: monta um disquete .dsk a partir de uma receita (Obifile)\n"
                    "  obi build [Obifile]   monta e grava o .dsk (sem argumento: o Obifile da pasta atual)\n"
                    "  obi check [Obifile]   monta tudo e confere, sem gravar o .dsk\n"
                    "ver: man obi\n");
        return sub.empty() ? 1 : 0;
    }
    if (sub != "build" && sub != "check") {
        std::fprintf(stderr, "obi: comando '%s' desconhecido (build ou check)\n", sub.c_str());
        return 1;
    }
    BuildOptions o;
    o.write = sub == "build";
    o.obifile = args.size() > 1 ? args[1] : FindObifile("");
    if (o.obifile.empty()) {
        std::fprintf(stderr, "obi: nao achei um Obifile nesta pasta (use: obi %s <Obifile>)\n", sub.c_str());
        return 1;
    }
    o.obifile = fs::absolute(o.obifile).lexically_normal().string();
    const BuildResult r = Build(o);
    for (const std::string &l : r.log) std::printf("%s\n", l.c_str());
    if (!r.ok) return 1;
    for (const BuiltFile &f : r.files) std::printf("  %-12s %7zu bytes  crc %08X  <- %s\n", f.name.c_str(), f.size, static_cast<unsigned>(f.crc), f.from.c_str());
    if (o.write) std::printf("%s: %zu arquivos, %ld KiB livres\n", r.dsk.c_str(), r.files.size(), r.free_bytes / 1024);
    else std::printf("receita ok: %zu arquivos, %ld KiB livres (nada gravado)\n", r.files.size(), r.free_bytes / 1024);
    return 0;
}

} // namespace obi
