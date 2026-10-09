// Teste da sessao do console (src/repl): comandos locais, encaminhamento ao emulador pela ponte de
// controle (servidor TCP real em outra thread) e `emu start` com um iniciador falso -- ver
// doc/repl-spec.md.
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "../../src/control/command.h"
#include "../../src/control/server.h"
#include "../../src/machine/machine.h"
#include "../../src/repl/repl_session.h"

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

bool Contains(const std::string &s, const std::string &part) { return s.find(part) != std::string::npos; }

class TestHost : public control::Host {
public:
    explicit TestHost(const machine::MachineConfig &config) {
        std::string error;
        machine_ = machine::Machine::Create(config, error);
    }
    machine::Machine &machine() override { return *machine_; }
    bool paused() const override { return paused_; }
    void set_paused(bool p) override { paused_ = p; }
    void quit() override { quit_ = true; }
    bool LoadCartridge(const std::string &, const std::string &, std::string &error) override {
        error = "nao usado neste teste";
        return false;
    }
    bool SaveScreenshot(const std::string &, std::string &error) override {
        error = "nao usado neste teste";
        return false;
    }
    bool ok() const { return machine_ != nullptr; }
    std::atomic<bool> quit_{false};

private:
    std::unique_ptr<machine::Machine> machine_;
    bool paused_ = false;
};

// Roda `script` (uma lista de linhas) numa thread de trabalho enquanto a thread principal faz o papel
// do emulador (Poll dos servidores). Devolve a saida de cada linha.
std::vector<std::string> RunScript(repl::ReplSession &session, const std::vector<std::string> &script,
                                   const std::vector<control::Server *> &servers, control::Commander &cmd) {
    std::vector<std::string> outputs;
    std::atomic<bool> done{false};
    std::thread worker([&] {
        for (const std::string &line : script) {
            std::ostringstream out;
            session.Execute(line, out);
            outputs.push_back(out.str());
        }
        done = true;
    });
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(20);
    while (!done && std::chrono::steady_clock::now() < deadline) {
        for (control::Server *s : servers) s->Poll(cmd);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    worker.join();
    return outputs;
}

} // namespace

int main() {
    // ---- sem emulador -------------------------------------------------------------------
    {
        repl::ReplSession s("fwMSX.exe");
        std::ostringstream o;
        check(s.Prompt() == "fwmsx> " && !s.connected(), "prompt sem emulador");
        check(s.Execute("peek 0 2", o) && Contains(o.str(), "erro: sem emulador conectado"), "comando sem emulador: erro explicando");
        o.str("");
        check(s.Execute("help", o) && Contains(o.str(), "emu start") && Contains(o.str(), "exit"), "help do console");
        o.str("");
        check(s.Execute("emu foo", o) && Contains(o.str(), "erro: use emu"), "emu com subcomando invalido");
        o.str("");
        check(s.Execute("emu attach 99999", o) && Contains(o.str(), "erro"), "emu attach com porta invalida");
        o.str("");
        check(s.Execute("emu stop", o) && Contains(o.str(), "erro: sem emulador"), "emu stop sem emulador");
        o.str("");
        check(s.Execute("type \"sem fechar", o) && Contains(o.str(), "aspas"), "aspas sem fechar = erro");
        o.str("");
        check(s.Execute("", o) && o.str().empty(), "linha vazia");
        check(!s.Execute("exit", o) && !s.Execute("quit", o), "exit/quit encerram o console");

        int received = -1;
        std::vector<std::string> got;
        s.AddTool("ferr", [&](const std::vector<std::string> &a) {
            got = a;
            received = 3;
            return 3;
        }, "ferramenta de teste");
        o.str("");
        check(s.Execute("ferr a \"b c\"", o) && received == 3 && got.size() == 2 && got[1] == "b c" && Contains(o.str(), "codigo de saida 3"),
              "ferramenta local: recebe os argumentos e o codigo de saida e' mostrado");
        o.str("");
        s.Execute("help", o);
        check(Contains(o.str(), "ferr"), "help lista as ferramentas registradas");
    }

#ifdef FWMSX_SOURCE_DIR
    const std::string bios = std::string(FWMSX_SOURCE_DIR) + "/resource/fMSX/ROMs/MSX.ROM";
    {
        std::ifstream probe(bios, std::ios::binary);
        if (!probe) {
            std::printf("[SKIP] BIOS nao encontrada em '%s'\n", bios.c_str());
            std::printf("\n%s (%d falha(s))\n", g_failures ? "FALHOU" : "OK", g_failures);
            return g_failures ? 1 : 0;
        }
    }
    machine::MachineConfig config;
    config.bios_path = bios;
    TestHost host(config);
    if (!host.ok()) {
        check(false, "Machine MSX1 para o teste");
        return 1;
    }
    control::Commander cmd(host);
    for (int i = 0; i < 250; ++i) {
        host.machine().RunFrame();
        cmd.Tick();
    }

    // ---- attach / encaminhamento / detach / stop ---------------------------------------
    control::Server server;
    std::string error;
    check(server.Start(0, error), "servidor de controle aberto (" + error + ")");
    {
        repl::ReplSession s("fwMSX.exe");
        const std::string port = std::to_string(server.port());
        const std::vector<std::string> out = RunScript(
            s,
            {"emu attach " + port, "poke 0xC200 0x33", "peek 0xC200", "oops", "emu status", "version", "emu detach", "peek 0 1",
             "emu attach " + port, "emu stop"},
            {&server}, cmd);
        check(out.size() == 10, "script completo executado (" + std::to_string(out.size()) + " respostas)");
        if (out.size() == 10) {
            check(Contains(out[0], "conectado (porta " + port + ")"), "emu attach: " + out[0]);
            check(out[1] == "ok\n", "poke encaminhado -> ok");
            check(out[2] == "33\n", "peek encaminhado devolve o byte (sem o prefixo ok) -> " + out[2]);
            check(Contains(out[3], "erro: comando desconhecido"), "erro do emulador mostrado como 'erro: ...'");
            check(Contains(out[4], "frame=") && Contains(out[4], "paused=0"), "emu status -> " + out[4]);
            check(Contains(out[5], "fwMSX 1."), "version encaminhado");
            check(Contains(out[6], "desconectado") && Contains(out[7], "erro: sem emulador"), "emu detach: volta a nao ter emulador");
            check(Contains(out[8], "conectado"), "emu attach de novo");
            check(Contains(out[9], "emulador encerrado") && host.quit_, "emu stop manda 'quit' ao emulador");
        }
    }
    server.Stop();

    // ---- emu start (iniciador falso: abre o servidor na porta que o console escolheu) -----
    {
        control::Server second;
        std::vector<std::string> spawned;
        repl::ReplSession s("C:/x/fwMSX.exe");
        s.SetSpawner([&](const std::string &exe, const std::vector<std::string> &args, std::string &err) {
            spawned = args;
            spawned.insert(spawned.begin(), exe);
            for (size_t i = 0; i + 1 < args.size(); ++i) {
                if (args[i] == "--ctl-port") return second.Start(std::atoi(args[i + 1].c_str()), err);
            }
            err = "sem --ctl-port";
            return false;
        });
        const std::vector<std::string> out = RunScript(s, {"emu start --msx2p --cart jogo.rom msxdos2", "emu start", "version", "emu detach"}, {&second}, cmd);
        check(out.size() == 4, "script do emu start executado");
        if (out.size() == 4) {
            check(Contains(out[0], "emulador iniciado e conectado"), "emu start: " + out[0]);
            check(spawned.size() >= 6 && spawned[0] == "C:/x/fwMSX.exe" && spawned[1] == "--msx" && spawned[2] == "--ctl-port" &&
                      spawned[4] == "--msx2p" && spawned[5] == "--cart" && spawned.back() == "msxdos2",
                  "emu start: --msx --ctl-port <porta> + as opcoes do usuario");
            check(Contains(out[1], "ja' ha' um emulador conectado"), "emu start com um emulador ja' conectado = erro");
            check(Contains(out[2], "fwMSX 1."), "depois do emu start os comandos chegam ao emulador");
        }
        second.Stop();

        repl::ReplSession fail("x");
        fail.SetSpawner([](const std::string &, const std::vector<std::string> &, std::string &err) {
            err = "nao deu";
            return false;
        });
        std::ostringstream o;
        fail.Execute("emu start", o);
        check(Contains(o.str(), "erro: nao deu") && !fail.connected(), "emu start: falha do iniciador e' mostrada");
    }

    // ---- emulador que cai no meio ----------------------------------------------------------
    {
        control::Server third;
        third.Start(0, error);
        repl::ReplSession s("x");
        std::ostringstream o;
        std::thread t([&] { s.Execute("emu attach " + std::to_string(third.port()), o); });
        t.join();
        third.Stop();
        std::ostringstream o2;
        s.Execute("version", o2);
        check(Contains(o2.str(), "erro:") && !s.connected(), "emulador que fecha: o console avisa e desconecta (" + o2.str() + ")");
    }
#else
    std::printf("[SKIP] FWMSX_SOURCE_DIR nao definido\n");
#endif
    std::printf("\n%s (%d falha(s))\n", g_failures ? "FALHOU" : "OK", g_failures);
    return g_failures ? 1 : 0;
}
