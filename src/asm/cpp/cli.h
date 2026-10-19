// fwMSX -- montador (fwasm): linha de comando (fwmsx --asm) e comando "asm" do console. Mesmas opcoes do asMSX.
#pragma once

#include <string>
#include <vector>

#include "assembler.h"

namespace fwasm {

// tokens = argumentos depois de --asm: [-z] [-s] [-vv] [-r] [-o <caminho>] <arquivo.asm>. Devolve o codigo de saida
// do montador (0 = ok; 2 = uso incorreto ou arquivo ilegivel).
int RunAsmCommand(const std::vector<std::string> &tokens);

// Monta um arquivo do DISCO (os includes/incbin saem do diretorio dele). `dir` = pasta do fonte com a barra final ("" = atual) e `base` = o
// caminho do fonte sem a extensao (a base dos arquivos de saida). false + `error` so' se o arquivo nao abre; erros de montagem ficam em result.rc/msgs.
bool AssembleFile(const std::string &source, const AsmOptions &opts, AsmResult &result, std::string &dir, std::string &base, std::string &error);

// Grava as saidas de `r` ao lado do fonte (a chave sem ponto e' a extensao; com ponto, um caminho relativo ao fonte, do .FILENAME). As chaves de
// `skip` ficam de fora. `written` recebe os caminhos gravados.
bool SaveOutputs(const std::string &dir, const std::string &base, const AsmResult &r, const std::vector<std::string> &skip,
                 std::vector<std::string> &written, std::string &error);

} // namespace fwasm
