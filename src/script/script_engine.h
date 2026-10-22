// fwMSX -- scripts em Lua no console (C++ sobre o Lua 5.4, MIT). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/script-spec.md.
//
// O Engine e' dono de UM estado Lua. Todo script roda numa CORROTINA: `msx.wait(n)` entrega o controle e o script volta n quadros depois
// (o Commander::Tick acorda as corrotinas e chama os ganchos uma vez por quadro). Nada roda por instrucao do Z80: os ganchos (`on_frame`,
// `on_change`, `on_stop`) so' olham a maquina entre um quadro e outro. O Lua nasce SEM io, os, package e debug: um script so' fala com a
// maquina pela tabela `msx`.
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace machine {
class Machine;
} // namespace machine

namespace script {

struct EngineHost {
    std::function<machine::Machine &()> machine;                       // a maquina de AGORA (o host pode recria-la)
    std::function<bool(const std::string &line, std::string &text)> command; // um comando do console; false = erro
    std::function<void(const std::string &line)> event;                // saida de um script que roda depois de iniciado (vira "! lua ...")
};

class Engine {
public:
    explicit Engine(EngineHost host);
    ~Engine();
    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;

    // Roda o texto / o arquivo numa corrotina ate' o 1o `msx.wait` ou o fim. `out` recebe o que o script imprimiu (e a mensagem de erro).
    // false = erro de sintaxe ou de execucao nesse trecho. Um script que fica esperando continua no Tick.
    // `echo` = estilo REPL: tenta `return <code>` antes e imprime os valores devolvidos (lua 1+2 mostra 3).
    bool RunString(const std::string &code, const std::string &chunk_name, std::string &out, bool echo = false);
    bool RunFile(const std::string &path, std::string &out);

    void Tick();   // uma vez por quadro emulado
    void Stop();   // acaba com as corrotinas e os ganchos
    std::string Status() const;

    // para os testes
    size_t Coroutines() const;
    size_t Hooks() const;

    struct Impl; // publico so' para as funcoes C do Lua (script_engine.cpp) alcancarem o estado; nao e' API

private:
    std::unique_ptr<Impl> impl_;
};

} // namespace script
