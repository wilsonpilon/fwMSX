// fwMSX -- criacao de arquivos .dsk em branco e formatados (C++ sobre o nucleo em C/Assembly
// de src/diskfmt/core). Ver doc/diskfmt-spec.md.
#pragma once

#include <string>

#include "../core/diskfmt.h"

namespace diskfmt {

// Cria `path` (sobrescreve se existir) com um disquete formatado do formato `spec`. O setor de
// boot leva o bootstrap do MSX-DOS 1 (o mesmo de msxdisk), entao o disco inicializa assim que
// receber MSXDOS.SYS e COMMAND.COM. false + `error` em caso de falha.
bool CreateBlankDisk(const std::string &path, const DiskFmtSpec *spec, std::string &error);

} // namespace diskfmt
