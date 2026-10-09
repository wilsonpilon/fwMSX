// fwMSX -- console (REPL): ligacao com um emulador em execucao e inicio do emulador como processo
// filho (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/repl-spec.md.
//
// O REPL NAO embute a maquina: ele e' um cliente da ponte de controle (doc/control-spec.md). Assim o
// console continua livre enquanto a janela do emulador roda em outro processo, e o MESMO caminho serve
// para um emulador iniciado por aqui ou por qualquer outro meio (`emu attach <porta>`).
#pragma once

#include <string>
#include <vector>

namespace repl {

class EmuLink {
public:
    EmuLink() = default;
    ~EmuLink() { Close(); }
    EmuLink(const EmuLink &) = delete;
    EmuLink &operator=(const EmuLink &) = delete;

    // Conecta em 127.0.0.1:`port`, tentando por ate' `timeout_ms` (o emulador recem-iniciado leva
    // um instante para abrir a porta).
    bool Connect(int port, int timeout_ms, std::string &error);
    void Close();
    bool connected() const { return socket_ >= 0; }
    int port() const { return port_; }

    // Manda UMA linha e devolve a linha de resposta ("ok ..." / "err ..."). false + `error` se a
    // conexao caiu (o emulador foi fechado, por exemplo); a ligacao e' encerrada nesse caso.
    bool Send(const std::string &line, std::string &reply, std::string &error);

private:
    long long socket_ = -1;
    int port_ = 0;
    std::string pending_;
};

// Uma porta TCP livre em 127.0.0.1 (0 se nao conseguir). Existe uma pequena janela de corrida entre
// escolher e o emulador abrir; para uso interativo e' aceitavel.
int FindFreePort();

// Caminho do proprio executavel (para se reiniciar como emulador). Vazio se nao conseguir.
std::string SelfExePath();

// Inicia `exe` com `args` como processo independente (stdin/stdout/stderr em NUL: o console do REPL
// nao e' sujo por mensagens do emulador). false + `error` em caso de falha.
bool SpawnDetached(const std::string &exe, const std::vector<std::string> &args, std::string &error);

} // namespace repl
