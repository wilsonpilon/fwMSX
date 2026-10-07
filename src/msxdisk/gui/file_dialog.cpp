//
// msxdisk (fwMSX): dialogos nativos de arquivo do Windows -- ver
// file_dialog.h.
//
// So' existe implementacao nativa para Windows (GetOpenFileName/
// GetSaveFileName). Descoberto ao validar o build no Linux (WSL2) pela
// primeira vez (2026-09-30): este arquivo incluia <windows.h> sem
// nenhuma guarda de plataforma, quebrando a compilacao inteira do
// msxdisk (e por tabela do fwMSX, que compila os mesmos fontes) fora do
// Windows. Guardado atras de _WIN32; em outras plataformas, as duas
// funcoes devolvem nullopt (sem dialogo nativo ainda) -- os itens de
// menu Novo/Abrir/Salvar Como continuam existindo, so' nao abrem um
// seletor de arquivo de verdade fora do Windows por enquanto. Um
// seletor nativo multiplataforma (ex.: GTK no Linux) fica para uma
// tarefa a parte, se/quando fizer sentido.

#include "file_dialog.h"

#ifdef _WIN32

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <windows.h>

#include <commdlg.h>

#include <algorithm>
#include <filesystem>

namespace msxdisk::gui {

namespace {

std::string WideToUtf8(const wchar_t *wide) {
    if (wide == nullptr || wide[0] == L'\0') return std::string();
    const int size = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return std::string();
    std::string result(static_cast<size_t>(size - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, result.data(), size, nullptr, nullptr);
    return result;
}

std::wstring Utf8ToWide(const std::string &utf8) {
    if (utf8.empty()) return std::wstring();
    const int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (size <= 0) return std::wstring();
    std::wstring result(static_cast<size_t>(size - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, result.data(), size);
    return result;
}

// GetOpenFileName/GetSaveFileName podem mudar o diretorio atual do
// processo mesmo com OFN_NOCHANGEDIR em alguns cenarios -- o painel LOCAL
// da GUI depende do cwd do processo, entao restauramos por garantia.
class CwdGuard {
public:
    CwdGuard() : saved_(std::filesystem::current_path()) {}
    ~CwdGuard() {
        std::error_code ec;
        std::filesystem::current_path(saved_, ec);
    }

private:
    std::filesystem::path saved_;
};

} // namespace

std::optional<std::string> ShowOpenDskDialog(GLFWwindow *window) {
    CwdGuard guard;

    wchar_t file_buf[MAX_PATH] = L"";

    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = (window != nullptr) ? glfwGetWin32Window(window) : nullptr;
    ofn.lpstrFilter = L"Imagens MSX (*.dsk)\0*.dsk\0Todos os arquivos (*.*)\0*.*\0";
    ofn.lpstrFile = file_buf;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = L"Abrir imagem de disco MSX";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn)) {
        return WideToUtf8(file_buf);
    }
    return std::nullopt;
}

std::optional<std::string> ShowOpenFileDialog(GLFWwindow *window, const std::string &title,
                                              const std::string &filter_name, const std::string &patterns) {
    CwdGuard guard;

    wchar_t file_buf[MAX_PATH] = L"";
    // Filtro no formato do GetOpenFileName: "nome\0padroes\0" pares, terminado em \0.
    const std::wstring filter = Utf8ToWide(filter_name + " (" + patterns + ")") + std::wstring(1, L'\0') +
                                Utf8ToWide(patterns) + std::wstring(1, L'\0') +
                                L"Todos os arquivos (*.*)" + std::wstring(1, L'\0') + L"*.*" + std::wstring(1, L'\0');
    const std::wstring title_wide = Utf8ToWide(title);

    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = (window != nullptr) ? glfwGetWin32Window(window) : nullptr;
    ofn.lpstrFilter = filter.c_str();
    ofn.lpstrFile = file_buf;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title_wide.c_str();
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn)) {
        return WideToUtf8(file_buf);
    }
    return std::nullopt;
}

std::optional<std::string> ShowSaveDskDialog(GLFWwindow *window, const std::string &initial_path) {
    CwdGuard guard;

    wchar_t file_buf[MAX_PATH] = L"";
    const std::wstring initial_wide = Utf8ToWide(initial_path);
    if (!initial_wide.empty()) {
        const size_t copy_len = std::min(initial_wide.size(), static_cast<size_t>(MAX_PATH - 1));
        std::copy_n(initial_wide.begin(), copy_len, file_buf);
        file_buf[copy_len] = L'\0';
    }

    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = (window != nullptr) ? glfwGetWin32Window(window) : nullptr;
    ofn.lpstrFilter = L"Imagens MSX (*.dsk)\0*.dsk\0Todos os arquivos (*.*)\0*.*\0";
    ofn.lpstrFile = file_buf;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = L"Salvar imagem de disco MSX";
    ofn.lpstrDefExt = L"dsk";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetSaveFileNameW(&ofn)) {
        return WideToUtf8(file_buf);
    }
    return std::nullopt;
}

std::optional<std::string> ShowSaveFileDialog(GLFWwindow *window, const std::string &title,
                                              const std::string &filter_name, const std::string &patterns,
                                              const std::string &default_ext, const std::string &initial_path) {
    CwdGuard guard;

    wchar_t file_buf[MAX_PATH] = L"";
    const std::wstring initial_wide = Utf8ToWide(initial_path);
    if (!initial_wide.empty()) {
        const size_t copy_len = std::min(initial_wide.size(), static_cast<size_t>(MAX_PATH - 1));
        std::copy_n(initial_wide.begin(), copy_len, file_buf);
        file_buf[copy_len] = L'\0';
    }
    const std::wstring filter = Utf8ToWide(filter_name + " (" + patterns + ")") + std::wstring(1, L'\0') +
                                Utf8ToWide(patterns) + std::wstring(1, L'\0') +
                                L"Todos os arquivos (*.*)" + std::wstring(1, L'\0') + L"*.*" + std::wstring(1, L'\0');
    const std::wstring title_wide = Utf8ToWide(title);
    const std::wstring ext_wide = Utf8ToWide(default_ext);

    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = (window != nullptr) ? glfwGetWin32Window(window) : nullptr;
    ofn.lpstrFilter = filter.c_str();
    ofn.lpstrFile = file_buf;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title_wide.c_str();
    ofn.lpstrDefExt = ext_wide.c_str();
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetSaveFileNameW(&ofn)) {
        return WideToUtf8(file_buf);
    }
    return std::nullopt;
}

} // namespace msxdisk::gui

#else // !_WIN32

namespace msxdisk::gui {

std::optional<std::string> ShowOpenDskDialog(GLFWwindow *) { return std::nullopt; }

std::optional<std::string> ShowSaveDskDialog(GLFWwindow *, const std::string &) { return std::nullopt; }

std::optional<std::string> ShowOpenFileDialog(GLFWwindow *, const std::string &, const std::string &, const std::string &) {
    return std::nullopt;
}

std::optional<std::string> ShowSaveFileDialog(GLFWwindow *, const std::string &, const std::string &, const std::string &,
                                              const std::string &, const std::string &) {
    return std::nullopt;
}

} // namespace msxdisk::gui

#endif // _WIN32
