// fwMSX -- downloads: o que o usuario baixa item por item (File Hunter e utilitarios de sites especificos). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver doc/download-spec.md.
//
// Regras de boa vizinhanca (o File Hunter avisa que quem baixa agressivo leva ban de IP): NUNCA se baixa em massa. O site inteiro e' navegado e
// pesquisado em cima de UM arquivo de indice (`allfiles.txt`, ~3,5 MB, baixado uma vez e relido so' quando o usuario pede); cada arquivo so' e'
// baixado quando o usuario clica nele, um de cada vez, com um intervalo minimo entre dois downloads.
#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace dl {

// ---------------------------------------------------------------------------------------------------------------
// Indice do File Hunter
// ---------------------------------------------------------------------------------------------------------------

extern const char *const kFileHunterBase;      // https://download.file-hunter.com/
extern const char *const kFileHunterIndexUrl;  // .../allfiles.txt

class Index {
public:
    // `text` = o allfiles.txt: UTF-8 (com BOM), um caminho por linha com '\\' entre as pastas. Devolve quantos caminhos leu.
    size_t Parse(const std::string &text);
    size_t size() const { return paths_.size(); }
    const std::string &path(size_t i) const { return paths_[i]; }  // sempre com '/' entre as pastas

    // O conteudo de uma pasta ("" = a raiz; sem barra final): os nomes das subpastas (ordenados) e os indices dos arquivos (ordenados pelo nome).
    void List(const std::string &dir, std::vector<std::string> &folders, std::vector<size_t> &files) const;

    // Os caminhos que contem TODAS as palavras da busca (sem diferenciar maiusculas), no maximo `limit`; `truncated` diz se havia mais.
    std::vector<size_t> Search(const std::string &query, size_t limit, bool *truncated = nullptr) const;

private:
    std::vector<std::string> paths_;
};

// A URL de um arquivo do indice (cada trecho do caminho codificado: nenhum caractere do nome chega cru a um comando).
std::string FileHunterUrl(const std::string &index_path);

// ---------------------------------------------------------------------------------------------------------------
// Utilitarios de sites especificos (lista curada)
// ---------------------------------------------------------------------------------------------------------------

struct Item {
    std::string name;     // como aparece na lista
    std::string url;      // de onde baixar
    std::string dest;     // onde fica, relativo a pasta de downloads (sempre com '/')
    std::string install;  // opcional: uma copia a mais, relativa a pasta do executavel (o NBASIC.BIN vai para badig/)
};

struct Entry {
    std::string id;       // nome curto para a linha de comando
    std::string title;
    std::string note;     // o que e', de onde vem, licenca conhecida
    std::vector<Item> items;
};

const std::vector<Entry> &Catalog();

// ---------------------------------------------------------------------------------------------------------------
// Caminhos e download
// ---------------------------------------------------------------------------------------------------------------

struct Paths {
    std::string exe_dir;  // ao lado do executavel
    std::string root() const { return exe_dir + "/downloads"; }
    std::string IndexCache() const { return root() + "/filehunter/allfiles.txt"; }
};

// true se `rel` e' um caminho relativo seguro (sem "..", sem barra invertida, sem unidade "C:", sem componente vazio ou de controle).
bool SafeRelative(const std::string &rel);

// O nome de um arquivo/pasta no disco local: tira o que o Windows nao aceita (<>:"|?* e controles) trocando por '_'.
std::string LocalComponent(const std::string &name);

// Onde um arquivo do indice fica dentro de paths.root(): "filehunter/" + o caminho com cada trecho passado por LocalComponent.
std::string FileHunterRel(const std::string &index_path);

// A pasta do programa em execucao (onde ficam badig/, emulator/ e downloads/).
std::string ExeDir();

// Garante o indice em `paths.IndexCache()`: baixa se nao existe ou se `force`. `fresh` diz se baixou agora.
bool EnsureIndex(const Paths &paths, bool force, bool &fresh, std::string &error);
bool LoadIndex(const Paths &paths, Index &index, std::string &error);

// Baixa UM arquivo para `rel_dest` (dentro de paths.root()); se `install` nao e' vazio, copia para exe_dir/install. Espera o intervalo minimo
// entre dois downloads. Baixa para um temporario e so' entao troca pelo destino (um download interrompido nunca deixa meio arquivo).
// `skipped` = o destino ja' existia e `overwrite` era false. `out_path` = onde ficou.
bool Fetch(const Paths &paths, const std::string &url, const std::string &rel_dest, const std::string &install, bool overwrite, std::string &out_path,
           bool &skipped, std::string &error);

// `fwmsx --download <comando>` (catalog, get, index, ls, search, fetch); devolve o codigo de saida.
int RunDownloadCommand(const std::vector<std::string> &args, const std::string &exe_dir);

} // namespace dl
