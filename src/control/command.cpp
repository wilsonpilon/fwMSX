// fwMSX -- despachante de comandos da ponte de controle. Ver command.h e doc/control-spec.md.
#include "command.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>

#include "../common/version.h"
#include "../debug/c/dbg_flow.h"
#include "../mamute/cpp/mamute_monitor.h"
#include "../mamute/cpp/mamute_online.h"
#include "../z80/debug/z80_disasm.h"

extern "C" void dbg_format_flags(int f, char *buf); // src/debug/fortran/dbg_fmt.f90

namespace control {

namespace {

Result Ok(std::string text = std::string()) { return Result{true, std::move(text)}; }
Result Err(std::string text) { return Result{false, std::move(text)}; }

std::string Hex(unsigned value, int digits) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%0*X", digits, value);
    return buf;
}

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// "A"/"B" (qualquer caixa) -> 0/1; -1 se invalido.
int DriveIndex(const std::string &s) {
    const std::string l = Lower(s);
    if (l == "a" || l == "a:") return 0;
    if (l == "b" || l == "b:") return 1;
    return -1;
}

const char *kHelp =
    "comandos: help version status reset pause resume step [n] type <texto|{tecla}> peek <end> [n] "
    "poke <end> <v>... regs cart <arq|-> [mapper] disk <A|B> <arq> eject <A|B> tape <arq|eject|rewind> "
    "state <save|load> <arq> screenshot <arq> quit | depurador: dbg break <end> unbreak <end|all> breaks "
    "watch <end> [tam] [r|w|rw] unwatch <end|all> watches stepi [n] next until <end> go [end] stop jump <end> "
    "dis [end] [n] mem <end> [n] stack [n] reg [nome valor] slots events <on|off> dbgui <open|show|key|char> (ver doc/debug-spec.md) | "
    "monitor Mamute: mon [linha] (ver doc/mamute-spec.md)";

std::string DisasmLine(machine::Machine &m, uint16_t addr, uint16_t *length = nullptr) {
    const z80::debug::DisasmResult d = z80::debug::Disassemble([&](uint16_t a) { return m.ReadMemory(a); }, addr);
    if (length) *length = d.length;
    std::string bytes;
    for (uint16_t i = 0; i < 4; ++i) {
        bytes += i < d.length ? Hex(m.ReadMemory(static_cast<uint16_t>(addr + i)), 2) + " " : "   ";
    }
    return Hex(addr, 4) + "  " + bytes + " " + d.text;
}

// "pc=4010  LD A,(HL)" -- a proxima instrucao, usada nas respostas dos passos.
std::string PcLine(machine::Machine &m) {
    const uint16_t pc = m.cpu().state().pc.w;
    uint16_t len = 0;
    const z80::debug::DisasmResult d = z80::debug::Disassemble([&](uint16_t a) { return m.ReadMemory(a); }, pc);
    (void)len;
    return "pc=" + Hex(pc, 4) + "  " + d.text;
}

// Maximo de ciclos que um comando sincrono (next, until) espera antes de devolver "running": 20 quadros.
constexpr long kSyncCycles = 20L * machine::Machine::kFrameCycles;

} // namespace

bool Commander::ParseNumber(const std::string &text, uint32_t &value) {
    if (text.empty()) return false;
    std::string s = text;
    int base = 10;
    if (s[0] == '$') {
        base = 16;
        s.erase(0, 1);
    } else if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        base = 16;
        s.erase(0, 2);
    } else if (s.size() > 1 && (s.back() == 'h' || s.back() == 'H')) {
        base = 16;
        s.pop_back();
    }
    if (s.empty()) return false;
    char *end = nullptr;
    const unsigned long v = std::strtoul(s.c_str(), &end, base);
    if (*end != '\0' || v > 0xFFFFFFFFul) return false;
    value = static_cast<uint32_t>(v);
    return true;
}

std::vector<std::string> Commander::Tokenize(const std::string &line, std::string &error) {
    std::vector<std::string> tokens;
    std::string cur;
    bool in_token = false;
    bool in_quotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (in_quotes) {
            if (c == '"') {
                in_quotes = false;
            } else if (c == '\\' && i + 1 < line.size() && line[i + 1] == '"') {
                // So' \" e' escape: o resto fica literal, senao "C:\temp" viraria "C:<tab>emp".
                cur += '"';
                ++i;
            } else {
                cur += c;
            }
        } else if (c == '"') {
            in_quotes = true;
            in_token = true;
        } else if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            if (in_token) {
                tokens.push_back(cur);
                cur.clear();
                in_token = false;
            }
        } else {
            cur += c;
            in_token = true;
        }
    }
    if (in_quotes) {
        error = "aspas sem fechar";
        return {};
    }
    if (in_token) tokens.push_back(cur);
    return tokens;
}

// Nomes alternativos aceitos dentro de {}.
static std::string KeyAlias(std::string n) {
    for (char &c : n) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    static const char *const kAlias[][2] = {
        {"return", "enter"}, {"cr", "enter"}, {"escape", "esc"}, {"backspace", "bs"}, {"delete", "del"}, {"insert", "ins"},
        {"control", "ctrl"}, {"cls", "home"}, {"spc", "space"}, {"break", "stop"}, {"kana", "code"}, {"lshift", "shift"},
    };
    for (const auto &a : kAlias)
        if (n == a[0]) return a[1];
    return n;
}

bool Commander::ParseTypeSpec(machine::Machine &m, const std::string &spec, bool literal, std::vector<TypedKey> &out, std::string &error) {
    const size_t n = spec.size();
    bool pending_space = false; // espaco entre duas palavras soltas vira UM espaco (como era antes das aspas/chaves)
    bool last_bare = false;

    auto add_char = [&](char c) -> bool {
        TypedKey k;
        if (c == '\n') {
            k.keys = {"enter"};
        } else {
            std::string key;
            bool shift = false;
            if (!m.KeysFor(c, key, shift)) {
                error = std::string("type: o caractere '") + c + "' nao tem tecla no MSX";
                return false;
            }
            if (shift) k.keys.push_back("shift");
            k.keys.push_back(key);
        }
        out.push_back(k);
        return true;
    };
    // Barra-n vira ENTER e barra-barra vira uma barra; qualquer outra barra e' literal (senao C:\temp quebraria).
    auto escape_at = [&](size_t i, char &c) -> bool {
        if (spec[i] != '\\' || i + 1 >= n) return false;
        if (spec[i + 1] == 'n') c = '\n';
        else if (spec[i + 1] == '\\') c = '\\';
        else return false;
        return true;
    };

    for (size_t i = 0; i < n;) {
        const char c = spec[i];
        if (!literal && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
            if (last_bare) pending_space = true;
            ++i;
        } else if (!literal && c == '"') {
            ++i;
            bool closed = false;
            while (i < n) {
                char e = 0;
                if (spec[i] == '"') {
                    closed = true;
                    ++i;
                    break;
                }
                if (spec[i] == '\\' && i + 1 < n && spec[i + 1] == '"') {
                    if (!add_char('"')) return false;
                    i += 2;
                } else if (escape_at(i, e)) {
                    if (!add_char(e)) return false;
                    i += 2;
                } else {
                    if (!add_char(spec[i])) return false;
                    ++i;
                }
            }
            if (!closed) {
                error = "aspas sem fechar";
                return false;
            }
            pending_space = last_bare = false;
        } else if (c == '{') {
            // {tecla} ou {a}+{b}+{c} (juntas); {tecla*3} repete. {a}{b} sao duas digitacoes em sequencia.
            TypedKey combo;
            int repeat = 1;
            while (true) {
                const size_t close = spec.find('}', i);
                if (close == std::string::npos) {
                    error = "type: '{' sem '}'";
                    return false;
                }
                std::string body = spec.substr(i + 1, close - i - 1);
                const size_t star = body.rfind('*');
                if (star != std::string::npos && star + 1 < body.size() &&
                    body.find_first_not_of("0123456789", star + 1) == std::string::npos) {
                    repeat = std::max(1, std::min(200, std::atoi(body.c_str() + star + 1)));
                    body.erase(star);
                }
                std::vector<std::string> names;
                const std::string whole = KeyAlias(body);
                if (machine::Machine::IsKeyName(whole)) {
                    names.push_back(whole);
                } else {  // {ctrl+c} como atalho de {ctrl}+{c}
                    size_t b = 0;
                    while (b <= body.size()) {
                        const size_t plus = body.find('+', b);
                        names.push_back(KeyAlias(body.substr(b, plus == std::string::npos ? std::string::npos : plus - b)));
                        if (plus == std::string::npos) break;
                        b = plus + 1;
                    }
                }
                for (const std::string &name : names) {
                    if (!machine::Machine::IsKeyName(name)) {
                        error = "type: tecla desconhecida '{" + body + "}' (ex.: {enter} {esc} {f1} {stop} {ctrl}+{c})";
                        return false;
                    }
                    combo.keys.push_back(name);
                }
                i = close + 1;
                if (i + 1 < n && spec[i] == '+' && spec[i + 1] == '{') {
                    ++i;
                    continue;
                }
                break;
            }
            for (int r = 0; r < repeat; ++r) out.push_back(combo);
            pending_space = last_bare = false;
        } else {
            char e = 0;
            if (pending_space && !add_char(' ')) return false;
            pending_space = false;
            if (escape_at(i, e)) {
                if (!add_char(e)) return false;
                i += 2;
            } else {
                if (!add_char(c)) return false;
                ++i;
            }
            last_bare = true;
        }
    }
    return true;
}

Commander::Commander(Host &host) : host_(host) {}
Commander::~Commander() = default;

Result Commander::Execute(const std::string &line) {
    std::string tokenize_error;
    const std::vector<std::string> t = Tokenize(line, tokenize_error);
    if (!tokenize_error.empty()) return Err(tokenize_error);
    if (t.empty()) return Ok();

    const std::string cmd = Lower(t[0]);
    machine::Machine &m = host_.machine();

    if (cmd == "help") return Ok(kHelp);

    // mon <linha do Mamute>: o monitor Z80 (doc/mamute-spec.md). O texto depois da palavra vai CRU para ele (virgulas,
    // apostrofos e barras dos caminhos do Windows sao dele). `mon @prompt` devolve o prompt (quem mantem o modo do
    // monitor, como o console, mostra "MON(on)> " ou "MON(off)> "); `mon` sozinho mostra a abertura.
    if (cmd == "mon" || cmd == "mamute") {
        std::string rest = line;
        const size_t sp = rest.find_first_of(" 	");
        rest = sp == std::string::npos ? std::string() : rest.substr(sp + 1);
        while (!rest.empty() && std::isspace(static_cast<unsigned char>(rest.front()))) rest.erase(0, 1);
        const bool first = !mon_;
        if (first) mon_ = std::make_unique<mamute::Monitor>(mamute::LoadConfig(mamute::DefaultConfigPath()), mamute::DefaultConfigPath());
        mamute::MachineMemory memory(m);
        mamute::MachineExec exec(m);
        mon_->AttachMachine(&memory, &exec);
        if (first) mon_->ApplyStartMode();
        std::string out;
        if (rest == "@prompt") out = mon_->Prompt();
        else if (rest.empty()) out = mon_->Banner();
        else out = mon_->Execute(rest);
        mon_->DetachMachine();
        return Ok(out);
    }

    if (cmd == "version") {
        return Ok(std::string(FWMSX_NAME) + " " + std::to_string(FWMSX_VERSION_MAJOR) + "." + std::to_string(FWMSX_VERSION_MINOR) +
                  "." + std::to_string(FWMSX_VERSION_PATCH));
    }

    if (cmd == "status") {
        std::string s = "frame=" + std::to_string(m.frame_count()) + " paused=" + (host_.paused() ? "1" : "0") +
                        " stopped=" + (m.debugger().stopped() ? "1" : "0") +
                        " screen=" + std::to_string(m.vdp_state().scr_mode) + " pc=" + Hex(m.cpu().state().pc.w, 4) +
                        " typing=" + (typing() ? "1" : "0") + " cart=" + (m.cart_info().empty() ? "-" : "\"" + m.cart_info() + "\"");
        for (int d = 0; d < 2; ++d) {
            if (!m.has_disk_interface()) break;
            s += std::string(" disk") + static_cast<char>('A' + d) + "=" + (m.disk(d).loaded() ? "\"" + m.disk(d).path() + "\"" : "-");
        }
        s += std::string(" tape=") + (m.tape().inserted() ? "\"" + m.tape().path() + "\"" : "-");
        return Ok(s);
    }

    if (cmd == "reset") {
        m.Reset();
        return Ok();
    }
    if (cmd == "pause") {
        host_.set_paused(true);
        return Ok();
    }
    if (cmd == "resume") {
        host_.set_paused(false);
        return Ok();
    }

    if (cmd == "step") {
        uint32_t n = 1;
        if (t.size() > 1 && (!ParseNumber(t[1], n) || n == 0 || n > 3600)) return Err("step: use um numero de quadros entre 1 e 3600");
        if (!host_.paused()) return Err("step: so' com a maquina pausada (use pause antes)");
        for (uint32_t i = 0; i < n; ++i) {
            m.RunFrame();
            Tick();
        }
        return Ok("frame=" + std::to_string(m.frame_count()));
    }

    if (cmd == "type") {
        // O argumento e' lido da linha CRUA (nao dos tokens): precisa saber o que estava entre aspas e o que era {tecla}.
        size_t p = 0;
        while (p < line.size() && (line[p] == ' ' || line[p] == '\t')) ++p;
        p += 4; // "type"
        std::vector<TypedKey> keys;
        std::string error;
        if (!ParseTypeSpec(m, line.substr(std::min(p, line.size())), false, keys, error)) return Err(error);
        if (keys.empty()) return Err("type: informe o texto (\"texto\", {enter}, {ctrl}+{c}...)");
        for (const TypedKey &k : keys) typing_.push_back(k);
        return Ok(std::to_string(keys.size()) + " teclas");
    }

    if (cmd == "peek") {
        uint32_t addr = 0, count = 1;
        if (t.size() < 2 || !ParseNumber(t[1], addr) || addr > 0xFFFF) return Err("peek: use peek <endereco 0-FFFF> [quantidade]");
        if (t.size() > 2 && (!ParseNumber(t[2], count) || count == 0 || count > 4096)) return Err("peek: quantidade entre 1 e 4096");
        std::string out;
        for (uint32_t i = 0; i < count; ++i) {
            if (i) out += ' ';
            out += Hex(m.ReadMemory(static_cast<uint16_t>((addr + i) & 0xFFFF)), 2);
        }
        return Ok(out);
    }

    if (cmd == "poke") {
        uint32_t addr = 0;
        if (t.size() < 3 || !ParseNumber(t[1], addr) || addr > 0xFFFF) return Err("poke: use poke <endereco 0-FFFF> <valor> [valor...]");
        std::vector<uint8_t> values;
        for (size_t i = 2; i < t.size(); ++i) {
            uint32_t v = 0;
            if (!ParseNumber(t[i], v) || v > 0xFF) return Err("poke: valor invalido '" + t[i] + "' (0-FF)");
            values.push_back(static_cast<uint8_t>(v));
        }
        for (size_t i = 0; i < values.size(); ++i) m.WriteMemory(static_cast<uint16_t>((addr + i) & 0xFFFF), values[i]);
        return Ok();
    }

    if (cmd == "regs") {
        const Z80State &s = m.cpu().state();
        return Ok("AF=" + Hex(s.af.w, 4) + " BC=" + Hex(s.bc.w, 4) + " DE=" + Hex(s.de.w, 4) + " HL=" + Hex(s.hl.w, 4) +
                  " IX=" + Hex(s.ix.w, 4) + " IY=" + Hex(s.iy.w, 4) + " SP=" + Hex(s.sp.w, 4) + " PC=" + Hex(s.pc.w, 4));
    }

    if (cmd == "cart") {
        if (t.size() < 2) return Err("cart: use cart <arquivo|-> [mapper]");
        std::string error;
        const std::string path = (t[1] == "-") ? std::string() : t[1];
        if (!host_.LoadCartridge(path, t.size() > 2 ? t[2] : std::string(), error)) return Err(error);
        return Ok();
    }

    if (cmd == "disk" || cmd == "eject") {
        if (!m.has_disk_interface()) return Err("sem interface de disquete (inicie com --disk ou troque o layout)");
        const bool is_eject = cmd == "eject";
        if (t.size() != (is_eject ? 2u : 3u)) return Err(is_eject ? "eject: use eject <A|B>" : "disk: use disk <A|B> <arquivo>");
        const int drive = DriveIndex(t[1]);
        if (drive < 0) return Err("drive invalido (use A ou B)");
        if (is_eject) {
            m.EjectDisk(drive);
            return Ok();
        }
        std::string error;
        if (!m.InsertDisk(drive, t[2], error)) return Err(error);
        return Ok();
    }

    if (cmd == "tape") {
        if (t.size() != 2) return Err("tape: use tape <arquivo|eject|rewind>");
        if (t[1] == "eject") {
            m.EjectTape();
            return Ok();
        }
        if (t[1] == "rewind") {
            m.RewindTape();
            return Ok();
        }
        std::string error;
        if (!m.InsertTape(t[1], error)) return Err(error);
        return Ok();
    }

    if (cmd == "state") {
        if (t.size() != 3 || (t[1] != "save" && t[1] != "load")) return Err("state: use state <save|load> <arquivo>");
        std::string error;
        if (t[1] == "save") {
            if (!m.SaveState(t[2], error)) return Err(error);
            return Ok();
        }
        if (!m.LoadState(t[2], error)) return Err(error);
        return Ok(m.state_warning().empty() ? std::string() : "aviso: " + m.state_warning());
    }

    if (cmd == "screenshot") {
        if (t.size() != 2) return Err("screenshot: use screenshot <arquivo>");
        std::string error;
        if (!host_.SaveScreenshot(t[1], error)) return Err(error);
        return Ok(t[1]);
    }

    if (cmd == "quit") {
        host_.quit();
        return Ok();
    }

    // ---- depurador (doc/debug-spec.md) --------------------------------------------------------------
    dbg::Debugger &dbgr = m.debugger();
    auto need_addr = [&](size_t idx, uint16_t &out) {
        uint32_t v = 0;
        if (t.size() <= idx || !ParseNumber(t[idx], v) || v > 0xFFFF) return false;
        out = static_cast<uint16_t>(v);
        return true;
    };

    // dbgui open|show|key <tecla>|char <c>: o painel Z80 Debugger (src/debug/cpp/debug_view) como TEXTO, para a TUI e
    // para qualquer cliente. A resposta tem a linha "action=none|close|quit prompt=0|1 mode=..." e as 32 linhas do painel,
    // cada uma precedida de uma letra de estilo: T titulo, R registradores, C cursor, P pergunta, D ajuda, N normal.
    if (cmd == "dbgui") {
        const std::string sub = t.size() > 1 ? Lower(t[1]) : std::string("show");
        dbgui::Action act = dbgui::Action::None;
        if (sub == "open") {
            dbgview_.Enter(m);
        } else if (sub == "show") {
            // so' desenha
        } else if (sub == "char") {
            if (t.size() != 3 || t[2].size() != 1) return Err("dbgui char: use dbgui char <um caractere>");
            dbgview_.TypeChar(t[2][0]);
        } else if (sub == "key") {
            static const struct {
                const char *name;
                dbgui::Key key;
            } kKeys[] = {{"enter", dbgui::Key::Enter}, {"up", dbgui::Key::Up},       {"down", dbgui::Key::Down},   {"left", dbgui::Key::Left},
                         {"right", dbgui::Key::Right}, {"g", dbgui::Key::Go},         {"d", dbgui::Key::Disasm},    {"m", dbgui::Key::Memory},
                         {"s", dbgui::Key::Stack},     {"j", dbgui::Key::Jump},       {"r", dbgui::Key::RunTo},     {"c", dbgui::Key::Continue},
                         {"q", dbgui::Key::Quit},      {"b", dbgui::Key::Break},      {"esc", dbgui::Key::Escape},  {"bs", dbgui::Key::Backspace}};
            bool found = false;
            for (const auto &k : kKeys) {
                if (t.size() == 3 && Lower(t[2]) == k.name) {
                    act = dbgview_.Press(k.key, m);
                    found = true;
                    break;
                }
            }
            if (!found) return Err("dbgui key: enter up down left right g d m s j r c q b esc bs");
        } else {
            return Err("dbgui: use dbgui open|show|key <tecla>|char <c>");
        }
        if (act == dbgui::Action::QuitEmulator) host_.quit();
        // Esc/Close deixa a maquina PARADA (C e R ja' a retomaram): marca como parada "manual" para nada reabrir sozinho.
        if (act == dbgui::Action::Close && m.debugger().stopped()) m.debugger().Stop("manual");
        dbgui::Grid grid;
        dbgview_.Render(m, grid);
        const char *mode = dbgview_.mode() == dbgui::ViewMode::Memory ? "memory" : dbgview_.mode() == dbgui::ViewMode::Stack ? "stack" : "disasm";
        std::string out = std::string("action=") + (act == dbgui::Action::Close ? "close" : act == dbgui::Action::QuitEmulator ? "quit" : "none") +
                          " prompt=" + (dbgview_.prompt_active() ? "1" : "0") + " mode=" + mode;
        for (int y = 0; y < dbgui::kRows; ++y) {
            const uint8_t st = grid.at(5, y).style;
            const char tag = st == dbgui::kTitle ? 'T' : st == dbgui::kReg ? 'R' : st == dbgui::kCursor ? 'C' : st == dbgui::kPrompt ? 'P' : st == dbgui::kDim ? 'D' : 'N';
            out += '\n';
            out += tag;
            out += dbgui::GridRowText(grid, y);
        }
        return Ok(out);
    }

    if (cmd == "events") {
        // A assinatura e' feita pelo servidor TCP (por conexao); aqui so' valida a sintaxe.
        if (t.size() != 2 || (Lower(t[1]) != "on" && Lower(t[1]) != "off")) return Err("events: use events <on|off>");
        return Ok();
    }

    if (cmd == "break") {
        uint16_t a = 0;
        if (!need_addr(1, a)) return Err("break: use break <endereco 0-FFFF>");
        const int id = dbgr.AddBreak(a);
        return Ok("id=" + std::to_string(id) + " addr=" + Hex(a, 4));
    }
    if (cmd == "unbreak") {
        if (t.size() == 2 && Lower(t[1]) == "all") {
            dbgr.ClearBreaks();
            return Ok();
        }
        uint16_t a = 0;
        if (!need_addr(1, a)) return Err("unbreak: use unbreak <endereco|all>");
        return dbgr.RemoveBreak(a) ? Ok() : Err("nao ha breakpoint em " + Hex(a, 4));
    }
    if (cmd == "breaks") {
        if (dbgr.breaks().empty()) return Ok("sem breakpoints");
        std::string out;
        for (const dbg::Breakpoint &b : dbgr.breaks()) {
            if (!out.empty()) out += '\n';
            out += "#" + std::to_string(b.id) + "  " + Hex(b.addr, 4) + "  hits=" + std::to_string(b.hits);
        }
        return Ok(out);
    }

    if (cmd == "watch") {
        uint16_t a = 0;
        if (!need_addr(1, a)) return Err("watch: use watch <endereco> [tamanho] [r|w|rw]");
        uint32_t len = 1;
        bool rd = false, wr = true;
        for (size_t i = 2; i < t.size(); ++i) {
            const std::string o = Lower(t[i]);
            if (o == "r") { rd = true; wr = false; }
            else if (o == "w") { rd = false; wr = true; }
            else if (o == "rw" || o == "wr") { rd = true; wr = true; }
            else if (!ParseNumber(t[i], len) || len == 0 || len > 0xFFFF) return Err("watch: tamanho entre 1 e FFFF, ou r|w|rw");
        }
        const int id = dbgr.AddWatch(a, static_cast<uint16_t>(len), rd, wr);
        return Ok("id=" + std::to_string(id) + " addr=" + Hex(a, 4) + " len=" + std::to_string(len) + " " + (rd && wr ? "rw" : rd ? "r" : "w"));
    }
    if (cmd == "unwatch") {
        if (t.size() == 2 && Lower(t[1]) == "all") {
            dbgr.ClearWatches();
            return Ok();
        }
        uint16_t a = 0;
        if (!need_addr(1, a)) return Err("unwatch: use unwatch <endereco|all>");
        return dbgr.RemoveWatch(a) ? Ok() : Err("nao ha watchpoint em " + Hex(a, 4));
    }
    if (cmd == "watches") {
        if (dbgr.watches().empty()) return Ok("sem watchpoints");
        std::string out;
        for (const dbg::Watchpoint &w : dbgr.watches()) {
            if (!out.empty()) out += '\n';
            out += "#" + std::to_string(w.id) + "  " + Hex(w.addr, 4) + "  len=" + std::to_string(w.len) + "  " +
                   (w.on_read && w.on_write ? "rw" : w.on_read ? "r" : "w") + "  hits=" + std::to_string(w.hits);
        }
        return Ok(out);
    }

    // Os comandos que andam a maquina "na hora" (stepi, next, until) ja' devolvem o resultado na resposta:
    // o evento correspondente, se houve, e' descartado para o cliente nao ver duas vezes.
    if (cmd == "stepi") {
        uint32_t n = 1;
        if (t.size() > 1 && (!ParseNumber(t[1], n) || n == 0 || n > 1000000)) return Err("stepi: use um numero de instrucoes entre 1 e 1000000");
        const size_t mark = dbgr.EventCount();
        if (!dbgr.stopped()) dbgr.Stop("manual");
        std::string extra;
        for (uint32_t i = 0; i < n; ++i) {
            m.StepInstruction();
            if (dbgr.TakeWatchHit(m.cpu().state().pc.w)) {
                extra = "  (watch disparou)";
                break;
            }
        }
        dbgr.TruncateEvents(mark);
        return Ok(PcLine(m) + extra);
    }

    if (cmd == "next" || cmd == "until") {
        uint16_t target = 0;
        const uint16_t pc = m.cpu().state().pc.w;
        if (cmd == "until") {
            if (!need_addr(1, target)) return Err("until: use until <endereco>");
        } else {
            uint16_t len = 0;
            const z80::debug::DisasmResult d = z80::debug::Disassemble([&](uint16_t a) { return m.ReadMemory(a); }, pc);
            len = d.length;
            const uint8_t bytes[2] = {m.ReadMemory(pc), m.ReadMemory(static_cast<uint16_t>(pc + 1))};
            const int kind = dbg_flow_kind(bytes);
            target = static_cast<uint16_t>(pc + (kind == DBG_FLOW_HALT ? 1 : len));
            if (kind == DBG_FLOW_PLAIN) {
                // Instrucao comum: um passo simples, como stepi 1.
                const size_t mark = dbgr.EventCount();
                if (!dbgr.stopped()) dbgr.Stop("manual");
                m.StepInstruction();
                dbgr.TakeWatchHit(m.cpu().state().pc.w);
                dbgr.TruncateEvents(mark);
                return Ok(PcLine(m));
            }
        }
        const size_t mark = dbgr.EventCount();
        dbgr.SetUntil(target);
        dbgr.Resume();
        if (m.RunUntilStop(kSyncCycles)) {
            dbgr.TruncateEvents(mark);
            return Ok(PcLine(m) + "  (" + dbgr.last_stop() + ")");
        }
        return Ok("running"); // nao chegou em 20 quadros: segue rodando e o evento avisa quando parar
    }

    if (cmd == "go") {
        if (t.size() > 1) {
            uint16_t a = 0;
            if (!need_addr(1, a)) return Err("go: use go [endereco]");
            m.cpu().set_pc(a);
        }
        dbgr.Resume();
        return Ok("running");
    }
    if (cmd == "stop") {
        if (!dbgr.stopped()) dbgr.Stop("manual");
        return Ok(PcLine(m));
    }
    if (cmd == "jump") {
        uint16_t a = 0;
        if (!need_addr(1, a)) return Err("jump: use jump <endereco>");
        m.cpu().set_pc(a);
        return Ok(PcLine(m));
    }

    if (cmd == "dis") {
        uint16_t a = m.cpu().state().pc.w;
        uint32_t n = 8;
        if (t.size() > 1 && !need_addr(1, a)) return Err("dis: use dis [endereco] [linhas]");
        if (t.size() > 2 && (!ParseNumber(t[2], n) || n == 0 || n > 256)) return Err("dis: linhas entre 1 e 256");
        std::string out;
        for (uint32_t i = 0; i < n; ++i) {
            uint16_t len = 1;
            const std::string line = DisasmLine(m, a, &len);
            char mark = a == m.cpu().state().pc.w ? '>' : ' ';
            for (const dbg::Breakpoint &b : dbgr.breaks()) {
                if (b.addr == a) mark = '*';
            }
            if (!out.empty()) out += '\n';
            out += mark + line;
            a = static_cast<uint16_t>(a + len);
        }
        return Ok(out);
    }

    if (cmd == "mem") {
        uint16_t a = 0;
        uint32_t n = 64;
        if (!need_addr(1, a)) return Err("mem: use mem <endereco> [bytes]");
        if (t.size() > 2 && (!ParseNumber(t[2], n) || n == 0 || n > 1024)) return Err("mem: bytes entre 1 e 1024");
        std::string out;
        for (uint32_t row = 0; row < n; row += 16) {
            std::string hex, asc;
            for (uint32_t i = 0; i < 16; ++i) {
                if (row + i < n) {
                    const uint8_t v = m.ReadMemory(static_cast<uint16_t>(a + row + i));
                    hex += Hex(v, 2) + " ";
                    asc += (v >= 32 && v < 127) ? static_cast<char>(v) : '.';
                } else {
                    hex += "   ";
                }
            }
            if (!out.empty()) out += '\n';
            out += Hex(static_cast<uint16_t>(a + row), 4) + ": " + hex + "|" + asc + "|";
        }
        return Ok(out);
    }

    if (cmd == "stack") {
        uint32_t n = 8;
        if (t.size() > 1 && (!ParseNumber(t[1], n) || n == 0 || n > 128)) return Err("stack: palavras entre 1 e 128");
        const uint16_t sp = m.cpu().state().sp.w;
        std::string out;
        for (uint32_t i = 0; i < n; ++i) {
            const uint16_t a = static_cast<uint16_t>(sp + 2 * i);
            const unsigned v = m.ReadMemory(a) | (m.ReadMemory(static_cast<uint16_t>(a + 1)) << 8);
            if (!out.empty()) out += '\n';
            out += "SP+" + Hex(2 * i, 2) + "  " + Hex(a, 4) + ": " + Hex(v, 4);
        }
        return Ok(out);
    }

    if (cmd == "reg") {
        Z80State &s = m.cpu().state();
        if (t.size() == 1) {
            char flags[9];
            dbg_format_flags(s.af.b.lo, flags);
            const char *im = (s.iff & Z80_IFF_IM2) ? "2" : (s.iff & Z80_IFF_IM1) ? "1" : "0";
            return Ok("AF=" + Hex(s.af.w, 4) + " BC=" + Hex(s.bc.w, 4) + " DE=" + Hex(s.de.w, 4) + " HL=" + Hex(s.hl.w, 4) +
                      "  IX=" + Hex(s.ix.w, 4) + " IY=" + Hex(s.iy.w, 4) + " SP=" + Hex(s.sp.w, 4) + " PC=" + Hex(s.pc.w, 4) + "\n" +
                      "AF'=" + Hex(s.af_alt.w, 4) + " BC'=" + Hex(s.bc_alt.w, 4) + " DE'=" + Hex(s.de_alt.w, 4) + " HL'=" + Hex(s.hl_alt.w, 4) +
                      "  I=" + Hex(s.i, 2) + " R=" + Hex(s.r, 2) + "  IM" + im + " " + ((s.iff & Z80_IFF_1) ? "EI" : "DI") +
                      ((s.iff & Z80_IFF_HALT) ? " HALT" : "") + "\n" + "F=" + flags);
        }
        uint32_t v = 0;
        if (t.size() != 3 || !ParseNumber(t[2], v)) return Err("reg: use reg  (mostra tudo)  ou  reg <nome> <valor>");
        const std::string r = Lower(t[1]);
        struct Reg16 { const char *name; Z80Pair *p; };
        const Reg16 r16[] = {{"af", &s.af}, {"bc", &s.bc}, {"de", &s.de}, {"hl", &s.hl}, {"ix", &s.ix}, {"iy", &s.iy},
                             {"sp", &s.sp}, {"pc", &s.pc}, {"af'", &s.af_alt}, {"bc'", &s.bc_alt}, {"de'", &s.de_alt}, {"hl'", &s.hl_alt}};
        for (const Reg16 &x : r16) {
            if (r == x.name) {
                if (v > 0xFFFF) return Err("reg: valor de 16 bits (0-FFFF)");
                x.p->w = static_cast<uint16_t>(v);
                return Ok();
            }
        }
        struct Reg8 { const char *name; uint8_t *p; };
        const Reg8 r8[] = {{"a", &s.af.b.hi}, {"f", &s.af.b.lo}, {"b", &s.bc.b.hi}, {"c", &s.bc.b.lo}, {"d", &s.de.b.hi}, {"e", &s.de.b.lo},
                           {"h", &s.hl.b.hi}, {"l", &s.hl.b.lo}, {"i", &s.i}, {"r", &s.r}};
        for (const Reg8 &x : r8) {
            if (r == x.name) {
                if (v > 0xFF) return Err("reg: valor de 8 bits (0-FF)");
                *x.p = static_cast<uint8_t>(v);
                return Ok();
            }
        }
        return Err("reg: registrador desconhecido '" + t[1] + "'");
    }

    if (cmd == "slots") {
        const auto view = m.memory().CurrentView();
        std::string out;
        for (size_t p = 0; p < view.size(); ++p) {
            if (!out.empty()) out += '\n';
            out += "pagina " + std::to_string(p) + " (" + Hex(static_cast<unsigned>(p * 0x4000), 4) + "-" +
                   Hex(static_cast<unsigned>(p * 0x4000 + 0x3FFF), 4) + "): slot " + std::to_string(view[p].primary) + "-" +
                   std::to_string(view[p].secondary) + (view[p].writable ? " (RAM)" : " (ROM/vazio)");
        }
        return Ok(out);
    }

    return Err("comando desconhecido '" + t[0] + "' (use help)");
}

std::vector<std::string> Commander::TakeEvents() { return host_.machine().debugger().TakeEvents(); }

void Commander::Tick() {
    machine::Machine &m = host_.machine();
    if (key_phase_ == 0) {
        if (typing_.empty()) return;
        current_ = typing_.front();
        typing_.pop_front();
        for (const std::string &k : current_.keys) m.KeyDown(k);
        key_phase_ = 1;
        key_ticks_ = 3;
        return;
    }
    if (--key_ticks_ > 0) return;
    if (key_phase_ == 1) {
        for (auto it = current_.keys.rbegin(); it != current_.keys.rend(); ++it) m.KeyUp(*it);
        key_phase_ = 2;
        key_ticks_ = 3;
    } else {
        key_phase_ = 0;
    }
}

} // namespace control
