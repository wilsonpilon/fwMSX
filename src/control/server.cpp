// fwMSX -- servidor TCP da ponte de controle. Ver server.h e doc/control-spec.md.
#include "server.h"

#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET sock_t;
#define SOCK_INVALID INVALID_SOCKET
static void sock_close(sock_t s) { closesocket(s); }
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int sock_t;
#define SOCK_INVALID (-1)
static void sock_close(sock_t s) {
    shutdown(s, SHUT_RDWR);
    close(s);
}
#endif

namespace control {

namespace {

// Tempo maximo que um cliente espera a thread do emulador executar o comando.
constexpr auto kReplyTimeout = std::chrono::seconds(10);
// Uma linha de comando maior que isto e' lixo (ou um cliente com defeito).
constexpr size_t kMaxLine = 64 * 1024;

} // namespace

Server::~Server() { Stop(); }

bool Server::Start(int port, std::string &error) {
    if (running_) {
        error = "servidor de controle ja' esta' ativo";
        return false;
    }
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        error = "WSAStartup falhou";
        return false;
    }
#endif
    sock_t s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == SOCK_INVALID) {
        error = "nao foi possivel criar o socket";
        return false;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // SO' localhost: nunca expor o emulador na rede
    if (bind(s, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0 || listen(s, 8) != 0) {
        error = "nao foi possivel abrir 127.0.0.1:" + std::to_string(port) + " (porta ocupada?)";
        sock_close(s);
        return false;
    }
    socklen_t len = sizeof(addr);
    getsockname(s, reinterpret_cast<sockaddr *>(&addr), &len);
    port_ = ntohs(addr.sin_port);
    listen_socket_ = static_cast<intptr_t>(s);
    running_ = true;
    accept_thread_ = std::thread(&Server::AcceptLoop, this);
    return true;
}

void Server::Stop() {
    if (!running_.exchange(false)) return;
    sock_close(static_cast<sock_t>(listen_socket_)); // destrava o accept()
    if (accept_thread_.joinable()) accept_thread_.join();
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (const intptr_t c : client_sockets_) sock_close(static_cast<sock_t>(c)); // destrava os recv()
        client_sockets_.clear();
    }
    cv_.notify_all();
    std::vector<std::thread> threads;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        threads.swap(client_threads_);
    }
    for (std::thread &t : threads) {
        if (t.joinable()) t.join();
    }
    listen_socket_ = -1;
    port_ = 0;
}

void Server::AcceptLoop() {
    while (running_) {
        sock_t c = accept(static_cast<sock_t>(listen_socket_), nullptr, nullptr);
        if (c == SOCK_INVALID) {
            if (!running_) break;
            continue;
        }
        std::lock_guard<std::mutex> lock(clients_mutex_);
        client_sockets_.push_back(static_cast<intptr_t>(c));
        client_threads_.emplace_back(&Server::ClientLoop, this, static_cast<intptr_t>(c));
    }
}

void Server::ClientLoop(intptr_t socket) {
    const sock_t s = static_cast<sock_t>(socket);
    std::string pending;
    char buf[1024];
    while (running_) {
        const int n = recv(s, buf, sizeof(buf), 0);
        if (n <= 0) break;
        pending.append(buf, static_cast<size_t>(n));
        if (pending.size() > kMaxLine && pending.find('\n') == std::string::npos) break;
        size_t nl;
        while ((nl = pending.find('\n')) != std::string::npos) {
            std::string line = pending.substr(0, nl);
            pending.erase(0, nl + 1);
            if (!line.empty() && line.back() == '\r') line.pop_back();

            auto req = std::make_shared<Request>();
            req->line = line;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                queue_.push_back(req);
            }
            std::string reply;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if (!cv_.wait_for(lock, kReplyTimeout, [&] { return req->done || !running_; })) {
                    reply = "err o emulador nao respondeu a tempo";
                } else if (!req->done) {
                    reply = "err servidor encerrado";
                } else {
                    reply = req->reply;
                }
            }
            reply += '\n';
            if (send(s, reply.data(), static_cast<int>(reply.size()), 0) <= 0) {
                CloseClient(socket);
                return;
            }
        }
    }
    CloseClient(socket);
}

// Fecha o socket de um cliente uma unica vez (Stop() tambem fecha os que sobraram).
void Server::CloseClient(intptr_t socket) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (size_t i = 0; i < client_sockets_.size(); ++i) {
        if (client_sockets_[i] == socket) {
            client_sockets_.erase(client_sockets_.begin() + static_cast<std::ptrdiff_t>(i));
            sock_close(static_cast<sock_t>(socket));
            return;
        }
    }
}

int Server::Poll(Commander &commander) {
    std::deque<std::shared_ptr<Request>> work;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        work.swap(queue_);
    }
    for (const std::shared_ptr<Request> &req : work) {
        const std::string reply = commander.Execute(req->line).Line();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            req->reply = reply;
            req->done = true;
        }
        cv_.notify_all();
    }
    return static_cast<int>(work.size());
}

} // namespace control
