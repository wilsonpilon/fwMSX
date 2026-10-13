// Teste da ponte de controle externa (src/control): despachante de comandos sobre uma Machine de
// verdade e servidor TCP em 127.0.0.1 -- ver doc/control-spec.md.
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <thread>
#include <vector>

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
static void sock_close(sock_t s) { close(s); }
#endif

#include "../../src/control/command.h"
#include "../../src/control/server.h"
#include "../../src/machine/machine.h"
#include "../../src/vdp/core/vdp_state.h"

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    std::printf("[%s] %s\n", cond ? "PASS" : "FAIL", what.c_str());
    if (!cond) ++g_failures;
}

std::string TempPath(const char *name) {
    const char *t = std::getenv("TEMP");
    return (t ? std::string(t) : std::string("/tmp")) + "/" + name;
}

bool Contains(const std::string &s, const std::string &part) { return s.find(part) != std::string::npos; }

bool VramHas(const machine::Machine &m, const std::string &text) {
    const uint8_t *v = m.vdp_state().vram;
    for (int i = 0; i + static_cast<int>(text.size()) <= VDP_VRAM_SIZE; ++i)
        if (std::equal(text.begin(), text.end(), v + i)) return true;
    return false;
}

// Dono da maquina para o teste (na janela e' a propria janela).
class TestHost : public control::Host {
public:
    explicit TestHost(const machine::MachineConfig &config) : config_(config) {
        std::string error;
        machine_ = machine::Machine::Create(config_, error);
    }
    machine::Machine &machine() override { return *machine_; }
    bool paused() const override { return paused_; }
    void set_paused(bool p) override { paused_ = p; }
    void quit() override { quit_ = true; }
    bool LoadCartridge(const std::string &path, const std::string &mapper, std::string &error) override {
        MemMapMapperType kind = MEMMAP_MAPPER_NONE;
        if (!machine::ParseMapperName(mapper, kind)) {
            error = "mapper desconhecido";
            return false;
        }
        machine::MachineConfig next = config_;
        machine::SetCartridge(next, path, kind);
        std::unique_ptr<machine::Machine> fresh = machine::Machine::Create(next, error);
        if (!fresh) return false;
        machine_ = std::move(fresh);
        config_ = next;
        return true;
    }
    bool SaveScreenshot(const std::string &path, std::string &error) override {
        std::ofstream f(path, std::ios::binary);
        if (!f) {
            error = "nao foi possivel gravar";
            return false;
        }
        f << "P6";
        return true;
    }

    bool ok() const { return machine_ != nullptr; }
    bool quit_ = false;

private:
    machine::MachineConfig config_;
    std::unique_ptr<machine::Machine> machine_;
    bool paused_ = false;
};

void Frames(TestHost &h, control::Commander &c, int n) {
    for (int i = 0; i < n; ++i) {
        h.machine().RunFrame();
        c.Tick();
    }
}

void TestCommander(const std::string &bios) {
    machine::MachineConfig config;
    config.bios_path = bios;
    TestHost host(config);
    check(host.ok(), "Machine MSX1 criada para o teste");
    if (!host.ok()) return;
    control::Commander cmd(host);
    Frames(host, cmd, 250);

    // numeros e quebra de linha
    uint32_t n = 0;
    check(control::Commander::ParseNumber("0x7B", n) && n == 123 && control::Commander::ParseNumber("$7B", n) && n == 123 &&
              control::Commander::ParseNumber("7Bh", n) && n == 123 && control::Commander::ParseNumber("123", n) && n == 123 &&
              !control::Commander::ParseNumber("xyz", n) && !control::Commander::ParseNumber("", n),
          "ParseNumber: decimal, 0x, $ e sufixo h; lixo recusado");
    std::string terr;
    const std::vector<std::string> toks = control::Commander::Tokenize("type \"a b\\nc\" x", terr);
    check(toks.size() == 3 && toks[1] == "a b\\nc" && toks[2] == "x", "Tokenize: aspas agrupam e a barra invertida e' literal");
    const std::vector<std::string> win = control::Commander::Tokenize("state save \"D:\\temp\\x y.sst\"", terr);
    check(win.size() == 3 && win[2] == "D:\\temp\\x y.sst", "Tokenize: caminho do Windows entre aspas fica intacto");
    control::Commander::Tokenize("type \"sem fechar", terr);
    check(!terr.empty(), "Tokenize: aspas sem fechar e' erro");

    // informativos
    check(cmd.Execute("help").ok && Contains(cmd.Execute("help").text, "peek"), "help lista os comandos");
    check(Contains(cmd.Execute("version").text, "fwMSX 1."), "version");
    const control::Result st = cmd.Execute("status");
    check(st.ok && Contains(st.text, "frame=") && Contains(st.text, "paused=0") && Contains(st.text, "cart=-"), "status: " + st.text);
    check(Contains(cmd.Execute("regs").text, "PC=") && Contains(cmd.Execute("regs").text, "SP="), "regs");
    check(cmd.Execute("").ok && cmd.Execute("   ").text.empty(), "linha vazia = ok, sem texto");

    const control::Result bad = cmd.Execute("naoexiste 1 2");
    check(!bad.ok && Contains(bad.Line(), "err comando desconhecido"), "comando desconhecido: err");

    // memoria
    std::vector<uint8_t> rom;
    {
        std::ifstream in(bios, std::ios::binary);
        rom.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }
    char want[32];
    std::snprintf(want, sizeof(want), "%02X %02X", rom[0], rom[1]);
    check(cmd.Execute("peek 0 2").text == want, std::string("peek 0 2 = bytes da BIOS (") + want + ")");
    check(cmd.Execute("poke 0xC000 0x41 0x42").ok && cmd.Execute("peek 0xC000 2").text == "41 42", "poke + peek na RAM");
    check(!cmd.Execute("peek 70000").ok && !cmd.Execute("poke 0 300").ok && !cmd.Execute("peek").ok, "peek/poke: argumentos invalidos = err");
    cmd.Execute("poke 0 0xAA");
    check(cmd.Execute("peek 0").text == want[0] + std::string(want).substr(1, 1), "poke em ROM nao altera a BIOS");

    // pausa e passo
    check(!cmd.Execute("step").ok, "step sem pausar = err");
    check(cmd.Execute("pause").ok && host.paused() && Contains(cmd.Execute("status").text, "paused=1"), "pause");
    const uint64_t f0 = host.machine().frame_count();
    check(cmd.Execute("step 5").ok && host.machine().frame_count() == f0 + 5, "step 5 avanca 5 quadros");
    check(cmd.Execute("resume").ok && !host.paused(), "resume");

    // digitacao: o BASIC executa o que foi digitado
    check(cmd.Execute("type \"print 1234\\n\"").ok, "type aceita o texto");
    check(cmd.typing(), "type: fila de digitacao ativa");
    Frames(host, cmd, 400);
    check(!cmd.typing() && VramHas(host.machine(), "print 1234") && VramHas(host.machine(), " 1234"),
          "type: o MSX BASIC executou 'print 1234' digitado pela ponte");
    check(!cmd.Execute("type \"\\x01\"").ok || true, "type com caractere sem tecla nao quebra");

    // reset
    check(cmd.Execute("reset").ok, "reset");

    // estado
    const std::string sst = TempPath("fwmsx_control_test.sst");
    check(cmd.Execute("state save \"" + sst + "\"").ok, "state save");
    check(cmd.Execute("poke 0xC000 0x99").ok, "(mutila a RAM)");
    check(cmd.Execute("state load \"" + sst + "\"").ok, "state load");
    check(!cmd.Execute("state load /nao/existe.sst").ok && !cmd.Execute("state foo x").ok, "state: erros");
    std::remove(sst.c_str());

    // discos, fita e cartucho
    check(!cmd.Execute("disk A x.dsk").ok && Contains(cmd.Execute("disk A x.dsk").text, "interface"), "disk sem interface de disquete = err explicando");
    check(!cmd.Execute("tape /nao/existe.cas").ok, "tape de arquivo inexistente = err");
    check(cmd.Execute("tape eject").ok && cmd.Execute("tape rewind").ok, "tape eject/rewind");
    check(!cmd.Execute("cart /nao/existe.rom").ok, "cart inexistente = err");
    check(!cmd.Execute("cart x.rom naoexiste").ok, "cart com mapper desconhecido = err");

    // captura de tela (o formato e' do host)
    const std::string shot = TempPath("fwmsx_control_test.shot");
    check(cmd.Execute("screenshot \"" + shot + "\"").ok, "screenshot delega ao host");
    std::remove(shot.c_str());

    check(cmd.Execute("quit").ok && host.quit_, "quit");
}

// ---- TCP ---------------------------------------------------------------------------------

std::string Roundtrip(sock_t s, const std::string &line) {
    const std::string out = line + "\n";
    send(s, out.data(), static_cast<int>(out.size()), 0);
    std::string reply;
    char c;
    while (recv(s, &c, 1, 0) == 1) {
        if (c == '\n') break;
        if (c != '\r') reply += c;
    }
    return reply;
}

void TestServer(const std::string &bios) {
    machine::MachineConfig config;
    config.bios_path = bios;
    TestHost host(config);
    if (!host.ok()) {
        check(false, "Machine para o teste do servidor");
        return;
    }
    control::Commander cmd(host);
    Frames(host, cmd, 250);

    control::Server server;
    std::string error;
    check(server.Start(0, error) && server.port() > 0, "servidor: abre 127.0.0.1 numa porta livre (" + error + ")");
    control::Server second;
    check(!second.Start(server.port(), error) && !error.empty(), "servidor: porta ocupada = erro claro");

    std::vector<std::string> replies;
    std::atomic<bool> client_done{false};
    std::thread client([&] {
#ifdef _WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
        sock_t s = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(server.port()));
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (s != SOCK_INVALID && connect(s, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == 0) {
            replies.push_back(Roundtrip(s, "version"));
            replies.push_back(Roundtrip(s, "poke 0xC100 0x5A"));
            replies.push_back(Roundtrip(s, "peek 0xC100"));
            replies.push_back(Roundtrip(s, "oops"));
            replies.push_back(Roundtrip(s, "pause"));
            sock_close(s);
        }
        client_done = true;
    });

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    int executed = 0;
    while (!client_done && std::chrono::steady_clock::now() < deadline) {
        executed += server.Poll(cmd);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    client.join();

    check(replies.size() == 5, "servidor: o cliente recebeu as 5 respostas (" + std::to_string(replies.size()) + ")");
    if (replies.size() == 5) {
        check(Contains(replies[0], "ok fwMSX 1."), "TCP: version -> " + replies[0]);
        check(replies[1] == "ok", "TCP: poke -> ok");
        check(replies[2] == "ok 5A", "TCP: peek devolve o byte gravado -> " + replies[2]);
        check(Contains(replies[3], "err comando desconhecido"), "TCP: comando invalido -> err");
        check(replies[4] == "ok" && host.paused(), "TCP: pause executado na thread do emulador");
    }
    check(executed == 5, "servidor: 5 comandos executados via Poll()");
    server.Stop();
    check(server.port() == 0, "servidor: Stop()");
}


// ---- depurador (doc/debug-spec.md) -----------------------------------------------------------------
void TestDebugger(const std::string &bios) {
    machine::MachineConfig config;
    config.bios_path = bios;
    TestHost host(config);
    if (!host.ok()) {
        check(false, "Machine para o teste do depurador");
        return;
    }
    control::Commander cmd(host);
    machine::Machine &m = host.machine();
    Frames(host, cmd, 250);

    // multilinha: "ok +N" e as N linhas
    const control::Result d0 = cmd.Execute("dis 0 3");
    check(d0.ok && d0.Line().rfind("ok +3\n", 0) == 0, "dis: resposta de varias linhas = 'ok +3' + 3 linhas");

    // breakpoint no vetor de interrupcao (todo quadro passa por 0038h)
    check(cmd.Execute("break 0x0038").ok, "break 0038");
    check(Contains(cmd.Execute("breaks").text, "0038"), "breaks lista o ponto");
    const size_t stop_mark = m.debugger().EventCount();
    (void)stop_mark;
    Frames(host, cmd, 3);
    check(m.debugger().stopped() && m.cpu().state().pc.w == 0x0038, "RunFrame para ANTES da instrucao no breakpoint (PC=0038)");
    check(Contains(cmd.Execute("status").text, "stopped=1"), "status mostra stopped=1");
    const uint64_t fc = m.frame_count();
    Frames(host, cmd, 5);
    check(m.frame_count() == fc, "parado: o quadro nao anda");
    const std::vector<std::string> ev = cmd.TakeEvents();
    check(!ev.empty() && ev[0].rfind("! break id=1 pc=0038", 0) == 0, "evento '! break id=1 pc=0038' na fila");
    check(cmd.TakeEvents().empty(), "TakeEvents esvazia a fila");

    const control::Result dis = cmd.Execute("dis");
    check(dis.ok && Contains(dis.text, "*0038"), "dis: '*' marca o breakpoint no PC");

    // passo de instrucao
    const control::Result st = cmd.Execute("stepi 2");
    check(st.ok && Contains(st.text, "pc=") && m.cpu().state().pc.w != 0x0038 && m.debugger().stopped(), "stepi 2: anda e continua parado -> " + st.text);
    check(cmd.TakeEvents().empty(), "stepi nao gera evento (a resposta ja' diz tudo)");

    // go: volta a rodar e para de novo no mesmo breakpoint no proximo quadro
    check(Contains(cmd.Execute("go").text, "running") && !m.debugger().stopped(), "go: roda");
    Frames(host, cmd, 3);
    check(m.debugger().stopped() && m.cpu().state().pc.w == 0x0038, "go: para de novo em 0038 (primeira instrucao pulada sem testar o PC)");
    cmd.TakeEvents();
    check(cmd.Execute("unbreak 0x0038").ok && Contains(cmd.Execute("breaks").text, "sem breakpoints"), "unbreak");

    // next sobre um CALL: programa curto na RAM
    //   C000: CALL C008   C003: NOP   C004: JR $
    //   C008: LD A,55h    C00A: RET
    const uint8_t prog[] = {0xCD, 0x08, 0xC0, 0x00, 0x18, 0xFE, 0x00, 0x00, 0x3E, 0x55, 0xC9};
    for (size_t i = 0; i < sizeof(prog); ++i) m.WriteMemory(static_cast<uint16_t>(0xC000 + i), prog[i]);
    check(cmd.Execute("jump 0xC000").ok, "jump C000");
    const control::Result nx = cmd.Execute("next");
    check(nx.ok && Contains(nx.text, "pc=C003"), "next: executa o CALL inteiro e para em C003 -> " + nx.text);
    check(m.cpu().state().af.b.hi == 0x55, "next: a subrotina rodou (A=55)");
    check(cmd.TakeEvents().empty(), "next sincrono nao gera evento");

    // until
    check(cmd.Execute("jump 0xC000").ok, "jump C000 de novo");
    const control::Result un = cmd.Execute("until 0xC00A");
    check(un.ok && Contains(un.text, "pc=C00A"), "until C00A -> " + un.text);

    // watchpoint de escrita:  C010: LD A,77h   C012: LD (C100),A   C015: JR $
    const uint8_t prog2[] = {0x3E, 0x77, 0x32, 0x00, 0xC1, 0x18, 0xFE};
    for (size_t i = 0; i < sizeof(prog2); ++i) m.WriteMemory(static_cast<uint16_t>(0xC010 + i), prog2[i]);
    check(cmd.Execute("watch 0xC100 1 w").ok && Contains(cmd.Execute("watches").text, "C100"), "watch C100 w");
    cmd.Execute("jump 0xC010");
    cmd.Execute("go");
    Frames(host, cmd, 2);
    check(m.debugger().stopped(), "watch: a escrita em C100 para a maquina");
    const std::vector<std::string> wev = cmd.TakeEvents();
    check(!wev.empty() && Contains(wev[0], "! watch") && Contains(wev[0], "addr=C100") && Contains(wev[0], "value=77") &&
              Contains(wev[0], "pc=C012"),
          "evento do watch: addr, valor e PC de quem escreveu -> " + (wev.empty() ? std::string("(nenhum)") : wev[0]));
    check(m.ReadMemory(0xC100) == 0x77, "a escrita aconteceu (so' parou depois da instrucao)");
    check(cmd.Execute("unwatch all").ok, "unwatch all");

    // registradores e memoria
    check(cmd.Execute("reg").text.find('\n') != std::string::npos && Contains(cmd.Execute("reg").text, "F="), "reg: tudo, com flags");
    check(cmd.Execute("reg hl 0x1234").ok && m.cpu().state().hl.w == 0x1234, "reg hl 1234");
    check(cmd.Execute("reg a 0x80").ok && m.cpu().state().af.b.hi == 0x80, "reg a 80");
    check(!cmd.Execute("reg zz 1").ok, "reg: nome desconhecido e' erro");
    const control::Result mem = cmd.Execute("mem 0xC010 16");
    check(mem.ok && Contains(mem.text, "C010: 3E 77 32 00 C1"), "mem: hexdump -> " + mem.text);
    check(cmd.Execute("stack 4").ok && cmd.Execute("slots").ok && Contains(cmd.Execute("slots").text, "pagina 3"), "stack e slots respondem");
    check(!cmd.Execute("break zzz").ok && !cmd.Execute("until").ok && !cmd.Execute("watch 1 0").ok, "argumentos invalidos viram erro");
    check(cmd.Execute("events on").ok && !cmd.Execute("events talvez").ok, "events: sintaxe");
    check(Contains(cmd.Execute("stop").text, "pc="), "stop");

    // comecar parado (--debug) e breakpoints/watchpoints vindos da configuracao (--break, --watch)
    machine::MachineConfig dc;
    dc.bios_path = bios;
    dc.debug_start_stopped = true;
    dc.debug_breaks = {0x0038};
    machine::DebugWatchSpec ws;
    ws.addr = 0xE000;
    dc.debug_watches.push_back(ws);
    TestHost dhost(dc);
    check(dhost.ok() && dhost.machine().debugger().stopped() && dhost.machine().debugger().breaks().size() == 1 &&
              dhost.machine().debugger().watches().size() == 1,
          "config: --debug comeca parado e --break/--watch ja' vem armados");
    control::Commander dcmd(dhost);
    check(Contains(dcmd.Execute("stepi").text, "pc=") && dhost.machine().cpu().state().pc.w != 0, "config: stepi sai do PC 0000");
}

// Cliente TCP que se inscreve nos eventos e le linhas soltas ("! ...").
bool ReadLine(sock_t s, std::string &line) {
    line.clear();
    char c;
    while (recv(s, &c, 1, 0) == 1) {
        if (c == '\n') return true;
        if (c != '\r') line += c;
    }
    return false;
}

void TestEvents(const std::string &bios) {
    machine::MachineConfig config;
    config.bios_path = bios;
    TestHost host(config);
    if (!host.ok()) {
        check(false, "Machine para o teste de eventos");
        return;
    }
    control::Commander cmd(host);
    Frames(host, cmd, 250);
    control::Server server;
    std::string error;
    if (!server.Start(0, error)) {
        check(false, "servidor para o teste de eventos: " + error);
        return;
    }

    std::string reply_events, reply_dis, reply_break, event_line, reply_after;
    std::atomic<bool> subscribed{false};
    std::atomic<bool> go_on{false};
    std::atomic<bool> run_frames{false}; // o "emulador" so' anda quadros depois do go (sem corrida com o dis)
    std::thread client([&] {
#ifdef _WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
        sock_t s = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(server.port()));
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (s == SOCK_INVALID || connect(s, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0) return;
        {
            // um teste que falha nao pode travar: 8 s sem resposta e' falha
#ifdef _WIN32
            DWORD ms = 8000;
            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&ms), sizeof(ms));
#else
            timeval tv{8, 0};
            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
        }
        reply_events = Roundtrip(s, "events on");
        subscribed = true;
        reply_break = Roundtrip(s, "break 0x0038");
        // resposta de varias linhas: "ok +2" e 2 linhas
        const std::string out = "dis 0 2\n";
        send(s, out.data(), static_cast<int>(out.size()), 0);
        std::string head, l1, l2;
        ReadLine(s, head);
        ReadLine(s, l1);
        ReadLine(s, l2);
        reply_dis = head + "|" + l1 + "|" + l2;
        // agora o emulador roda e o breakpoint dispara sozinho: chega "! break ..." sem ninguem pedir
        run_frames = true;
        const std::string go = "go\n";
        send(s, go.data(), static_cast<int>(go.size()), 0);
        std::string a, b;
        ReadLine(s, a);
        ReadLine(s, b);
        // a ordem entre a resposta do go e o evento nao e' fixa
        event_line = a.rfind("! ", 0) == 0 ? a : b;
        reply_after = a.rfind("! ", 0) == 0 ? b : a;
        sock_close(s);
        go_on = true;
    });

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (!go_on && std::chrono::steady_clock::now() < deadline) {
        server.Poll(cmd);
        // o "emulador" anda quadros como a janela faz
        if (run_frames && !host.machine().debugger().stopped()) Frames(host, cmd, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    client.join();
    server.Stop();

    check(reply_events == "ok", "TCP: events on -> ok");
    check(reply_break.rfind("ok id=", 0) == 0, "TCP: break -> " + reply_break);
    check(reply_dis.rfind("ok +2|", 0) == 0 && Contains(reply_dis, "|"), "TCP: dis responde 'ok +2' e 2 linhas -> " + reply_dis);
    check(event_line.rfind("! break id=1 pc=0038", 0) == 0, "TCP: evento assincrono '! break ...' chegou -> " + event_line);
    check(reply_after == "ok running", "TCP: go -> ok running (" + reply_after + ")");
}

} // namespace

int main() {
#ifdef FWMSX_SOURCE_DIR
    const std::string bios = std::string(FWMSX_SOURCE_DIR) + "/resource/fMSX/ROMs/MSX.ROM";
    {
        std::ifstream probe(bios, std::ios::binary);
        if (!probe) {
            std::printf("[SKIP] BIOS nao encontrada em '%s'\n", bios.c_str());
            return 0;
        }
    }
    TestCommander(bios);
    TestServer(bios);
    TestDebugger(bios);
    TestEvents(bios);
#else
    std::printf("[SKIP] FWMSX_SOURCE_DIR nao definido\n");
#endif
    std::printf("\n%s (%d falha(s))\n", g_failures ? "FALHOU" : "OK", g_failures);
    return g_failures ? 1 : 0;
}
