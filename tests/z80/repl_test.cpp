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
        check(Contains(o.str(), "Depurador") && Contains(o.str(), "peek <end> [n]") && Contains(o.str(), "----"), "help: tabela agrupada com separadores");
        o.str("");
        check(s.Execute("help peek", o) && Contains(o.str(), "peek <end> [n]") && Contains(o.str(), "man peek") && !Contains(o.str(), "Depurador"),
              "help <comando>: so' o uso e o resumo");
        o.str("");
        check(s.Execute("help pek", o) && Contains(o.str(), "nao ha'"), "help de comando inexistente");
        o.str("");
        check(s.Execute("man peek", o) && Contains(o.str(), "DESCRICAO") && Contains(o.str(), "EXEMPLOS"), "man <comando> imprime a pagina (sem visualizador)");
        o.str("");
        check(s.Execute("man sysv", o) && Contains(o.str(), "sysvars"), "man de nome parcial sugere o topico");
        o.str("");
        {
            std::string seen_title, seen_text;
            s.SetViewer([&](const std::string &t, const std::string &x) {
                seen_title = t;
                seen_text = x;
            });
            check(s.Execute("man bios", o) && o.str().empty() && seen_title == "bios" && Contains(seen_text, "CHPUT"), "man com visualizador entrega (titulo, texto) e nao imprime");
            check(s.Execute("man", o) && seen_title == "man" && Contains(seen_text, "Referencia MSX"), "man sem argumento = indice");
            s.SetViewer(nullptr);
        }
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

        // ---- modo monitor (Mamute) sem emulador: monitor local, so' offline ----
        {
            repl::ReplSession ms("x");
            std::ostringstream mo;
            check(ms.Execute("mon", mo) && ms.monitor_mode() && Contains(mo.str(), "MAMUTE") && ms.MonitorPrompt() == "MON(off)> ", "mon: entra no monitor local (prompt MON(off)>)");
            mo.str("");
            ms.Execute("m c000,3e,2a", mo);
            check(Contains(mo.str(), "GRAVADOS 2"), "monitor: as linhas vao para o Mamute");
            mo.str("");
            ms.Execute("online", mo);
            check(Contains(mo.str(), "SEM MAQUINA"), "monitor: ONLINE sem emulador explica");
            mo.str("");
            ms.Execute("!version", mo);
            check(Contains(mo.str(), "erro: sem emulador") && ms.monitor_mode(), "monitor: !comando roda um comando do console e continua no monitor");
            mo.str("");
            ms.Execute("ba", mo);
            check(!ms.monitor_mode() && ms.MonitorPrompt().empty() && Contains(mo.str(), "volta ao console"), "monitor: BA volta ao console");
            mo.str("");
            ms.Execute("mon d c000,c001", mo);
            check(Contains(mo.str(), "C000  3E 2A") && !ms.monitor_mode(), "mon <linha>: comando avulso, sem entrar no modo, e a memoria continua a mesma");
        }

        // ---- o editor de fonte (EDIT) dentro do monitor: QUIT volta ao monitor, BA sai de tudo ----
        {
            repl::ReplSession es("x");
            std::ostringstream eo;
            es.Execute("mon", eo);
            es.Execute("edit", eo);
            check(es.monitor_mode() && es.MonitorPrompt() == "EDIT> ", "monitor: EDIT muda o prompt para EDIT>");
            es.Execute("10 org 8000h", eo);
            es.Execute("20 ld a,10", eo);
            eo.str("");
            es.Execute("a", eo);
            check(Contains(eo.str(), "8000  3E 10"), "EDIT: A monta e lista");
            eo.str("");
            es.Execute("quit", eo);
            check(es.monitor_mode() && es.MonitorPrompt() == "MON(off)> ", "QUIT dentro do EDIT volta ao monitor (nao ao console)");
            es.Execute("edit", eo);
            es.Execute("ba", eo);
            check(!es.monitor_mode(), "BA dentro do EDIT volta ao console");
        }

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

    // ---- monitor Mamute online, pelo console conectado ----------------------------------------
    {
        control::Server msrv;
        std::string merr;
        check(msrv.Start(0, merr), "servidor de controle para o monitor (" + merr + ")");
        repl::ReplSession s("fwMSX.exe");
        const std::string mport = std::to_string(msrv.port());
        const std::vector<std::string> out = RunScript(
            s, {"emu attach " + mport, "mon", "online", "m c000,3e,2a,c9", "d c000,c003", "c 1", "!version", "ba", "peek 0xC000 3", "emu detach"},
            {&msrv}, cmd);
        check(out.size() == 10, "script do monitor executado (" + std::to_string(out.size()) + " respostas)");
        if (out.size() == 10) {
            check(Contains(out[1], "MAMUTE") && Contains(out[1], "MODO ONLINE") == false, "mon conectado: abertura do monitor da maquina");
            check(Contains(out[2], "MODO ONLINE"), "ONLINE pelo console conectado");
            check(Contains(out[3], "GRAVADOS 3 BYTES EM C000"), "M escreve na maquina");
            check(Contains(out[4], "C000  3E 2A C9") && Contains(out[5], "MODO 1"), "D le a maquina e C troca o formato (no monitor da maquina)");
            check(Contains(out[6], "fwMSX 1."), "!comando dentro do monitor vai ao console");
            check(Contains(out[7], "volta ao console"), "BA sai do monitor");
            check(Contains(out[8], "3E 2A C9"), "a maquina de fato recebeu os bytes (peek)");
        }
        msrv.Stop();
    }

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
                      spawned[4] == "--msx2p" && spawned[5] == "--cart" && spawned[spawned.size() - 2] == "msxdos2" &&
                      spawned.back() == "--no-splash",
                  "emu start: --msx --ctl-port <porta> + as opcoes do usuario + --no-splash (a janela aberta pelo console nao mostra o splash)");
            check(Contains(out[1], "ja' ha' um emulador conectado"), "emu start com um emulador ja' conectado = erro");
            check(Contains(out[2], "fwMSX 1."), "depois do emu start os comandos chegam ao emulador");
        }
        second.Stop();

        {
            // Quem pede --splash o recebe: o console nao acrescenta --no-splash.
            control::Server third;
            std::vector<std::string> spawned2;
            repl::ReplSession s2("C:/x/fwMSX.exe");
            s2.SetSpawner([&](const std::string &exe, const std::vector<std::string> &args, std::string &err) {
                spawned2 = args;
                spawned2.insert(spawned2.begin(), exe);
                for (size_t i = 0; i + 1 < args.size(); ++i) {
                    if (args[i] == "--ctl-port") return third.Start(std::atoi(args[i + 1].c_str()), err);
                }
                err = "sem --ctl-port";
                return false;
            });
            RunScript(s2, {"emu start --splash", "emu detach"}, {&third}, cmd);
            bool has_splash = false, has_no_splash = false;
            for (const std::string &a : spawned2) {
                has_splash = has_splash || a == "--splash";
                has_no_splash = has_no_splash || a == "--no-splash";
            }
            check(has_splash && !has_no_splash, "emu start --splash: o console nao acrescenta --no-splash");
            third.Stop();
        }

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
