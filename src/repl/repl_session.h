// fwMSX -- console (REPL): sessao de comandos (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/repl-spec.md.
//
// A sessao entende comandos LOCAIS (emu, help, exit e as ferramentas registradas: disk new, rom,
// cas...) e ENCAMINHA todo o resto ao emulador pela ponte de controle. Nao conhece a interface de
// linha (replxx) -- isso fica em repl.cpp --, entao e' testavel sem terminal.
#pragma once

#include <functional>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "emu_link.h"

namespace repl {

class ReplSession {
public:
    // Ferramenta local: recebe os argumentos (sem o nome) e devolve o codigo de saida. Escreve na
    // saida padrao por conta propria (sao os CLIs que ja' existem: --romdb, --cas, ...).
    using Tool = std::function<int(const std::vector<std::string> &)>;
    using Spawner = std::function<bool(const std::string &exe, const std::vector<std::string> &args, std::string &error)>;

    // `exe_path` = o executavel que sera' iniciado por `emu start` (o proprio fwMSX).
    explicit ReplSession(std::string exe_path);

    void AddTool(const std::string &name, Tool tool, const std::string &help);
    void SetSpawner(Spawner spawner) { spawner_ = std::move(spawner); }

    // Executa uma linha, escrevendo o resultado em `out`. Devolve false quando o usuario pediu para
    // sair (exit/quit).
    bool Execute(const std::string &line, std::ostream &out);

    // "fwmsx> " ou "fwmsx:PORTA> " quando ha' emulador conectado.
    std::string Prompt() const;

    bool connected() const { return link_.connected(); }
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
    void Help(std::ostream &out);

    std::string exe_path_;
    Spawner spawner_;
    EmuLink link_;
    std::map<std::string, ToolEntry> tools_;
};

} // namespace repl
