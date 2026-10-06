// fwMSX -- acesso HTTP/HTTPS para o banco de ROMs. Usa o programa `curl`, que vem
// no Windows 10 (1803 ou mais recente) e na maioria dos Linux. Codigo ORIGINAL do
// fwMSX (BSD-3-Clause). Ver doc/romdb-spec.md, secao 4.
#pragma once

#include <string>

namespace romdb {

// Texto da URL (HTML, JSON...). false com `error` se falhar.
bool HttpGetText(const std::string &url, std::string &out, std::string &error);

// Baixa a URL para o arquivo `path` (cria o arquivo). false com `error` se falhar.
bool HttpDownload(const std::string &url, const std::string &path, std::string &error);

// Codifica um nome de arquivo para usar num caminho de URL (espacos e acentos viram %XX).
std::string UrlEncodePath(const std::string &name);

// Decodifica %XX de uma URL (o caminho que o servidor devolve no indice).
std::string UrlDecode(const std::string &text);

} // namespace romdb
