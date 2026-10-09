// fwMSX -- console interativo (replxx). Ver repl.h e doc/repl-spec.md.
#include "repl.h"

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>

#include <replxx.hxx>

#include "../diskfmt/cpp/cli.h"
#include "../msxdisk/entry.h"
#include "../romdb/cli.h"
#include "../tape/cli/cas_tool.h"
#include "../tapedb/cli.h"
#include "repl_session.h"

namespace repl {

namespace {

std::string HistoryFilePath() {
    const char *home =
#if defined(_WIN32)
        std::getenv("USERPROFILE");
#else
        std::getenv("HOME");
#endif
    if (home == nullptr || *home == '\0') return ".fwmsx_history";
    return std::string(home) + "/.fwmsx_history";
}

// Comandos conhecidos, para o TAB (locais + os da ponte de controle).
const std::vector<std::string> kCompletions = {
    "help", "exit", "quit", "emu start", "emu attach", "emu detach", "emu stop", "emu status", "newdisk", "romdb", "cas",
    "fitadb", "msxdisk", "version", "status", "reset", "pause", "resume", "step", "type", "peek", "poke", "regs", "cart",
    "disk", "eject", "tape", "state save", "state load", "screenshot"};

} // namespace

int RunReplCommand(const std::vector<std::string> &args, const std::string &argv0) {
    std::string exe = SelfExePath();
    if (exe.empty()) exe = argv0;
    // `emu start` abre a JANELA: no Windows e' o fwMSX.exe (janela), nao o fwMSXc.exe (console) em que
    // o console esta' rodando -- procura o irmao na mesma pasta.
    {
        namespace fs = std::filesystem;
        const fs::path self(exe);
        std::string name = self.filename().string();
        for (char &ch : name) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        if (name == "fwmsxc.exe") {
            const fs::path sibling = self.parent_path() / "fwMSX.exe";
            std::error_code ec;
            if (fs::exists(sibling, ec)) exe = sibling.string();
        }
    }

    ReplSession session(exe);
    // As ferramentas que ja' existiam como opcoes soltas viram comandos do console.
    session.AddTool("newdisk", [](const std::vector<std::string> &a) { return diskfmt::RunDiskNewCommand(a); },
                    "cria um disquete novo e formatado: newdisk <arq.dsk> <ss525|ds525|ss35|ds35>");
    session.AddTool("romdb", [&](const std::vector<std::string> &a) { return romdb::RunRomDbCommand(a, argv0); },
                    "banco de ROMs (o fwmsx --romdb)");
    session.AddTool("cas", [&](const std::vector<std::string> &a) { return tape::RunCasToolCommand(a, argv0); },
                    "ferramentas de fita .CAS/.TSX (o fwmsx --cas)");
    session.AddTool("fitadb", [&](const std::vector<std::string> &a) { return tapedb::RunTapeDbCommand(a, argv0); },
                    "banco de fitas (o fwmsx --fitadb)");
    session.AddTool("msxdisk", [](const std::vector<std::string> &a) { return msxdisk::RunEntryPoint(a); },
                    "utilitario de imagens de disco (o fwmsx --msxdisk)");

    for (size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == "--attach") session.Execute("emu attach " + args[i + 1], std::cout);
    }

    replxx::Replxx rx;
    const std::string history_path = HistoryFilePath();
    rx.history_load(history_path);
    rx.set_completion_callback([](const std::string &input, int &context_len) {
        replxx::Replxx::completions_t out;
        context_len = static_cast<int>(input.size());
        for (const std::string &c : kCompletions) {
            if (c.compare(0, input.size(), input) == 0) out.emplace_back(c);
        }
        return out;
    });

    std::cout << "fwMSX - console. 'help' lista os comandos; 'emu start' abre o emulador; 'exit' sai." << std::endl;
    while (true) {
        const char *raw = rx.input(session.Prompt());
        if (raw == nullptr) {
            std::cout << std::endl;
            break;
        }
        const std::string line(raw);
        if (line.empty()) continue;
        rx.history_add(line);
        if (!session.Execute(line, std::cout)) break;
        std::cout.flush();
    }
    rx.history_save(history_path);
    return 0;
}

} // namespace repl
