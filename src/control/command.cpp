// fwMSX -- despachante de comandos da ponte de controle. Ver command.h e doc/control-spec.md.
#include "command.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <filesystem>
#include <fstream>

#include "../asm/cpp/cli.h"
#include "../badig/cpp/badig.h"
#include "../obi/cpp/obi.h"
#include "../common/version.h"
#include "../debug/c/dbg_flow.h"
#include "../mamute/cpp/mamute_monitor.h"
#include "../mamute/cpp/mamute_online.h"
#include "../msxdisk/core/disk_image.h"
#include "../project/cpp/project.h"
#include "../script/script_engine.h"
#include "../tape/cpp/cas_format.h"
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
    "comandos: help version status reset pause resume step [n] type <texto|{tecla}> peek <end> [n] screen [info] (texto da tela / modo e tabelas) vdp [pal|n [v]] vdpbios n v (registradores do VDP) "
    "poke <end> <v>... regs cart <arq|-> [mapper] dos2 [rom] [disco]|off (MSX-DOS 2) mapper (RAM mapeada) disk <A|B> <arq> eject <A|B> tape <arq|eject|rewind|new|list|seek|protect|write|mode|save|put|load> "
    "state <save|load> <arq> screenshot <arq> quit | depurador: dbg break <end> unbreak <end|all> breaks "
    "watch <end> [tam] [r|w|rw] unwatch <end|all> watches stepi [n] next until <end> go [end] stop jump <end> "
    "dis [end] [n] mem <end> [n] stack [n] reg [nome valor] slots events <on|off> dbgui <open|show|key|char> (ver doc/debug-spec.md) | "
    "Basic Dignified: badig [opcoes] <fonte.dmx> (gera .amx e .bmx) badigrun <fonte.dmx> (converte, monta o disco e executa) | "
    "scripts: lua <codigo|arquivo.lua|stop|status> source <arquivo> (ver man lua) | "
    "disco por receita: obi <build|check> [Obifile] (monta o .dsk) obirun [Obifile] (monta, insere em A: e reinicia; ver man obi) | "
    "projeto: project <new|open|close|info|ls|add|get|rm|set|root|status|sync|pull|push|export|import> (ver man project) | "
    "montador e simbolos: asmrun [-z] [-n] [--dos-disk <arq>] <fonte.asm> sym [load <arq.sym>|clear|list [txt]|<nome|end>] src [n] markers <on|off> | "
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
    std::string where = m.debugger().symbols().Describe(pc);
    const std::string src = m.debugger().symbols().DescribeLine(pc);
    if (!src.empty()) where += (where.empty() ? "" : " ") + src;
    return "pc=" + Hex(pc, 4) + (where.empty() ? "" : " <" + where + ">") + "  " + d.text;
}

// Maximo de ciclos que um comando sincrono (next, until) espera antes de devolver "running": 20 quadros.
constexpr long kSyncCycles = 20L * machine::Machine::kFrameCycles;

// As entradas da fita na tabela de saltos da BIOS (0000-00FF), para o depurador: "break tapin", "until tapoon", "dis tapion".
bool TapeBiosAddress(const std::string &name, uint32_t &out) {
    static const struct {
        const char *name;
        uint32_t addr;
    } kNames[] = {{"tapion", 0x00E1}, {"tapin", 0x00E4}, {"tapiof", 0x00E7}, {"tapoon", 0x00EA}, {"tapout", 0x00ED}, {"tapoof", 0x00F0}};
    const std::string l = Lower(name);
    for (const auto &n : kNames)
        if (l == n.name) {
            out = n.addr;
            return true;
        }
    return false;
}

// ---- fita (doc/tape-spec.md, secao 12) -----------------------------------------------------------
const char *kTapeUsage =
    "tape: use tape [<arquivo>|insert <arquivo>|eject|rewind|new <arquivo>|list|seek <n|nome|->|protect <on|off>|"
    "write <append|overwrite|new>|mode <fast|normal>|save <arquivo>|put <nome> <ini> <fim> [exec]|load <n|nome> [fim]]";

Result TapeCommand(machine::Machine &m, const std::vector<std::string> &t) {
    tape::TapeEngine &tp = m.tape();
    if (t.size() == 1) return Ok(tape::DescribeDeck(tp));
    const std::string sub = Lower(t[1]);
    std::string error;

    if (t.size() == 2 && sub == "eject") {
        m.EjectTape();
        return Ok();
    }
    if (t.size() == 2 && sub == "rewind") {
        m.RewindTape();
        return Ok();
    }
    if (t.size() == 2 && sub == "list") return Ok(tape::ListDeck(tp));
    // qualquer outra palavra solta e' o caminho de uma fita (.cas, .tsx/.tzx, .wav); "insert" serve para um arquivo com nome de comando
    if (t.size() == 2 || (t.size() == 3 && sub == "insert")) {
        if (!m.InsertTape(t.back(), error)) return Err(error);
        return Ok(tape::DescribeDeck(tp));
    }
    if (sub == "new" && t.size() == 3) {
        if (!m.NewBlankTape(t[2], error)) return Err(error);
        return Ok(tape::DescribeDeck(tp));
    }
    if (sub == "seek" && t.size() == 3) {
        if (t[2] == "-") {
            tp.ClearMark();
            tp.Rewind();
            return Ok("ponto: inicio");
        }
        size_t index = 0;
        if (!tp.FindFile(t[2], 0, index)) return Err("tape seek: arquivo '" + t[2] + "' nao esta' na fita (tape list)");
        tp.SeekToFile(index);
        return Ok("ponto: " + std::to_string(index) + " " + tp.files()[index].name);
    }
    if (sub == "protect" && t.size() == 3) {
        const std::string v = Lower(t[2]);
        if (v != "on" && v != "off") return Err("tape protect: use on ou off");
        tp.SetReadOnly(v == "on");
        return Ok(v == "on" ? "fita protegida" : "fita gravavel");
    }
    if (sub == "write" && t.size() == 3) {
        const std::string v = Lower(t[2]);
        if (v == "append") tp.SetWriteMode(tape::TapeWriteMode::AppendAtEnd);
        else if (v == "overwrite") tp.SetWriteMode(tape::TapeWriteMode::OverwriteAtPoint);
        else if (v == "new") tp.SetWriteMode(tape::TapeWriteMode::NewTape);
        else return Err("tape write: use append, overwrite ou new");
        return Ok("gravacao: " + v);
    }
    if (sub == "mode" && t.size() == 3) {
        const std::string v = Lower(t[2]);
        if (v != "fast" && v != "normal" && v != "rapido") return Err("tape mode: use fast ou normal");
        m.SetTapeMode(v == "normal" ? tape::TapeMode::Normal : tape::TapeMode::Fast);
        return Ok(std::string("carregamento: ") + (v == "normal" ? "normal (pulsos de verdade)" : "rapido (gancho de BIOS)"));
    }
    if (sub == "save" && t.size() == 3) {
        if (!tp.SaveAs(t[2], error)) return Err(error);
        return Ok("fita gravada em " + t[2]);
    }
    if (sub == "put" && t.size() >= 5 && t.size() <= 6) {
        uint32_t a = 0, b = 0, e = 0;
        if (!Commander::ParseNumber(t[3], a) || !Commander::ParseNumber(t[4], b) || a > 0xFFFF || b > 0xFFFF || b < a) return Err("tape put: enderecos invalidos");
        e = a;
        if (t.size() == 6 && (!Commander::ParseNumber(t[5], e) || e > 0xFFFF)) return Err("tape put: exec invalido");
        std::vector<uint8_t> program;
        for (uint32_t x = a; x <= b; ++x) program.push_back(m.ReadMemory(static_cast<uint16_t>(x)));
        if (!tp.AppendFile(tape::kCasIdBinary, t[2], tape::MakeBinaryContent(static_cast<uint16_t>(a), static_cast<uint16_t>(e), program), error)) return Err(error);
        return Ok("gravado '" + t[2] + "' " + Hex(a, 4) + "-" + Hex(b, 4) + " exec " + Hex(e, 4));
    }
    if (sub == "load" && (t.size() == 3 || t.size() == 4)) {
        size_t index = 0;
        if (!tp.FindFile(t[2], 0, index)) return Err("tape load: arquivo '" + t[2] + "' nao esta' na fita (tape list)");
        tape::TapeFileData d;
        if (!tp.ReadFile(index, d, error)) return Err(error);
        if (d.type != tape::TapeFileType::Binary) return Err("tape load: so' carrega arquivo binario (bin); '" + d.name + "' e' " + tape::FileTypeName(d.type));
        for (uint32_t i = 6; i < d.data.size(); ++i) m.WriteMemory(static_cast<uint16_t>(d.bin_start + i - 6), d.data[i]);
        return Ok("carregado '" + d.name + "' " + Hex(d.bin_start, 4) + "-" + Hex(d.bin_end, 4) + " exec " + Hex(d.bin_exec, 4));
    }
    return Err(kTapeUsage);
}

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

// A tela de texto como o BIOS a mantem: SCRMOD (FCAF) diz o modo; a tabela de nomes esta' em TXTNAM (F3B3, SCREEN 0) ou T32NAM (F3BD, SCREEN 1);
// LINLEN (F3B0) e CRTCNT (F3B1) dao as colunas e as linhas. O texto sai da VRAM, entao vale mesmo com a maquina parada.
std::string ScreenText(machine::Machine &m) {
    const uint8_t mode = m.ReadMemory(0xFCAF);
    if (mode > 1) return std::string();
    const uint16_t base_addr = mode == 0 ? 0xF3B3 : 0xF3BD;
    const uint32_t base = m.ReadMemory(base_addr) | (m.ReadMemory(static_cast<uint16_t>(base_addr + 1)) << 8);
    const int cols = m.ReadMemory(0xF3B0);
    const int rows = m.ReadMemory(0xF3B1);
    if (cols < 1 || cols > 80 || rows < 1 || rows > 27) return std::string();
    const VdpState &v = m.vdp_state();
    std::string out;
    for (int r = 0; r < rows; ++r) {
        std::string l;
        for (int c = 0; c < cols; ++c) {
            const uint32_t a = base + static_cast<uint32_t>(r * cols + c);
            const uint8_t ch = a < sizeof(v.vram) ? v.vram[a] : 0;
            l.push_back(ch >= 32 && ch < 127 ? static_cast<char>(ch) : ' ');
        }
        while (!l.empty() && l.back() == ' ') l.pop_back();
        out += l;
        if (r + 1 < rows) out += '\n';
    }
    return out;
}

namespace {

const char *ScreenName(int mode) {
    switch (mode) {
        case 0: return "SCREEN 0 (texto 40x24)";
        case 1: return "SCREEN 1 (grafico 1, 32x24)";
        case 2: return "SCREEN 2 (grafico 2, 256x192)";
        case 3: return "SCREEN 3 (multicor, 64x48)";
        case 4: return "SCREEN 4 (grafico 3, 256x192)";
        case 5: return "SCREEN 5 (bitmap 256x212, 16 cores)";
        case 6: return "SCREEN 6 (bitmap 512x212, 4 cores)";
        case 7: return "SCREEN 7 (bitmap 512x212, 16 cores)";
        case 8: return "SCREEN 8 (bitmap 256x212, 256 cores)";
        case 10: return "SCREEN 10 (bitmap YJK/RGB)";
        case 11: return "SCREEN 11 (bitmap YJK/RGB)";
        case 12: return "SCREEN 12 (bitmap YJK)";
        case 13: return "SCREEN 0 (texto 80x24)";
        default: return "sem modo de tela";
    }
}

// uma linha da lista de tabelas: o nome, o endereco na VRAM e (quando e' fixo naquele modo) o tamanho
void TableLine(std::string &out, const char *name, uint32_t addr, int size) {
    char line[96];
    if (size > 0) std::snprintf(line, sizeof(line), "  %-22s %05X  (%d bytes)\n", name, static_cast<unsigned>(addr), size);
    else std::snprintf(line, sizeof(line), "  %-22s %05X\n", name, static_cast<unsigned>(addr));
    out += line;
}

// O modo de tela e onde ficam as tabelas na VRAM (R#2..R#6, R#10, R#11 ja' calculados pelo VDP): o que o programa precisa para achar
// os padroes, as cores e os sprites com `vpeek`/o monitor.
std::string ScreenInfo(machine::Machine &m) {
    const VdpState &v = m.vdp_state();
    const int mode = v.scr_mode;
    std::string out = std::string(ScreenName(mode)) + "  [VDP " + (v.model == VDP_MODEL_MSX1 ? "TMS9918" : v.model == VDP_MODEL_MSX2 ? "V9938" : "V9958") +
                      ", modo interno " + std::to_string(mode) + "]\n";
    if (mode > 13 || mode == 9) return out + "  (as tabelas nao se aplicam)";
    const bool text = mode == 0 || mode == 13;
    const bool bitmap = mode >= 5 && mode != 13;
    if (bitmap) {
        TableLine(out, "bitmap (pagina atual)", v.chr_tab, 0);
    } else {
        TableLine(out, "nomes (pattern name)", v.chr_tab, mode == 0 ? 960 : mode == 13 ? 1920 : 768);
        if (mode == 1) TableLine(out, "cores (color table)", v.col_tab, 32);
        if (mode == 2 || mode == 4) TableLine(out, "cores (color table)", v.col_tab, 6144);
        if (mode == 13) TableLine(out, "piscar (blink table)", v.col_tab, 240);
        TableLine(out, "padroes (pattern gen.)", v.chr_gen, mode == 2 || mode == 4 ? 6144 : mode == 3 ? 0 : 2048);
    }
    if (text || mode == 9) return out.substr(0, out.size() - 1);
    TableLine(out, "atributos de sprite", v.spr_tab, 128);
    if (mode >= 4) TableLine(out, "cores de sprite (modo 2)", v.spr_tab >= 512 ? v.spr_tab - 512 : 0, 512);
    TableLine(out, "padroes de sprite", v.spr_gen, 2048);
    return out.substr(0, out.size() - 1);
}

// Os 64 registradores em hexa (R#0..R#7 no MSX1, R#0..R#46 no MSX2), o status e uma linha de bits ja' decodificados.
std::string VdpDump(machine::Machine &m) {
    const VdpState &v = m.vdp_state();
    const bool v9938 = v.model != VDP_MODEL_MSX1;
    const int top = v9938 ? 46 : 7;
    std::string out = std::string(ScreenName(v.scr_mode)) + ", VDP " + (v.model == VDP_MODEL_MSX1 ? "TMS9918" : v.model == VDP_MODEL_MSX2 ? "V9938" : "V9958") + "\n";
    for (int base = 0; base <= top; base += 8) {
        char head[24];
        std::snprintf(head, sizeof(head), "R#%02d-%02d ", base, std::min(base + 7, top));
        out += head;
        for (int i = base; i <= std::min(base + 7, top); ++i) out += Hex(v.regs[i], 2) + " ";
        out.pop_back();
        out += "\n";
    }
    out += "status  ";
    for (int i = 0; i <= (v9938 ? 9 : 0); ++i) out += Hex(v.status[i], 2) + " ";
    out.pop_back();
    out += "\n";
    const uint8_t r0 = v.regs[0], r1 = v.regs[1], r8 = v.regs[8], r9 = v.regs[9];
    out += std::string("tela ") + ((r1 & 0x40) ? "ligada" : "apagada") + ", IE0(vblank) " + ((r1 & 0x20) ? "sim" : "nao") + ", IE1(hblank) " +
           ((r0 & 0x10) ? "sim" : "nao") + ", sprites " + ((r1 & 0x02) ? "16x16" : "8x8") + ((r1 & 0x01) ? " ampliados" : "");
    if (v9938) {
        out += std::string(", ") + ((r9 & 0x80) ? "212" : "192") + " linhas, " + ((r9 & 0x02) ? "PAL" : "NTSC") + ((r8 & 0x02) ? ", sprites desligados" : "") +
               ((r8 & 0x20) ? ", cor 0 opaca" : "");
    }
    return out;
}

std::string VdpPalette(machine::Machine &m) {
    const VdpState &v = m.vdp_state();
    if (v.model == VDP_MODEL_MSX1) return "a paleta do TMS9918 e' fixa (sem registrador de paleta)";
    std::string out;
    for (int i = 0; i < 16; ++i) {
        const int r = (v.palette_r[i] * 7 + 127) / 255, g = (v.palette_g[i] * 7 + 127) / 255, b = (v.palette_b[i] * 7 + 127) / 255;
        char line[64];
        std::snprintf(line, sizeof(line), "%2d: R%d G%d B%d  #%02X%02X%02X%s", i, r, g, b, v.palette_r[i], v.palette_g[i], v.palette_b[i], i < 15 ? "\n" : "");
        out += line;
    }
    return out;
}

// Os espelhos do BIOS dos registradores de escrita apenas: RG0SAV..RG7SAV (F3DF), RG8SAV..RG23SAV (FFE7), RG25SAV..RG27SAV (FFFA). 0 = sem espelho.
uint16_t VdpMirror(int reg) {
    if (reg >= 0 && reg <= 7) return static_cast<uint16_t>(0xF3DF + reg);
    if (reg >= 8 && reg <= 23) return static_cast<uint16_t>(0xFFE7 + reg - 8);
    if (reg >= 25 && reg <= 27) return static_cast<uint16_t>(0xFFFA + reg - 25);
    return 0;
}

} // namespace

Commander::Commander(Host &host) : host_(host) {}
Commander::~Commander() = default;

Result Commander::Execute(const std::string &line) {
    {   // lua <codigo>: o texto vai CRU para o Lua, antes de partir a linha em palavras (as aspas do codigo nao sao do console)
        size_t b = 0;
        while (b < line.size() && std::isspace(static_cast<unsigned char>(line[b]))) ++b;
        if (line.compare(b, 3, "lua") == 0 && (b + 3 == line.size() || std::isspace(static_cast<unsigned char>(line[b + 3])))) return LuaCmd(line.substr(b));
    }
    std::string tokenize_error;
    const std::vector<std::string> t = Tokenize(line, tokenize_error);
    if (!tokenize_error.empty()) return Err(tokenize_error);
    if (t.empty()) return Ok();

    const std::string cmd = Lower(t[0]);
    machine::Machine &m = host_.machine();

    if (cmd == "help") return Ok(kHelp);
    if (cmd == "source") return SourceCmd(t, source_depth_);

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

    if (cmd == "vdp" || cmd == "vdpbios") {
        const bool bios = cmd == "vdpbios";
        const bool v9938 = m.vdp_state().model != VDP_MODEL_MSX1;
        const uint32_t top = v9938 ? 46 : 7;
        if (!bios && t.size() == 1) return Ok(VdpDump(m));
        if (!bios && t.size() == 2 && (t[1] == "pal" || t[1] == "paleta")) return Ok(VdpPalette(m));
        uint32_t reg = 0, value = 0;
        if (t.size() < 2 || t.size() > 3 || !ParseNumber(t[1], reg) || reg > top) {
            return Err(bios ? "vdpbios: use vdpbios <registrador 0-" + std::to_string(top) + "> <valor>"
                            : "vdp: use vdp | vdp pal | vdp <registrador 0-" + std::to_string(top) + "> [valor]");
        }
        if (t.size() == 2) {
            if (bios) return Err("vdpbios: use vdpbios <registrador> <valor>");
            return Ok("R#" + std::to_string(reg) + "=" + Hex(m.vdp_state().regs[reg], 2));
        }
        if (!ParseNumber(t[2], value) || value > 0xFF) return Err(cmd + ": valor invalido '" + t[2] + "' (0-FF)");
        m.WriteVdpRegister(static_cast<int>(reg), static_cast<uint8_t>(value));
        std::string out = "R#" + std::to_string(reg) + "=" + Hex(m.vdp_state().regs[reg], 2);
        if (bios) {
            const uint16_t mirror = VdpMirror(static_cast<int>(reg));
            if (mirror == 0) return Ok(out + " (esse registrador nao tem espelho no BIOS)");
            m.WriteMemory(mirror, static_cast<uint8_t>(value));
            out += " e o espelho " + Hex(mirror, 4) + " do BIOS";
        }
        return Ok(out);
    }

    if (cmd == "screen") {
        if (t.size() > 1) {
            if (t[1] != "info") return Err("screen: use screen | screen info");
            return Ok(ScreenInfo(m));
        }
        const std::string text = ScreenText(m);
        if (text.empty()) return Err("screen: a tela nao esta' em SCREEN 0 ou 1 (so' o texto e' lido)");
        return Ok(text);
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

    if (cmd == "dos2") {
        // dos2 [rom.rom] [disco.dsk] | dos2 off: o kernel do MSX-DOS 2 como cartucho no slot 1, a interface de disquete e o disco em A:
        std::string rom, disk;
        for (size_t i = 1; i < t.size(); ++i) {
            std::string ext = std::filesystem::path(t[i]).extension().string();
            for (char &c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (t[i] == "off" && t.size() == 2) {
                std::string error;
                if (!host_.BootDos2(std::string(), std::string(), error)) return Err("dos2: " + error);
                return Ok("MSX-DOS 2 desligado; maquina reiniciada");
            }
            if (ext == ".rom" || ext == ".bin") rom = t[i];
            else if (disk.empty()) disk = t[i];
            else return Err("dos2: use dos2 [rom.rom] [disco.dsk] | dos2 off");
        }
        std::string error;
        if (rom.empty()) rom = host_.FindDos2Rom(error);
        if (rom.empty()) return Err("dos2: " + error);
        if (!host_.BootDos2(rom, disk, error)) return Err("dos2: " + error);
        return Ok("MSX-DOS 2: " + rom + " no slot 1" + (disk.empty() ? std::string() : "; A: " + disk) + "; maquina reiniciada");
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

    if (cmd == "tape") return TapeCommand(m, t);

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
        if (t.size() > idx && TapeBiosAddress(t[idx], v)) { // break tapin, until tapoon...: as 6 entradas da fita na BIOS
            out = static_cast<uint16_t>(v);
            return true;
        }
        if (t.size() > idx && ParseNumber(t[idx], v) && v <= 0xFFFF) {
            out = static_cast<uint16_t>(v);
            return true;
        }
        if (t.size() <= idx) return false;
        // um rotulo do fonte ("loop", "loop+4") ou uma linha dele ("jogo.asm:42"): ver `sym` e `asmrun`
        if (dbgr.symbols().Resolve(t[idx], out)) return true;
        const size_t colon = t[idx].rfind(':');
        if (colon != std::string::npos && colon > 0) {
            uint32_t ln = 0;
            if (ParseNumber(t[idx].substr(colon + 1), ln) && dbgr.symbols().AddressOfLine(t[idx].substr(0, colon), static_cast<int>(ln), out)) return true;
        }
        return false;
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

    // ---- simbolos, linhas do fonte, marcadores e o montar-e-rodar (doc/debug-spec.md, secao 8) -----------------------
    if (cmd == "markers") {
        if (t.size() == 1) return Ok(std::string("marcadores .break/.debug: ") + (dbgr.markers() ? "ligados" : "desligados"));
        const std::string v = Lower(t[1]);
        if (t.size() != 2 || (v != "on" && v != "off")) return Err("markers: use markers <on|off>");
        dbgr.set_markers(v == "on");
        return Ok(std::string("marcadores .break/.debug: ") + (v == "on" ? "ligados" : "desligados"));
    }

    if (cmd == "sym") {
        dbg::Symbols &sy = dbgr.symbols();
        if (t.size() == 1) {
            return Ok(std::to_string(sy.symbol_count()) + " simbolos, " + std::to_string(sy.line_count()) + " linhas de fonte, marcadores " + (dbgr.markers() ? "ligados" : "desligados"));
        }
        const std::string sub = Lower(t[1]);
        if (sub == "clear" && t.size() == 2) {
            sy.Clear();
            return Ok();
        }
        if (sub == "load" && t.size() == 3) {
            std::ifstream in(t[2], std::ios::binary);
            if (!in) return Err("sym load: nao foi possivel abrir '" + t[2] + "'");
            const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            const size_t n = sy.ParseSym(text);
            return Ok(std::to_string(n) + " simbolos carregados de " + t[2]);
        }
        if (sub == "list" && t.size() <= 3) {
            std::vector<dbg::Symbol> all = sy.symbols();
            std::stable_sort(all.begin(), all.end(), [](const dbg::Symbol &a, const dbg::Symbol &b) { return a.addr < b.addr; });
            const std::string filter = t.size() == 3 ? Lower(t[2]) : std::string();
            std::string out;
            int shown = 0;
            for (const dbg::Symbol &s : all) {
                if (!filter.empty() && Lower(s.name).find(filter) == std::string::npos) continue;
                if (++shown > 200) {
                    out += "\n... (mais; use um filtro: sym list <texto>)";
                    break;
                }
                if (!out.empty()) out += '\n';
                out += Hex(s.addr, 4) + "  " + s.name + (s.is_label ? "" : "  (constante)");
            }
            return Ok(out.empty() ? "nenhum simbolo" : out);
        }
        if (t.size() == 2) { // sym <nome|endereco>: nome -> endereco, endereco -> rotulo mais proximo
            uint16_t a = 0;
            uint32_t v = 0;
            if (ParseNumber(t[1], v) && v <= 0xFFFF) {
                const std::string d = sy.Describe(static_cast<uint16_t>(v), 0xFFFF);
                return d.empty() ? Err("nenhum simbolo abaixo de " + Hex(v, 4)) : Ok(Hex(v, 4) + " = " + d);
            }
            if (sy.Resolve(t[1], a)) return Ok(t[1] + " = " + Hex(a, 4));
            return Err("sym: '" + t[1] + "' nao e' um simbolo (sym list)");
        }
        return Err("sym: use sym [load <arq.sym>|clear|list [texto]|<nome|endereco>]");
    }

    if (cmd == "src") {
        uint32_t around = 4;
        uint16_t a = m.cpu().state().pc.w;
        if (t.size() > 1 && !ParseNumber(t[1], around)) return Err("src: use src [linhas em volta]");
        if (around > 40) around = 40;
        std::string file;
        int line = 0;
        if (!dbgr.symbols().LineAt(a, file, line)) return Err("sem linha de fonte para " + Hex(a, 4) + " (monte com asmrun)");
        std::ifstream in(file);
        if (!in) return Err("src: nao foi possivel abrir '" + file + "'");
        std::string out = file + ":" + std::to_string(line) + "  " + Hex(a, 4);
        std::string text;
        int n = 0;
        while (std::getline(in, text)) {
            ++n;
            if (n + static_cast<int>(around) < line) continue;
            if (n > line + static_cast<int>(around)) break;
            if (!text.empty() && text.back() == '\r') text.pop_back();
            char num[16];
            std::snprintf(num, sizeof(num), "%5d", n);
            out += std::string("\n") + (n == line ? ">" : " ") + num + "  " + text;
        }
        return Ok(out);
    }

    if (cmd == "asmrun") return AsmRun(t);
    if (cmd == "badig") return BadigCmd(t, false);
    if (cmd == "badigrun") return BadigCmd(t, true);
    if (cmd == "obi") return ObiCmd(t, false);
    if (cmd == "obirun") return ObiCmd(t, true);
    if (cmd == "project") return ProjectCmd(t);

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
            out += "#" + std::to_string(b.id) + "  " + Hex(b.addr, 4) + "  hits=" + std::to_string(b.hits) + dbgr.Where(b.addr);
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
            std::string label;
            if (dbgr.symbols().LabelAt(a, label)) {
                if (!out.empty()) out += '\n';
                out += label + ":";
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

    if (cmd == "mapper") {
        // Os mappers de RAM (portas FCh-FFh): tamanho, segmentos e o que cada pagina enxerga agora. Util com dois mappers: cada um mascara o valor
        // escrito na porta pelo proprio tamanho, entao os registradores podem diferir.
        if (m.mapper_count() == 0) return Ok("sem mapper de RAM nesta maquina");
        std::string out;
        for (size_t i = 0; i < m.mapper_count(); ++i) {
            const memmap::RamMapperDevice *d = m.mapper_at(i);
            const int segs = m.memory().MapperSegments(d->primary(), d->secondary());
            if (!out.empty()) out += '\n';
            out += "slot " + std::to_string(d->primary()) + ":" + std::to_string(d->secondary()) + "  " + std::to_string(segs * 16) + " KB (" +
                   std::to_string(segs) + " segmentos)  paginas 0-3 (FCh-FFh):";
            for (int page = 0; page < 4; ++page) out += " " + std::to_string(d->segment(page));
        }
        return Ok(out);
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

// asmrun [-z] [-n] [--dos-disk <arq.dsk>] <fonte.asm>
Result Commander::AsmRun(const std::vector<std::string> &t) {
    bool run = true;
    fwasm::AsmOptions opts;
    opts.want_listing = true; // o mapa endereco -> arquivo:linha do depurador
    opts.want_tape = true;    // um programa .BASIC carrega por uma fita montada na hora
    std::string source, dos_disk;
    for (size_t i = 1; i < t.size(); ++i) {
        const std::string &a = t[i];
        if (a == "-z") opts.zilog = true;
        else if (a == "-n" || a == "--no-run") run = false;
        else if (a == "--dos-disk" && i + 1 < t.size()) dos_disk = t[++i];
        else if (!a.empty() && a[0] == '-') return Err("asmrun: opcao desconhecida '" + a + "'");
        else if (source.empty()) source = a;
        else return Err("asmrun: so' um fonte por vez");
    }
    if (source.empty()) return Err("asmrun: use asmrun [-z] [-n] [--dos-disk <arq.dsk>] <fonte.asm>   (-n = so' montar, sem carregar)");

    fwasm::AsmResult r;
    std::string dir, base, error;
    if (!fwasm::AssembleFile(source, opts, r, dir, base, error)) return Err("asmrun: " + error);
    std::string out;
    for (const std::string &msg : r.msgs) out += msg + "\n";
    if (r.rc != 0) {
        while (!out.empty() && out.back() == '\n') out.pop_back();
        return Err(out.empty() ? "asmrun: a montagem falhou (codigo " + std::to_string(r.rc) + ")" : out);
    }

    // O .cas que o proprio fonte pediu (.cas "NOME") sai ao lado dele; o que so' montamos para carregar um programa BASIC vai para a pasta temporaria.
    bool tape_requested = false;
    for (const std::string &msg : r.msgs) tape_requested = tape_requested || msg.rfind("cas file ", 0) == 0;
    std::vector<std::string> skip;
    if (!tape_requested) skip.push_back("cas");
    std::vector<std::string> written;
    if (!fwasm::SaveOutputs(dir, base, r, skip, written, error)) return Err("asmrun: " + error);
    for (const std::string &w : written) out += "gerado: " + w + "\n";

    auto find_out = [&](const char *ext, std::string &path) {
        for (const auto &kv : r.outs) {
            const std::string &k = kv.first;
            if (k == ext) {
                path = base + "." + ext;
                return true;
            }
            if (k.size() > std::strlen(ext) + 1 && k.compare(k.size() - std::strlen(ext) - 1, std::string::npos, std::string(".") + ext) == 0) {
                path = dir + k;
                return true;
            }
        }
        return false;
    };
    auto pause_frames = [&](int entries) {
        for (int i = 0; i < entries; ++i) typing_.push_back(TypedKey{}); // cada entrada vazia espera 6 quadros: a BIOS precisa subir antes de digitar
    };
    const std::string stem = std::filesystem::path(base).filename().string();

    if (run) {
        if (r.format == "rom" || r.format == "megarom") {
            std::string rom;
            if (!find_out("rom", rom)) return Err("asmrun: o montador nao gerou o .rom");
            std::string mapper;
            if (r.format == "megarom") mapper = r.mapper == "konami" ? "konami4" : r.mapper == "konamiscc" ? "konami5" : r.mapper;
            if (!host_.LoadCartridge(rom, mapper, error)) return Err("asmrun: " + error);
            out += "cartucho " + rom + (mapper.empty() ? "" : " (mapper " + mapper + ")") + " inserido; maquina reiniciada\n";
        } else if (r.format == "basic") {
            std::string cas;
            if (tape_requested) {
                find_out("cas", cas);
            } else {
                cas = (std::filesystem::temp_directory_path() / (stem + "_asmrun.cas")).string();
                std::ofstream f(cas, std::ios::binary | std::ios::trunc);
                const std::vector<uint8_t> &bytes = r.outs.at("cas");
                f.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
                if (!f) return Err("asmrun: nao foi possivel gravar a fita temporaria '" + cas + "'");
            }
            machine::Machine &mm = host_.machine();
            if (!mm.InsertTape(cas, error)) return Err("asmrun: " + error);
            mm.Reset();
            pause_frames(60);
            const Result typed = Execute("type \"bload\\\"cas:\\\",r\" {enter}");
            if (!typed.ok) return Err("asmrun: " + typed.text);
            out += "fita " + cas + " inserida; maquina reiniciada; BLOAD\"CAS:\",R sera digitado sozinho\n";
        } else if (r.format == "msxdos") {
            machine::Machine &mm = host_.machine();
            if (!mm.has_disk_interface()) return Err("asmrun: este programa e' do MSX-DOS e a maquina nao tem interface de disquete (inicie o emulador com --disk)");
            std::string com;
            if (!find_out("com", com)) return Err("asmrun: o montador nao gerou o .com");
            std::string sys = dos_disk;
            if (sys.empty()) {
                for (const char *c : {"msxdos1.dsk", "dos/msxdos1.dsk", "disks/msxdos1.dsk"})
                    if (std::filesystem::exists(c)) {
                        sys = c;
                        break;
                    }
            }
            if (sys.empty()) return Err("asmrun: nao achei um disco de sistema do MSX-DOS (msxdos1.dsk); use --dos-disk <arquivo.dsk>");
            msxdisk::DiskError derr;
            std::optional<msxdisk::DiskImage> img = msxdisk::DiskImage::Load(sys, &derr);
            if (!img) return Err("asmrun: nao foi possivel abrir o disco de sistema '" + sys + "'");
            std::string name = stem;
            for (char &c : name) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            if (name.size() > 8) name.resize(8);
            const std::string msx_name = name + ".COM";
            img->DeleteFile(msx_name); // se ja' estava no disco de sistema, troca
            if (!img->AddFile(com, msx_name, 0x20, &derr)) return Err("asmrun: nao foi possivel copiar o .com para o disco");
            const std::string tmp = (std::filesystem::temp_directory_path() / (stem + "_asmrun.dsk")).string();
            if (!img->Save(tmp, &derr)) return Err("asmrun: nao foi possivel gravar o disco temporario '" + tmp + "'");
            if (!mm.InsertDisk(0, tmp, error)) return Err("asmrun: " + error);
            mm.Reset();
            // o MSX-DOS 1 pergunta a data na partida ("Enter date (M-D-Y):"): um ENTER a pula; depois digita o nome do programa
            pause_frames(70);
            Execute("type {enter}");
            pause_frames(25);
            const Result typed = Execute("type \"" + name + "\" {enter}");
            if (!typed.ok) return Err("asmrun: " + typed.text);
            out += "disco " + tmp + " (copia de " + sys + " com " + msx_name + ") em A:; maquina reiniciada; o ENTER da data e o nome " + name + " serao digitados sozinhos\n";
        } else {
            out += "formato '" + (r.format.empty() ? std::string("cru") : r.format) + "': montado, nada para carregar na maquina\n";
        }
    }

    // Simbolos e linhas do fonte: na maquina de AGORA (a troca do cartucho a recriou), para break/dis/src e as paradas.
    dbg::Debugger &dbgr = host_.machine().debugger();
    dbgr.symbols().Clear();
    for (const fwasm::SymbolRec &s : r.symbols) dbgr.symbols().Add(s.name, static_cast<uint16_t>(s.value & 0xFFFF), s.is_label);
    for (const fwasm::ListEntry &e : r.listing)
        if (!e.bytes.empty() && !e.is_const) dbgr.symbols().AddLine(static_cast<uint16_t>(e.addr & 0xFFFF), e.file, e.line);
    out += std::to_string(dbgr.symbols().symbol_count()) + " simbolos, " + std::to_string(dbgr.symbols().line_count()) + " linhas de fonte para o depurador";
    if (r.uses_markers) {
        dbgr.set_markers(true);
        out += "; o fonte tem .break/.debug: marcadores ligados";
    }
    return Ok(out);
}

// badig [opcoes do badig.py] <fonte.dmx> | badigrun <fonte.dmx>
Result Commander::BadigCmd(const std::vector<std::string> &t, bool run) {
    namespace fs = std::filesystem;
    const std::string name = run ? "badigrun" : "badig";
    std::vector<std::string> args(t.begin() + 1, t.end());
    if (args.empty()) return Err(name + ": use " + name + (run ? " <fonte.dmx>" : " [opcoes] <fonte.dmx> (ver fwmsx --badig)"));

    const badig::Options base = badig::LoadDefaults();
    badig::Result r = badig::Convert(base, args, nullptr, true);
    std::string out;
    for (const std::string &l : r.log) out += l + "\n";
    while (!out.empty() && out.back() == '\n') out.pop_back();
    if (!r.ok) return Err(out.empty() ? name + ": a conversao falhou" : out);
    if (!run) return Ok(out);

    if (r.bmx.empty()) return Err(name + ": o .bmx nao foi gerado (ligue a opcao de tokenizar)");
    machine::Machine &mm = host_.machine();
    if (!mm.has_disk_interface()) return Err(name + ": a maquina nao tem interface de disquete (inicie o emulador com --disk); o .bmx foi gerado: " + r.bmx_path);

    // o disco: so' o programa e o AUTOEXEC.BAS que o roda (o DISK BASIC executa o AUTOEXEC.BAS ao ligar)
    std::string stem = fs::path(r.bmx_path).stem().string();
    std::string msx;
    for (const char c : stem) {
        const unsigned char u = static_cast<unsigned char>(std::toupper(static_cast<unsigned char>(c)));
        msx.push_back((std::isalnum(u) || u == '_') ? static_cast<char>(u) : '_');
    }
    if (msx.empty()) msx = "PROGRAM";
    if (msx.size() > 8) msx.resize(8);

    const fs::path tmp = fs::temp_directory_path();
    const fs::path autoexec = tmp / (stem + "_autoexec.bas");
    {
        std::ofstream f(autoexec, std::ios::binary | std::ios::trunc);
        f << "10 RUN \"" << msx << ".BMX\"\r\n";
        if (!f) return Err(name + ": nao foi possivel gravar o AUTOEXEC.BAS temporario");
    }
    msxdisk::DiskError derr = msxdisk::DiskError::None;
    msxdisk::DiskImage img = msxdisk::DiskImage::CreateBlank();
    if (!img.AddFile(r.bmx_path, msx + ".BMX", 0x20, &derr)) return Err(name + ": nao foi possivel copiar o .bmx para o disco");
    if (!img.AddFile(autoexec, "AUTOEXEC.BAS", 0x20, &derr)) return Err(name + ": nao foi possivel copiar o AUTOEXEC.BAS para o disco");
    // arquivos de apoio (disk_files, NBASIC.BIN): o programa os acha em A: como na maquina de verdade
    std::vector<badig::DiskFile> extra;
    std::string cerr;
    if (!badig::CollectDiskFiles(r, extra, cerr)) return Err(name + ": " + cerr);
    std::string names;
    for (const badig::DiskFile &f : extra) {
        if (!img.AddFile(f.host, f.msx, 0x20, &derr)) {
            return Err(name + ": nao foi possivel copiar " + f.msx + " para o disco" + (derr == msxdisk::DiskError::DiskFull ? " (disco cheio)" : derr == msxdisk::DiskError::DirectoryFull ? " (diretorio cheio)" : ""));
        }
        names += " + " + f.msx;
    }
    const std::string dsk = (tmp / (stem + "_badig.dsk")).string();
    if (!img.Save(dsk, &derr)) return Err(name + ": nao foi possivel gravar o disco temporario '" + dsk + "'");
    std::string error;
    if (!mm.InsertDisk(0, dsk, error)) return Err(name + ": " + error);
    mm.Reset();
    out += (out.empty() ? "" : "\n") + std::string("disco ") + dsk + " (" + msx + ".BMX + AUTOEXEC.BAS" + names + ") em A:; maquina reiniciada: o DISK BASIC roda o programa sozinho";
    return Ok(out);
}

// obi <build|check> [Obifile] | obirun [Obifile]
Result Commander::ObiCmd(const std::vector<std::string> &t, bool run) {
    namespace fs = std::filesystem;
    const std::string name = run ? "obirun" : "obi";
    obi::BuildOptions o;
    size_t arg = 1;
    if (!run) {
        if (t.size() < 2 || (t[1] != "build" && t[1] != "check")) return Err("obi: use obi <build|check> [Obifile] (ver man obi)");
        o.write = t[1] == "build";
        arg = 2;
    }
    o.obifile = t.size() > arg ? t[arg] : obi::FindObifile("");
    if (o.obifile.empty()) return Err(name + ": nao achei um Obifile na pasta atual; informe o caminho");
    std::error_code ec;
    o.obifile = fs::absolute(o.obifile, ec).lexically_normal().string();
    const obi::BuildResult r = obi::Build(o);
    std::string out;
    for (const std::string &l : r.log) out += l + "\n";
    if (!r.ok) {
        while (!out.empty() && out.back() == '\n') out.pop_back();
        return Err(out.empty() ? name + ": a build falhou" : out);
    }
    for (const obi::BuiltFile &f : r.files) {
        char line[160];
        std::snprintf(line, sizeof(line), "  %-12s %7zu bytes  crc %08X  <- %s\n", f.name.c_str(), f.size, static_cast<unsigned>(f.crc), f.from.c_str());
        out += line;
    }
    out += (o.write ? r.dsk : std::string("(nada gravado)")) + ": " + std::to_string(r.files.size()) + " arquivos, " + std::to_string(r.free_bytes / 1024) + " KiB livres";
    if (!run) return Ok(out);

    std::string error;
    if (r.dos_version == 2) { // `dos: 2`: o disco so' tem os arquivos do sistema; o kernel vem do cartucho no slot 1 (como no MSX real com o MSX-DOS 2)
        const std::string rom = host_.FindDos2Rom(error);
        if (rom.empty()) return Err(name + ": " + error + "\n" + out);
        if (!host_.BootDos2(rom, r.dsk, error)) return Err(name + ": " + error + "\n" + out);
        return Ok(out + "\nMSX-DOS 2 (" + rom + ") no slot 1; disco em A:; maquina reiniciada");
    }
    machine::Machine &mm = host_.machine();
    if (!mm.has_disk_interface()) return Err(name + ": a maquina nao tem interface de disquete (inicie o emulador com --disk); o disco foi gerado: " + r.dsk + "\n" + out);
    if (!mm.InsertDisk(0, r.dsk, error)) return Err(name + ": " + error);
    mm.Reset();
    return Ok(out + "\ndisco em A:; maquina reiniciada");
}

script::Engine &Commander::Lua() {
    if (!lua_) {
        script::EngineHost h;
        h.machine = [this]() -> machine::Machine & { return host_.machine(); };
        h.command = [this](const std::string &line, std::string &text) {
            const Result r = Execute(line);
            text = r.text;
            return r.ok;
        };
        h.event = [this](const std::string &line) { host_.machine().debugger().PostEvent("! lua " + line); };
        lua_ = std::make_unique<script::Engine>(std::move(h));
    }
    return *lua_;
}

// lua <codigo> | lua <arquivo.lua> | lua stop | lua status. O texto depois da palavra vai CRU para o Lua (aspas, virgulas e barras sao dele).
Result Commander::LuaCmd(const std::string &line) {
    std::string rest = line;
    const size_t sp = rest.find_first_of(" 	");
    rest = sp == std::string::npos ? std::string() : rest.substr(sp + 1);
    while (!rest.empty() && std::isspace(static_cast<unsigned char>(rest.front()))) rest.erase(0, 1);
    while (!rest.empty() && std::isspace(static_cast<unsigned char>(rest.back()))) rest.pop_back();
    if (rest.empty()) return Err("lua: use lua <codigo> | lua <arquivo.lua> | lua stop | lua status (ver man lua)");
    if (rest == "stop") {
        if (lua_) lua_->Stop();
        return Ok();
    }
    if (rest == "status") return Ok(lua_ ? lua_->Status() : std::string("lua: nao iniciado"));
    std::string path = rest;
    if (path.size() >= 2 && path.front() == '"' && path.back() == '"') path = path.substr(1, path.size() - 2);
    std::error_code ec;
    const bool is_file = path.find('(') == std::string::npos && path.find(' ') == std::string::npos
                             ? (path.size() > 4 && Lower(path.substr(path.size() - 4)) == ".lua") || std::filesystem::is_regular_file(path, ec)
                             : (path.size() > 4 && Lower(path.substr(path.size() - 4)) == ".lua" && std::filesystem::is_regular_file(path, ec));
    std::string out;
    const bool ok = is_file ? Lua().RunFile(path, out) : Lua().RunString(rest, "=lua", out, true);
    while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) out.pop_back();
    return ok ? Ok(out) : Err(out);
}

// source <arquivo>: um comando do console por linha; linhas em branco e as que comecam com # ou -- sao comentarios. Para na 1a que falhar.
Result Commander::SourceCmd(const std::vector<std::string> &t, int depth) {
    if (t.size() != 2) return Err("source: use source <arquivo>");
    if (depth >= 8) return Err("source: arquivos demais um dentro do outro (limite 8)");
    std::ifstream f(t[1]);
    if (!f) return Err("source: nao foi possivel abrir " + t[1]);
    std::string out, line;
    int n = 0;
    ++source_depth_;
    while (std::getline(f, line)) {
        ++n;
        while (!line.empty() && (line.back() == '\r' || std::isspace(static_cast<unsigned char>(line.back())))) line.pop_back();
        size_t b = 0;
        while (b < line.size() && std::isspace(static_cast<unsigned char>(line[b]))) ++b;
        if (b == line.size() || line[b] == '#' || line.compare(b, 2, "--") == 0) continue;
        const Result r = Execute(line.substr(b));
        if (!r.ok) {
            --source_depth_;
            return Err(t[1] + ":" + std::to_string(n) + ": " + r.text);
        }
        if (!r.text.empty()) out += (out.empty() ? "" : "\n") + r.text;
    }
    --source_depth_;
    return Ok(out);
}

void Commander::Tick() {
    if (lua_) lua_->Tick();
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
