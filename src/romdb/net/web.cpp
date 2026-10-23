// fwMSX -- acesso HTTP/HTTPS via programa `curl`. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "web.h"

#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace romdb {
namespace {

// Navegador comum: alguns servidores (ex.: download.file-hunter.com) recusam o
// User-Agent padrao do curl com 405.
const char *kUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) fwMSX/1.17 (estudo)";

// ---------------------------------------------------------------------------------------------------------------
// o processo do curl, SEM janela de console (no Windows o _popen abria um cmd.exe visivel a cada download)
// ---------------------------------------------------------------------------------------------------------------

#ifdef _WIN32
struct Proc {
    HANDLE process = nullptr;
    HANDLE out_read = nullptr;  // so' quando a saida e' capturada
};

// Dispara `command` (a linha de comando inteira) escondido. `capture` liga a saida padrao a um pipe; senao ela vai para o nada.
bool ProcStart(const std::string &command, bool capture, Proc &p) {
    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    HANDLE rd = nullptr, wr = nullptr;
    if (capture) {
        if (!CreatePipe(&rd, &wr, &sa, 0)) return false;
        SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);
    } else {
        wr = CreateFileA("NUL", GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, &sa, OPEN_EXISTING, 0, nullptr);
    }
    HANDLE nul = CreateFileA("NUL", GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, &sa, OPEN_EXISTING, 0, nullptr);
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = wr;
    si.hStdError = nul;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    PROCESS_INFORMATION pi{};
    std::string line = command; // CreateProcess pode alterar o buffer
    const BOOL ok = CreateProcessA(nullptr, &line[0], nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    if (wr) CloseHandle(wr);
    if (nul != INVALID_HANDLE_VALUE) CloseHandle(nul);
    if (!ok) {
        if (rd) CloseHandle(rd);
        return false;
    }
    CloseHandle(pi.hThread);
    p.process = pi.hProcess;
    p.out_read = rd;
    return true;
}

// Espera ate `ms`; true se o processo terminou (e entao `status` e' o codigo de saida).
bool ProcWait(Proc &p, int ms, int &status) {
    if (WaitForSingleObject(p.process, static_cast<DWORD>(ms)) != WAIT_OBJECT_0) return false;
    DWORD code = 1;
    GetExitCodeProcess(p.process, &code);
    status = static_cast<int>(code);
    return true;
}

void ProcClose(Proc &p) {
    if (p.process) CloseHandle(p.process);
    if (p.out_read) CloseHandle(p.out_read);
    p = Proc{};
}

size_t ProcRead(Proc &p, char *buf, size_t n) {
    DWORD got = 0;
    if (!p.out_read || !ReadFile(p.out_read, buf, static_cast<DWORD>(n), &got, nullptr)) return 0;
    return got;
}
#else
struct Proc {
    pid_t pid = -1;
    FILE *pipe = nullptr;
};

bool ProcStart(const std::string &command, bool capture, Proc &p) {
    if (capture) {
        p.pipe = popen(command.c_str(), "r");
        return p.pipe != nullptr;
    }
    const pid_t pid = fork();
    if (pid < 0) return false;
    if (pid == 0) {
        if (!freopen("/dev/null", "w", stdout)) _exit(127);
        if (!freopen("/dev/null", "w", stderr)) _exit(127);
        execl("/bin/sh", "sh", "-c", command.c_str(), static_cast<char *>(nullptr));
        _exit(127);
    }
    p.pid = pid;
    return true;
}

bool ProcWait(Proc &p, int ms, int &status) {
    if (p.pipe) { // capturado: a leitura ja' bloqueou ate o fim
        const int raw = pclose(p.pipe);
        p.pipe = nullptr;
        status = WIFEXITED(raw) ? WEXITSTATUS(raw) : -1;
        return true;
    }
    int raw = 0;
    const pid_t r = waitpid(p.pid, &raw, WNOHANG);
    if (r == 0) {
        usleep(static_cast<useconds_t>(ms) * 1000);
        return false;
    }
    p.pid = -1;
    status = (r > 0 && WIFEXITED(raw)) ? WEXITSTATUS(raw) : -1;
    return true;
}

void ProcClose(Proc &p) {
    if (p.pipe) pclose(p.pipe);
    p = Proc{};
}

size_t ProcRead(Proc &p, char *buf, size_t n) { return p.pipe ? std::fread(buf, 1, n, p.pipe) : 0; }
#endif

// Roda um comando e devolve a saida; false se o status nao for zero.
bool RunCapture(const std::string &command, std::string &out, int &status) {
    Proc p;
    if (!ProcStart(command, true, p)) return false;
    char buffer[4096];
    size_t n;
    while ((n = ProcRead(p, buffer, sizeof buffer)) > 0) out.append(buffer, n);
    while (!ProcWait(p, 50, status)) {
    }
    ProcClose(p);
    return status == 0;
}

// ---------------------------------------------------------------------------------------------------------------
// andamento
// ---------------------------------------------------------------------------------------------------------------

std::mutex g_progress_mutex;
DownloadProgress g_progress;
std::function<void(const DownloadProgress &)> g_hook;

void Publish(const DownloadProgress &p) {
    std::function<void(const DownloadProgress &)> hook;
    {
        std::lock_guard<std::mutex> lock(g_progress_mutex);
        g_progress = p;
        hook = g_hook;
    }
    if (hook) hook(p);
}

// O tamanho anunciado pelo servidor (HEAD, seguindo redirecionamentos): o ultimo Content-Length da resposta. -1 se nao disse.
int64_t RemoteSize(const std::string &url) {
    std::string head;
    int status = 0;
    const std::string command = std::string("curl -sSIL --max-time 30 -A \"") + kUserAgent + "\" \"" + url + "\"";
    if (!RunCapture(command, head, status)) return -1;
    int64_t size = -1;
    size_t pos = 0;
    while (pos < head.size()) {
        size_t nl = head.find('\n', pos);
        if (nl == std::string::npos) nl = head.size();
        std::string line = head.substr(pos, nl - pos);
        pos = nl + 1;
        for (char &c : line) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (line.rfind("content-length:", 0) == 0) size = std::atoll(line.c_str() + 15);
        if (line.rfind("http/", 0) == 0) size = -1; // uma nova resposta (redirecionamento): vale so' a ultima
    }
    return size;
}

} // namespace

DownloadProgress CurrentProgress() {
    std::lock_guard<std::mutex> lock(g_progress_mutex);
    return g_progress;
}

void SetProgressHook(std::function<void(const DownloadProgress &)> hook) {
    std::lock_guard<std::mutex> lock(g_progress_mutex);
    g_hook = std::move(hook);
}

bool HttpGetText(const std::string &url, std::string &out, std::string &error) {
    out.clear();
    const std::string command = std::string("curl -sSfL --max-time 120 -A \"") + kUserAgent + "\" \"" + url + "\"";
    int status = 0;
    if (!RunCapture(command, out, status)) {
        error = "falha ao buscar '" + url + "' (curl, codigo " + std::to_string(status) +
                "). Verifique a conexao e se o curl esta instalado.";
        return false;
    }
    return true;
}

bool HttpDownload(const std::string &url, const std::string &path, std::string &error) {
    namespace fs = std::filesystem;
    DownloadProgress prog;
    prog.active = true;
    prog.name = fs::path(path).filename().string();
    prog.total = RemoteSize(url);
    Publish(prog);

    const std::string command = std::string("curl -sSfL --max-time 1800 -A \"") + kUserAgent + "\" -o \"" + path + "\" \"" + url + "\"";
    Proc p;
    int status = 0;
    if (!ProcStart(command, false, p)) {
        prog.active = false;
        Publish(prog);
        error = "falha ao iniciar o curl para baixar '" + url + "'. Verifique se o curl esta instalado.";
        return false;
    }
    // o tamanho do arquivo em disco, de 0,2 em 0,2 s, e' o andamento
    while (!ProcWait(p, 200, status)) {
        std::error_code ec;
        const auto sz = fs::file_size(path, ec);
        if (!ec) prog.done = static_cast<int64_t>(sz);
        Publish(prog);
    }
    ProcClose(p);
    std::error_code ec;
    const auto sz = fs::file_size(path, ec);
    if (!ec) prog.done = static_cast<int64_t>(sz);
    if (status == 0 && prog.total < 0) prog.total = prog.done;
    prog.active = false;
    Publish(prog);
    if (status != 0) {
        error = "falha ao baixar '" + url + "' (curl, codigo " + std::to_string(status) + ")";
        return false;
    }
    return true;
}

bool HttpDownloadMany(const std::vector<std::pair<std::string, std::string>> &jobs, const std::string &label, std::string &error) {
    namespace fs = std::filesystem;
    if (jobs.empty()) return true;
    // A lista de transferencias vai num arquivo de configuracao do curl (-K): nenhum nome chega cru a uma linha de comando. Barras sempre '/'.
    const fs::path cfg = fs::path(jobs.front().second).parent_path() / "_curl.cfg";
    {
        std::FILE *f = std::fopen(cfg.string().c_str(), "wb");
        if (!f) {
            error = "nao foi possivel gravar '" + cfg.string() + "'";
            return false;
        }
        std::fprintf(f, "fail\nsilent\nshow-error\nlocation\nuser-agent = \"%s\"\nmax-time = 120\n", kUserAgent);
        for (const auto &j : jobs) {
            std::string out = j.second;
            for (char &c : out) {
                if (c == '\\') c = '/';
            }
            std::fprintf(f, "url = \"%s\"\noutput = \"%s\"\n", j.first.c_str(), out.c_str());
        }
        std::fclose(f);
    }
    DownloadProgress prog;
    prog.active = true;
    prog.items = true;
    prog.name = label;
    prog.total = static_cast<int64_t>(jobs.size());
    Publish(prog);
    Proc p;
    int status = 0;
    if (!ProcStart("curl -K \"" + cfg.generic_string() + "\"", false, p)) {
        prog.active = false;
        Publish(prog);
        error = "falha ao iniciar o curl. Verifique se ele esta instalado.";
        return false;
    }
    while (!ProcWait(p, 200, status)) {
        int64_t n = 0;
        std::error_code ec;
        for (const auto &j : jobs) n += fs::exists(j.second, ec) ? 1 : 0;
        prog.done = n;
        Publish(prog);
    }
    ProcClose(p);
    std::error_code ec;
    fs::remove(cfg, ec);
    prog.done = prog.total;
    prog.active = false;
    Publish(prog);
    if (status != 0) {
        error = "falha ao baixar " + std::to_string(jobs.size()) + " arquivos (curl, codigo " + std::to_string(status) + ")";
        return false;
    }
    return true;
}

std::string UrlEncodePath(const std::string &name) {
    static const char *hex = "0123456789ABCDEF";
    std::string out;
    for (unsigned char c : name) {
        const bool safe = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                          c == '-' || c == '_' || c == '.' || c == '~' || c == '/';
        if (safe) {
            out += static_cast<char>(c);
        } else {
            out += '%';
            out += hex[c >> 4];
            out += hex[c & 15];
        }
    }
    return out;
}

std::string UrlDecode(const std::string &text) {
    std::string out;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '%' && i + 2 < text.size()) {
            const std::string pair = text.substr(i + 1, 2);
            char *end = nullptr;
            const long value = std::strtol(pair.c_str(), &end, 16);
            if (end && *end == '\0') {
                out += static_cast<char>(value);
                i += 2;
                continue;
            }
        }
        out += text[i];
    }
    return out;
}

} // namespace romdb
