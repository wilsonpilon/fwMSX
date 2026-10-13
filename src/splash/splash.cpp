// fwMSX -- splash de abertura. Ver splash.h e doc/splash-spec.md.
#include "splash.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <thread>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace splash {

bool ShouldShow(Mode mode, Flag flag, bool enabled_in_settings) {
    if (mode == Mode::Command) return false; // --cmd: de modo algum
    if (flag == Flag::ForceOff) return false;
    if (flag == Flag::ForceOn) return true;
    return mode == Mode::Window && enabled_in_settings;
}

Flag ExtractFlag(std::vector<std::string> &args) {
    bool on = false, off = false;
    std::vector<std::string> rest;
    for (const std::string &a : args) {
        if (a == "--splash") on = true;
        else if (a == "--no-splash") off = true;
        else rest.push_back(a);
    }
    args.swap(rest);
    return off ? Flag::ForceOff : (on ? Flag::ForceOn : Flag::Default);
}

std::string FindSplashFile(const std::string &exe_dir) {
    const char *name = "fwMSX-splash.png";
    std::vector<fs::path> candidates;
    if (!exe_dir.empty()) {
        candidates.push_back(fs::path(exe_dir) / name);
        candidates.push_back(fs::path(exe_dir) / "images" / name);
    }
    candidates.push_back(fs::path(name));
    candidates.push_back(fs::path("dist") / name);
    candidates.push_back(fs::path("images") / name);
    std::error_code ec;
    for (const fs::path &p : candidates) {
        if (fs::is_regular_file(p, ec)) return p.string();
    }
    return std::string();
}

std::string RenderHalfBlocks(const Image &image, int cols, int rows) {
    if (image.width <= 0 || image.height <= 0 || cols < 4 || rows < 2) return std::string();
    // Cada celula = 1 "pixel" de largura e 2 de altura (meio bloco em cima, meio embaixo).
    const double scale = std::min(static_cast<double>(cols) / image.width, static_cast<double>(rows * 2) / image.height);
    int w = std::max(1, static_cast<int>(image.width * scale));
    int h = std::max(2, static_cast<int>(image.height * scale));
    h &= ~1; // altura par: duas linhas de pixel por linha de texto
    if (h < 2) h = 2;

    auto sample = [&](int x, int y, uint8_t rgb[3]) {
        // Media da caixa da imagem original que cai neste pixel reduzido.
        const int x0 = x * image.width / w, x1 = std::max(x0 + 1, (x + 1) * image.width / w);
        const int y0 = y * image.height / h, y1 = std::max(y0 + 1, (y + 1) * image.height / h);
        unsigned sum[3] = {0, 0, 0};
        unsigned n = 0;
        for (int yy = y0; yy < y1 && yy < image.height; ++yy) {
            for (int xx = x0; xx < x1 && xx < image.width; ++xx) {
                const uint8_t *p = &image.rgba[(static_cast<size_t>(yy) * image.width + xx) * 4];
                sum[0] += p[0];
                sum[1] += p[1];
                sum[2] += p[2];
                ++n;
            }
        }
        for (int c = 0; c < 3; ++c) rgb[c] = static_cast<uint8_t>(n ? sum[c] / n : 0);
    };

    std::string out;
    const std::string margin(static_cast<size_t>(std::max(0, (cols - w) / 2)), ' ');
    char buf[64];
    for (int y = 0; y < h; y += 2) {
        out += margin;
        for (int x = 0; x < w; ++x) {
            uint8_t top[3], bottom[3];
            sample(x, y, top);
            sample(x, y + 1, bottom);
            std::snprintf(buf, sizeof(buf), "\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm", top[0], top[1], top[2], bottom[0], bottom[1], bottom[2]);
            out += buf;
            out += "\xE2\x96\x80"; // U+2580 meio bloco superior
        }
        out += "\x1b[0m\n";
    }
    return out;
}

namespace {

bool StdoutIsTerminal() {
#ifdef _WIN32
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(1) != 0;
#endif
}

void TerminalSize(int &cols, int &rows) {
    cols = 100;
    rows = 30;
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        cols = info.srWindow.Right - info.srWindow.Left + 1;
        rows = info.srWindow.Bottom - info.srWindow.Top + 1;
    }
#else
    winsize ws{};
    if (ioctl(1, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
        cols = ws.ws_col;
        rows = ws.ws_row;
    }
#endif
}

} // namespace

bool ShowTerminalSplash(const std::string &exe_dir, double seconds) {
    if (!StdoutIsTerminal()) return false;
    const std::string file = FindSplashFile(exe_dir);
    if (file.empty()) return false;
    Image image;
    std::string error;
    if (!LoadPngFile(file, image, error)) return false;

#ifdef _WIN32
    // Cores de 24 bits e UTF-8 no console do Windows.
    const HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (!GetConsoleMode(h, &mode) || !SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) return false;
    const UINT old_cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif
    int cols = 0, rows = 0;
    TerminalSize(cols, rows);
    const std::string frame = RenderHalfBlocks(image, std::min(cols, 160), std::max(2, rows - 1));
    if (!frame.empty()) {
        std::fputs("\x1b[2J\x1b[H", stdout);
        std::fputs(frame.c_str(), stdout);
        std::fflush(stdout);
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(seconds * 1000)));
        std::fputs("\x1b[0m\x1b[2J\x1b[H", stdout);
        std::fflush(stdout);
    }
#ifdef _WIN32
    SetConsoleOutputCP(old_cp);
#endif
    return !frame.empty();
}

void HandleTerminalArgs(std::vector<std::string> &args, const std::string &argv0) {
    const Flag flag = ExtractFlag(args);
    if (!ShouldShow(Mode::Terminal, flag, false)) return;
    std::error_code ec;
    ShowTerminalSplash(fs::absolute(fs::path(argv0), ec).parent_path().string(), 2.5);
}

} // namespace splash
