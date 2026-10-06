// fwMSX -- leitura de indices de pastas (estilo "Index of", servidor Abyss, usado por
// download.file-hunter.com) e do fMSX. Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#pragma once

#include <string>
#include <vector>

namespace romdb {

struct ListingEntry {
    std::string name;  // nome decodificado (ex.: "Full Set System ROMs for OpenMSX - 15-08-2026.zip")
    std::string url;   // URL completa para baixar ou abrir
    bool is_dir = false;
    std::string size;  // texto como no servidor ("32.06 MB"), vazio em pastas
    std::string type;  // tipo MIME ou vazio
};

// Entradas de um indice HTML. `base_url` termina em '/'. Ignora o link "../".
std::vector<ListingEntry> ParseListing(const std::string &html, const std::string &base_url);

// Data (DD-MM-AAAA) de um nome como "Full Set System ROMs for OpenMSX - 15-08-2026.zip".
// Devolve false se o nome nao tiver data.
bool ParseFullSetDate(const std::string &name, int &year, int &month, int &day);

// Entre os "Full Set System ROMs" de uma lista, o de data mais recente (nullptr se nenhum).
const ListingEntry *LatestFullSet(const std::vector<ListingEntry> &entries);

// Dentro da pagina de downloads do fMSX, a URL do pacote "fMSXNN-Windows-bin.zip" de
// versao mais alta. Vazio se nao houver.
std::string LatestFmsxWindowsZip(const std::string &html, const std::string &base_url);

} // namespace romdb
