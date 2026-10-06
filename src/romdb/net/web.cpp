// fwMSX -- acesso HTTP/HTTPS via programa `curl`. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "web.h"

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <sys/wait.h>
#define POPEN popen
#define PCLOSE pclose
#endif

namespace romdb {
namespace {

// Navegador comum: alguns servidores (ex.: download.file-hunter.com) recusam o
// User-Agent padrao do curl com 405.
const char *kUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) fwMSX/1.17 (estudo)";

// Roda um comando do shell e devolve a saida; false se o status nao for zero.
bool RunCapture(const std::string &command, std::string &out, int &status) {
    FILE *pipe = POPEN(command.c_str(), "r");
    if (!pipe) return false;
    char buffer[4096];
    size_t n;
    while ((n = std::fread(buffer, 1, sizeof buffer, pipe)) > 0) out.append(buffer, n);
    const int raw = PCLOSE(pipe);
#ifdef _WIN32
    status = raw;
#else
    status = WIFEXITED(raw) ? WEXITSTATUS(raw) : -1;
#endif
    return status == 0;
}

} // namespace

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
    const std::string command = std::string("curl -sSfL --max-time 1800 -A \"") + kUserAgent + "\" -o \"" + path +
                                "\" \"" + url + "\"";
    std::string ignored;
    int status = 0;
    if (!RunCapture(command, ignored, status)) {
        error = "falha ao baixar '" + url + "' (curl, codigo " + std::to_string(status) + ")";
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
