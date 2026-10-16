// Ferramenta de medicao (so' Windows): latencia de digitacao do console interativo do fwMSX.
//
//   g++ -O1 -o conperf.exe tools/conperf/conperf.cpp -static
//   conperf.exe E:/fwMSX/dist/fwMSXc.exe <idle|emu|audio> [intervalo_ms] [saida.txt] [--cli|--tui]
//
// Abre o console (--cli ou --tui) num console OCULTO, injeta as teclas com WriteConsoleInput (o mesmo caminho do teclado, depois do
// conhost) e le o buffer da tela para ver quando cada letra aparece. "idle" = sem emulador; "emu" = depois de `emu start --mute`;
// "audio" = com audio. Resultado de 2026-10-16 (conhost, 8 nucleos): 36/36 letras, mediana 16 ms no --cli e 32-47 ms no --tui,
// com e sem o emulador rodando -- nao reproduz a lentidao relatada (que pode ser do terminal, ver OUTLINE).
// Mede a latencia de digitacao do console interativo do fwMSX: injeta teclas no console do processo (WriteConsoleInput)
// e le o buffer da tela (ReadConsoleOutputCharacter) para ver quando cada letra aparece.
//   conperf <fwMSXc.exe> <idle|emu> [intervalo_ms] [saida.txt]
#include <windows.h>

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>

static HANDLE g_in, g_out;
static FILE *g_log;

static void Key(wchar_t c, bool enter) {
    INPUT_RECORD r[2] = {};
    for (int i = 0; i < 2; ++i) {
        r[i].EventType = KEY_EVENT;
        r[i].Event.KeyEvent.bKeyDown = i == 0;
        r[i].Event.KeyEvent.wRepeatCount = 1;
        r[i].Event.KeyEvent.wVirtualKeyCode = enter ? VK_RETURN : static_cast<WORD>(VkKeyScanW(c) & 0xFF);
        r[i].Event.KeyEvent.wVirtualScanCode = static_cast<WORD>(MapVirtualKeyW(r[i].Event.KeyEvent.wVirtualKeyCode, MAPVK_VK_TO_VSC));
        r[i].Event.KeyEvent.uChar.UnicodeChar = enter ? L'\r' : c;
    }
    DWORD w = 0;
    WriteConsoleInputW(g_in, r, 2, &w);
}

static std::string Screen() {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(g_out, &info)) return std::string();
    const DWORD n = info.dwSize.X * (info.dwCursorPosition.Y + 2);
    std::wstring buf(n, L' ');
    DWORD got = 0;
    ReadConsoleOutputCharacterW(g_out, &buf[0], n, COORD{0, 0}, &got);
    std::string s;
    for (DWORD i = 0; i < got; ++i) s += buf[i] < 128 ? static_cast<char>(buf[i]) : '?';
    return s;
}

static void TypeText(const std::string &t, int interval_ms) {
    for (char c : t) {
        Key(static_cast<wchar_t>(c), false);
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
}

int main(int argc, char **argv) {
    if (argc < 3) return 2;
    const std::string exe = argv[1], scenario = argv[2];
    const int interval = argc > 3 ? std::atoi(argv[3]) : 30;
    g_log = std::fopen(argc > 4 ? argv[4] : "conperf_out.txt", "a");

    STARTUPINFOA si = {sizeof(si)};
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi = {};
    std::string mode = argc > 5 ? argv[5] : "--cli";
    std::string cmd = "\"" + exe + "\" " + mode + " --no-splash";
    if (!CreateProcessA(nullptr, &cmd[0], nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)) {
        std::fprintf(g_log, "CreateProcess falhou\n");
        return 1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    FreeConsole();
    if (!AttachConsole(pi.dwProcessId)) {
        std::fprintf(g_log, "AttachConsole falhou (%lu)\n", GetLastError());
        return 1;
    }
    g_in = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    g_out = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

    if (scenario != "idle") {
        TypeText(scenario == "audio" ? "emu start --no-splash" : "emu start --mute --no-splash", 10);
        Key(0, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(7000));
    }
    const std::string text = "abcdefghijklmnopqrstuvwxyz0123456789";
    auto t_start = std::chrono::steady_clock::now();
    auto ms = [&]() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t_start).count(); };
    size_t seen = 0;
    std::vector<long long> lat;
    for (size_t k = 0; k < text.size(); ++k) {
        const long long t0 = ms();
        Key(static_cast<wchar_t>(text[k]), false);
        const std::string want = text.substr(0, seen + 1);
        bool ok = false;
        while (ms() < t0 + 2000) {
            if (Screen().find(want) != std::string::npos) {
                ok = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (ok) {
            ++seen;
            lat.push_back(ms() - t0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }
    std::sort(lat.begin(), lat.end());
    std::fprintf(g_log, "cenario=%s intervalo=%dms digitadas=%zu apareceram=%zu", scenario.c_str(), interval, text.size(), seen);
    if (!lat.empty()) std::fprintf(g_log, "  latencia ms: min=%lld mediana=%lld max=%lld", lat.front(), lat[lat.size() / 2], lat.back());
    std::fprintf(g_log, "\n");
    std::fflush(g_log);
    TerminateProcess(pi.hProcess, 0);
    system("taskkill /F /IM fwMSX.exe >nul 2>&1");
    return 0;
}
