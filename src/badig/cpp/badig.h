// fwMSX -- clone do Basic Dignified Suite (MSX): converte `.dmx` (Dignified) em `.amx` (BASIC ASCII classico) e em `.bmx` (BASIC tokenizado).
// Codigo ORIGINAL do fwMSX (BSD-3-Clause), reescrito a partir do comportamento do badig.py v2.2.1 (Fred Rique) e do tokenizador msxbatoken.
// Ver doc/badig-spec.md.
//
// Nucleo nas 4 linguagens: C++ (lexer, parser e gerador), C (tabela de palavras do tokenizador MSX), Fortran (arredondamento dos digitos
// das constantes reais em BCD) e Assembly NASM (comparacao de prefixo sem diferenciar maiusculas).
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace badig {

// Uma mensagem do Badig apontando (ou nao) para uma linha de um fonte.
struct Diag {
    int level = 1;            // 1 = erro (a conversao para), 2 = aviso
    std::string file;         // o arquivo como o Badig o viu (o .dmx ou um .include)
    int line = 0;             // base 1; 0 = mensagem sem posicao
    int col = 0;              // base 1
    std::string text;         // a mensagem sem a decoracao
    std::string formatted;    // como o badig.py imprime (com a linha do codigo e o ^)
};

// As opcoes do Badig (Settings do badig.py + as do modulo MSX + as do tokenizador). Os padroes sao os do badig.py, menos `tokenize`, que no fwMSX
// e' ligado: o objetivo do programa e' gerar o .bmx.
struct Options {
    int tab_length = 4;              // -tl: espacos por TAB (so' para as colunas dos erros)
    int line_start = 10;             // -ls
    int line_step = 10;              // -lp
    bool rem_header = true;          // -rh: false desliga o cabecalho REM
    bool strip_spaces = false;       // -ss
    bool capitalise_all = false;     // -ca
    bool translate = false;          // -tr: caracteres Unicode -> MSX (le o fonte como UTF-8)
    int verbose = 3;                 // -vb: 0 mudo, 1 erros, 2 +avisos, 3 +cabecalhos, 4 +detalhes, 5 +passadas
    bool print_report = false;       // -prr: mostra os relatorios em vez de gravar
    bool label_report = false;       // -lbr
    bool line_report = false;        // -lnr
    bool var_report = false;         // -var
    bool lexer_report = false;       // -lex
    bool parser_report = false;      // -par
    bool code_is_ascii = false;      // -asc: o arquivo ja' e' BASIC ASCII classico (so' tokeniza)
    std::string convert_print;       // -cp: "" (deixa como esta), "?" ou "p"
    std::string strip_then_goto;     // -tg: "" (deixa), "t" (tira o THEN antes do GOTO) ou "g" (tira o GOTO depois do THEN/ELSE)
    // tokenizador (--tk_*)
    bool tokenize = true;            // gera o .bmx
    int tk_list = 0;                 // --tk_list: bytes por linha do .lmx (0 = nao gera; maximo 32)
    bool tk_del_ascii = false;       // --tk_del_ascii: apaga o .amx depois de tokenizar
    int tk_verbose = 3;              // --tk_verbose
    // arquivos
    // disco de execucao (badigrun / F5)
    std::string disk_files;          // arquivos de apoio da pasta do fonte que vao junto para o disco (nomes ou mascaras, separados por espaco)
    bool nbasic = false;             // NestorBASIC: leva o NBASIC.BIN para o disco (ja' vai sozinho se o fonte inclui o nbasic.dmx)
    std::string source;              // o .dmx
    std::string dest;                // o .amx (vazio = o nome do fonte com a extensao .amx)
};

// Uma linha do BASIC gerado (.amx) e a linha do Dignified que a originou (a tabela do relatorio -lnr).
struct LineMapEntry {
    int basic = 0;       // numero da linha BASIC
    std::string file;    // o fonte como o Badig o viu (o .dmx ou um .include)
    int line = 0;        // base 1; 0 = linha gerada pelo Badig (cabecalho REM, rotinas automaticas...)
};

struct Result {
    bool ok = false;
    std::vector<LineMapEntry> line_map;         // sempre preenchida (nao so' com -lnr): leva um erro de execucao do BASIC de volta ao fonte
    std::string amx_path, bmx_path, lmx_path;   // vazios se nao foram gerados
    std::string amx;                            // o texto do .amx (bytes latin1, linhas CRLF)
    std::vector<uint8_t> bmx;                   // o BASIC tokenizado
    std::string lmx;                            // a listagem do tokenizador
    std::vector<Diag> diags;                    // erros e avisos
    std::vector<std::string> log;               // o que o badig.py imprimiria (respeita `verbose`)
    std::vector<std::pair<std::string, std::string>> reports; // (caminho, conteudo) dos relatorios gravados
    int line_count = 0;                         // linhas do .amx
    std::vector<std::string> includes;          // os arquivos incluidos (caminhos como o Badig os abriu)
    Options options;                            // as opcoes que valeram (as da configuracao, da linha de comando e das remtags)
};

// Converte `opts.source`. `text` (se nao for nullptr) e' o conteudo do fonte (o buffer do editor): os .include continuam saindo do disco.
// `cli_args` sao os argumentos da linha de comando (-ss, --tk_list 16...) aplicados sobre `base`; a remtag `##BB:arguments=` do proprio fonte
// acrescenta mais argumentos, como no original. Grava o .amx (e o .bmx, o .lmx e os relatorios) em disco se `write_files`.
Result Convert(const Options &base, const std::vector<std::string> &cli_args, const std::string *text, bool write_files);

// Aplica os argumentos (os do badig.py: -ls 100 -ss ...) sobre `o`; os dois nomes posicionais vao para source/dest. false + `err` se invalido.
bool ApplyArgs(const std::vector<std::string> &args, Options &o, std::string &err);

// Configuracao em arquivo INI (badig.cfg): secoes [CONFIGS] (as chaves do badig.ini), [MSX] e [TOKENIZER].
bool LoadConfig(const std::string &path, Options &o);
bool SaveConfig(const std::string &path, const Options &o, std::string &err);

// A pasta da biblioteca (ao lado do executavel, badig/): o nbasic.dmx que `include "nbasic.dmx"` acha quando nao ha' um na pasta do fonte, e o NBASIC.BIN.
void SetLibraryDir(const std::string &dir);
const std::string &LibraryDir();

// Leitor de fontes que vale ANTES do disco para os .include (o editor com um projeto aberto: o fonte e' o do banco `.msxproject`, nao a copia da pasta,
// que pode estar atrasada). Devolve false = "nao tenho": cai no disco. Vazio (padrao) = so' o disco. `path` e' como o Badig o abriria.
using SourceReader = std::function<bool(const std::string &path, std::string &data)>;
void SetSourceReader(SourceReader r);
bool SourceReadable(const std::string &path); // o leitor tem, ou o arquivo existe em disco

// Os arquivos que vao para o disquete de execucao alem do .bmx e do AUTOEXEC.BAS: os `disk_files` e, com NestorBASIC, o NBASIC.BIN. host = onde esta';
// msx = o nome 8.3 em maiusculas no disco. false + `err` se um nome nao serve, um arquivo nao existe ou o NBASIC.BIN nao foi achado.
struct DiskFile {
    std::string host, msx;
};
bool CollectDiskFiles(const Result &r, std::vector<DiskFile> &out, std::string &err);

// O numero da linha BASIC de uma mensagem de execucao ("Syntax error in 130", "Type mismatch in 40"), ou um numero solto ("130"): o ultimo
// numero do texto, depois de " in " se houver. 0 se nao achou.
int ParseBasicErrorLine(const std::string &text);
// A linha do fonte que gerou a linha BASIC `basic`. false se o BASIC nao tem essa linha; `entry.line` = 0 quando e' linha gerada pelo Badig.
bool MapBasicLine(const Result &r, int basic, LineMapEntry &entry);

// Onde fica o badig.cfg: quem hospeda o programa (a janela, o terminal, o editor) diz uma vez; vazio = sem arquivo (so' os padroes).
void SetConfigPath(const std::string &path);
const std::string &ConfigPath();
// Os padroes: Options{} com o badig.cfg por cima (se existir).
Options LoadDefaults();

// `fwmsx --badig <argumentos>` e o comando `badig` do console: devolve o codigo de saida (0 = ok).
int RunBadigCommand(const std::vector<std::string> &args, const std::string &config_path);

// So' o tokenizador (msxbatoken): `ascii` e' o BASIC ASCII (linhas numeradas). Devolve false e `err` se algo para a conversao.
// `list_bytes` > 0 tambem monta a listagem (.lmx) com esse numero de bytes por linha.
bool Tokenize(const std::string &ascii, const std::string &file_name, int list_bytes, std::vector<uint8_t> &out, std::string &lmx, Diag &err);

} // namespace badig
