// fwMSX -- ponte de controle externo: servidor TCP em 127.0.0.1 (C++). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver doc/control-spec.md.
//
// Protocolo: uma linha de texto por comando, uma linha de resposta ("ok ..." ou "err ..."; "ok +N" seguido de N
// linhas nas respostas de varias linhas). Alem disso, o cliente que mandou "events on" recebe, a qualquer
// momento, linhas "! evento chave=valor ..." (breakpoint atingido, etc.) -- doc/debug-spec.md. As
// threads do servidor SO' leem/escrevem no socket e enfileiram os comandos; quem os EXECUTA e' a
// thread do emulador, chamando Poll() entre quadros -- nenhum codigo do emulador roda fora dela.
#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <set>
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
    // `local_events`, se dado, tambem recebe uma copia dos eventos do depurador (para quem hospeda a maquina
    // e quer mostra-los, como o modo terminal).
    int Poll(Commander &commander, std::vector<std::string> *local_events = nullptr);

    // Manda uma linha de evento ("! ...") a todos os clientes inscritos (events on). Da' para chamar da
    // thread do emulador; Poll() ja' faz isso com os eventos do depurador.
    void PushEvent(const std::string &line);

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
    std::set<intptr_t> subscribers_; // sockets que pediram "events on" (protegido por clients_mutex_)
    std::mutex write_mutex_;         // uma escrita por vez no mesmo socket (resposta x evento)
    bool SendText(intptr_t socket, const std::string &text);
    std::vector<std::thread> client_threads_;
    std::vector<intptr_t> client_sockets_;
};

} // namespace control
