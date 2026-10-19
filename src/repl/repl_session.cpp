// fwMSX -- sessao do console. Ver repl_session.h e doc/repl-spec.md.
#include "repl_session.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

#include "../control/command.h"
#include "../mamute/cpp/mamute_monitor.h"
#include "../man/cpp/man.h"

namespace repl {

namespace {

// "man put sprite" -> "put sprite": o nome do topico pode ter varias palavras (as do MSX-BASIC tem).
std::string JoinWords(const std::vector<std::string> &t) {
    std::string name;
    for (size_t i = 1; i < t.size(); ++i) name += (i > 1 ? " " : "") + t[i];
    return name;
}

} // namespace

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

// help            -> a tabela de comandos (agrupada, com separadores)
// help <comando>  -> uso e resumo; o manual completo e' o `man <comando>`
void ReplSession::Help(const std::vector<std::string> &t, std::ostream &out) {
    if (t.size() > 1) {
        const std::string name = JoinWords(t);
        const man::Topic *topic = man::Resolve(name);
        if (topic) out << man::HelpFor(*topic);
        else out << man::NotFound(name);
        return;
    }
    out << man::HelpTable(std::min(std::max(width_, 40), 100));
    // ferramentas registradas por quem hospeda a sessao e que o man ainda nao conhece
    bool header = false;
    for (const auto &[name, entry] : tools_) {
        if (man::Find(name)) continue;
        if (!header) {
            out << "Outras ferramentas:\n";
            header = true;
        }
        out << "  " << name << "  -- " << entry.help << "\n";
    }
}

// man             -> o indice de todos os topicos
// man <topico>    -> o manual (numa janelinha/pager quando ha' visualizador)
void ReplSession::Man(const std::vector<std::string> &t, std::ostream &out) {
    const int width = std::min(std::max(width_, 40), 100);
    std::string title, text;
    // man -l [categoria] lista as categorias / os nomes de uma categoria; man -k texto procura em nomes, apelidos e resumos
    std::string rest;
    for (size_t i = 2; i < t.size(); ++i) rest += (i > 2 ? " " : "") + t[i];
    if (t.size() >= 2 && (t[1] == "-l" || t[1] == "--list")) {
        title = "man -l";
        text = man::ListCategory(rest, width);
    } else if (t.size() >= 2 && (t[1] == "-k" || t[1] == "--apropos")) {
        title = "man -k " + rest;
        text = man::Apropos(rest, width);
    } else if (t.size() < 2) {
        title = "man";
        text = man::Index(width);
    } else {
        const std::string name = JoinWords(t);
        const man::Topic *topic = man::Resolve(name);
        if (!topic) {
            out << man::NotFound(name);
            return;
        }
        title = topic->name;
        text = man::Render(*topic, width);
    }
    if (viewer_) viewer_(title, text);
    else out << text;
}

bool ReplSession::PrintEvents(std::ostream &out, int wait_ms) {
    std::vector<std::string> events;
    if (!link_.PollEvents(events, wait_ms)) return false;
    for (const std::string &e : events) out << "[evento] " << e << "\n";
    return true;
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
        bool wants_splash = false;
        for (size_t i = 2; i < t.size(); ++i) {
            args.push_back(t[i]);
            if (t[i] == "--splash" || t[i] == "--no-splash") wants_splash = true;
        }
        // Aberto pelo console/TUI o emulador nao mostra o splash, a menos que o usuario peca (--splash).
        if (!wants_splash) args.push_back("--no-splash");
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

ReplSession::~ReplSession() = default;

// Uma linha do Mamute: com emulador conectado vai para o monitor dele (`mon <linha>`); sem emulador, o local.
void ReplSession::Monitor(const std::string &line, std::ostream &out) {
    if (link_.connected()) {
        Forward("mon " + line, out);
        return;
    }
    if (!local_mon_) local_mon_ = std::make_unique<mamute::Monitor>(mamute::LoadConfig(mamute::DefaultConfigPath()), mamute::DefaultConfigPath());
    const std::string text = line.empty() ? local_mon_->Banner() : local_mon_->Execute(line);
    if (!text.empty()) out << text << "\n";
}

void ReplSession::MonitorEnter(const std::string &rest, std::ostream &out) {
    if (!rest.empty()) { // `mon <linha>`: um comando so', sem entrar no modo
        Monitor(rest, out);
        return;
    }
    mon_mode_ = true;
    Monitor("", out);
    out << "(BA ou QUIT volta ao console; uma linha comecando com ! e' um comando do console)\n";
}

std::string ReplSession::MonitorPrompt() {
    if (!mon_mode_) return "";
    if (link_.connected()) {
        std::string reply, error;
        if (link_.Send("mon @prompt", reply, error) && reply.rfind("ok ", 0) == 0) return reply.substr(3);
        return "MON(?)> ";
    }
    if (!local_mon_) local_mon_ = std::make_unique<mamute::Monitor>(mamute::LoadConfig(mamute::DefaultConfigPath()), mamute::DefaultConfigPath());
    return local_mon_->Prompt();
}

bool ReplSession::Execute(const std::string &line, std::ostream &out) {
    // modo monitor: toda linha e' do Mamute (BA/QUIT/XQT saem; "!comando" roda um comando do console)
    if (mon_mode_) {
        const size_t a = line.find_first_not_of(" 	");
        if (a == std::string::npos) return true;
        if (line[a] == '!') { // "!comando": um comando do CONSOLE, e continua no monitor
            mon_mode_ = false;
            const bool keep = Execute(line.substr(a + 1), out);
            mon_mode_ = true;
            return keep;
        }
        size_t e = a;
        while (e < line.size() && !std::isspace(static_cast<unsigned char>(line[e])) && line[e] != ',') ++e;
        std::string word = line.substr(a, e - a);
        for (char &c : word) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        // QUIT dentro do editor de fonte (prompt EDIT>) volta ao monitor; BA e XQT saem de tudo
        const bool in_editor = word == "QUIT" && MonitorPrompt().rfind("EDIT", 0) == 0;
        if (word == "BA" || word == "XQT" || (word == "QUIT" && !in_editor)) {
            mon_mode_ = false;
            out << "(volta ao console)\n";
            return true;
        }
        Monitor(line.substr(a), out);
        return true;
    }
    std::string error;
    const std::vector<std::string> t = Tokenize(line, error);
    if (!error.empty()) {
        out << "erro: " << error << "\n";
        return true;
    }
    if (t.empty()) return true;

    if (t[0] == "exit" || t[0] == "quit") return false;
    if (t[0] == "help" || t[0] == "?") {
        Help(t, out);
        return true;
    }
    if (t[0] == "man" || t[0] == "manual") {
        Man(t, out);
        return true;
    }
    if (t[0] == "mon" || t[0] == "mamute") {
        const size_t sp = line.find_first_of(" 	");
        std::string rest = sp == std::string::npos ? std::string() : line.substr(sp + 1);
        const size_t a = rest.find_first_not_of(" 	");
        rest = a == std::string::npos ? std::string() : rest.substr(a);
        MonitorEnter(rest, out);
        return true;
    }
    if (t[0] == "emu") {
        Emu(t, out);
        return true;
    }
    if (t[0] == "wait") {
        // wait [segundos]: espera um evento do emulador (ex.: depois de `go`, o breakpoint atingido).
        if (!link_.connected()) {
            out << "erro: sem emulador conectado\n";
            return true;
        }
        int seconds = 10;
        if (t.size() > 1) seconds = std::atoi(t[1].c_str());
        if (seconds < 1) seconds = 1;
        if (!PrintEvents(out, seconds * 1000)) out << "(nenhum evento em " << seconds << " s)\n";
        return true;
    }
    const auto tool = tools_.find(t[0]);
    if (tool != tools_.end()) {
        const int rc = tool->second.tool(std::vector<std::string>(t.begin() + 1, t.end()));
        if (rc != 0) out << "(codigo de saida " << rc << ")\n";
        return true;
    }
    Forward(line, out);
    PrintEvents(out, 0); // um evento que ja' chegou (ex.: o watch disparado pelo proprio comando)
    return true;
}

} // namespace repl
