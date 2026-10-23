// fwMSX -- assinaturas das ROMs de sistema do openMSX: le os XML de maquinas (share/machines) e extensoes (share/extensions) e devolve, para cada
// SHA-1 que o openMSX reconhece, o nome oficial do arquivo, o papel (id do dispositivo ROM ou nome da extensao), a familia e a maquina/extensao que
// a usa. Sao so' fatos (hashes e nomes); nenhum codigo do openMSX. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/romdb-spec.md, secao 7.
#pragma once

#include <string>
#include <vector>

#include "../store/romdb.h"

namespace romdb {

// Le UM arquivo XML do openMSX. `stem` = o nome do arquivo sem extensao (usado se o XML nao traz nome/fabricante); `extension` = veio de share/extensions.
void ParseOpenmsxXml(const std::string &xml, const std::string &stem, bool extension, std::vector<SigRow> &out);

// Le as pastas share/machines e share/extensions de `share_dir` (a pasta "share" do openMSX). Devolve quantas linhas leu; false com `error` se a pasta nao serve.
bool ReadOpenmsxShare(const std::string &share_dir, std::vector<SigRow> &rows, int &files, std::string &error);

}  // namespace romdb
