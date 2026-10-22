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
    bool dos = false;     // o disco leva o MSX-DOS (um .COM so' roda assim)
    int dos_version = 0;  // `dos: 1` ou `dos: 2`: monta o sistema a partir da pasta emulator/msxdos1 ou msxdos2 (SetEmulatorDir)
    bool dos_tools = false; // `dos_tools: true` (MSX-DOS 2): leva tambem UTILS, HELP e o AUTOEXEC.BAT que os poe no PATH
    std::string dos_disk; // `dos: true` (antigo): parte deste disco de sistema pronto (relativo a pasta do Obifile); vazio = procura msxdos1.dsk
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

// A pasta `emulator` ao lado do executavel: msxdos1/ e msxdos2/ com os arquivos do sistema (copiados de resource/ na build) e os discos de
// sistema gerados. Vazia = procura "emulator" e "resource" a partir da pasta atual.
void SetEmulatorDir(const std::string &dir);
const std::string &EmulatorDir();

// O disquete de sistema puro do MSX-DOS `version` (1 ou 2) gravado em `out`; `tools` como no Obifile. false + `msg` se faltam os arquivos.
bool BuildSystemDisk(int version, bool tools, const std::string &out, std::string &msg);

// `fwmsx --obi build [Obifile]` e o comando `obi` do console: devolve o codigo de saida (0 = ok).
int RunObiCommand(const std::vector<std::string> &args);

// Procura o Obifile em `dir`: "Obifile" ou o primeiro "*.obi". Vazio se nao achou.
std::string FindObifile(const std::string &dir);

} // namespace obi
