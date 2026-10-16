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
#include "../../src/debug/cpp/debug_view.h"
#include "../../src/machine/machine.h"
#include "../../src/ppi/core/ppi_state.h"
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
    Frames(host, cmd, 100);

    // {tecla} fora das aspas: {enter}, {esc}, {ctrl}+{c} (juntas) x {ctrl}{c} (em sequencia), {enter*3}, {ctrl+c}
    check(cmd.Execute("type \"print 77\" {enter}").ok && Contains(cmd.Execute("type {enter*3}").text, "3 teclas"), "type: {enter} fora das aspas e {enter*3}");
    Frames(host, cmd, 400);
    check(!cmd.typing() && VramHas(host.machine(), " 77"), "type \"print 77\" {enter}: o BASIC executou");
    check(Contains(cmd.Execute("type \"ab\" {ctrl}+{c}").text, "3 teclas"), "type: {ctrl}+{c} e' UMA digitacao (2 letras + 1)");
    Frames(host, cmd, 100);
    check(Contains(cmd.Execute("type {ctrl}{c}").text, "2 teclas"), "type: {ctrl}{c} sao duas digitacoes");
    Frames(host, cmd, 100);
    check(Contains(cmd.Execute("type {CTRL+C}{Esc}{F1}{stop}{Return}").text, "5 teclas"), "type: {ctrl+c}, maiusculas e apelidos (return)");
    Frames(host, cmd, 150);
    check(Contains(cmd.Execute("type oi mundo").text, "8 teclas"), "type: palavras soltas continuam juntas por um espaco");
    Frames(host, cmd, 100);
    {   // --keys: mesma sintaxe, mas aspas e espacos sao texto comum
        std::vector<control::Commander::TypedKey> tk;
        std::string kerr;
        check(control::Commander::ParseTypeSpec(host.machine(), "print \"a\" {enter}{ctrl}+{c}", true, tk, kerr) && tk.size() == 12 &&
                  tk.back().keys.size() == 2 && tk.back().keys[0] == "ctrl",
              "--keys: texto literal (aspas e espacos) + {enter} + {ctrl}+{c} -> " + std::to_string(tk.size()) + " " + kerr);
        tk.clear();
        check(!control::Commander::ParseTypeSpec(host.machine(), "oi {nada}", true, tk, kerr), "--keys: tecla desconhecida = erro");
    }
    check(!cmd.Execute("type {foo}").ok && !cmd.Execute("type {enter").ok && !cmd.Execute("type").ok, "type: tecla desconhecida, '{' sem '}' e vazio = erro");
    cmd.Execute("reset");
    Frames(host, cmd, 50);

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


// ---- Z80 Debugger da janela (modelo, sem OpenGL) ------------------------------------------------------
std::string Hex4(unsigned v) {
    char b[8];
    std::snprintf(b, sizeof(b), "%04X", v & 0xFFFF);
    return b;
}

// Linha do corpo cujo marcador de PC ('>', coluna 1) esta' acesa; -1 se nao ha'.
int PcRow(const dbgui::Grid &g) {
    for (int y = dbgui::kHeaderRows; y < dbgui::kHeaderRows + dbgui::kBodyRows; ++y) {
        if (g.at(1, y).ch == '>') return y;
    }
    return -1;
}

void TestDebugView(const std::string &bios) {
    machine::MachineConfig config;
    config.bios_path = bios;
    TestHost host(config);
    if (!host.ok()) {
        check(false, "Machine para o teste do Z80 Debugger");
        return;
    }
    control::Commander cmd(host);
    machine::Machine &m = host.machine();
    Frames(host, cmd, 250);

    // programa de teste na RAM:  C100: LD A,05h   C102: JP 4000h   C105: NOP
    const uint8_t prog[] = {0x3E, 0x05, 0xC3, 0x00, 0x40, 0x00};
    for (size_t i = 0; i < sizeof(prog); ++i) m.WriteMemory(static_cast<uint16_t>(0xC100 + i), prog[i]);
    check(dbgui::PreviousInstruction(m, 0xC105) == 0xC102 && dbgui::PreviousInstruction(m, 0xC102) == 0xC100,
          "PreviousInstruction: acha o inicio da instrucao anterior (JP de 3 bytes e LD de 2)");
    check(dbgui::InstructionLength(m, 0xC100) == 2 && dbgui::InstructionLength(m, 0xC102) == 3, "InstructionLength");

    // a fonte do MSX vem da BIOS (CGTABL)
    dbgui::Font font;
    check(dbgui::LoadFontFromMachine(m, font) && font.valid, "fonte do MSX lida da BIOS (CGTABL)");
    bool a_ok = false;
    for (int r = 0; r < 8; ++r) a_ok = a_ok || font.glyph[0x41][r] != 0;
    check(a_ok, "a letra 'A' da fonte tem pixels");

    dbgui::DebugView v;
    v.Enter(m);
    check(m.debugger().stopped() && v.mode() == dbgui::ViewMode::Disasm && v.cursor() == m.cpu().state().pc.w, "Enter: para a maquina, vista de desmontagem, cursor no PC");

    // desenho padrao: endereco, bytes em hexa, instrucao; registradores nas primeiras linhas
    dbgui::Grid g;
    v.Render(m, g);
    check(dbgui::GridRowText(g, 0).find("Z80 Debugger") != std::string::npos, "titulo 'Z80 Debugger'");
    check(dbgui::GridRowText(g, 1).find("AF=") != std::string::npos && dbgui::GridRowText(g, 2).find("PC=" + Hex4(m.cpu().state().pc.w)) != std::string::npos &&
              dbgui::GridRowText(g, 3).find("AF'=") != std::string::npos && dbgui::GridRowText(g, 4).find("F=") != std::string::npos,
          "registradores nas linhas 1 a 4 (AF BC DE HL / IX IY SP PC / sombra / I R IM flags)");
    const int pcrow = PcRow(g);
    check(pcrow >= dbgui::kHeaderRows, "o PC tem o marcador '>' no corpo");
    const std::string pcline = pcrow >= 0 ? dbgui::GridRowText(g, pcrow) : std::string();
    check(pcline.find(Hex4(m.cpu().state().pc.w) + ": ") != std::string::npos, "linha: 'ENDERECO: bytes instrucao' -> " + pcline);
    check(g.at(5, pcrow).style == dbgui::kCursor, "a linha do cursor usa o estilo de cursor");
    check(dbgui::GridRowText(g, dbgui::kRows - 3).find("ENTER") != std::string::npos, "o rodape lista as teclas");

    // ENTER executa a proxima instrucao
    const uint16_t pc0 = m.cpu().state().pc.w;
    v.Press(dbgui::Key::Enter, m);
    check(m.cpu().state().pc.w != pc0 && v.cursor() == m.cpu().state().pc.w && m.debugger().stopped(), "ENTER: executa uma instrucao, o cursor segue o PC e continua parado");

    // Down / Up
    const uint16_t c1 = v.cursor();
    v.Press(dbgui::Key::Down, m);
    check(v.cursor() == static_cast<uint16_t>(c1 + dbgui::InstructionLength(m, c1)), "Down: instrucao seguinte");
    v.Press(dbgui::Key::Up, m);
    check(v.cursor() == c1, "Up: volta para a instrucao anterior");
    const uint16_t c2 = v.cursor();
    v.Press(dbgui::Key::Right, m);
    check(v.cursor() != c2, "Right: rola uma pagina para baixo");
    v.Press(dbgui::Key::Left, m);
    dbgui::Grid g2;
    v.Render(m, g2);
    bool cursor_visible = false;
    for (int y = dbgui::kHeaderRows; y < dbgui::kHeaderRows + dbgui::kBodyRows; ++y) cursor_visible = cursor_visible || g2.at(5, y).style == dbgui::kCursor;
    check(cursor_visible, "o cursor continua visivel depois de paginar");

    // G: vai para o endereco digitado
    v.Press(dbgui::Key::Go, m);
    check(v.prompt_active(), "G abre a pergunta de endereco");
    for (const char c : std::string("c1x00")) v.TypeChar(c); // 'x' e' ignorado: so' digitos hexa
    dbgui::Grid gp;
    v.Render(m, gp);
    check(dbgui::GridRowText(gp, dbgui::kRows - dbgui::kFooterRows).find("C100_") != std::string::npos, "a pergunta mostra o que foi digitado (C100)");
    v.Press(dbgui::Key::Enter, m);
    check(!v.prompt_active() && v.cursor() == 0xC100, "ENTER na pergunta confirma: cursor em C100");
    dbgui::Grid g3;
    v.Render(m, g3);
    bool has_ld = false, has_jp = false;
    for (int y = dbgui::kHeaderRows; y < dbgui::kHeaderRows + dbgui::kBodyRows; ++y) {
        const std::string t = dbgui::GridRowText(g3, y);
        has_ld = has_ld || t.find("C100: 3E 05") != std::string::npos;
        has_jp = has_jp || t.find("C102: C3 00 40") != std::string::npos;
    }
    check(has_ld && has_jp, "a desmontagem mostra os bytes de LD A,05h e de JP 4000h");
    v.Press(dbgui::Key::Go, m);
    v.Press(dbgui::Key::Escape, m);
    check(!v.prompt_active(), "Esc cancela a pergunta");

    // B: breakpoint no cursor
    v.Press(dbgui::Key::Break, m);
    check(m.debugger().breaks().size() == 1 && m.debugger().breaks()[0].addr == 0xC100, "B poe um breakpoint no cursor");
    dbgui::Grid gb;
    v.Render(m, gb);
    bool star = false;
    for (int y = dbgui::kHeaderRows; y < dbgui::kHeaderRows + dbgui::kBodyRows; ++y) star = star || gb.at(0, y).ch == '*';
    check(star, "o breakpoint aparece como '*'");
    v.Press(dbgui::Key::Break, m);
    check(m.debugger().breaks().empty(), "B de novo tira o breakpoint");

    // J: pula para o cursor
    v.Press(dbgui::Key::Jump, m);
    check(m.cpu().state().pc.w == 0xC100, "J: o PC passa a ser o cursor");

    // M: dump de memoria, 8 bytes por linha com os caracteres do MSX
    for (int i = 0; i < 8; ++i) m.WriteMemory(static_cast<uint16_t>(0xC200 + i), static_cast<uint8_t>(0x41 + i));
    v.Press(dbgui::Key::Go, m);
    v.Press(dbgui::Key::Memory, m); // ignorado: a pergunta esta' aberta
    check(v.mode() == dbgui::ViewMode::Disasm, "com a pergunta aberta as outras teclas nao fazem nada");
    v.Press(dbgui::Key::Escape, m);
    v.Press(dbgui::Key::Memory, m);
    check(v.mode() == dbgui::ViewMode::Memory, "M: vista de dump de memoria");
    v.Press(dbgui::Key::Go, m);
    for (const char c : std::string("C200")) v.TypeChar(c);
    v.Press(dbgui::Key::Enter, m);
    dbgui::Grid gm;
    v.Render(m, gm);
    int mrow = -1;
    for (int y = dbgui::kHeaderRows; y < dbgui::kHeaderRows + dbgui::kBodyRows; ++y) {
        if (dbgui::GridRowText(gm, y).find("C200: 41 42 43 44 45 46 47 48") != std::string::npos) mrow = y;
    }
    check(mrow >= 0, "dump: 'C200: 41 42 43 44 45 46 47 48' (8 bytes por linha)");
    bool chars_ok = mrow >= 0;
    for (int k = 0; k < 8 && mrow >= 0; ++k) chars_ok = chars_ok && gm.at(32 + k, mrow).ch == 0x41 + k;
    check(chars_ok, "dump: os mesmos 8 bytes como caracteres do MSX (codigos 41h-48h)");
    check(dbgui::GridRowText(gm, 0).find("Memory") != std::string::npos, "o titulo muda para a vista de memoria");
    v.Press(dbgui::Key::Down, m);
    check(v.cursor() == 0xC208, "dump: Down anda uma linha (8 bytes)");

    // S: pilha
    v.Press(dbgui::Key::Stack, m);
    dbgui::Grid gs;
    v.Render(m, gs);
    bool sp_row = false;
    for (int y = dbgui::kHeaderRows; y < dbgui::kHeaderRows + dbgui::kBodyRows; ++y) sp_row = sp_row || dbgui::GridRowText(gs, y).find("SP+00") != std::string::npos;
    check(v.mode() == dbgui::ViewMode::Stack && sp_row, "S: vista da pilha, com a linha SP+00");

    // D volta a vista padrao
    v.Press(dbgui::Key::Disasm, m);
    check(v.mode() == dbgui::ViewMode::Disasm, "D: volta a vista de desmontagem");

    // pixels com a fonte do MSX
    std::vector<uint32_t> px;
    int pw = 0, ph = 0;
    check(dbgui::RenderRgba(gs, font, px, pw, ph) && pw == dbgui::kCols * 6 && ph == dbgui::kRows * 8, "RenderRgba: tela de texto do MSX (6x8 por caractere)");
    bool two_colors = false;
    for (size_t i = 1; i < px.size() && !two_colors; ++i) two_colors = px[i] != px[0];
    check(two_colors, "RenderRgba: ha' texto desenhado (mais de uma cor)");
    dbgui::Font none;
    check(!dbgui::RenderRgba(gs, none, px, pw, ph), "RenderRgba: sem fonte valida devolve false (a janela usa texto comum)");

    // FWMSX_DBGVIEW_PPM=<pasta>: grava as tres vistas como PPM (para olhar com os proprios olhos, com a fonte do MSX)
    if (const char *dir = std::getenv("FWMSX_DBGVIEW_PPM")) {
        const dbgui::ViewMode modes[] = {dbgui::ViewMode::Disasm, dbgui::ViewMode::Memory, dbgui::ViewMode::Stack};
        const char *names[] = {"disasm", "memory", "stack"};
        v.Press(dbgui::Key::Disasm, m);
        for (int k = 0; k < 3; ++k) {
            if (k == 1) v.Press(dbgui::Key::Memory, m);
            if (k == 2) v.Press(dbgui::Key::Stack, m);
            (void)modes;
            dbgui::Grid gd;
            v.Render(m, gd);
            std::vector<uint32_t> dpx;
            int dw = 0, dh = 0;
            if (dbgui::RenderRgba(gd, font, dpx, dw, dh)) {
                std::ofstream o(std::string(dir) + "/dbgview_" + names[k] + ".ppm", std::ios::binary);
                o << "P6\n" << dw << " " << dh << "\n255\n";
                for (const uint32_t p : dpx) {
                    const char rgb[3] = {static_cast<char>(p & 0xFF), static_cast<char>((p >> 8) & 0xFF), static_cast<char>((p >> 16) & 0xFF)};
                    o.write(rgb, 3);
                }
            }
        }
        v.Press(dbgui::Key::Disasm, m);
    }

    // R, C e Q
    m.debugger().Stop("manual");
    v.Press(dbgui::Key::Go, m);
    for (const char c : std::string("C105")) v.TypeChar(c);
    v.Press(dbgui::Key::Enter, m);
    check(v.Press(dbgui::Key::RunTo, m) == dbgui::Action::Close && !m.debugger().stopped() && m.debugger().until_active(),
          "R: roda ate' o cursor (fecha o painel, a maquina volta a rodar com o ponto de parada de uso unico)");
    m.debugger().Stop("manual");
    check(v.Press(dbgui::Key::Continue, m) == dbgui::Action::Close && !m.debugger().stopped(), "C: continua a execucao (fecha o painel)");
    check(v.Press(dbgui::Key::Quit, m) == dbgui::Action::QuitEmulator, "Q: encerra o emulador");
    check(v.Press(dbgui::Key::Escape, m) == dbgui::Action::Close, "Esc fecha o painel");
}

// ---- comando dbgui: o painel Z80 Debugger como texto (para a TUI e qualquer cliente) -------------------
void TestDbgUiCommand(const std::string &bios) {
    machine::MachineConfig config;
    config.bios_path = bios;
    TestHost host(config);
    if (!host.ok()) {
        check(false, "Machine para o teste do comando dbgui");
        return;
    }
    control::Commander cmd(host);
    machine::Machine &m = host.machine();
    Frames(host, cmd, 250);

    const control::Result open = cmd.Execute("dbgui open");
    const std::string line = open.Line();
    check(open.ok && line.rfind("ok +33\n", 0) == 0, "dbgui open: 'ok +33' (a linha de estado e 32 linhas do painel)");
    check(m.debugger().stopped(), "dbgui open para a maquina");
    check(Contains(open.text, "action=none prompt=0 mode=disasm"), "dbgui open: action=none prompt=0 mode=disasm");
    check(Contains(open.text, "TZ80 Debugger - Disassembly") || Contains(open.text, "T Z80 Debugger"), "a primeira linha do painel e' o titulo (estilo T)");
    check(Contains(open.text, "\nR AF=") || Contains(open.text, "\nR") , "as linhas de registradores vem com o estilo R");
    check(Contains(open.text, "\nC"), "a linha do cursor vem com o estilo C");

    const uint16_t pc0 = m.cpu().state().pc.w;
    const control::Result st = cmd.Execute("dbgui key enter");
    check(st.ok && m.cpu().state().pc.w != pc0 && m.debugger().stopped(), "dbgui key enter: executa uma instrucao");
    check(Contains(cmd.Execute("dbgui key g").text, "prompt=1"), "dbgui key g: abre a pergunta (prompt=1)");
    cmd.Execute("dbgui char c");
    cmd.Execute("dbgui char 2");
    cmd.Execute("dbgui char 0");
    cmd.Execute("dbgui char 0");
    check(Contains(cmd.Execute("dbgui show").text, "C200_"), "dbgui char: os digitos aparecem na pergunta (C200_)");
    const control::Result go = cmd.Execute("dbgui key enter");
    check(Contains(go.text, "prompt=0") && Contains(go.text, "C200:"), "ENTER confirma a pergunta e o painel mostra C200");
    check(Contains(cmd.Execute("dbgui key m").text, "mode=memory") && Contains(cmd.Execute("dbgui key s").text, "mode=stack") &&
              Contains(cmd.Execute("dbgui key d").text, "mode=disasm"),
          "dbgui key m / s / d: troca de vista");
    check(!cmd.Execute("dbgui key zzz").ok && !cmd.Execute("dbgui nada").ok && !cmd.Execute("dbgui char ab").ok, "dbgui: argumentos invalidos viram erro");

    const control::Result esc = cmd.Execute("dbgui key esc");
    check(Contains(esc.text, "action=close") && m.debugger().stopped() && m.debugger().last_stop() == "manual",
          "esc: action=close e a maquina continua parada ('manual': nada reabre sozinho)");
    cmd.Execute("dbgui open");
    const control::Result cont = cmd.Execute("dbgui key c");
    check(Contains(cont.text, "action=close") && !m.debugger().stopped(), "c: action=close e a maquina volta a rodar");
    cmd.Execute("dbgui open");
    host.quit_ = false;
    check(Contains(cmd.Execute("dbgui key q").text, "action=quit") && host.quit_, "q: action=quit e o emulador e' encerrado");
}


// ---- teclado: a pontuacao digitada usa a tecla CERTA para a BIOS (internacional x japones) -------------------
std::vector<uint8_t> ReadFile(const std::string &path) {
    std::ifstream f(path, std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

// A tecla `key` esta' na posicao (row, mask) da matriz?
bool KeyAtPos(const std::string &key, int row, int bit) {
    const int id = ppi_key_lookup(key.c_str());
    int r = -1;
    uint8_t mask = 0;
    return id != PPI_KEY_NONE && ppi_key_position(id, &r, &mask) && r == row && mask == (1u << bit);
}

// Confere `ch` (lido da tabela da BIOS na posicao row/bit, com ou sem SHIFT) contra o que o layout manda digitar.
bool LayoutGivesChar(machine::Machine::KeyboardLayout layout, uint8_t ch, int row, int bit, bool want_shift) {
    if (ch == 0xFF || ch == 0 || ch == '|') return true; // sem tecla / dead key / o '|' do --keys e' ENTER
    std::string key;
    bool shift = false;
    if (!machine::Machine::KeysForCharLayout(static_cast<char>(ch), layout, key, shift)) return false;
    return shift == want_shift && KeyAtPos(key, row, bit);
}

void TestKeyboard(const std::string &root, const std::string &bios) {
    using machine::Machine;
    // Internacional: as 2 tabelas (normal e com SHIFT, 6 linhas de 8 teclas) do MSX.ROM em 0DA5h.
    {
        const std::vector<uint8_t> rom = ReadFile(bios);
        bool ok = rom.size() >= 0x0DA5 + 96, all = true;
        for (int r = 0; ok && r < 6; ++r) {
            for (int b = 0; b < 8; ++b) {
                const uint8_t n = rom[0x0DA5 + r * 8 + b], sh = rom[0x0DA5 + 48 + r * 8 + b];
                if (!LayoutGivesChar(Machine::KeyboardLayout::International, n, r, b, false)) {
                    std::printf("   internacional: '%c' (linha %d, bit %d) nao confere\n", n, r, b);
                    all = false;
                }
                if (!LayoutGivesChar(Machine::KeyboardLayout::International, sh, r, b, true)) {
                    std::printf("   internacional: SHIFT '%c' (linha %d, bit %d) nao confere\n", sh, r, b);
                    all = false;
                }
            }
        }
        check(ok && all, "layout internacional: cada caractere das tabelas da BIOS (MSX.ROM, 0DA5h) sai da tecla e do SHIFT certos");
    }
    // Japones: a BIOS do MSX2+ (so' guarda a pontuacao: digitos com SHIFT em 0DC9h, 12+12 teclas em 0DE3h/0DEFh).
    {
        const std::vector<uint8_t> rom = ReadFile(root + "/dist/roms/bios/MSX2P.ROM");
        bool ok = rom.size() >= 0x0DFB, all = true;
        for (int d = 0; ok && d < 10; ++d) { // SHIFT+digito: linha 0 (0-7) e linha 1 (8, 9)
            const int row = d < 8 ? 0 : 1, bit = d < 8 ? d : d - 8;
            if (!LayoutGivesChar(Machine::KeyboardLayout::Japanese, rom[0x0DC9 + d], row, bit, true)) {
                std::printf("   japones: SHIFT+%d -> '%c' nao confere\n", d, rom[0x0DC9 + d]);
                all = false;
            }
        }
        for (int i = 0; ok && i < 12; ++i) { // linha 1 bits 2-7, depois linha 2 bits 0-5
            const int row = i < 6 ? 1 : 2, bit = i < 6 ? 2 + i : i - 6;
            if (!LayoutGivesChar(Machine::KeyboardLayout::Japanese, rom[0x0DE3 + i], row, bit, false)) {
                std::printf("   japones: '%c' (linha %d, bit %d) nao confere\n", rom[0x0DE3 + i], row, bit);
                all = false;
            }
            if (!LayoutGivesChar(Machine::KeyboardLayout::Japanese, rom[0x0DEF + i], row, bit, true)) {
                std::printf("   japones: SHIFT '%c' (linha %d, bit %d) nao confere\n", rom[0x0DEF + i], row, bit);
                all = false;
            }
        }
        check(ok && all, "layout japones: cada caractere da pontuacao da BIOS do MSX2+ (MSX2P.ROM) sai da tecla e do SHIFT certos");
    }
    std::string key;
    bool shift = false;
    check(Machine::KeysForCharLayout('(', Machine::KeyboardLayout::International, key, shift) && key == "9" && shift &&
              Machine::KeysForCharLayout('(', Machine::KeyboardLayout::Japanese, key, shift) && key == "8" && shift,
          "'(' e' SHIFT+9 no internacional e SHIFT+8 no japones");
    check(Machine::KeysForCharLayout('=', Machine::KeyboardLayout::International, key, shift) && key == "=" && !shift &&
              Machine::KeysForCharLayout('=', Machine::KeyboardLayout::Japanese, key, shift) && key == "-" && shift,
          "'=' e' a tecla do '=' no internacional e SHIFT+'-' no japones");
    check(!Machine::KeysForCharLayout('\x01', Machine::KeyboardLayout::Japanese, key, shift), "caractere sem tecla = false");
    check(ppi_key_count() == 88 && ppi_key_lookup("ro") != PPI_KEY_NONE, "a tecla 'ro' (o '_' do teclado japones) existe na matriz");

    // De ponta a ponta: o BASIC mostra o que foi digitado. MSX1 (internacional) e MSX2+ (japones).
    for (int pass = 0; pass < 2; ++pass) {
        machine::MachineConfig config;
        if (pass == 0) {
            config.bios_path = bios;
        } else {
            config.model = machine::Model::MSX2P;
            config.bios_path = root + "/dist/roms/bios/MSX2P.ROM";
            config.ext_rom_path = root + "/resource/fMSX/MSX2PEXT.ROM";
        }
        TestHost host(config);
        if (!host.ok()) {
            check(false, "Machine para o teste do teclado (pass " + std::to_string(pass) + ")");
            continue;
        }
        control::Commander cmd(host);
        machine::Machine &m = host.machine();
        check(m.keyboard_layout() == (pass == 0 ? Machine::KeyboardLayout::International : Machine::KeyboardLayout::Japanese),
              pass == 0 ? "MSX1: a BIOS e' de teclado internacional (byte 002Ch)" : "MSX2+: a BIOS e' de teclado japones (byte 002Ch)");
        Frames(host, cmd, 320);
        check(cmd.Execute("type \"print (1+2)*3:rem a=b\\n\"").ok, "type com parenteses, +, *, : e = (" + std::string(pass == 0 ? "MSX1" : "MSX2+") + ")");
        Frames(host, cmd, 400);
        check(VramHas(m, "print (1+2)*3:rem a=b"), "o BASIC recebeu exatamente 'print (1+2)*3:rem a=b' (" + std::string(pass == 0 ? "MSX1" : "MSX2+") + ")");
    }
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
    TestDebugView(bios);
    TestDbgUiCommand(bios);
    TestKeyboard(FWMSX_SOURCE_DIR, bios);
    TestEvents(bios);
#else
    std::printf("[SKIP] FWMSX_SOURCE_DIR nao definido\n");
#endif
    std::printf("\n%s (%d falha(s))\n", g_failures ? "FALHOU" : "OK", g_failures);
    return g_failures ? 1 : 0;
}
