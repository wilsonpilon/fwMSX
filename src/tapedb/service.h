// fwMSX -- caminhos padrao do banco de fitas (`fwmsx --fitadb`). Codigo
// ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/tape-spec.md, secao 11.
#pragma once

#include <string>

#include "store/tapedb.h"

namespace tapedb {

// Pasta de fitas e banco. Padrao: `fitas/` ao lado do executavel.
struct TapeDbPaths {
    std::string root;
    std::string db_file() const; // root/fitas.db
};

// Padrao da pasta de fitas: `fitas/` ao lado do executavel (argv0), ou no diretorio atual.
TapeDbPaths DefaultTapeDbPaths(const std::string &argv0);

// Cria a pasta e abre o banco.
bool OpenTapeDb(const TapeDbPaths &paths, TapeDb &db, std::string &error);

} // namespace tapedb
