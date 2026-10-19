// fwMSX -- Mamute: configuracao inicial (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md.
//
// Um arquivo de texto `chave=valor` (mamute.cfg) ao lado do executavel, no mesmo estilo do fwmsx.cfg. Arquivo
// ausente ou chave desconhecida = padrao. Chaves:
//   online=0|1            modo ao abrir o monitor (1 so' vale se houver maquina; senao cai em offline)
//   vram=16|128|192       tamanho da VRAM do modo offline, em KB
//   dump=0..3             modo inicial do comando C
//   cell.S.P=vazio|ram|rom|basic   tipo da celula do slot S (0-3), pagina P (0-3)
//   file.S.P=caminho      arquivo carregado na celula (so' para rom/basic)
//   half.S.P=1            usa os ULTIMOS 16 KB do arquivo (BIOS+BASIC de 32 KB repartidos em duas celulas)
//   skeys=16 caracteres   o teclado do comando S
//   color=F,B,R           as cores do XCO (frente, fundo e borda, indices 0-15 da paleta do MSX1)
#pragma once

#include <string>

namespace mamute {

enum class CellType { Empty, Ram, Rom, Basic, Unknown };

const char *CellName(CellType t);               // "VAZIO", "RAM", "ROM", "BASIC", "?"
bool ParseCellType(const std::string &s, CellType &t); // vazio|ram|rom|basic (sem distinguir maiusculas)

struct Config {
    bool start_online = false;
    int vram_kb = 16;
    int dump_mode = 0;
    std::string skeys = "1234qwerasdfzxcv"; // as teclas do comando S: os digitos 1 2 3 4 5 6 7 8 9 A B C D E F 0
    int color_fg = 3, color_bg = 1, color_border = 1; // XCO: verde claro sobre preto, como o monitor do Mamute original
    CellType cell[4][4];   // [slot][pagina]
    std::string file[4][4];
    bool half[4][4];

    Config(); // padrao: slot 3 todo RAM (um MSX1 simples), o resto vazio
};

// `exe_dir`/mamute.cfg (ou ./mamute.cfg sem pasta).
std::string ConfigPathFor(const std::string &exe_dir);

// A pasta do executavel, guardada uma vez pelo main; DefaultConfigPath() = essa pasta + mamute.cfg (ou ./mamute.cfg
// enquanto ninguem informou). E' o arquivo que o monitor le ao abrir e onde CFG SAVE grava.
void SetConfigDir(const std::string &exe_dir);
std::string DefaultConfigPath();

Config LoadConfig(const std::string &path);
bool SaveConfig(const std::string &path, const Config &cfg);

} // namespace mamute
