// fwMSX -- console (REPL): sessao de comandos (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/repl-spec.md.
//
// A sessao entende comandos LOCAIS (emu, help, exit e as ferramentas registradas: disk new, rom,
// cas...) e ENCAMINHA todo o resto ao emulador pela ponte de controle. Nao conhece a interface de
// linha (replxx) -- isso fica em repl.cpp --, entao e' testavel sem terminal.
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "emu_link.h"

namespace mamute {
class Monitor;
} // namespace mamute

namespace repl {

class ReplSession {
public:
    // Ferramenta local: recebe os argumentos (sem o nome) e devolve o codigo de saida. Escreve na
    // saida padrao por conta propria (sao os CLIs que ja' existem: --romdb, --cas, ...).
    using Tool = std::function<int(const std::vector<std::string> &)>;
    using Spawner = std::function<bool(const std::string &exe, const std::vector<std::string> &args, std::string &error)>;

    // `exe_path` = o executavel que sera' iniciado por `emu start` (o proprio fwMSX).
    explicit ReplSession(std::string exe_path);
    ~ReplSession();

    void AddTool(const std::string &name, Tool tool, const std::string &help);
    void SetSpawner(Spawner spawner) { spawner_ = std::move(spawner); }

    // `man <topico>`: quem tem uma janelinha/pager recebe (titulo, texto) aqui; sem visualizador a pagina e'
    // impressa em `out`. `width` = colunas disponiveis para o texto (o console usa a largura do terminal).
    using Viewer = std::function<void(const std::string &title, const std::string &text)>;
    void SetViewer(Viewer viewer) { viewer_ = std::move(viewer); }
    void SetWidth(int width) { width_ = width; }

    // Executa uma linha, escrevendo o resultado em `out`. Devolve false quando o usuario pediu para
    // sair (exit/quit).
    bool Execute(const std::string &line, std::ostream &out);

    // "fwmsx> " ou "fwmsx:PORTA> " quando ha' emulador conectado.
    std::string Prompt() const;

    bool connected() const { return link_.connected(); }

    // Modo monitor (comando `mon`): enquanto ligado, cada linha digitada e' um comando do Mamute. Com emulador conectado
    // a linha vai para o monitor DELE (modo online disponivel); sem emulador roda um monitor local, so' offline.
    bool monitor_mode() const { return mon_mode_; }
    // O prompt do monitor ("MON(off)> "), ou vazio fora do modo monitor.
    std::string MonitorPrompt();

    // Mostra os eventos do emulador (breakpoint atingido...) que chegaram, como "[evento] break id=1 ...".
    // Espera ate' `wait_ms` por um evento. Devolve true se mostrou algum.
    bool PrintEvents(std::ostream &out, int wait_ms = 0);
    EmuLink &link() { return link_; }

    // Quebra a linha em palavras (aspas duplas agrupam; a barra invertida e' literal -- caminhos do
    // Windows). Igual ao da ponte de controle.
    static std::vector<std::string> Tokenize(const std::string &line, std::string &error);

private:
    struct ToolEntry {
        Tool tool;
        std::string help;
    };

    void Emu(const std::vector<std::string> &t, std::ostream &out);
    void Forward(const std::string &line, std::ostream &out);
    void Help(const std::vector<std::string> &t, std::ostream &out);
    void Man(const std::vector<std::string> &t, std::ostream &out);
    void Monitor(const std::string &line, std::ostream &out);    // uma linha do Mamute (remota ou local)
    void MonitorEnter(const std::string &rest, std::ostream &out); // `mon` / `mon <linha>`

    std::string exe_path_;
    Spawner spawner_;
    Viewer viewer_;
    int width_ = 78;
    EmuLink link_;
    std::map<std::string, ToolEntry> tools_;
    std::unique_ptr<mamute::Monitor> local_mon_; // monitor local (sem emulador): so' offline
    bool mon_mode_ = false;
};

} // namespace repl
