// fwMSX -- sessao do console. Ver repl_session.h e doc/repl-spec.md.
#include "repl_session.h"

#include <cstdlib>

#include "../control/command.h"

namespace repl {

ReplSession::ReplSession(std::string exe_path) : exe_path_(std::move(exe_path)), spawner_(SpawnDetached) {}

void ReplSession::AddTool(const std::string &name, Tool tool, const std::string &help) {
    tools_[name] = ToolEntry{std::move(tool), help};
}

std::vector<std::string> ReplSession::Tokenize(const std::string &line, std::string &error) {
    return control::Commander::Tokenize(line, error);
}

std::string ReplSession::Prompt() const {
    return link_.connected() ? "fwmsx:" + std::to_string(link_.port()) + "> " : "fwmsx> ";
}

void ReplSession::Help(std::ostream &out) {
    out << "Comandos do console:\n"
           "  emu start [opcoes do --msx]   inicia o emulador (janela) e conecta; ex.: emu start --msx2p --cart jogo.rom\n"
           "  emu attach <porta>            conecta num emulador ja' aberto com --ctl-port\n"
           "  emu detach | emu stop | emu status\n"
           "  exit | quit                   sai do console (o emulador continua aberto)\n";
    for (const auto &[name, entry] : tools_) out << "  " << name << "  -- " << entry.help << "\n";
    out << "Qualquer outro comando vai para o emulador conectado (peek, poke, type, cart, disk, state, ...).\n";
    if (link_.connected()) {
        std::string reply, error;
        if (link_.Send("help", reply, error)) out << "Emulador: " << reply << "\n";
    }
}

void ReplSession::Forward(const std::string &line, std::ostream &out) {
    if (!link_.connected()) {
        out << "erro: sem emulador conectado (use 'emu start' ou 'emu attach <porta>')\n";
        return;
    }
    std::string reply, error;
    if (!link_.Send(line, reply, error)) {
        out << "erro: " << error << "\n";
        return;
    }
    if (reply.rfind("ok ", 0) == 0) out << reply.substr(3) << "\n";
    else if (reply == "ok") out << "ok\n";
    else if (reply.rfind("err ", 0) == 0) out << "erro: " << reply.substr(4) << "\n";
    else out << reply << "\n";
}

void ReplSession::Emu(const std::vector<std::string> &t, std::ostream &out) {
    const std::string sub = t.size() > 1 ? t[1] : std::string();

    if (sub == "start") {
        if (link_.connected()) {
            out << "erro: ja' ha' um emulador conectado (porta " << link_.port() << "); use 'emu stop' ou 'emu detach'\n";
            return;
        }
        const int port = FindFreePort();
        if (port == 0) {
            out << "erro: nao foi possivel reservar uma porta local\n";
            return;
        }
        std::vector<std::string> args = {"--msx", "--ctl-port", std::to_string(port)};
        for (size_t i = 2; i < t.size(); ++i) args.push_back(t[i]);
        std::string error;
        if (!spawner_(exe_path_, args, error)) {
            out << "erro: " << error << "\n";
            return;
        }
        if (!link_.Connect(port, 15000, error)) {
            out << "erro: " << error << "\n";
            return;
        }
        out << "emulador iniciado e conectado (porta " << port << ")\n";
        return;
    }

    if (sub == "attach") {
        uint32_t port = 0;
        if (t.size() != 3 || !control::Commander::ParseNumber(t[2], port) || port == 0 || port > 65535) {
            out << "erro: use emu attach <porta>\n";
            return;
        }
        std::string error;
        if (!link_.Connect(static_cast<int>(port), 3000, error)) {
            out << "erro: " << error << "\n";
            return;
        }
        out << "conectado (porta " << port << ")\n";
        return;
    }

    if (sub == "detach") {
        link_.Close();
        out << "desconectado (o emulador continua aberto)\n";
        return;
    }

    if (sub == "stop") {
        if (!link_.connected()) {
            out << "erro: sem emulador conectado\n";
            return;
        }
        std::string reply, error;
        link_.Send("quit", reply, error);
        link_.Close();
        out << "emulador encerrado\n";
        return;
    }

    if (sub == "status") {
        if (!link_.connected()) {
            out << "sem emulador conectado\n";
            return;
        }
        Forward("status", out);
        return;
    }

    out << "erro: use emu start | attach <porta> | detach | stop | status\n";
}

bool ReplSession::Execute(const std::string &line, std::ostream &out) {
    std::string error;
    const std::vector<std::string> t = Tokenize(line, error);
    if (!error.empty()) {
        out << "erro: " << error << "\n";
        return true;
    }
    if (t.empty()) return true;

    if (t[0] == "exit" || t[0] == "quit") return false;
    if (t[0] == "help" || t[0] == "?") {
        Help(out);
        return true;
    }
    if (t[0] == "emu") {
        Emu(t, out);
        return true;
    }
    const auto tool = tools_.find(t[0]);
    if (tool != tools_.end()) {
        const int rc = tool->second.tool(std::vector<std::string>(t.begin() + 1, t.end()));
        if (rc != 0) out << "(codigo de saida " << rc << ")\n";
        return true;
    }
    Forward(line, out);
    return true;
}

} // namespace repl
