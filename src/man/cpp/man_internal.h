// fwMSX -- MAN/HELP: as listas de topicos (paginas em man_pages_*.cpp). Interno ao modulo.
#pragma once

#include "man.h"

namespace man {

std::vector<Topic> CommandTopics();   // man_pages_cmd.cpp: comandos do console, do emulador e do depurador
std::vector<Topic> ToolTopics();      // man_pages_tools.cpp: newdisk, asm, romdb, cas, fitadb, msxdisk
std::vector<Topic> ReferenceTopics(); // man_pages_msx.cpp: BIOS, variaveis do sistema, ganchos, BASIC, cores...
std::vector<Topic> BiosTopics();      // man_pages_msxbios.cpp (GERADO por tools/paleobasic/import_bios_help.py): rotinas/variaveis da BIOS e do BDOS
std::vector<Topic> HardwareTopics();  // idem: chips e perifericos (RTC, PSG, SCC, OPLL, V9958, V9990...)
std::vector<Topic> BookTopics();      // man_pages_books.cpp (GERADO por tools/paleobasic/import_books_help.py): Red Book, MSX2 Technical Handbook, Basic Dignified
std::vector<Topic> FwasmTopics();     // man_pages_fwasm.cpp: manual do montador fwasm (escrito a mao; as tabelas da BIOS saem de src/asm/cpp/bios_tables.inc)
std::vector<Topic> MamuteTopics();    // man_pages_mamute.cpp: o monitor Z80 Mamute (escrito a mao)
std::vector<Topic> ToolsHelpTopics(); // man_pages_tools_help.cpp (GERADO por tools/paleobasic/import_tools_help.py): NestorBASIC, SEE Tracker, MSXBAS2ROM
std::vector<Topic> ManualTopics();    // man_pages_manuals.cpp (GERADO por tools/paleobasic/import_manuals_help.py): MSX-DOS 2, MSX-DOS 1, Z80, R800, FM-PAC, referencias do BASIC
std::vector<Topic> BasicTopics();     // man_pages_basic.cpp (GERADO por tools/paleobasic/import_basic_help.py): palavras do MSX-BASIC e o manual

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
