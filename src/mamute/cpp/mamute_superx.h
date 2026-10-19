// fwMSX -- Mamute: o estado e as pecas dos comandos do SUPER-X, prefixo X (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/mamute-spec.md, "Fase 4".
//
// O SUPER-X era um monitor japones de MSX. O Mamute do Paleobasic portou os comandos dele com o prefixo X (para nao colidir com os do
// MegaAssembler). Aqui cada comando abre uma janela no original; no fwMSX viram PAINEIS na TUI e comandos de LINHA no console.
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "mamute_exec.h"
#include "mamute_panel.h"
#include "mamute_target.h"

namespace mamute {

// Uma nota do arquivo de notas do SUPER-X (XIM/XIC/XIL/XIS/XIR): um endereco, uma classificacao e um texto de ate' 60 caracteres.
// `slot` (0-4: GERAL MAIN SUB FDC RAM) e `type` (0-7: GERAL BIOS WORK DATA PORT MATH KEY HOOK) sao os codigos do SUPER-X e nao tem
// nada a ver com o #slot do enderecamento estendido -- so' coincidem de nome.
struct Note {
    uint32_t addr = 0;
    int slot = 0;
    int type = 0;
    std::string text;
};

const char *NoteSlotName(int slot);
const char *NoteTypeName(int type);
// O formato de texto das notas: "AAAA;slot;tipo;texto", uma por linha (um BOM no inicio e' ignorado). false + error se nao abriu.
bool LoadNotes(const std::string &path, std::vector<Note> &out, std::string &error);
bool SaveNotes(const std::string &path, const std::vector<Note> &notes);
// O arquivo que o XIL sem argumento le': SUPER-X-PT.notas ao lado do executavel.
std::string DefaultNotesPath();

// O que o XD/XA/XI/XM/XH guardam para "reabrir onde ficou" (comando sem endereco).
struct LastView {
    bool has = false;
    SxAddr a;
    int shift = 0; // so' o XD
};

struct SuperX {
    Vars vars;
    bool checksum_addr = false; // XCS: o checksum por linha do XD soma tambem o byte baixo do endereco
    LastView xd, xa, xi, xm, xh;

    // XRG/XGO: os 5 breakpoints nomeados (BP, BP1, BP2, BP3, BPF) e quantas vezes o XGO ja foi chamado desde o ultimo "XGO <endereco>"
    bool bp_has[5]{};
    uint16_t bp_addr[5]{};
    int go_step = 0;
    bool has_go = false;

    PortTable ports; // XPP/XPI/XPO
    std::vector<Note> notes;
    std::string disk; // o disco corrente (imagem .dsk) dos comandos de disco

    // a sessao do XM (o prompt vira "XM 4000> "): cada linha e' uma instrucao, dados ou um endereco
    bool xm_active = false;
    SxAddr xm_cur;
    std::vector<std::string> xm_history;
};

// ---- disassembler sobre um alvo (XI, XSD, XFD, XTR, painel do XI) -------------------------------------------------------------
struct DisasmLine {
    uint32_t addr = 0;
    int len = 1;
    std::string bytes; // "3E 05"
    std::string text;  // "LD A,05H"
};
DisasmLine DisasmAt(IMemory &mem, const Target &t, uint32_t addr);
// "AAAA  3E 05        LD A,05H" (o mesmo formato do L)
std::string FormatDisasm(const DisasmLine &d, int digits);

// ---- paineis do SUPER-X (mamute_superx_panels.cpp) -----------------------------------------------------------------------------
std::unique_ptr<Panel> MakeXdPanel(const SxAddr &a, int shift);                           // mamute_panel.cpp
std::unique_ptr<Panel> MakeXaPanel(const SxAddr &a);                                       // XA: 256 bytes em ASCII, digitavel
std::unique_ptr<Panel> MakeXiPanel(const SxAddr &a);                                       // XI: o disassembly rolando
std::unique_ptr<Panel> MakeXhPanel(const SxAddr &a);                                       // XH: 4 caracteres/sprites 8x8 pixel a pixel
std::unique_ptr<Panel> MakeNotesPanel(std::vector<Note> *notes, int first);                // XIR: as notas, uma por vez, com busca
std::unique_ptr<Panel> MakePortsPanel(PortTable *ports);                                   // XPP: o painel de portas de E/S
std::unique_ptr<Panel> MakeTextPanel(const std::string &title, const std::vector<std::string> &lines); // XTP: texto com rolagem e busca
// XTR: o trace passo a passo. `step` executa UMA instrucao e devolve as linhas do passo (vazio = a CPU parou).
std::unique_ptr<Panel> MakeTracePanel(std::function<std::vector<std::string>()> step, std::vector<std::string> first);

// As linhas de texto que o painel de portas e o XIR mostram na versao de linha.
std::string FormatNote(const Note &n);

} // namespace mamute
