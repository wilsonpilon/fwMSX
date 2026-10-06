// fwMSX -- extracao de ZIP com miniz. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "unzip.h"

#include <cstring>
#include <filesystem>
#include <utility>

#include "miniz.h"

namespace fs = std::filesystem;

namespace romdb {
namespace {

// Nome relativo e seguro: sem "..", sem barra inicial, sem letra de unidade.
bool SafeRelative(const std::string &name) {
    if (name.empty() || name[0] == '/' || name[0] == '\\') return false;
    if (name.size() >= 2 && name[1] == ':') return false;
    size_t start = 0;
    while (start <= name.size()) {
        size_t end = name.find_first_of("/\\", start);
        if (end == std::string::npos) end = name.size();
        const std::string part = name.substr(start, end - start);
        if (part == "..") return false;
        start = end + 1;
    }
    return true;
}

} // namespace

bool ExtractZip(const std::string &zip_path, const std::string &dest_dir, std::vector<std::string> &files,
                std::string &error) {
    mz_zip_archive zip;
    std::memset(&zip, 0, sizeof zip);
    if (!mz_zip_reader_init_file(&zip, zip_path.c_str(), 0)) {
        error = "nao foi possivel abrir o ZIP '" + zip_path + "'";
        return false;
    }
    const mz_uint count = mz_zip_reader_get_num_files(&zip);
    std::error_code ec;
    for (mz_uint i = 0; i < count; ++i) {
        mz_zip_archive_file_stat st;
        if (!mz_zip_reader_file_stat(&zip, i, &st)) continue;
        const std::string name = st.m_filename;
        if (!SafeRelative(name)) continue;
        const fs::path out = fs::path(dest_dir) / fs::path(name);
        if (mz_zip_reader_is_file_a_directory(&zip, i)) {
            fs::create_directories(out, ec);
            continue;
        }
        fs::create_directories(out.parent_path(), ec);
        if (!mz_zip_reader_extract_to_file(&zip, i, out.string().c_str(), 0)) {
            error = "falha ao extrair '" + name + "' do ZIP";
            mz_zip_reader_end(&zip);
            return false;
        }
        files.push_back(out.string());
    }
    mz_zip_reader_end(&zip);
    return true;
}

bool WriteZip(const std::string &zip_path, const std::vector<std::pair<std::string, std::string>> &entries,
              std::string &error) {
    mz_zip_archive zip;
    std::memset(&zip, 0, sizeof zip);
    if (!mz_zip_writer_init_file(&zip, zip_path.c_str(), 0)) {
        error = "nao foi possivel criar o ZIP '" + zip_path + "'";
        return false;
    }
    for (const auto &entry : entries) {
        if (!mz_zip_writer_add_mem(&zip, entry.first.c_str(), entry.second.data(), entry.second.size(),
                                   MZ_DEFAULT_COMPRESSION)) {
            error = "falha ao gravar '" + entry.first + "' no ZIP";
            mz_zip_writer_end(&zip);
            return false;
        }
    }
    mz_zip_writer_finalize_archive(&zip);
    mz_zip_writer_end(&zip);
    return true;
}

} // namespace romdb
