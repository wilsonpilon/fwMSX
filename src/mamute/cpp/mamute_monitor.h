// fwMSX -- Mamute: o monitor (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md.
//
// O Mamute Assembler do Paleobasic (monitor de memoria/disassembler/depurador no estilo dos monitores de
// linha dos anos 80) reescrito para o fwMSX. Uma linha entra, o texto do log sai -- sem nenhuma janela, entao
// o MESMO monitor serve ao console (CLI), a TUI e a ponte de controle. Cada linha digitada e' um comando;
// todo endereco e' hexadecimal.
//
// Dois modos:
//   offline  memoria PRIVADA (16 blocos de 16 KB) + CPU privada: criar, depurar e rodar sem a maquina.
//   online   a memoria e a CPU da maquina emulada: o que se escreve aparece no programa em execucao.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "mamute_config.h"
#include "mamute_exec.h"
#include "mamute_memory.h"
#include "mamute_panel.h"
#include "mamute_source.h"
#include "mamute_superx.h"

namespace mamute {

class Monitor {
public:
    // `config_path` e' onde CFG SAVE grava (vazio = nao grava). Comeca em offline (o `online`/`start_online`
    // so' vale depois de AttachMachine()).
    explicit Monitor(const Config &cfg, std::string config_path = "");

    // Liga o lado online (a memoria e a CPU da maquina). Os ponteiros so' precisam viver ate' DetachMachine():
    // quem hospeda a maquina (que pode ser recriada) liga e desliga a cada comando. O modo escolhido (online/
    // offline) e' do monitor e sobrevive a isso; sem maquina ligada ele age como offline.
    void AttachMachine(IMemory *memory, IExec *exec) {
        online_mem_ = memory;
        online_exec_ = exec;
    }
    void DetachMachine() {
        online_mem_ = nullptr;
        online_exec_ = nullptr;
    }
    // Uma vez, na criacao: se a configuracao manda abrir online e ha' maquina, passa para online.
    void ApplyStartMode() {
        if (cfg_.start_online && online_mem_) online_ = true;
    }
    bool has_machine() const { return online_mem_ != nullptr; }

    // Executa UMA linha. `quit` (se nao nulo) vira true com BA/QUIT/XQT. Nunca lanca excecao.
    std::string Execute(const std::string &line, bool *quit = nullptr);

    // "MON(off)> " ou "MON(on)> "; "EDIT> " dentro do editor de fonte.
    std::string Prompt() const;
    bool editing() const { return editing_ || sx_.xm_active; }
    bool online() const { return online_ && online_mem_ != nullptr; }

    // Mensagem de abertura (modo, memoria, aviso de arquivos que nao abriram).
    std::string Banner();

private:
    IMemory &Mem() { return online() ? *online_mem_ : static_cast<IMemory &>(offline_); }
    IExec &Cpu() { return online() ? *online_exec_ : static_cast<IExec &>(offline_exec_); }

    std::string CmdMode(bool online);
    std::string CmdStatus();
    std::string CmdPage(const std::vector<std::string> &f);
    std::string CmdDm(const std::vector<std::string> &f);
    std::string CmdM(const std::vector<std::string> &f);
    std::string CmdSh(const std::string &rest);
    std::string CmdMs(const std::string &rest);
    std::string CmdDump(const std::vector<std::string> &f, bool vram, bool to_file);
    std::string CmdC(const std::vector<std::string> &f);
    std::string CmdT(const std::vector<std::string> &f);
    std::string CmdF(const std::vector<std::string> &f);
    std::string CmdXfer(const std::vector<std::string> &f, bool push);
    std::string CmdLoad(const std::vector<std::string> &f);
    int ChooseSlot(const std::string &arg, unsigned addr, std::string &error);
    std::string PlaceBlock(int slot, unsigned addr, const uint8_t *data, size_t n, CellType kind);
    // fita (mamute_tape.cpp): TAPE, R, TLOAD, TSAVE e o LOAD/SAVE de .cas/.tsx/.wav
    tape::TapeDeck &Deck() {
        tape::TapeDeck *t = Mem().Tape();
        return t ? *t : offline_tape_;
    }
    std::string CmdTape(const std::string &rest);
    std::string CmdTapeR(const std::vector<std::string> &f);
    std::string CmdTload(const std::vector<std::string> &f);
    std::string CmdTsave(const std::vector<std::string> &f);
    std::string CmdLoadTape(const std::vector<std::string> &f);
    std::string CmdSaveTape(const std::string &path, unsigned a, unsigned b, unsigned e);
    std::string PlaceTapeFile(const tape::TapeFileData &d, const std::string &slot_arg, const std::string &end_arg, unsigned displacement, bool &ok);
    std::string StoreOnTape(const std::string &name, const std::string &tape_file, const AsmProduct &p);
    // o arquivo da fita que o proximo R le: sobe a cada R e volta ao zero quando a fita muda (TAPE arq, NEW, REW, SEEK)
    size_t tape_next_ = 0;
    tape::TapeDeck offline_tape_;
    std::string CmdSave(const std::vector<std::string> &f);
    std::string CmdList(const std::vector<std::string> &f, bool to_file);
    std::string CmdX(const std::vector<std::string> &f);
    std::string CmdG(const std::vector<std::string> &f);
    std::string CmdCfg(const std::string &rest);
    std::string CmdHelp();
    std::string ExecuteEdit(const std::string &line); // o editor de fonte (mamute_edit.cpp)
    std::string CmdAssemble(const std::string &args);
    std::string CmdMap();
    // Fase 3: telas. Com paineis ligados (a TUI) DM/M/S/ZAP/SCR abrem um painel interativo; sem eles sao comandos de linha.
    std::string CmdInternal(const std::string &line);         // @panels on|off, @key <tecla>, @panel
    std::string OpenPanel(std::unique_ptr<Panel> panel);
    std::string PanelText();                                   // a resposta do painel aberto (protocolo da TUI ou texto simples)
    std::string CmdS(const std::vector<std::string> &f);
    std::string CmdZap(const std::vector<std::string> &f);
    std::string CmdZapW(const std::vector<std::string> &f);
    std::string CmdScr(const std::vector<std::string> &f);

    // Fase 4: os comandos do SUPER-X (prefixo X, mamute_superx_cmds.cpp). `handled` = false quando `cmd` nao e' um deles.
    std::string CmdSuperX(const std::string &cmd, const std::string &rest, bool printer, bool &handled);
    std::string CmdVar(const std::string &line);                  // @, @0=8000#3-1
    std::string CmdCl(const std::string &rest);                   // a calculadora
    std::string CmdXview(char which, const std::string &rest, bool printer); // XD, XA, XI
    std::string CmdXm(const std::string &rest);                   // abre a sessao do XM
    std::string XmExecute(const std::string &line);               // uma linha da sessao do XM
    std::string CmdXh(const std::string &rest);
    std::string CmdXbt(const std::string &rest);
    std::string CmdXrt(const std::string &rest);
    std::string CmdXfl(const std::string &rest);
    std::string CmdXcm(const std::string &rest);
    std::string CmdXfd(const std::string &rest);
    std::string CmdXts(const std::string &rest);
    std::string CmdXco(const std::string &rest);
    std::string CmdXsd(const std::string &rest);
    std::string CmdXrg(const std::string &rest);
    std::string CmdXgo(const std::string &rest);
    std::string CmdXtr(const std::string &rest);
    std::string RegsXrg();                                        // o bloco de registradores do XRG/XGO
    std::string RegsShort();                                      // o bloco compacto do XTR
    // disco: XDK XFS XCI XTP XSV XLD XS# XL# XL% XS%
    std::string CmdXdisk(const std::string &cmd, const std::string &rest);
    std::string NeedDisk(std::string &error);                     // o disco corrente (erro se nao ha')
    // notas e portas: XIM XIC XIL XIS XIR XPP XPI XPO
    std::string CmdXnotes(const std::string &cmd, const std::string &rest);
    std::string CmdXports(const std::string &cmd, const std::string &rest);
    // um alvo + um fim "puro" (o <fim> de um bloco: sem sufixo, no MESMO alvo do inicio)
    bool ParseBlock(const std::string &start_tok, const std::string &end_tok, SxAddr &s, uint32_t &e);

    // O arquivo de listagem dos comandos P e LP (texto; o PDF do Mamute original nao existe aqui).
    std::string WriteListing(const std::string &path, const std::string &title, const std::string &body);

    Config cfg_;
    std::string config_path_;
    OfflineMemory offline_;
    OfflineExec offline_exec_;
    IMemory *online_mem_ = nullptr;
    IExec *online_exec_ = nullptr;
    bool online_ = false;

    // editor de fonte Z80 (Fase 2): o programa vive no monitor e sobrevive a saida do EDIT
    Source src_;
    bool editing_ = false;
    AsmProduct last_; // a ultima montagem bem-sucedida (MAP)
    bool last_ok_ = false;

    // SUPER-X (Fase 4): variaveis de debugger, breakpoints do XGO, painel de portas, notas, disco corrente
    SuperX sx_;

    // paineis interativos (Fase 3)
    std::unique_ptr<Panel> panel_;
    bool panels_ = false; // true quando o hospedeiro sabe desenhar paineis (a TUI)
    int dm_last_ = -1;    // onde o ultimo DM de linha parou (DM sozinho continua 128 bytes depois)
    int dm_shift_ = 0;

    int dump_mode_ = 0;
    int last_find_ = -1;       // onde o ultimo SH achou (o proximo, sem endereco, continua em +1)
    int last_list_ = -1;       // onde L/LP pararam
    std::string load_warnings_;
};

} // namespace mamute
