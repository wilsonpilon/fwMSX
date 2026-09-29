//
// msxdisk (fwMSX): dialogos nativos de arquivo do Windows -- ver
// file_dialog.h.
//

#include "file_dialog.h"

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

} // namespace msxdisk::gui
