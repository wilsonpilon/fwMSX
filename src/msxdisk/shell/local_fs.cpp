//
// msxdisk (fwMSX): comandos locais do shell interativo -- ver local_fs.h.
//

#include "local_fs.h"

#include <cctype>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <vector>

namespace msxdisk::shell {

// Casamento de coringas generico (qualquer tamanho de string, nao apenas
// nomes 8.3 do MSX-DOS -- por isso nao reaproveita o msxdisk_name_match
// em Assembly, que so trabalha com os 11 bytes fixos do formato MSX).
// Algoritmo classico de duas pontas com retrocesso em '*'.
bool LocalWildcardMatch(const std::string &pattern, const std::string &text) {
    size_t p = 0;
    size_t t = 0;
    size_t star_p = std::string::npos;
    size_t star_t = 0;

    const auto lower = [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); };

    while (t < text.size()) {
        if (p < pattern.size() && (pattern[p] == '?' || lower(pattern[p]) == lower(text[t]))) {
            ++p;
            ++t;
        } else if (p < pattern.size() && pattern[p] == '*') {
            star_p = p++;
            star_t = t;
        } else if (star_p != std::string::npos) {
            p = star_p + 1;
            t = ++star_t;
        } else {
            return false;
        }
    }
    while (p < pattern.size() && pattern[p] == '*') ++p;
    return p == pattern.size();
}

void CmdLocalList(const std::string &pattern) {
    namespace fs = std::filesystem;

    std::error_code ec;
    std::vector<fs::directory_entry> entries;
    for (const auto &entry : fs::directory_iterator(fs::current_path(), ec)) {
        if (!pattern.empty() && !LocalWildcardMatch(pattern, entry.path().filename().string())) continue;
        entries.push_back(entry);
    }
    if (ec) {
        std::cerr << "ls: " << ec.message() << std::endl;
        return;
    }
    if (entries.empty()) {
        std::cout << "(vazio)" << std::endl;
        return;
    }

    for (const auto &entry : entries) {
        if (entry.is_directory()) {
            std::cout << "     <DIR>  " << entry.path().filename().string() << std::endl;
        } else {
            std::error_code size_ec;
            const auto size = fs::file_size(entry.path(), size_ec);
            std::cout << std::setw(10) << (size_ec ? 0 : static_cast<uintmax_t>(size)) << "  "
                       << entry.path().filename().string() << std::endl;
        }
    }
}

void CmdLocalChangeDir(const std::string &path) {
    std::error_code ec;
    std::filesystem::current_path(path, ec);
    if (ec) {
        std::cerr << "cd: " << ec.message() << std::endl;
        return;
    }
    std::cout << std::filesystem::current_path().string() << std::endl;
}

void CmdLocalMakeDir(const std::string &path) {
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    if (ec) {
        std::cerr << "md: " << ec.message() << std::endl;
        return;
    }
    std::cout << "Diretorio criado: " << path << std::endl;
}

void CmdLocalRemoveFile(const std::string &path) {
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        std::cerr << "rm: '" << path << "' nao encontrado" << std::endl;
        return;
    }
    if (std::filesystem::is_directory(path, ec)) {
        std::cerr << "rm: '" << path << "' e um diretorio -- rm so remove arquivos" << std::endl;
        return;
    }
    if (!std::filesystem::remove(path, ec) || ec) {
        std::cerr << "rm: falha ao remover '" << path << "': " << ec.message() << std::endl;
        return;
    }
    std::cout << "Removido: " << path << std::endl;
}

void CmdLocalPrintWorkingDir() {
    std::error_code ec;
    std::cout << std::filesystem::current_path(ec).string() << std::endl;
}

} // namespace msxdisk::shell
