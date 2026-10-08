// fwMSX -- caminhos padrao do banco de fitas. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#include "service.h"

#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

namespace tapedb {

std::string TapeDbPaths::db_file() const { return (fs::path(root) / "fitas.db").string(); }

TapeDbPaths DefaultTapeDbPaths(const std::string &argv0) {
    std::error_code ec;
    const fs::path exe_dir = fs::absolute(fs::path(argv0), ec).parent_path();
    TapeDbPaths p;
    p.root = (exe_dir.empty() ? fs::path(".") : exe_dir / "fitas").string();
    return p;
}

bool OpenTapeDb(const TapeDbPaths &paths, TapeDb &db, std::string &error) {
    std::error_code ec;
    fs::create_directories(paths.root, ec);
    if (ec) {
        error = "nao foi possivel criar a pasta de fitas '" + paths.root + "': " + ec.message();
        return false;
    }
    return db.Open(paths.db_file(), error);
}

} // namespace tapedb
