// fwMSX -- ponte de controle externo: servidor TCP em 127.0.0.1 (C++). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver doc/control-spec.md.
//
// Protocolo: uma linha de texto por comando, uma linha de resposta ("ok ..." ou "err ..."). As
// threads do servidor SO' leem/escrevem no socket e enfileiram os comandos; quem os EXECUTA e' a
// thread do emulador, chamando Poll() entre quadros -- nenhum codigo do emulador roda fora dela.
#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "command.h"

namespace control {

class Server {
public:
    Server() = default;
    ~Server();
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;

    // Abre 127.0.0.1:`port` (0 = o sistema escolhe uma porta livre) e comeca a aceitar conexoes.
    // false + `error` se nao conseguir (porta ocupada, rede indisponivel...).
    bool Start(int port, std::string &error);
    void Stop();

    // Porta efetivamente aberta (0 se parado).
    int port() const { return port_; }

    // Executa, com `commander`, os comandos que chegaram desde a ultima chamada e devolve as
    // respostas aos clientes. Chamar da thread do emulador, uma vez por quadro.
    // Devolve quantos comandos executou.
    int Poll(Commander &commander);

private:
    struct Request {
        std::string line;
        std::string reply;
        bool done = false;
    };

    void AcceptLoop();
    void ClientLoop(intptr_t socket);
    void CloseClient(intptr_t socket);

    std::atomic<bool> running_{false};
    intptr_t listen_socket_ = -1;
    int port_ = 0;
    std::thread accept_thread_;

    std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<std::shared_ptr<Request>> queue_;

    std::mutex clients_mutex_;
    std::vector<std::thread> client_threads_;
    std::vector<intptr_t> client_sockets_;
};

} // namespace control
