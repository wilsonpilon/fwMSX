// fwMSX -- "fwmsx --disknew": cria um disquete MSX em branco e formatado pela linha de comando.
#pragma once

#include <string>
#include <vector>

namespace diskfmt {

// fwmsx --disknew <arquivo.dsk> <ss525|ds525|ss35|ds35>   (sem argumentos: mostra o uso).
int RunDiskNewCommand(const std::vector<std::string> &args);

} // namespace diskfmt
