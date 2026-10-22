// fwMSX -- downloads (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver download.h e doc/download-spec.md.
#include "download.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <set>
#include <thread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "../../romdb/net/web.h"
#include "../c/dl_match.h"

namespace fs = std::filesystem;

namespace dl {

const char *const kFileHunterBase = "https://download.file-hunter.com/";
const char *const kFileHunterIndexUrl = "https://download.file-hunter.com/allfiles.txt";

namespace {

constexpr int kMinGapMs = 1200;  // entre o inicio de dois downloads

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

bool LessCi(const std::string &a, const std::string &b) { return Lower(a) < Lower(b); }

} // namespace

// ---------------------------------------------------------------------------------------------------------------
// Indice
// ---------------------------------------------------------------------------------------------------------------

size_t Index::Parse(const std::string &text) {
    paths_.clear();
    size_t pos = 0;
    if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF && static_cast<unsigned char>(text[1]) == 0xBB && static_cast<unsigned char>(text[2]) == 0xBF) pos = 3;
    while (pos < text.size()) {
        size_t nl = text.find('\n', pos);
        if (nl == std::string::npos) nl = text.size();
        std::string line = text.substr(pos, nl - pos);
        pos = nl + 1;
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
        if (line.empty()) continue;
        for (char &c : line)
            if (c == '\\') c = '/';
        paths_.push_back(std::move(line));
    }
    return paths_.size();
}

void Index::List(const std::string &dir, std::vector<std::string> &folders, std::vector<size_t> &files) const {
    folders.clear();
    files.clear();
    const std::string prefix = dir.empty() ? std::string() : dir + "/";
    std::set<std::string> subs;
    for (size_t i = 0; i < paths_.size(); ++i) {
        const std::string &p = paths_[i];
        if (p.size() <= prefix.size() || p.compare(0, prefix.size(), prefix) != 0) continue;
        const size_t slash = p.find('/', prefix.size());
        if (slash == std::string::npos) files.push_back(i);
        else subs.insert(p.substr(prefix.size(), slash - prefix.size()));
    }
    folders.assign(subs.begin(), subs.end());
    std::sort(folders.begin(), folders.end(), LessCi);
    std::sort(files.begin(), files.end(), [this](size_t a, size_t b) { return LessCi(paths_[a], paths_[b]); });
}

std::vector<size_t> Index::Search(const std::string &query, size_t limit, bool *truncated) const {
    std::vector<std::string> words;
    size_t i = 0;
    while (i < query.size()) {
        while (i < query.size() && query[i] == ' ') ++i;
        size_t j = i;
        while (j < query.size() && query[j] != ' ') ++j;
        if (j > i) words.push_back(query.substr(i, j - i));
        i = j;
    }
    std::vector<size_t> out;
    if (truncated) *truncated = false;
    if (words.empty()) return out;
    for (size_t k = 0; k < paths_.size(); ++k) {
        const std::string &p = paths_[k];
        bool all = true;
        for (const std::string &w : words) {
            if (!dl_contains_ci(p.data(), static_cast<int>(p.size()), w.data(), static_cast<int>(w.size()))) {
                all = false;
                break;
            }
        }
        if (!all) continue;
        if (out.size() >= limit) {
            if (truncated) *truncated = true;
            break;
        }
        out.push_back(k);
    }
    return out;
}

std::string FileHunterUrl(const std::string &index_path) { return std::string(kFileHunterBase) + romdb::UrlEncodePath(index_path); }

// ---------------------------------------------------------------------------------------------------------------
// Catalogo
// ---------------------------------------------------------------------------------------------------------------

const std::vector<Entry> &Catalog() {
    static const std::vector<Entry> catalog = [] {
        const std::string raw = "https://raw.githubusercontent.com/Konamiman/MSX/master/SRC/NBASIC/";
        std::vector<Entry> c;
        auto item = [&](Entry &e, const std::string &sub, const std::string &file, const std::string &install = std::string()) {
            Item it;
            it.name = file;
            it.url = raw + (sub.empty() ? std::string() : sub + "/") + file;
            it.dest = "konamiman/" + e.id + "/" + file;
            it.install = install;
            e.items.push_back(it);
        };
        Entry nb;
        nb.id = "nbasic";
        nb.title = "NestorBASIC 1.11 (Konamiman)";
        nb.note = "Extensao do BASIC para MSX-DOS (arquivos, matrizes, som, tela). github.com/Konamiman/MSX, SRC/NBASIC; o repositorio nao declara licenca, por isso o fwMSX nao o distribui: "
                  "baixe aqui. O NBASIC.BIN tambem vai para badig/, onde o Basic Dignified o procura.";
        item(nb, "", "NBASIC.BIN", "badig/NBASIC.BIN");
        item(nb, "", "NBASIC.DAT");
        item(nb, "", "NBASIC.ASM");
        item(nb, "", "nbas111e.txt");
        c.push_back(nb);
        Entry see;
        see.id = "see";
        see.title = "SEE 3.10 (Konamiman, em SRC/NBASIC/see)";
        see.note = "Os arquivos da pasta see/ do NestorBASIC: o editor/tocador SEE e a biblioteca SEEV3_10.LIB. github.com/Konamiman/MSX.";
        for (const char *f : {"SEEV3_10.BIN", "SEEV3_10.LIB", "SEEV3_10.BAS", "SEE3PLAY.ASM", "SEE3HELP.TXT", "SEE3HELP.TED", "SEE3.PRF", "SEE.BAT", "PLICS.SEE", "SEEDRUMS.SEE"}) item(see, "see", f);
        c.push_back(see);
        Entry np;
        np.id = "nplayer";
        np.title = "NPLAYER (Konamiman, em SRC/NBASIC)";
        np.note = "Tocador de musica do NestorBASIC. github.com/Konamiman/MSX.";
        for (const char *f : {"NPLAYER.BAS", "NPLAYER.DAT", "NPLAYER.PRG", "NPLAYER.TXT"}) item(np, "nplayer", f);
        item(np, "", "NPLAYER.ASM");
        c.push_back(np);
        return c;
    }();
    return catalog;
}

// ---------------------------------------------------------------------------------------------------------------
// Caminhos
// ---------------------------------------------------------------------------------------------------------------

bool SafeRelative(const std::string &rel) {
    if (rel.empty() || rel.front() == '/') return false;
    size_t i = 0;
    while (i <= rel.size()) {
        size_t j = rel.find('/', i);
        if (j == std::string::npos) j = rel.size();
        const std::string comp = rel.substr(i, j - i);
        if (comp.empty() || comp == "." || comp == ".." || comp == ".part") return false;
        for (const char c : comp)
            if (c == '\\' || c == ':' || static_cast<unsigned char>(c) < 32) return false;
        i = j + 1;
    }
    return true;
}

std::string LocalComponent(const std::string &name) {
    std::string out;
    for (const char c : name) {
        const unsigned char u = static_cast<unsigned char>(c);
        out.push_back((u < 32 || std::string("<>:\"|?*\\").find(c) != std::string::npos) ? '_' : c);
    }
    while (!out.empty() && (out.back() == '.' || out.back() == ' ')) out.pop_back();
    return out.empty() ? std::string("_") : out;
}

std::string FileHunterRel(const std::string &index_path) {
    std::string rel = "filehunter";
    size_t i = 0;
    while (i <= index_path.size()) {
        size_t j = index_path.find('/', i);
        if (j == std::string::npos) j = index_path.size();
        rel += "/" + LocalComponent(index_path.substr(i, j - i));
        i = j + 1;
    }
    return rel;
}

std::string ExeDir() {
#ifdef _WIN32
    char buf[MAX_PATH * 2];
    const DWORD n = GetModuleFileNameA(nullptr, buf, sizeof(buf));
    if (n > 0 && n < sizeof(buf)) return fs::path(std::string(buf, n)).parent_path().generic_string();
#else
    char buf[4096];
    const ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n > 0) return fs::path(std::string(buf, static_cast<size_t>(n))).parent_path().string();
#endif
    std::error_code ec;
    return fs::current_path(ec).generic_string();
}

// ---------------------------------------------------------------------------------------------------------------
// Rede
// ---------------------------------------------------------------------------------------------------------------

bool EnsureIndex(const Paths &paths, bool force, bool &fresh, std::string &error) {
    fresh = false;
    std::error_code ec;
    const fs::path cache(paths.IndexCache());
    if (!force && fs::is_regular_file(cache, ec) && fs::file_size(cache, ec) > 1000) return true;
    std::string text;
    if (!romdb::HttpGetText(kFileHunterIndexUrl, text, error)) return false;
    if (text.size() < 1000 || text.find('/') == std::string::npos) {
        error = "o indice do File Hunter veio vazio ou em outro formato";
        return false;
    }
    fs::create_directories(cache.parent_path(), ec);
    const fs::path tmp = cache.string() + ".part";
    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        f.write(text.data(), static_cast<std::streamsize>(text.size()));
        if (!f) {
            error = "nao foi possivel gravar " + tmp.string();
            return false;
        }
    }
    fs::rename(tmp, cache, ec);
    if (ec) {
        error = "nao foi possivel gravar " + cache.string() + ": " + ec.message();
        return false;
    }
    fresh = true;
    return true;
}

bool LoadIndex(const Paths &paths, Index &index, std::string &error) {
    std::ifstream f(paths.IndexCache(), std::ios::binary);
    if (!f) {
        error = "o indice ainda nao foi baixado (" + paths.IndexCache() + ")";
        return false;
    }
    const std::string text((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (index.Parse(text) == 0) {
        error = "o indice esta vazio";
        return false;
    }
    return true;
}

bool Fetch(const Paths &paths, const std::string &url, const std::string &rel_dest, const std::string &install, bool overwrite, std::string &out_path,
           bool &skipped, std::string &error) {
    skipped = false;
    if (!SafeRelative(rel_dest) || (!install.empty() && !SafeRelative(install))) {
        error = "caminho de destino recusado: " + rel_dest;
        return false;
    }
    std::error_code ec;
    const fs::path target = fs::path(paths.root()) / fs::path(rel_dest);
    out_path = target.string();
    if (!overwrite && fs::is_regular_file(target, ec)) {
        skipped = true;
    } else {
        // intervalo minimo entre dois downloads (um processo so'; as janelas e a linha de comando passam por aqui)
        static std::mutex gate;
        static std::chrono::steady_clock::time_point last{};
        std::lock_guard<std::mutex> lock(gate);
        const auto now = std::chrono::steady_clock::now();
        if (last.time_since_epoch().count() != 0 && now - last < std::chrono::milliseconds(kMinGapMs)) std::this_thread::sleep_for(std::chrono::milliseconds(kMinGapMs) - (now - last));
        last = std::chrono::steady_clock::now();
        // o curl grava num nome NOSSO e simples: nenhum nome vindo do servidor chega a um comando de shell
        static std::atomic<int> counter{0};
        const fs::path part_dir = fs::path(paths.root()) / ".part";
        fs::create_directories(part_dir, ec);
        const fs::path part = part_dir / (std::to_string(++counter) + ".part");
        if (!romdb::HttpDownload(url, part.string(), error)) {
            fs::remove(part, ec);
            return false;
        }
        fs::create_directories(target.parent_path(), ec);
        fs::remove(target, ec);
        fs::rename(part, target, ec);
        if (ec) {
            error = "nao foi possivel gravar " + target.string() + ": " + ec.message();
            fs::remove(part, ec);
            return false;
        }
    }
    if (!install.empty()) {
        const fs::path copy = fs::path(paths.exe_dir) / fs::path(install);
        fs::create_directories(copy.parent_path(), ec);
        fs::copy_file(target, copy, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            error = "baixado, mas nao foi possivel copiar para " + copy.string() + ": " + ec.message();
            return false;
        }
    }
    return true;
}

} // namespace dl
