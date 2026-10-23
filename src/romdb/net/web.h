// fwMSX -- acesso HTTP/HTTPS para o banco de ROMs. Usa o programa `curl`, que vem
// no Windows 10 (1803 ou mais recente) e na maioria dos Linux. Codigo ORIGINAL do
// fwMSX (BSD-3-Clause). Ver doc/romdb-spec.md, secao 4.
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace romdb {

// Texto da URL (HTML, JSON...). false com `error` se falhar.
bool HttpGetText(const std::string &url, std::string &out, std::string &error);

// Baixa a URL para o arquivo `path` (cria o arquivo). false com `error` se falhar.
bool HttpDownload(const std::string &url, const std::string &path, std::string &error);

// Andamento do download em curso (thread-safe, para a interface desenhar a barra de qualquer thread). `total` = -1 quando o servidor nao diz o tamanho;
// `active` e' false fora de um download. O curl roda sem janela de console (Windows: CREATE_NO_WINDOW); o andamento sai do tamanho do arquivo no disco.
struct DownloadProgress {
    bool active = false;
    int64_t done = 0;
    int64_t total = -1;
    std::string name;  // o arquivo que esta' sendo baixado (so' o nome)
    bool items = false;  // true: done/total contam arquivos (HttpDownloadMany), nao bytes
};
DownloadProgress CurrentProgress();
// Quem quer escrever o andamento (a CLI) registra um gancho; chamado da thread do download, a cada ~0,2 s.
void SetProgressHook(std::function<void(const DownloadProgress &)> hook);

// Baixa varios arquivos de uma vez com UM processo do curl (a conexao e' reaproveitada): `jobs` = {url, caminho de destino}. O andamento conta arquivos.
// Para no primeiro erro. As pastas de destino precisam existir.
bool HttpDownloadMany(const std::vector<std::pair<std::string, std::string>> &jobs, const std::string &label, std::string &error);

// Codifica um nome de arquivo para usar num caminho de URL (espacos e acentos viram %XX).
std::string UrlEncodePath(const std::string &name);

// Decodifica %XX de uma URL (o caminho que o servidor devolve no indice).
std::string UrlDecode(const std::string &text);

} // namespace romdb
