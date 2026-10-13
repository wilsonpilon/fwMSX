// fwMSX -- montador (fwasm): interface publica (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause), escrito a partir
// do manual do asMSX 1.2.0 e do comportamento observado do programa (caixa-preta); NAO e' derivado do fonte
// dele (GPLv3). Ver doc/asm-spec.md.
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace fwasm {

struct AsmOptions {
    bool zilog = false;    // -z: sintaxe Zilog sem precisar da diretiva .ZILOG
    bool silent = true;    // -s: sem mensagens de progresso
    bool verbose = false;  // -vv
    std::string out_base;  // -o: nome-base (sem extensao) das saidas; vazio = nome do fonte
};

// Acesso a arquivos de .INCLUDE / .INCBIN (o teste usa um provedor em memoria).
class FileProvider {
public:
    virtual ~FileProvider() = default;
    // `path` ja' relativo ao diretorio do fonte principal. false se nao existe.
    virtual bool Read(const std::string &path, std::vector<uint8_t> &data) = 0;
};

struct AsmResult {
    int rc = 0;                       // codigo de saida (0 = ok)
    std::vector<std::string> msgs;    // erros e avisos, formatados ("x.asm, line 3: ...")
    // Arquivos gerados: extensao ("z80", "bin", "rom", "sym", "txt"...) -> conteudo.
    std::map<std::string, std::vector<uint8_t>> outs;
    std::string base_name;            // nome-base das saidas (sem extensao)
};

// Monta `text` (conteudo do arquivo `source_name`). `files` pode ser nullptr (sem includes).
AsmResult Assemble(const std::string &source_name, const std::string &text, const AsmOptions &opts, FileProvider *files);

} // namespace fwasm
