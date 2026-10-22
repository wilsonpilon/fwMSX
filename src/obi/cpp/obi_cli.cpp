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
                    "  obi dosdisk <1|2> [saida.dsk] [--tools]   disquete de sistema do MSX-DOS (padrao: emulator/msxdosN.dsk)\n"
                    "ver: man obi\n");
        return sub.empty() ? 1 : 0;
    }
    if (sub == "dosdisk") { // obi dosdisk <1|2> [saida.dsk] [--tools]
        int version = 0;
        bool tools = false;
        std::string out;
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i] == "--tools") tools = true;
            else if (version == 0 && (args[i] == "1" || args[i] == "2")) version = args[i][0] - '0';
            else out = args[i];
        }
        if (version == 0) {
            std::fprintf(stderr, "obi: use obi dosdisk <1|2> [saida.dsk] [--tools]\n");
            return 1;
        }
        if (out.empty()) out = (fs::path(EmulatorDir().empty() ? std::string("emulator") : EmulatorDir()) / ("msxdos" + std::to_string(version) + ".dsk")).string();
        std::string msg;
        const bool ok = BuildSystemDisk(version, tools, out, msg);
        std::printf("%s\n", msg.c_str());
        return ok ? 0 : 1;
    }
    if (sub != "build" && sub != "check") {
        std::fprintf(stderr, "obi: comando '%s' desconhecido (build, check ou dosdisk)\n", sub.c_str());
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
