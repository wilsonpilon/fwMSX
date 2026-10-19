// fwMSX -- Mamute: os comandos de memoria, execucao e portas do SUPER-X (prefixo X) (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/mamute-spec.md, "Fase 4". Os de disco e de notas estao em mamute_superx_disk.cpp.
//
// No Mamute do Paleobasic cada um destes abria uma janela. Com paineis ligados (a TUI) o comando abre o painel; sem eles (console, scripts, o
// TCP) e' a versao de LINHA: mostra a tela uma vez, ou despeja a listagem.
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>

#include "../asm/mamute_cmp.h"
#include "../core/mamute_dump.h"
#include "../core/mamute_reloc.h"
#include "mamute_calc.h"
#include "mamute_monitor.h"
#include "mamute_util.h"

namespace mamute {

using util::Hex;
using util::Join;
using util::ParseHexN;
using util::Printable;
using util::SplitCommas;
using util::Trim;
using util::Upper;

namespace {

// 256 bytes quando o <fim> nao e' informado, sem passar do teto do alvo
uint32_t Default256(IMemory &mem, const SxAddr &s) {
    const uint32_t cap = TargetMax(mem, s.t);
    return std::min<uint32_t>(s.addr + 255, cap);
}

// O IExec so' tem a leitura da CPU; os registradores de 8 bits e os pares, por nome
struct RegRef {
    uint8_t *p8 = nullptr;
    Z80Pair *p16 = nullptr;
};

RegRef FindReg(Z80State &s, const std::string &name) {
    RegRef r;
    struct R8 { const char *n; uint8_t *p; };
    const R8 r8[] = {{"A", &s.af.b.hi}, {"F", &s.af.b.lo}, {"B", &s.bc.b.hi}, {"C", &s.bc.b.lo}, {"D", &s.de.b.hi}, {"E", &s.de.b.lo}, {"H", &s.hl.b.hi}, {"L", &s.hl.b.lo},
                     {"A'", &s.af_alt.b.hi}, {"F'", &s.af_alt.b.lo}, {"B'", &s.bc_alt.b.hi}, {"C'", &s.bc_alt.b.lo}, {"D'", &s.de_alt.b.hi}, {"E'", &s.de_alt.b.lo},
                     {"H'", &s.hl_alt.b.hi}, {"L'", &s.hl_alt.b.lo}, {"IXH", &s.ix.b.hi}, {"IXL", &s.ix.b.lo}, {"IYH", &s.iy.b.hi}, {"IYL", &s.iy.b.lo}};
    for (const R8 &x : r8)
        if (name == x.n) r.p8 = x.p;
    struct R16 { const char *n; Z80Pair *p; };
    const R16 r16[] = {{"AF", &s.af}, {"BC", &s.bc}, {"DE", &s.de}, {"HL", &s.hl}, {"AF'", &s.af_alt}, {"BC'", &s.bc_alt}, {"DE'", &s.de_alt}, {"HL'", &s.hl_alt},
                       {"IX", &s.ix}, {"IY", &s.iy}, {"SP", &s.sp}, {"PC", &s.pc}};
    for (const R16 &x : r16)
        if (name == x.n) r.p16 = x.p;
    return r;
}

const char *const kBpNames[5] = {"BP", "BP1", "BP2", "BP3", "BPF"};

} // namespace

// ---------------------------------------------------------------------------------------------------------------------
// o despachante: devolve `handled = false` quando `cmd` nao e' do SUPER-X
// ---------------------------------------------------------------------------------------------------------------------
std::string Monitor::CmdSuperX(const std::string &cmd, const std::string &rest, bool printer, bool &handled) {
    handled = true;
    if (cmd == "XD") return CmdXview('D', rest, printer);
    if (cmd == "XA") return CmdXview('A', rest, printer);
    if (cmd == "XI") return CmdXview('I', rest, printer);
    if (printer) { // "?" so' vale para os que geram listagem
        handled = false;
        return "";
    }
    if (cmd == "XM") return CmdXm(rest);
    if (cmd == "XH") return CmdXh(rest);
    if (cmd == "XBT") return CmdXbt(rest);
    if (cmd == "XRT") return CmdXrt(rest);
    if (cmd == "XFL") return CmdXfl(rest);
    if (cmd == "XCM") return CmdXcm(rest);
    if (cmd == "XFD") return CmdXfd(rest);
    if (cmd == "XCS") {
        sx_.checksum_addr = !sx_.checksum_addr;
        return std::string("CHECKSUM DO XD: ") + (sx_.checksum_addr ? "+ADDR (SOMA TAMBEM O BYTE BAIXO DO ENDERECO DA LINHA)" : "NORMAL (SO' OS BYTES)");
    }
    if (cmd == "XTS") return CmdXts(rest);
    if (cmd == "XCO") return CmdXco(rest);
    if (cmd == "XSD") return CmdXsd(rest);
    if (cmd == "XRG") return CmdXrg(rest);
    if (cmd == "XGO") return CmdXgo(rest);
    if (cmd == "XTR") return CmdXtr(rest);
    if (cmd == "XDK" || cmd == "XFS" || cmd == "XCI" || cmd == "XTP" || cmd == "XSV" || cmd == "XLD" || cmd == "XS#" || cmd == "XL#" || cmd == "XL%" || cmd == "XS%")
        return CmdXdisk(cmd, rest);
    if (cmd == "XIM" || cmd == "XIC" || cmd == "XIL" || cmd == "XIS" || cmd == "XIR") return CmdXnotes(cmd, rest);
    if (cmd == "XPP" || cmd == "XPI" || cmd == "XPO") return CmdXports(cmd, rest);
    handled = false;
    return "";
}

bool Monitor::ParseBlock(const std::string &start_tok, const std::string &end_tok, SxAddr &s, uint32_t &e) {
    if (!ParseSx(Mem(), sx_.vars, start_tok, s)) return false;
    return ParsePure(Mem(), s.t, end_tok, e);
}

// ---------------------------------------------------------------------------------------------------------------------
// @ -- as variaveis de debugger
// ---------------------------------------------------------------------------------------------------------------------
std::string Monitor::CmdVar(const std::string &line) {
    auto show = [&](char name) {
        const Vars::Slot *s = sx_.vars.Find(name);
        return std::string("@") + static_cast<char>(std::toupper(static_cast<unsigned char>(name))) + " = " + (s && s->has ? FormatSx(s->v) : std::string("----"));
    };
    if (line.size() == 1) {
        std::string out;
        for (const char n : {'0', '1', '2', '3', 'B', 'E', 'S'}) out += (out.empty() ? "" : "\n") + show(n);
        return out;
    }
    Vars::Slot *slot = sx_.vars.Find(line[1]);
    if (!slot) return "SINTAXE: @<0-3|B|E|S>[=<endereco>[#slot[-sub]|#V|#S]]";
    if (line.size() == 2 || std::isspace(static_cast<unsigned char>(line[2]))) return show(line[1]);
    SxAddr a;
    if (!ParseSx(Mem(), sx_.vars, line.substr(3), a)) return "SINTAXE: @<0-3|B|E|S>=<endereco>[#slot[-sub]|#V|#S]";
    slot->has = true;
    slot->v = a;
    return show(line[1]);
}

// ---------------------------------------------------------------------------------------------------------------------
// CL -- a calculadora
// ---------------------------------------------------------------------------------------------------------------------
std::string Monitor::CmdCl(const std::string &rest) {
    int v = 0;
    std::string err;
    if (Trim(rest).empty()) return "SINTAXE: CL <expressao>  (hexa por padrao; sufixos D B H O; + - * / % | & ^ ! e parenteses; @0..@3 @B @E @S)";
    if (!CalcEval(rest, &sx_.vars, &v, err)) return "ERRO: " + err;
    std::string bin;
    for (int b = 15; b >= 0; --b) bin += ((v >> b) & 1) ? '1' : '0';
    const int sv = v >= 0x8000 ? v - 0x10000 : v;
    return "HEX  : " + Hex(static_cast<unsigned>(v), 4) + "H\nBIN  : " + bin + "\nDEC+ : " + std::to_string(v) + "\nDEC+-: " + std::to_string(sv);
}

// ---------------------------------------------------------------------------------------------------------------------
// XD / XA / XI -- UM endereco abre a tela; dois ou tres campos despejam uma listagem
// ---------------------------------------------------------------------------------------------------------------------
std::string Monitor::CmdXview(char which, const std::string &rest, bool printer) {
    const char *name = which == 'D' ? "XD" : which == 'A' ? "XA" : "XI";
    IMemory &mem = Mem();
    const std::string usage = std::string("SINTAXE: ") + name + " [<end>[#slot[-sub]|#V|#S]]  |  " + name + " <ini>[,<fim>[,<arquivo>]]  (sem fim: 256 bytes)";
    LastView &last = which == 'D' ? sx_.xd : which == 'A' ? sx_.xa : sx_.xi;

    if (rest.find(',') != std::string::npos) {
        const std::vector<std::string> f = SplitCommas(rest);
        if (f.size() > 3) return usage;
        SxAddr s;
        if (!ParseSx(mem, sx_.vars, f[0], s)) return usage;
        uint32_t end = 0;
        std::string file;
        const std::string f3 = f.size() >= 3 ? f[2] : "";
        if (ParsePure(mem, s.t, f[1], end)) {
            if (end < s.addr || (f.size() == 3 && f3.empty())) return usage;
            file = f3;
        } else if (f[1].empty()) { // "XD 4000,,arq" ou "XD 4000,": o fim vazio vale 256 bytes
            end = Default256(mem, s);
            file = f3;
        } else if (f.size() == 2) { // "XD 4000,arq": o segundo campo nao e' endereco, e' o arquivo
            end = Default256(mem, s);
            file = f[1];
        } else {
            return usage;
        }
        const int digits = TargetDigits(s.t);
        std::vector<std::string> lines;
        std::string head = std::string(printer ? "?" : "") + name + " " + Hex(s.addr, digits) + "-" + Hex(end, digits) + TargetSuffix(s.t);
        if (which == 'D') {
            head += std::string(" CHECKSUM ") + (sx_.checksum_addr ? "+ADDR" : "NORMAL");
            for (uint32_t a = s.addr; a <= end; a += 8) {
                std::string hex, txt;
                signed char buf[8];
                int n = 0;
                for (uint32_t k = 0; k < 8 && a + k <= end; ++k) {
                    const uint8_t b = TRead(mem, s.t, a + k);
                    hex += Hex(b, 2) + " ";
                    txt += Printable(b);
                    buf[n++] = static_cast<signed char>(b);
                }
                const int sum = mamute_checksum8(buf, n, static_cast<int>(a & 0xFF), sx_.checksum_addr ? 1 : 0);
                lines.push_back(Hex(a, digits) + " " + Trim(hex) + " : " + Hex(static_cast<unsigned>(sum), 2) + " : " + txt);
            }
        } else if (which == 'A') {
            for (uint32_t a = s.addr; a <= end; a += 64) {
                std::string txt;
                for (uint32_t k = 0; k < 64 && a + k <= end; ++k) txt += Printable(TRead(mem, s.t, a + k));
                lines.push_back(Hex(a, digits) + ": " + txt);
            }
        } else {
            uint32_t a = s.addr;
            while (a <= end && lines.size() < 65536) {
                const DisasmLine d = DisasmAt(mem, s.t, a);
                lines.push_back(FormatDisasm(d, digits));
                a += static_cast<uint32_t>(d.len);
            }
        }
        if (printer) { // a "impressora": um PDF (ou texto, se o nome do arquivo nao termina em .pdf)
            return WriteListing(file.empty() ? std::string("mamute-") + (which == 'D' ? "xd" : which == 'A' ? "xa" : "xi") + ".pdf" : file, head, Join(lines));
        }
        if (!file.empty()) {
            if (which == 'I') return WriteListing(file, head, Join(lines)); // XI grava a LISTAGEM em texto
            std::ofstream o(file, std::ios::binary | std::ios::trunc);
            if (!o) return "NAO FOI POSSIVEL GRAVAR " + file;
            for (uint32_t a = s.addr; a <= end; ++a) o.put(static_cast<char>(TRead(mem, s.t, a)));
            return "SALVO \"" + file + "\" - " + Hex(s.addr, digits) + "-" + Hex(end, digits) + " - TAMANHO " + Hex(end - s.addr + 1, 4) + " (BYTES CRUS)";
        }
        return Join(lines);
    }

    if (printer) return std::string("A IMPRESSORA SO' FUNCIONA COM DOIS ENDERECOS (") + name + " <ini>,<fim>)";
    SxAddr a;
    int shift = 0;
    if (Trim(rest).empty()) {
        if (!last.has) return usage;
        a = last.a;
        shift = last.shift;
    } else if (!ParseSx(mem, sx_.vars, rest, a)) {
        return usage;
    }
    sx_.vars.StoreBase(a); // "commands store base address in variable 0", como no SUPER-X
    last.has = true;
    last.a = a;
    std::unique_ptr<Panel> p = which == 'D' ? MakeXdPanel(a, shift) : which == 'A' ? MakeXaPanel(a) : MakeXiPanel(a);
    if (panels_) return OpenPanel(std::move(p));
    p->ShowCursor(false);
    return Join(PlainLines(p->Render(mem)));
}

std::string Monitor::CmdXh(const std::string &rest) {
    SxAddr a;
    if (Trim(rest).empty()) {
        if (!sx_.xh.has) return "SINTAXE: XH [<end>[#slot[-sub]|#V|#S]]  (4 caracteres/sprites 8x8 consecutivos)";
        a = sx_.xh.a;
    } else if (!ParseSx(Mem(), sx_.vars, rest, a)) {
        return "SINTAXE: XH [<end>[#slot[-sub]|#V|#S]]  (4 caracteres/sprites 8x8 consecutivos)";
    }
    sx_.vars.StoreBase(a);
    sx_.xh.has = true;
    sx_.xh.a = a;
    std::unique_ptr<Panel> p = MakeXhPanel(a);
    if (panels_) return OpenPanel(std::move(p));
    p->ShowCursor(false);
    return Join(PlainLines(p->Render(Mem())));
}

// ---------------------------------------------------------------------------------------------------------------------
// XM -- monta instrucoes Z80 e grava dados direto na memoria, avancando o endereco sozinho
// ---------------------------------------------------------------------------------------------------------------------
std::string Monitor::CmdXm(const std::string &rest) {
    SxAddr a;
    if (Trim(rest).empty()) {
        if (!sx_.xm.has) return "SINTAXE: XM <end>[#slot[-sub]|#V|#S]";
        a = sx_.xm.a;
    } else if (!ParseSx(Mem(), sx_.vars, rest, a)) {
        return "SINTAXE: XM <end>[#slot[-sub]|#V|#S]";
    }
    sx_.vars.StoreBase(a);
    sx_.xm.has = true;
    sx_.xm.a = a;
    sx_.xm_cur = a;
    sx_.xm_active = true;
    return "SESSAO XM EM " + FormatSx(a) + ": digite uma instrucao Z80, dados (. 1 byte  : 2 bytes  ; numerico 2 bytes  [ numerico 1 byte  \" texto), um endereco[#alvo] ou I [n]; QUIT volta ao monitor";
}

std::string Monitor::XmExecute(const std::string &line_in) {
    IMemory &mem = Mem();
    const std::string line = Trim(line_in);
    if (line.empty()) return "";
    SxAddr &cur = sx_.xm_cur;
    const int digits = TargetDigits(cur.t);

    // "I [n]": lista as proximas n instrucoes (20 por padrao) a partir do endereco atual; so' leitura
    {
        size_t e = 0;
        while (e < line.size() && !std::isspace(static_cast<unsigned char>(line[e]))) ++e;
        if (Upper(line.substr(0, e)) == "I") {
            int n = 20;
            const std::string arg = Trim(line.substr(e));
            if (!arg.empty()) {
                std::string err;
                if (!CalcEval(arg, &sx_.vars, &n, err) || n < 1) return "SINTAXE: I [<quantas instrucoes>]";
                n = std::min(n, 1000);
            }
            std::vector<std::string> lines;
            uint32_t a = cur.addr;
            for (int k = 0; k < n; ++k) {
                const DisasmLine d = DisasmAt(mem, cur.t, a);
                lines.push_back(FormatDisasm(d, digits));
                a = TargetWrap(mem, cur.t, static_cast<int64_t>(a) + d.len);
            }
            return Join(lines);
        }
    }

    // grava `bytes` no endereco atual e avanca; devolve o eco "AAAA  bytes  texto"
    auto put = [&](const std::vector<uint8_t> &bytes, const std::string &text) {
        std::string b;
        unsigned ok = 0;
        for (size_t k = 0; k < bytes.size(); ++k) {
            b += (k ? " " : "") + Hex(bytes[k], 2);
            if (TWrite(mem, cur.t, TargetWrap(mem, cur.t, static_cast<int64_t>(cur.addr) + static_cast<int64_t>(k)), bytes[k])) ++ok;
        }
        while (b.size() < 11) b += ' ';
        std::string out = Hex(cur.addr, digits) + "  " + b + "  " + text;
        if (ok < bytes.size()) out += "   (" + std::to_string(bytes.size() - ok) + " IGNORADOS: NAO E' RAM)";
        cur.addr = TargetWrap(mem, cur.t, static_cast<int64_t>(cur.addr) + static_cast<int64_t>(bytes.size()));
        sx_.xm.a = cur;
        return out;
    };

    // dados: o sinal de tipo e os itens separados por virgula
    auto data = [&](const std::string &s) -> std::string {
        const char sign = s[0];
        const std::string body = s.substr(1);
        std::vector<uint8_t> bytes;
        if (sign == '"') { // o resto da linha inteiro vira bytes ASCII, sem precisar fechar as aspas
            for (const char c : body) bytes.push_back(static_cast<uint8_t>(c));
            if (bytes.empty()) return "SINTAXE: \"texto";
            return put(bytes, "\"" + body);
        }
        const bool word = sign == ':' || sign == ';';
        for (const std::string &item : SplitCommas(body)) {
            int v = 0;
            std::string err;
            if (item.empty() || !CalcEval(item, &sx_.vars, &v, err)) return "ERRO: " + (err.empty() ? std::string("ITEM VAZIO") : err) + " (" + item + ")";
            bytes.push_back(static_cast<uint8_t>(v & 0xFF));
            if (word) bytes.push_back(static_cast<uint8_t>((v >> 8) & 0xFF)); // 2 bytes, little-endian
        }
        if (bytes.empty()) return "SINTAXE: " + std::string(1, sign) + "<item>[,<item>...]";
        return put(bytes, std::string(1, sign) + Trim(body));
    };

    if (line[0] == '.' || line[0] == ':' || line[0] == ';' || line[0] == '[' || line[0] == '"') return data(line);

    // "endereco[#alvo]" sozinho, ou seguido de dados. Um endereco sem sufixo MANTEM o alvo atual (so' #S volta ao PAGE).
    {
        size_t e = 0;
        while (e < line.size() && !std::isspace(static_cast<unsigned char>(line[e]))) ++e;
        const std::string tok = line.substr(0, e);
        const std::string after = Trim(line.substr(e));
        const bool data_follows = !after.empty() && std::string(".:;[\"").find(after[0]) != std::string::npos;
        // DAA e CCF sao hexadecimais validos E mnemonicos sem operando: sozinhos, sao a instrucao
        const bool bare_mnemonic = after.empty() && (Upper(tok) == "DAA" || Upper(tok) == "CCF");
        if ((after.empty() || data_follows) && !bare_mnemonic) {
            SxAddr a;
            if (ParseSx(mem, sx_.vars, tok, a)) {
                if (tok.find('#') == std::string::npos && tok[0] != '@') a.t = cur.t; // sem sufixo, o alvo continua
                cur = a;
                sx_.xm.a = cur;
                sx_.vars.StoreBase(cur);
                return after.empty() ? "" : data(after);
            }
        }
    }

    // uma instrucao Z80: monta com o fwasm no endereco atual e grava
    Source src;
    src.Set(10, "ORG 0" + Hex(cur.addr & 0xFFFF, 4) + "H");
    src.Set(20, line);
    const AsmProduct p = src.Assemble(0);
    if (!p.ok) return "ERRO: " + p.error;
    std::vector<uint8_t> bytes;
    for (const auto &kv : p.image) bytes.push_back(kv.second);
    if (bytes.empty()) return "NADA A GRAVAR";
    return put(bytes, line);
}

// ---------------------------------------------------------------------------------------------------------------------
// XBT / XRT / XFL / XCM / XFD / XTS
// ---------------------------------------------------------------------------------------------------------------------
std::string Monitor::CmdXbt(const std::string &rest) {
    IMemory &mem = Mem();
    const std::vector<std::string> f = SplitCommas(rest);
    SxAddr s, d;
    uint32_t e = 0;
    char usage[160];
    std::snprintf(usage, sizeof(usage), "SINTAXE: XBT <ini>[#alvo],<fim>,<dest>[#alvo]  (origem e destino podem estar em slots/sub-slots/VRAM diferentes)");
    if (f.size() != 3 || !ParseBlock(f[0], f[1], s, e) || !ParseSx(mem, sx_.vars, f[2], d) || e < s.addr) return usage;
    const uint32_t n = e - s.addr + 1;
    if (static_cast<uint64_t>(d.addr) + n - 1 > TargetMax(mem, d.t)) return "O DESTINO PASSA DO FIM DO ALVO (NUNCA DA A VOLTA)";
    std::vector<uint8_t> buf(n);
    for (uint32_t k = 0; k < n; ++k) buf[k] = TRead(mem, s.t, s.addr + k); // le tudo antes de gravar: origem e destino podem se sobrepor
    uint32_t ok = 0;
    for (uint32_t k = 0; k < n; ++k)
        if (TWrite(mem, d.t, d.addr + k, buf[k])) ++ok;
    std::string out = "COPIADOS " + std::to_string(ok) + " BYTES DE " + FormatRange(s, e) + " PARA " + FormatSx(d);
    if (ok < n) out += " - " + std::to_string(n - ok) + " IGNORADOS (DESTINO NAO E' RAM)";
    return out;
}

std::string Monitor::CmdXrt(const std::string &rest) {
    IMemory &mem = Mem();
    const std::vector<std::string> f = SplitCommas(rest);
    SxAddr s, d;
    uint32_t e = 0;
    const char *usage = "SINTAXE: XRT <ini>[#alvo],<fim>,<dest>[#alvo]  (copia o programa ajustando os enderecos absolutos que apontam para dentro dele)";
    if (f.size() != 3 || !ParseBlock(f[0], f[1], s, e) || !ParseSx(mem, sx_.vars, f[2], d) || e < s.addr) return usage;
    const uint32_t n = e - s.addr + 1;
    if (static_cast<uint64_t>(d.addr) + n - 1 > TargetMax(mem, d.t)) return "O DESTINO PASSA DO FIM DO ALVO (NUNCA DA A VOLTA)";
    if (s.t.vram || d.t.vram) return "XRT NAO RELOCA VRAM (E' PARA PROGRAMAS Z80)";
    std::vector<uint8_t> buf(n);
    for (uint32_t k = 0; k < n; ++k) buf[k] = TRead(mem, s.t, s.addr + k);
    const int delta = static_cast<int>(d.addr) - static_cast<int>(s.addr);
    unsigned fixed_abs = 0, fixed_rel = 0, insns = 0;
    for (uint32_t i = 0; i < n;) {
        int abs_off = -1, rel_off = -1;
        const int len = mamute_insn_info(&buf[i], static_cast<int>(n - i), &abs_off, &rel_off);
        ++insns;
        if (i + static_cast<uint32_t>(len) > n) break; // a ultima instrucao saiu do bloco: nao mexe nela
        if (abs_off >= 0) { // JP/CALL/LD com endereco absoluto: so' o que aponta para DENTRO do bloco anda junto
            const unsigned v = buf[i + static_cast<uint32_t>(abs_off)] | (buf[i + static_cast<uint32_t>(abs_off) + 1] << 8);
            if (v >= s.addr && v <= e) {
                const unsigned nv = static_cast<unsigned>(static_cast<int>(v) + delta) & 0xFFFF;
                buf[i + static_cast<uint32_t>(abs_off)] = static_cast<uint8_t>(nv & 0xFF);
                buf[i + static_cast<uint32_t>(abs_off) + 1] = static_cast<uint8_t>(nv >> 8);
                ++fixed_abs;
            }
        }
        if (rel_off >= 0) { // JR/DJNZ: so' recalcula quando o alvo fica FORA do bloco (dentro, os dois andam juntos)
            const int disp = static_cast<int8_t>(buf[i + static_cast<uint32_t>(rel_off)]);
            const int target = static_cast<int>(s.addr + i) + len + disp;
            if (target < static_cast<int>(s.addr) || target > static_cast<int>(e)) {
                const int nd = target - (static_cast<int>(d.addr + i) + len);
                if (nd < -128 || nd > 127) return "RELOCACAO INVALIDA: O SALTO RELATIVO EM " + Hex(s.addr + i, 4) + " NAO ALCANCA " + Hex(static_cast<unsigned>(target) & 0xFFFF, 4) + " DEPOIS DO MOVIMENTO (NADA FOI GRAVADO)";
                buf[i + static_cast<uint32_t>(rel_off)] = static_cast<uint8_t>(nd);
                ++fixed_rel;
            }
        }
        i += static_cast<uint32_t>(len);
    }
    uint32_t ok = 0;
    for (uint32_t k = 0; k < n; ++k)
        if (TWrite(mem, d.t, d.addr + k, buf[k])) ++ok;
    std::string out = "RELOCADOS " + std::to_string(ok) + " BYTES DE " + FormatRange(s, e) + " PARA " + FormatSx(d) + " - " + std::to_string(fixed_abs) +
                      " ENDERECOS ABSOLUTOS E " + std::to_string(fixed_rel) + " SALTOS RELATIVOS AJUSTADOS EM " + std::to_string(insns) + " INSTRUCOES";
    if (ok < n) out += " - " + std::to_string(n - ok) + " IGNORADOS (DESTINO NAO E' RAM)";
    return out;
}

std::string Monitor::CmdXfl(const std::string &rest) {
    IMemory &mem = Mem();
    const std::vector<std::string> f = SplitCommas(rest);
    SxAddr s;
    uint32_t e = 0, v = 0;
    if (f.size() != 3 || !ParseBlock(f[0], f[1], s, e) || !ParseHexN(f[2], v, 2) || e < s.addr) return "SINTAXE: XFL <ini>[#alvo],<fim>,<byte>";
    uint32_t ok = 0;
    for (uint32_t a = s.addr; a <= e; ++a)
        if (TWrite(mem, s.t, a, static_cast<uint8_t>(v))) ++ok;
    const uint32_t n = e - s.addr + 1;
    std::string out = "PREENCHIDOS " + std::to_string(ok) + " BYTES (" + FormatRange(s, e) + ") COM " + Hex(v, 2);
    if (ok < n) out += " - " + std::to_string(n - ok) + " IGNORADOS (NAO E' RAM)";
    return out;
}

std::string Monitor::CmdXcm(const std::string &rest) {
    IMemory &mem = Mem();
    const std::vector<std::string> f = SplitCommas(rest);
    SxAddr s, c;
    uint32_t e = 0;
    const char *usage = "SINTAXE: XCM <ini>[#alvo],<fim>,<comp>[#alvo][,S]  (lista os bytes DIFERENTES; ,S lista os IGUAIS)";
    if ((f.size() != 3 && f.size() != 4) || !ParseBlock(f[0], f[1], s, e) || !ParseSx(mem, sx_.vars, f[2], c) || e < s.addr) return usage;
    bool equal = false;
    if (f.size() == 4) {
        if (Upper(f[3]) != "S") return usage;
        equal = true;
    }
    const uint32_t n = e - s.addr + 1;
    if (static_cast<uint64_t>(c.addr) + n - 1 > TargetMax(mem, c.t)) return "O BLOCO DE COMPARACAO PASSA DO FIM DO ALVO";
    std::vector<uint8_t> a(n), b(n);
    for (uint32_t k = 0; k < n; ++k) {
        a[k] = TRead(mem, s.t, s.addr + k);
        b[k] = TRead(mem, c.t, c.addr + k);
    }
    constexpr int kMaxLines = 4096;
    std::vector<int> idx(kMaxLines);
    const int total = mamute_diff(a.data(), b.data(), static_cast<int>(n), equal ? 1 : 0, idx.data(), kMaxLines); // Assembly: o laco quente
    std::vector<std::string> lines;
    const int da = TargetDigits(s.t), db = TargetDigits(c.t);
    for (int k = 0; k < total && k < kMaxLines; ++k) {
        const uint32_t i = static_cast<uint32_t>(idx[static_cast<size_t>(k)]);
        lines.push_back(Hex(s.addr + i, da) + ": " + Hex(a[i], 2) + (equal ? " == " : " <> ") + Hex(c.addr + i, db) + ": " + Hex(b[i], 2));
    }
    if (total > kMaxLines) lines.push_back("... (SO' AS PRIMEIRAS " + std::to_string(kMaxLines) + " LINHAS)");
    lines.push_back(std::to_string(total) + (equal ? " BYTES IGUAIS" : " DIFERENCAS") + " EM " + std::to_string(n) + " BYTES");
    return Join(lines);
}

std::string Monitor::CmdXfd(const std::string &rest) {
    IMemory &mem = Mem();
    // "ini,fim,instrucao": a instrucao tem virgulas (LD A,5), entao e' tudo depois da segunda virgula
    const size_t c1 = rest.find(',');
    const size_t c2 = c1 == std::string::npos ? std::string::npos : rest.find(',', c1 + 1);
    const char *usage = "SINTAXE: XFD <ini>[#alvo],<fim>,<instrucao Z80>   por exemplo: XFD 4000,7FFF,CALL 8012H";
    if (c2 == std::string::npos) return usage;
    SxAddr s;
    uint32_t e = 0;
    const std::string instr = Trim(rest.substr(c2 + 1));
    if (!ParseBlock(Trim(rest.substr(0, c1)), Trim(rest.substr(c1 + 1, c2 - c1 - 1)), s, e) || e < s.addr || instr.empty()) return usage;
    Source src; // a instrucao e' montada UMA vez, ancorada no inicio: so' vale para operandos absolutos (JR/DJNZ casam a mesma distancia relativa)
    src.Set(10, "ORG 0" + Hex(s.addr & 0xFFFF, 4) + "H");
    src.Set(20, instr);
    const AsmProduct p = src.Assemble(0);
    if (!p.ok) return "ERRO: " + p.error;
    std::vector<uint8_t> pat;
    for (const auto &kv : p.image) pat.push_back(kv.second);
    if (pat.empty()) return "A INSTRUCAO NAO GEROU BYTES";
    std::vector<std::string> lines;
    unsigned hits = 0;
    const int digits = TargetDigits(s.t);
    for (uint32_t a = s.addr; a <= e;) { // so' conta instrucao REAL, alinhada num limite de instrucao: nao e' uma busca de bytes crua
        const DisasmLine d = DisasmAt(mem, s.t, a);
        if (static_cast<size_t>(d.len) == pat.size()) {
            bool same = true;
            for (size_t k = 0; k < pat.size() && same; ++k) same = TRead(mem, s.t, a + static_cast<uint32_t>(k)) == pat[k];
            if (same) {
                ++hits;
                if (lines.size() < 4096) lines.push_back(FormatDisasm(d, digits));
            }
        }
        a += static_cast<uint32_t>(d.len);
    }
    lines.push_back(std::to_string(hits) + (hits == 1 ? " OCORRENCIA" : " OCORRENCIAS"));
    return Join(lines);
}

std::string Monitor::CmdXts(const std::string &rest) {
    IMemory &mem = Mem();
    const std::vector<std::string> f = SplitCommas(rest);
    SxAddr s;
    uint32_t e = 0;
    if (f.size() != 2 || !ParseBlock(f[0], f[1], s, e) || e < s.addr) return "SINTAXE: XTS <ini>[#alvo],<fim>  (soma de 16 bits do bloco inteiro)";
    std::vector<signed char> buf(e - s.addr + 1);
    for (uint32_t k = 0; k < buf.size(); ++k) buf[k] = static_cast<signed char>(TRead(mem, s.t, s.addr + k));
    const int v = mamute_sum16(buf.data(), static_cast<int>(buf.size())); // Fortran
    std::string bin;
    for (int b = 15; b >= 0; --b) bin += ((v >> b) & 1) ? '1' : '0';
    char oct[16];
    std::snprintf(oct, sizeof(oct), "%06o", static_cast<unsigned>(v));
    const int sv = v >= 0x8000 ? v - 0x10000 : v;
    return "HEX  : " + Hex(static_cast<unsigned>(v), 4) + "H\nBIN  : " + bin + "\nDEC+ : " + std::to_string(v) + "\nDEC+-: " + std::to_string(sv) + "\nOCT  : " + oct;
}

// ---------------------------------------------------------------------------------------------------------------------
// XCO -- as cores do Mamute original. Aqui nao ha' janela para pintar; a escolha fica guardada no mamute.cfg (a TUI e a janela
// grafica podem usa-la depois) e o XCO mostra o estado.
// ---------------------------------------------------------------------------------------------------------------------
std::string Monitor::CmdXco(const std::string &rest) {
    static const char *const kNames[16] = {"TRANSPARENTE", "PRETO", "VERDE", "VERDE CLARO", "AZUL ESCURO", "AZUL CLARO", "VERMELHO ESCURO", "CIANO",
                                           "VERMELHO", "VERMELHO CLARO", "AMARELO ESCURO", "AMARELO CLARO", "VERDE ESCURO", "MAGENTA", "CINZA", "BRANCO"};
    if (!Trim(rest).empty()) {
        const std::vector<std::string> f = SplitCommas(rest);
        if (f.size() > 3) return "SINTAXE: XCO [<frente>],[<fundo>],[<borda>]  (indices 0-15 em DECIMAL; vazio mantem)";
        int* const dst[3] = {&cfg_.color_fg, &cfg_.color_bg, &cfg_.color_border};
        int vals[3] = {cfg_.color_fg, cfg_.color_bg, cfg_.color_border};
        for (size_t k = 0; k < f.size(); ++k) {
            if (f[k].empty()) continue;
            int v = 0;
            if (!util::ParseDec(f[k], v) || v > 15) return "SINTAXE: XCO [<frente>],[<fundo>],[<borda>]  (indices 0-15 em DECIMAL; vazio mantem)";
            vals[k] = v;
        }
        for (int k = 0; k < 3; ++k) *dst[k] = vals[k];
        if (!config_path_.empty()) SaveConfig(config_path_, cfg_); // o XCO do original gravava na hora
    }
    return "FRENTE: " + std::to_string(cfg_.color_fg) + " " + kNames[cfg_.color_fg] + "   FUNDO: " + std::to_string(cfg_.color_bg) + " " + kNames[cfg_.color_bg] +
           "   BORDA: " + std::to_string(cfg_.color_border) + " " + kNames[cfg_.color_border];
}

// ---------------------------------------------------------------------------------------------------------------------
// XSD -- o super disassembler: grava uma listagem assembly, ou os bytes crus como DEFB / DATA do BASIC / dados do X-BASIC
// ---------------------------------------------------------------------------------------------------------------------
std::string Monitor::CmdXsd(const std::string &rest) {
    IMemory &mem = Mem();
    const std::vector<std::string> f = SplitCommas(rest);
    const char *usage = "SINTAXE: XSD <arquivo>,<ini>[#alvo],<fim>[,B|D|X]   (sem modo: listagem assembly; B = DEFB; D = DATA do BASIC; X = dados do X-BASIC)";
    if ((f.size() != 3 && f.size() != 4) || f[0].empty()) return usage;
    SxAddr s;
    uint32_t e = 0;
    if (!ParseBlock(f[1], f[2], s, e) || e < s.addr) return usage;
    char mode = 0;
    if (f.size() == 4) {
        const std::string m = Upper(f[3]);
        if (m != "B" && m != "D" && m != "X") return usage;
        mode = m[0];
    }
    if (mode == 'D' && s.t.vram) return "O MODO D FAZ POKE (MEMORIA COMUM): NAO SERVE PARA VRAM";
    std::vector<std::string> lines;
    if (mode == 0) {
        lines.push_back("        ORG " + Hex(s.addr, TargetDigits(s.t)) + "H");
        for (uint32_t a = s.addr; a <= e;) {
            const DisasmLine d = DisasmAt(mem, s.t, a);
            lines.push_back("        " + d.text);
            a += static_cast<uint32_t>(d.len);
        }
    } else {
        int line_no = 10000;
        for (uint32_t a = s.addr; a <= e; a += 8) {
            std::string items;
            for (uint32_t k = 0; k < 8 && a + k <= e; ++k) {
                const std::string h = Hex(TRead(mem, s.t, a + k), 2);
                if (k) items += ",";
                items += mode == 'B' ? h + "H" : "&H" + h;
            }
            if (mode == 'B') lines.push_back("DEFB " + items);
            else if (mode == 'D') lines.push_back(std::to_string(line_no) + " DATA " + items);
            else lines.push_back(std::to_string(line_no) + " '#" + items);
            line_no += 10;
        }
        if (mode == 'D') lines.push_back(std::to_string(line_no) + " FOR I=&H" + Hex(s.addr, 4) + " TO &H" + Hex(e, 4) + ":READ A:POKE I,A:NEXT I");
    }
    std::string path = f[0];
    if (path.find('.') == std::string::npos) path += (mode == 'D' || mode == 'X') ? ".bas" : ".asm";
    std::ofstream o(path, std::ios::trunc);
    if (!o) return "NAO FOI POSSIVEL GRAVAR " + path;
    for (const std::string &l : lines) o << l << "\n";
    return "GRAVADO " + path + " - " + std::to_string(lines.size()) + " LINHAS";
}

// ---------------------------------------------------------------------------------------------------------------------
// XRG / XGO / XTR -- registradores, execucao com breakpoints nomeados e trace
// ---------------------------------------------------------------------------------------------------------------------
std::string Monitor::RegsXrg() {
    const Z80State &s = Cpu().State();
    std::string out = "AF=" + Hex(s.af.w, 4) + " BC=" + Hex(s.bc.w, 4) + " DE=" + Hex(s.de.w, 4) + " HL=" + Hex(s.hl.w, 4) + "\n";
    out += "IX=" + Hex(s.ix.w, 4) + " IY=" + Hex(s.iy.w, 4) + " SP=" + Hex(s.sp.w, 4) + "\n";
    out += "AF'=" + Hex(s.af_alt.w, 4) + " BC'=" + Hex(s.bc_alt.w, 4) + " DE'=" + Hex(s.de_alt.w, 4) + " HL'=" + Hex(s.hl_alt.w, 4) + "\n";
    out += "PC=" + Hex(s.pc.w, 4) + "\n";
    for (int k = 0; k < 5; ++k) out += std::string(k ? " " : "") + kBpNames[k] + "=" + (sx_.bp_has[k] ? Hex(sx_.bp_addr[k], 4) : "----");
    return out;
}

std::string Monitor::RegsShort() {
    const Z80State &s = Cpu().State();
    return "AF=" + Hex(s.af.w, 4) + " BC=" + Hex(s.bc.w, 4) + " DE=" + Hex(s.de.w, 4) + " HL=" + Hex(s.hl.w, 4) + "\nIX=" + Hex(s.ix.w, 4) + " IY=" + Hex(s.iy.w, 4) + " SP=" + Hex(s.sp.w, 4);
}

std::string Monitor::CmdXrg(const std::string &rest) {
    const std::string arg = Upper(Trim(rest));
    Z80State &s = Cpu().State();
    if (arg.empty()) return RegsXrg();
    if (arg == "*") { // limpa tudo, menos a pilha
        s.af.w = s.bc.w = s.de.w = s.hl.w = 0;
        s.af_alt.w = s.bc_alt.w = s.de_alt.w = s.hl_alt.w = 0;
        s.ix.w = s.iy.w = s.pc.w = 0;
        s.i = s.r = 0;
        s.iff = 0;
        return RegsXrg();
    }
    if (arg == "+") { // so' a pilha
        s.sp.w = 0;
        return RegsXrg();
    }
    const std::vector<std::string> f = SplitCommas(arg);
    const char *usage = "SINTAXE: XRG | XRG * (limpa, menos SP) | XRG + (SP=0000) | XRG <registro>,<valor>   (registro: A..L, A'..L', AF BC DE HL, AF'..HL', IX IY IXH IXL IYH IYL SP PC, BP BP1 BP2 BP3 BPF)";
    if (f.size() != 2 || f[0].empty() || f[1].empty()) return usage;
    for (int k = 0; k < 5; ++k) {
        if (f[0] != kBpNames[k]) continue;
        if (f[1] == "-") { // extensao: limpa o breakpoint
            sx_.bp_has[k] = false;
            return RegsXrg();
        }
        uint32_t v = 0;
        if (!ParseHexN(f[1], v, 4)) return usage;
        sx_.bp_has[k] = true;
        sx_.bp_addr[k] = static_cast<uint16_t>(v);
        return RegsXrg();
    }
    const RegRef r = FindReg(s, f[0]);
    uint32_t v = 0;
    if (r.p8) {
        if (!ParseHexN(f[1], v, 2)) return usage;
        *r.p8 = static_cast<uint8_t>(v);
    } else if (r.p16) {
        if (!ParseHexN(f[1], v, 4)) return usage;
        r.p16->w = static_cast<uint16_t>(v);
    } else {
        return usage;
    }
    return RegsXrg();
}

std::string Monitor::CmdXgo(const std::string &rest) {
    IMemory &mem = Mem();
    IExec &cpu = Cpu();
    std::string prefix;
    if (!Trim(rest).empty()) {
        SxAddr a;
        if (!ParseSx(mem, sx_.vars, rest, a) || a.t.vram || (a.t.is_explicit && a.t.sub != 0)) return "SINTAXE: XGO <end>[#slot]  (VRAM e sub-slot nao: a CPU so' executa a memoria mapeada)";
        if (a.t.is_explicit) { // #slot troca o slot mapeado na pagina do endereco, como o PAGE
            if (online()) return "XGO COM #SLOT: O MAPEAMENTO E' DA MAQUINA (ONLINE); USE O ENDERECO SEM SUFIXO";
            std::array<int, 4> map = mem.PageMap();
            const int page = static_cast<int>((a.addr >> 14) & 3);
            map[static_cast<size_t>(page)] = a.t.slot;
            mem.SetPageMap(map);
            prefix = "PAGINA " + std::to_string(page) + " NO SLOT " + std::to_string(a.t.slot) + "\n";
        }
        cpu.State().pc.w = static_cast<uint16_t>(a.addr);
        cpu.State().iff &= static_cast<uint8_t>(~Z80_IFF_HALT); // comecar num endereco e' um programa novo: o HALT do anterior nao vale mais
        sx_.go_step = 0;
        sx_.has_go = true;
    } else if (!sx_.has_go) {
        return "XGO SEM ENDERECO SO' DEPOIS DE UM XGO <end>";
    }
    // o alvo desta chamada: BP, BP1, BP2, BP3 e dai so' BPF; sem o da vez cai no BPF; sem nenhum, roda "livre"
    int target = -1;
    const int step = std::min(sx_.go_step, 4);
    if (step < 4 && sx_.bp_has[step]) target = sx_.bp_addr[step];
    else if (sx_.bp_has[4]) target = sx_.bp_addr[4];
    if (sx_.go_step < 4) ++sx_.go_step;

    if (online()) { // na maquina: solta a CPU com o breakpoint (ou sem ele) e o resto e' com o depurador
        std::vector<uint16_t> bps;
        if (target >= 0) bps.push_back(static_cast<uint16_t>(target));
        return prefix + cpu.Go(false, 0, bps);
    }
    Z80State &st = cpu.State();
    if (cpu.Halted()) return prefix + "CPU HALTADA (HALT) EM " + Hex(st.pc.w, 4) + "\n" + RegsXrg();
    const uint16_t entry_sp = st.sp.w;
    std::string status;
    for (long steps = 1;; ++steps) {
        cpu.StepOne();
        if (cpu.Halted()) {
            status = "CPU HALTADA (HALT) EM " + Hex(st.pc.w, 4);
            break;
        }
        if (target >= 0) {
            if (st.pc.w == target) {
                status = "PARADO NO BREAKPOINT (" + Hex(st.pc.w, 4) + ")";
                break;
            }
        } else {
            // "fim do programa": o RET que devolve alem da pilha de onde o XGO comecou (SP andou PARA CIMA, com a volta de 16 bits)
            const uint16_t moved = static_cast<uint16_t>(st.sp.w - entry_sp);
            if (moved != 0 && moved < 0x8000) {
                status = "PROGRAMA TERMINOU (RET NO TOPO DA PILHA) EM " + Hex(st.pc.w, 4);
                break;
            }
        }
        if (steps >= 2000000) {
            status = "RUN INTERROMPIDO - LIMITE DE 2000000 INSTRUCOES ATINGIDO EM " + Hex(st.pc.w, 4);
            break;
        }
    }
    return prefix + status + "\n" + RegsXrg();
}

std::string Monitor::CmdXtr(const std::string &rest) {
    const std::vector<std::string> f = SplitCommas(rest);
    const char *usage = "SINTAXE: XTR <end> | XTR (continua do PC) | XTR [<end>],<passos>   (na TUI abre o trace: ENTER = proxima instrucao, ESC interrompe)";
    if (f.size() > 2) return usage;
    uint32_t a = 0, n = 1;
    if (!f.empty() && !f[0].empty()) {
        if (!ParseHexN(f[0], a, 4)) return usage;
        Cpu().State().pc.w = static_cast<uint16_t>(a);
        Cpu().State().iff &= static_cast<uint8_t>(~Z80_IFF_HALT);
    }
    if (f.size() == 2 && (!ParseHexN(f[1], n, 4) || n < 1)) return usage;
    // um passo: o endereco, os bytes e o mnemonico da instrucao que roda, mais os registradores depois dela
    auto step = [this]() -> std::vector<std::string> {
        IExec &cpu = Cpu();
        if (cpu.Halted()) return {};
        Target page; // o trace roda contra a memoria mapeada (como o TR original: sem #slot)
        const DisasmLine d = DisasmAt(Mem(), page, cpu.State().pc.w);
        std::vector<std::string> lines{FormatDisasm(d, 4)};
        cpu.StepOne();
        lines.push_back(RegsShort());
        if (cpu.Halted()) lines.push_back("CPU HALTADA (HALT) EM " + Hex(cpu.State().pc.w, 4));
        return lines;
    };
    if (panels_ && f.size() <= 1) {
        std::vector<std::string> first{"TRACE EM " + Hex(Cpu().State().pc.w, 4)};
        const std::vector<std::string> s1 = step();
        first.insert(first.end(), s1.begin(), s1.end());
        return OpenPanel(MakeTracePanel(step, first));
    }
    std::vector<std::string> out;
    for (uint32_t k = 0; k < n; ++k) {
        const std::vector<std::string> s = step();
        if (s.empty()) {
            out.push_back("CPU HALTADA (HALT) EM " + Hex(Cpu().State().pc.w, 4));
            break;
        }
        out.insert(out.end(), s.begin(), s.end());
    }
    return Join(out);
}

// ---------------------------------------------------------------------------------------------------------------------
// XPP / XPI / XPO -- as portas de E/S
// ---------------------------------------------------------------------------------------------------------------------
std::string Monitor::CmdXports(const std::string &cmd, const std::string &rest) {
    if (cmd == "XPP") {
        if (panels_) return OpenPanel(MakePortsPanel(&sx_.ports));
        std::vector<std::string> lines{"PORTA  ENTRADA  SAIDA"};
        for (int p = 0; p < 256; ++p)
            if (sx_.ports.mon[p]) lines.push_back(Hex(static_cast<unsigned>(p), 2) + (sx_.ports.changed[p] ? "*" : " ") + "    " + Hex(sx_.ports.in[p], 2) + "      " + Hex(sx_.ports.out[p], 2));
        if (lines.size() == 1) lines.push_back("(nenhuma porta monitorada: XPI/XPO criam)");
        else lines.push_back("(* = a porta sofreu um OUT ou XPO)");
        return Join(lines);
    }
    if (cmd == "XPI") {
        uint32_t p = 0;
        if (!ParseHexN(rest, p, 2)) return "SINTAXE: XPI <porta>  (hexa, 00-FF)";
        sx_.ports.Add(static_cast<int>(p)); // a porta aparece no painel com Saida = FF (barramento flutuante)
        return "PORTA " + Hex(p, 2) + " = " + Hex(sx_.ports.out[p], 2);
    }
    const std::vector<std::string> f = SplitCommas(rest);
    uint32_t p = 0, v = 0;
    if (f.size() != 2 || !ParseHexN(f[0], p, 2) || !ParseHexN(f[1], v, 2)) return "SINTAXE: XPO <porta>,<byte>  (hexa)";
    sx_.ports.IoOut(static_cast<int>(p), static_cast<uint8_t>(v));
    return "PORTA " + Hex(p, 2) + " <- " + Hex(v, 2);
}

} // namespace mamute
