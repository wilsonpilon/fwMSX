// fwMSX -- "fwmsx --hdnew": cria (e inspeciona) imagens de disco rigido/cartao SD para o Nextor pela linha de comando. Ver doc/hd-spec.md.
#pragma once

#include <string>
#include <vector>

namespace hd {

// fwmsx --hdnew <arquivo> <tamanho> [tamanho...]   uma particao FAT16 por tamanho (512, 512M, 2G); ate' 4 particoes de ate' 2G, soma ate' 4G
// fwmsx --hdnew --split <total> <n> <arquivo>      n particoes iguais cobrindo o total (ex.: 4G em 2 de 2G)
// fwmsx --hdnew --info <arquivo>                   mostra as particoes de uma imagem
// Sem argumentos mostra o uso. Devolve o codigo de saida.
int RunHdNewCommand(const std::vector<std::string> &args);

// As linhas de `--info` (e do comando `hdinfo` do console) de uma imagem; false + `error` se nao abre.
bool DescribeImage(const std::string &path, std::string &text, std::string &error);

} // namespace hd
