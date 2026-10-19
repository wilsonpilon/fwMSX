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

    // "MON(off)> " ou "MON(on)> ".
    std::string Prompt() const;
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
    std::string CmdSave(const std::vector<std::string> &f);
    std::string CmdList(const std::vector<std::string> &f, bool to_file);
    std::string CmdX(const std::vector<std::string> &f);
    std::string CmdG(const std::vector<std::string> &f);
    std::string CmdCfg(const std::string &rest);
    std::string CmdHelp();

    // O arquivo de listagem dos comandos P e LP (texto; o PDF do Mamute original nao existe aqui).
    std::string WriteListing(const std::string &path, const std::string &title, const std::string &body);

    Config cfg_;
    std::string config_path_;
    OfflineMemory offline_;
    OfflineExec offline_exec_;
    IMemory *online_mem_ = nullptr;
    IExec *online_exec_ = nullptr;
    bool online_ = false;

    int dump_mode_ = 0;
    int last_find_ = -1;       // onde o ultimo SH achou (o proximo, sem endereco, continua em +1)
    int last_list_ = -1;       // onde L/LP pararam
    std::string load_warnings_;
};

} // namespace mamute
