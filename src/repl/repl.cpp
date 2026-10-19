// fwMSX -- console interativo (replxx). Ver repl.h e doc/repl-spec.md.
#include "repl.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>

#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

#include <replxx.hxx>

#include "../asm/cpp/cli.h"
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
    "disk", "eject", "tape", "state save", "state load", "screenshot", "man", "break", "unbreak", "breaks", "watch", "unwatch",
    "watches", "stepi", "next", "until", "go", "stop", "jump", "dis", "mem", "stack", "reg", "slots", "events", "dbgui", "wait", "asm", "asmrun", "sym", "src", "markers", "mon", "mamute"};

// Tamanho do terminal em colunas x linhas; tty = false quando a saida foi redirecionada (sem paginar).
struct TermSize {
    int cols = 80;
    int rows = 24;
    bool tty = false;
};

TermSize QueryTerm() {
    TermSize t;
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        t.cols = info.srWindow.Right - info.srWindow.Left + 1;
        t.rows = info.srWindow.Bottom - info.srWindow.Top + 1;
        t.tty = true;
    }
#else
    struct winsize ws;
    if (isatty(STDOUT_FILENO) && ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        t.cols = ws.ws_col;
        t.rows = ws.ws_row;
        t.tty = true;
    }
#endif
    return t;
}

// Uma tecla, sem eco e sem esperar Enter. Setas e teclas de funcao devolvem 0 (ignoradas pelo pager).
int ReadKey() {
#if defined(_WIN32)
    int c = _getch();
    if (c == 0 || c == 0xE0) {
        _getch();
        return 0;
    }
    return c;
#else
    struct termios old_t, raw;
    if (tcgetattr(STDIN_FILENO, &old_t) != 0) return std::getchar();
    raw = old_t;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    int c = std::getchar();
    if (c == 27) { // sequencia de escape (setas...): engole o resto
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        while (std::getchar() != EOF) {
        }
        c = 27;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
    return c;
#endif
}

// `man`: mostra o texto de pagina em pagina (espaco = pagina, Enter = linha, q/Esc = sai). Sem terminal, imprime tudo.
void Page(const std::string &title, const std::string &text) {
    (void)title;
    const TermSize term = QueryTerm();
    std::istringstream in(text);
    std::string line;
    const int page = std::max(4, term.rows - 1);
    int shown = 0;
    int budget = page;
    while (std::getline(in, line)) {
        std::cout << line << "\n";
        if (!term.tty) continue;
        if (++shown < budget) continue;
        if (in.peek() == EOF) break;
        std::cout << "-- mais (espaco = pagina, Enter = linha, q = sai) --" << std::flush;
        const int k = ReadKey();
        std::cout << "\r" << std::string(52, ' ') << "\r" << std::flush;
        if (k == 'q' || k == 'Q' || k == 27 || k == 3) break;
        budget = (k == '\r' || k == '\n') ? shown + 1 : shown + page;
    }
    std::cout << std::flush;
}

} // namespace

std::unique_ptr<ReplSession> BuildSession(const std::string &argv0) {
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

    auto session = std::make_unique<ReplSession>(exe);
    // As ferramentas que ja' existiam como opcoes soltas viram comandos do console.
    session->AddTool("newdisk", [](const std::vector<std::string> &a) { return diskfmt::RunDiskNewCommand(a); },
                     "cria um disquete novo e formatado: newdisk <arq.dsk> <ss525|ds525|ss35|ds35>");
    session->AddTool("asm", [](const std::vector<std::string> &a) { return fwasm::RunAsmCommand(a); },
                     "montador Z80 (o fwmsx --asm): asm [-z] [-s] [-vv] [-o <base>] <arq.asm>");
    session->AddTool("romdb", [argv0](const std::vector<std::string> &a) { return romdb::RunRomDbCommand(a, argv0); },
                     "banco de ROMs (o fwmsx --romdb)");
    session->AddTool("cas", [argv0](const std::vector<std::string> &a) { return tape::RunCasToolCommand(a, argv0); },
                     "ferramentas de fita .CAS/.TSX (o fwmsx --cas)");
    session->AddTool("fitadb", [argv0](const std::vector<std::string> &a) { return tapedb::RunTapeDbCommand(a, argv0); },
                     "banco de fitas (o fwmsx --fitadb)");
    session->AddTool("msxdisk", [](const std::vector<std::string> &a) { return msxdisk::RunEntryPoint(a); },
                     "utilitario de imagens de disco (o fwmsx --msxdisk)");
    return session;
}

int RunReplCommand(const std::vector<std::string> &args, const std::string &argv0) {
    std::unique_ptr<ReplSession> session_ptr = BuildSession(argv0);
    ReplSession &session = *session_ptr;

    for (size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == "--attach") session.Execute("emu attach " + args[i + 1], std::cout);
    }

    session.SetViewer(Page);
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
    for (const std::string &a : args) {
        if (a == "--mon") session.Execute("mon", std::cout); // abre ja' dentro do monitor Mamute
    }
    while (true) {
        session.PrintEvents(std::cout); // eventos que chegaram enquanto se digitava (breakpoint atingido...)
        const char *raw = rx.input(session.monitor_mode() ? session.MonitorPrompt() : session.Prompt());
        if (raw == nullptr) {
            std::cout << std::endl;
            break;
        }
        const std::string line(raw);
        if (line.empty()) continue;
        rx.history_add(line);
        session.SetWidth(QueryTerm().cols - 1); // help/man seguem a largura atual do terminal
        if (!session.Execute(line, std::cout)) break;
        std::cout.flush();
    }
    rx.history_save(history_path);
    return 0;
}

} // namespace repl
