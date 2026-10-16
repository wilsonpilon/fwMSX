// fwMSX -- MAN/HELP: as listas de topicos (paginas em man_pages_*.cpp). Interno ao modulo.
#pragma once

#include "man.h"

namespace man {

std::vector<Topic> CommandTopics();   // man_pages_cmd.cpp: comandos do console, do emulador e do depurador
std::vector<Topic> ToolTopics();      // man_pages_tools.cpp: newdisk, asm, romdb, cas, fitadb, msxdisk
std::vector<Topic> ReferenceTopics(); // man_pages_msx.cpp: BIOS, variaveis do sistema, ganchos, BASIC, cores...

// Uma linha das tabelas de referencia (BIOS, variaveis do sistema, ganchos). As tabelas sao dados: as paginas do man
// saem delas e o teste (tests/z80/man_test.cpp) confere os enderecos contra a BIOS e a maquina emulada.
struct MsxEntry {
    const char *name;
    unsigned addr;
    unsigned size;  // variaveis: tamanho em bytes (0 = nao se aplica)
    const char *text;
    unsigned flags; // kNotJump: a entrada da BIOS e' codigo e nao um JP; kMsx2: so' da BIOS do MSX2 em diante; kMsx2p: so' do MSX2+
};
enum : unsigned { kNotJump = 1, kMsx2 = 2, kMsx2p = 4 }; // kMsx2p: so' da BIOS do MSX2+ em diante

const std::vector<MsxEntry> &BiosEntries();
const std::vector<MsxEntry> &SysVarEntries();
const std::vector<MsxEntry> &HookEntries();

} // namespace man
