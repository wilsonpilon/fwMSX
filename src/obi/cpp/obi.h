// fwMSX -- obi: a receita (Obifile) que monta um disquete .dsk pronto para rodar. Codigo ORIGINAL do fwMSX (BSD-3-Clause); a sintaxe do
// Obifile e' o subconjunto do Kizuna (chave: valor, listas "- campo: valor"), mas aqui a receita junta os arquivos do programa em UM disco
// em vez de chamar o linker do Kizuna. Ver doc/obi-spec.md.
//
// Nucleo nas 4 linguagens: C++ (receita, build e comando), C (nomes 8.3 e tipo pela extensao), Fortran (quanto do disco os arquivos
// ocupam) e Assembly NASM (CRC-32 de cada arquivo no log).
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace obi {

// Um arquivo da receita. `source` e' relativo a pasta do Obifile; `name` (opcional) e' o nome 8.3 no disco.
struct FileSpec {
    std::string source;
    std::string name;
    int line = 0; // a linha do Obifile (para as mensagens)
};

struct Recipe {
    std::string target;   // o .dsk a gerar (relativo a pasta do Obifile)
    std::string boot;     // o arquivo (nome no disco) que roda sozinho; vazio = o primeiro .BMX, se houver
    bool dos = false;     // parte de um disco de sistema do MSX-DOS (MSXDOS.SYS + COMMAND.COM) em vez de um disco em branco
    std::string dos_disk; // o disco de sistema (relativo a pasta do Obifile); vazio = procura msxdos1.dsk
    std::vector<FileSpec> files;
};

// Le o texto de um Obifile. false + `err` ("Obifile, line N: ...").
bool ParseRecipe(const std::string &text, Recipe &out, std::string &err);

// Quem le os arquivos da receita (o Obifile, os fontes, os dados). Vazio = o disco. O editor passa o leitor do projeto: o fonte e' o do banco
// `.msxproject`, nao a copia da pasta. `path` e' absoluto (a pasta do Obifile + o caminho da receita). false = "nao tenho": cai no disco.
using Reader = std::function<bool(const std::string &path, std::vector<uint8_t> &data)>;

struct BuiltFile {
    std::string name; // no disco
    size_t size = 0;
    uint32_t crc = 0;
    std::string from; // a origem, como a receita escreveu
};

// Uma mensagem da build com lugar no fonte (o editor leva o cursor para ela). `file` e' o caminho do arquivo (vazio = mensagem geral).
struct Message {
    std::string file;
    int line = 0;
    bool warning = false;
    std::string text;
};

struct BuildResult {
    bool ok = false;
    std::vector<std::string> log; // o que mostrar: erros e avisos dos montadores (no formato deles, "arq, line N: ...") e o resumo
    std::vector<Message> msgs;    // as mesmas mensagens com arquivo e linha
    std::string dsk;              // o .dsk gravado
    std::vector<BuiltFile> files;
    long free_bytes = 0;          // o que sobrou no disco
};

struct BuildOptions {
    std::string obifile; // o caminho do Obifile
    Reader reader;       // veja Reader
    bool write = true;   // false = monta tudo e confere, sem gravar o .dsk (so' valida)
};

BuildResult Build(const BuildOptions &opts);

// `fwmsx --obi build [Obifile]` e o comando `obi` do console: devolve o codigo de saida (0 = ok).
int RunObiCommand(const std::vector<std::string> &args);

// Procura o Obifile em `dir`: "Obifile" ou o primeiro "*.obi". Vazio se nao achou.
std::string FindObifile(const std::string &dir);

} // namespace obi
