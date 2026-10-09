// fwMSX -- ligacao do console com o emulador. Ver emu_link.h e doc/repl-spec.md.
#include "emu_link.h"

#include <chrono>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
typedef SOCKET sock_t;
#define SOCK_INVALID INVALID_SOCKET
static void sock_close(sock_t s) { closesocket(s); }
static void sock_init() {
    static bool done = false;
    if (done) return;
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    done = true;
}
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <spawn.h>
#include <sys/socket.h>
#include <unistd.h>
extern char **environ;
typedef int sock_t;
#define SOCK_INVALID (-1)
static void sock_close(sock_t s) { close(s); }
static void sock_init() {}
#endif

namespace repl {

namespace {

// Tempo maximo esperando a resposta de um comando (um `step` grande ou um `cart` demoram).
constexpr int kReplyTimeoutMs = 20000;

} // namespace

bool EmuLink::Connect(int port, int timeout_ms, std::string &error) {
    Close();
    sock_init();
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    while (true) {
        sock_t s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == SOCK_INVALID) {
            error = "nao foi possivel criar o socket";
            return false;
        }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(port));
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (connect(s, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == 0) {
#ifdef _WIN32
            DWORD ms = kReplyTimeoutMs;
            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&ms), sizeof(ms));
#else
            timeval tv{kReplyTimeoutMs / 1000, 0};
            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
            socket_ = static_cast<long long>(s);
            port_ = port;
            pending_.clear();
            return true;
        }
        sock_close(s);
        if (std::chrono::steady_clock::now() >= deadline) {
            error = "nao foi possivel conectar em 127.0.0.1:" + std::to_string(port) + " (o emulador esta' aberto com --ctl-port?)";
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void EmuLink::Close() {
    if (socket_ >= 0) sock_close(static_cast<sock_t>(socket_));
    socket_ = -1;
    port_ = 0;
    pending_.clear();
}

bool EmuLink::Send(const std::string &line, std::string &reply, std::string &error) {
    if (socket_ < 0) {
        error = "sem emulador conectado";
        return false;
    }
    const sock_t s = static_cast<sock_t>(socket_);
    const std::string out = line + "\n";
    if (send(s, out.data(), static_cast<int>(out.size()), 0) <= 0) {
        Close();
        error = "a conexao com o emulador caiu";
        return false;
    }
    while (true) {
        const size_t nl = pending_.find('\n');
        if (nl != std::string::npos) {
            reply = pending_.substr(0, nl);
            pending_.erase(0, nl + 1);
            if (!reply.empty() && reply.back() == '\r') reply.pop_back();
            return true;
        }
        char buf[2048];
        const int n = recv(s, buf, sizeof(buf), 0);
        if (n <= 0) {
            Close();
            error = "o emulador foi encerrado (ou nao respondeu a tempo)";
            return false;
        }
        pending_.append(buf, static_cast<size_t>(n));
    }
}

int FindFreePort() {
    sock_init();
    sock_t s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == SOCK_INVALID) return 0;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    int port = 0;
    if (bind(s, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == 0) {
        socklen_t len = sizeof(addr);
        if (getsockname(s, reinterpret_cast<sockaddr *>(&addr), &len) == 0) port = ntohs(addr.sin_port);
    }
    sock_close(s);
    return port;
}

std::string SelfExePath() {
#ifdef _WIN32
    char buf[MAX_PATH * 2];
    const DWORD n = GetModuleFileNameA(nullptr, buf, sizeof(buf));
    return (n > 0 && n < sizeof(buf)) ? std::string(buf, n) : std::string();
#else
    char buf[4096];
    const ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    return n > 0 ? std::string(buf, static_cast<size_t>(n)) : std::string();
#endif
}

bool SpawnDetached(const std::string &exe, const std::vector<std::string> &args, std::string &error) {
#ifdef _WIN32
    std::string cmdline = "\"" + exe + "\"";
    for (const std::string &a : args) {
        cmdline += " \"";
        cmdline += a;
        cmdline += "\"";
    }
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    HANDLE nul = CreateFileA("NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, nullptr);
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = si.hStdOutput = si.hStdError = nul;
    PROCESS_INFORMATION pi{};
    std::vector<char> mutable_cmd(cmdline.begin(), cmdline.end());
    mutable_cmd.push_back('\0');
    const BOOL ok = CreateProcessA(exe.c_str(), mutable_cmd.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi);
    if (nul != INVALID_HANDLE_VALUE) CloseHandle(nul);
    if (!ok) {
        error = "nao foi possivel iniciar '" + exe + "' (erro " + std::to_string(GetLastError()) + ")";
        return false;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
#else
    signal(SIGCHLD, SIG_IGN); // o emulador e' independente: nada de zumbis
    std::vector<std::string> storage;
    storage.push_back(exe);
    for (const std::string &a : args) storage.push_back(a);
    std::vector<char *> argv;
    for (std::string &s : storage) argv.push_back(s.data());
    argv.push_back(nullptr);
    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_addopen(&fa, 0, "/dev/null", O_RDONLY, 0);
    posix_spawn_file_actions_addopen(&fa, 1, "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_addopen(&fa, 2, "/dev/null", O_WRONLY, 0);
    pid_t pid = 0;
    const int rc = posix_spawn(&pid, exe.c_str(), &fa, nullptr, argv.data(), environ);
    posix_spawn_file_actions_destroy(&fa);
    if (rc != 0) {
        error = "nao foi possivel iniciar '" + exe + "' (erro " + std::to_string(rc) + ")";
        return false;
    }
    return true;
#endif
}

} // namespace repl
