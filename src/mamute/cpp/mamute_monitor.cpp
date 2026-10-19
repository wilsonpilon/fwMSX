// fwMSX -- Mamute: o monitor (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md.
#include "mamute_monitor.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>

#include "../../z80/debug/z80_disasm.h"
#include "../asm/mamute_scan.h"
#include "../core/mamute_dump.h"

namespace mamute {

namespace {

std::string Upper(std::string s) {
    for (char &c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

std::string Trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

std::string Hex(unsigned v, int digits) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%0*X", digits, v);
    return buf;
}

// Hexadecimal tolerante: "4000", "$4000", "0x4000", "4000h". `max_digits` limita (4 para enderecos da CPU, 5 da VRAM).
bool ParseHex(std::string s, unsigned &out, int max_digits = 4) {
    s = Trim(s);
    if (!s.empty() && s[0] == '$') s.erase(0, 1);
    else if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s.erase(0, 2);
    else if (s.size() > 1 && (s.back() == 'h' || s.back() == 'H')) s.pop_back();
    if (s.empty() || static_cast<int>(s.size()) > max_digits) return false;
    unsigned v = 0;
    for (const char c : s) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
        v = v * 16 + static_cast<unsigned>(std::isdigit(static_cast<unsigned char>(c)) ? c - '0' : std::toupper(static_cast<unsigned char>(c)) - 'A' + 10);
    }
    out = v;
    return true;
}

// Deslocamento do texto: hexa com sinal opcional, de -7F a +80 (vazio = 0).
bool ParseShift(const std::string &s, int &out) {
    std::string t = Trim(s);
    if (t.empty()) {
        out = 0;
        return true;
    }
    int sign = 1;
    if (t[0] == '+' || t[0] == '-') {
        sign = t[0] == '-' ? -1 : 1;
        t.erase(0, 1);
    }
    unsigned v = 0;
    if (!ParseHex(t, v, 2)) return false;
    const int d = sign * static_cast<int>(v);
    if (d < -0x7F || d > 0x80) return false;
    out = d;
    return true;
}

std::string Signed(int d) { return (d < 0 ? "-" : "+") + Hex(static_cast<unsigned>(d < 0 ? -d : d), 2); }

// Campos de argumento: com virgula, separa por virgula e GUARDA os vazios (o curinga do SH); sem virgula, por espaco.
std::vector<std::string> Fields(const std::string &rest) {
    std::vector<std::string> out;
    const std::string r = Trim(rest);
    if (r.empty()) return out;
    if (r.find(',') != std::string::npos) {
        size_t pos = 0;
        while (true) {
            const size_t c = r.find(',', pos);
            out.push_back(Trim(r.substr(pos, c == std::string::npos ? std::string::npos : c - pos)));
            if (c == std::string::npos) break;
            pos = c + 1;
        }
    } else {
        size_t i = 0;
        while (i < r.size()) {
            while (i < r.size() && std::isspace(static_cast<unsigned char>(r[i]))) ++i;
            size_t j = i;
            while (j < r.size() && !std::isspace(static_cast<unsigned char>(r[j]))) ++j;
            if (j > i) out.push_back(r.substr(i, j - i));
            i = j;
        }
    }
    return out;
}

char Printable(int b) { return (b >= 0x20 && b < 0x7F) ? static_cast<char>(b) : '.'; }

const char *kHelp =
    "COMANDOS DO MAMUTE (enderecos em HEXA)\n"
    "  BA | QUIT | XQT              sai do monitor\n"
    "  ONLINE | OFFLINE | STATUS    escolhe a memoria (maquina emulada ou privada) / mostra o estado\n"
    "  PUSH ini,fim[,dest]          copia um bloco da memoria OFFLINE para a maquina (ONLINE)\n"
    "  PULL ini,fim[,dest]          copia um bloco da maquina para a memoria OFFLINE\n"
    "  PAGE [?|a,b,c,d]             mostra/troca o slot de cada pagina (so' offline)\n"
    "  DM end[,desloc]              128 bytes em hexa+ASCII (16 linhas de 8)\n"
    "  M end[,b1,b2...]             sem bytes: mostra como o DM; com bytes: grava\n"
    "  SH [end],b1,,b3 | SH [end],'texto     procura bytes (virgula dupla = curinga) ou texto\n"
    "  MS end,[desloc],'texto       grava um texto\n"
    "  C modo                       formato de D/P/V: 0 = 4 B/linha, 1 = 16 B, 2 = 8 B + checksum, 3 = idem sem o endereco\n"
    "  D ini[,fim] | P ini[,fim][,arq] | V ini[,fim]    despejo (log | arquivo texto | VRAM)\n"
    "  T ini,fim,dest | F ini,fim,byte     copia / preenche\n"
    "  L [ini[,fim]] | LP [ini[,fim]][,arq]   disassembler (log | arquivo texto)\n"
    "  X [reg [valor]] | G [ini[,bp1[,bp2]]]  registradores / executa\n"
    "  LOAD arq[,slot[,end]] | SAVE arq,ini,fim[,exec[,BIN|ROM|RAW]]   arquivos\n"
    "  EDIT | LIST | A [opcoes] | MAP   editor de fonte Z80 (linhas numeradas) e montagem pelo fwasm (HELP no EDIT)\n"
    "  CFG [SET s,p,tipo[,arq[,HALF]] | VRAM n | ONLINE 0|1 | DUMP n | SAVE | LOAD]   configuracao inicial\n"
    "  HELP                         esta lista (o manual completo: man mamute)";

} // namespace

Monitor::Monitor(const Config &cfg, std::string config_path)
    : cfg_(cfg), config_path_(std::move(config_path)), offline_(cfg), offline_exec_(offline_), dump_mode_(cfg.dump_mode) {
    // Reconfigure ja' rodou no construtor da memoria; os avisos (arquivo que nao abriu) saem no banner
    load_warnings_ = offline_.Reconfigure(cfg_);
}

std::string Monitor::Prompt() const {
    if (editing_) return "EDIT> ";
    return online() ? "MON(on)> " : "MON(off)> ";
}

std::string Monitor::Banner() {
    std::string out = "MAMUTE - MONITOR Z80 DO fwMSX (modo " + Upper(Mem().Name()) + ", HELP lista os comandos)";
    if (!has_machine()) out += "\nSEM MAQUINA: so' o modo OFFLINE (ONLINE aparece com o emulador ligado)";
    if (!load_warnings_.empty()) out += "\n" + Trim(load_warnings_);
    return out;
}

std::string Monitor::Execute(const std::string &line_in, bool *quit) {
    if (quit) *quit = false;
    const std::string line = Trim(line_in);
    if (line.empty()) return "";
    size_t i = 0;
    while (i < line.size() && !std::isspace(static_cast<unsigned char>(line[i])) && line[i] != ',' && line[i] != '\'' && line[i] != '/' && line[i] != '>') ++i;
    const std::string cmd = Upper(line.substr(0, i));
    const std::string rest = Trim(line.substr(i));
    const std::vector<std::string> f = Fields(rest);

    if (cmd == "BA" || cmd == "XQT" || (cmd == "QUIT" && !editing_)) {
        editing_ = false;
        if (quit) *quit = true;
        return "";
    }
    if (editing_) { // dentro do EDIT: tudo e' do editor de fonte; QUIT volta ao monitor
        if (cmd == "QUIT") {
            editing_ = false;
            return "VOLTA AO MONITOR (O PROGRAMA-FONTE CONTINUA NA MEMORIA)";
        }
        return ExecuteEdit(line);
    }
    if (cmd == "EDIT") {
        if (!rest.empty()) return ExecuteEdit(rest); // um comando do editor sem entrar nele: edit list, edit a o
        editing_ = true;
        return "EDITOR DE FONTE Z80 - " + std::to_string(src_.size()) + " LINHAS (HELP lista, QUIT volta ao monitor)";
    }
    if (cmd == "LIST" || cmd == "A" || cmd == "MAP") return ExecuteEdit(line);
    if (cmd == "HELP" || cmd == "?") return CmdHelp();
    if (cmd == "ONLINE") return CmdMode(true);
    if (cmd == "OFFLINE") return CmdMode(false);
    if (cmd == "STATUS" || cmd == "ST") return CmdStatus();
    if (cmd == "PUSH") return CmdXfer(f, true);
    if (cmd == "PULL") return CmdXfer(f, false);
    if (cmd == "PAGE") return CmdPage(f);
    if (cmd == "DM") return CmdDm(f);
    if (cmd == "M") return CmdM(f);
    if (cmd == "SH") return CmdSh(rest);
    if (cmd == "MS") return CmdMs(rest);
    if (cmd == "C") return CmdC(f);
    if (cmd == "D") return CmdDump(f, false, false);
    if (cmd == "P") return CmdDump(f, false, true);
    if (cmd == "V") return CmdDump(f, true, false);
    if (cmd == "T") return CmdT(f);
    if (cmd == "F") return CmdF(f);
    if (cmd == "L") return CmdList(f, false);
    if (cmd == "LP") return CmdList(f, true);
    if (cmd == "X") return CmdX(f);
    if (cmd == "G") return CmdG(f);
    if (cmd == "R") return "R: CARREGAR UM ASSEMBLADO DE FITA AINDA NAO EXISTE (use LOAD)";
    if (cmd == "LOAD") return CmdLoad(f);
    if (cmd == "SAVE") return CmdSave(f);
    if (cmd == "CFG") return CmdCfg(rest);
    return "COMANDO DESCONHECIDO: " + cmd + " (HELP lista os comandos)";
}

std::string Monitor::CmdHelp() { return kHelp; }

std::string Monitor::CmdMode(bool online) {
    if (online && !has_machine()) return "SEM MAQUINA: ligue o emulador (no console: emu start) para usar ONLINE";
    online_ = online;
    return std::string("MODO ") + (online ? "ONLINE: o monitor agora le e escreve na MAQUINA EMULADA" : "OFFLINE: memoria e CPU privadas do monitor");
}

std::string Monitor::CmdStatus() {
    std::string out = std::string("MODO: ") + Upper(Mem().Name()) + (has_machine() ? "" : "   (sem maquina: so' offline)");
    const auto map = Mem().PageMap();
    out += "\nPAGINAS (slot de cada uma): " + std::to_string(map[0]) + " " + std::to_string(map[1]) + " " + std::to_string(map[2]) + " " + std::to_string(map[3]);
    out += "\nFORMATO DE DESPEJO (C): " + std::to_string(dump_mode_) + "   VRAM: " + std::to_string(Mem().VramSize() / 1024) + " KB";
    const Z80State &s = Cpu().State();
    out += "\nPC=" + Hex(s.pc.w, 4) + " SP=" + Hex(s.sp.w, 4);
    return out;
}

std::string Monitor::CmdXfer(const std::vector<std::string> &f, bool push) {
    const char *name = push ? "PUSH" : "PULL";
    if (!has_machine()) return std::string(name) + ": SEM MAQUINA (so' existe o offline)";
    unsigned a = 0, b = 0, d = 0;
    if (f.size() < 2 || !ParseHex(f[0], a) || !ParseHex(f[1], b) || b < a) return std::string("SINTAXE: ") + name + " <ini>,<fim>[,<dest>]";
    d = a;
    if (f.size() > 2 && !f[2].empty() && !ParseHex(f[2], d)) return std::string("SINTAXE: ") + name + " <ini>,<fim>[,<dest>]";
    if (d + (b - a) > 0xFFFF) return "DESTINO PASSA DE FFFF";
    IMemory &src = push ? static_cast<IMemory &>(offline_) : *online_mem_;
    IMemory &dst = push ? *online_mem_ : static_cast<IMemory &>(offline_);
    std::vector<uint8_t> buf;
    for (unsigned x = a; x <= b; ++x) buf.push_back(src.Read(static_cast<uint16_t>(x)));
    unsigned written = 0;
    for (size_t k = 0; k < buf.size(); ++k)
        if (dst.Write(static_cast<uint16_t>(d + k), buf[k])) ++written;
    std::string out = std::string(push ? "ENVIADOS " : "TRAZIDOS ") + std::to_string(written) + " BYTES (" + Hex(a, 4) + "-" + Hex(b, 4) + " -> " + Hex(d, 4) + ")";
    if (written < buf.size()) out += " - " + std::to_string(buf.size() - written) + " IGNORADOS (DESTINO NAO E' RAM)";
    return out;
}

std::string Monitor::CmdPage(const std::vector<std::string> &f) {
    IMemory &m = Mem();
    if (!f.empty() && f[0] == "?") {
        // so' mostra
    } else if (f.empty()) {
        if (online()) return "PAGE: ONLINE USA O MAPEAMENTO DA MAQUINA (OS PROGRAMAS TROCAM OS SLOTS; USE PAGE ? PARA VER)";
        offline_.Boot();
    } else {
        if (online()) return "PAGE: ONLINE USA O MAPEAMENTO DA MAQUINA (OS PROGRAMAS TROCAM OS SLOTS; USE PAGE ? PARA VER)";
        unsigned s[4];
        if (f.size() != 4) return "SINTAXE: PAGE [?|X,Y,Z,W]  (slot de cada pagina, 0-3)";
        for (int k = 0; k < 4; ++k)
            if (!ParseHex(f[static_cast<size_t>(k)], s[k], 1) || s[k] > 3) return "SINTAXE: PAGE [?|X,Y,Z,W]  (slot de cada pagina, 0-3)";
        m.SetPageMap({{static_cast<int>(s[0]), static_cast<int>(s[1]), static_cast<int>(s[2]), static_cast<int>(s[3])}});
    }
    const auto map = m.PageMap();
    std::string out;
    for (int p = 0; p < 4; ++p) {
        if (p) out += '\n';
        out += "PAGINA " + std::to_string(p) + " (" + Hex(static_cast<unsigned>(p * 0x4000), 4) + "-" + Hex(static_cast<unsigned>(p * 0x4000 + 0x3FFF), 4) + "): SLOT " +
               std::to_string(map[static_cast<size_t>(p)]) + " " + CellName(m.Cell(map[static_cast<size_t>(p)], p));
    }
    return out;
}

std::string Monitor::CmdDm(const std::vector<std::string> &f) {
    unsigned a = 0;
    int shift = 0;
    if (f.empty() || !ParseHex(f[0], a) || (f.size() > 1 && !ParseShift(f[1], shift))) return "SINTAXE: DM <end>[,<desloc>]  (desloc de -7F a +80)";
    IMemory &m = Mem();
    std::string out;
    for (int row = 0; row < 16; ++row) {
        const unsigned base = (a + static_cast<unsigned>(row) * 8) & 0xFFFF;
        std::string hex, txt;
        for (unsigned k = 0; k < 8; ++k) {
            const uint8_t b = m.Read(static_cast<uint16_t>(base + k));
            hex += Hex(b, 2) + " ";
            txt += Printable((b + shift) & 0xFF);
        }
        out += Hex(base, 4) + "  " + hex + " " + txt + "\n";
    }
    out += "ENDERECO: " + Hex(a, 4) + "  DESLOC.: " + Signed(shift);
    return out;
}

std::string Monitor::CmdM(const std::vector<std::string> &f) {
    unsigned a = 0;
    if (f.empty() || !ParseHex(f[0], a)) return "SINTAXE: M <end>[,<byte>[,<byte>...]]";
    if (f.size() == 1) return CmdDm(f);
    std::vector<uint8_t> bytes;
    for (size_t k = 1; k < f.size(); ++k) {
        unsigned v = 0;
        if (!ParseHex(f[k], v, 2)) return "BYTE INVALIDO: '" + f[k] + "' (00-FF)";
        bytes.push_back(static_cast<uint8_t>(v));
    }
    IMemory &m = Mem();
    unsigned ok = 0;
    for (size_t k = 0; k < bytes.size(); ++k)
        if (m.Write(static_cast<uint16_t>(a + k), bytes[k])) ++ok;
    std::string out = "GRAVADOS " + std::to_string(ok) + " BYTES EM " + Hex(a, 4);
    if (ok < bytes.size()) out += " - " + std::to_string(bytes.size() - ok) + " IGNORADOS (NAO E' RAM)";
    return out;
}

std::string Monitor::CmdSh(const std::string &rest) {
    // "[end],b1,,b3" (bytes, vazio = curinga) ou "[end],'texto" (o apostrofo abre o texto; nada fecha)
    const size_t q = rest.find('\'');
    const bool text_mode = q != std::string::npos;
    const std::string head = text_mode ? rest.substr(0, q) : rest;
    const std::string text = text_mode ? rest.substr(q + 1) : "";
    std::vector<std::string> f;
    {
        size_t pos = 0;
        const std::string r = head;
        while (true) {
            const size_t c = r.find(',', pos);
            f.push_back(Trim(r.substr(pos, c == std::string::npos ? std::string::npos : c - pos)));
            if (c == std::string::npos) break;
            pos = c + 1;
        }
    }
    if (text_mode && !f.empty() && f.back().empty()) f.pop_back(); // a virgula antes do apostrofo
    unsigned start = 0;
    if (f.empty() || f[0].empty()) {
        if (last_find_ < 0) return "SH SEM ENDERECO SO' DEPOIS DE UMA BUSCA QUE ACHOU ALGO";
        start = static_cast<unsigned>(last_find_ + 1) & 0xFFFF;
    } else if (!ParseHex(f[0], start)) {
        return "SINTAXE: SH [<end>],<byte>[,<byte>...]  ou  SH [<end>],'<texto>";
    }
    IMemory &m = Mem();
    std::vector<uint8_t> img(0x10000);
    for (unsigned x = 0; x < 0x10000; ++x) img[x] = m.Read(static_cast<uint16_t>(x));

    // acha a PRIMEIRA ocorrencia a partir de `start`, dando a volta (ordem de busca = distancia de `start`)
    auto first = [&](const std::vector<uint8_t> &pat, const std::vector<uint8_t> &mask) -> long {
        const int plen = static_cast<int>(pat.size());
        const int r1 = mamute_find(img.data() + start, static_cast<int>(0x10000 - start), pat.data(), mask.data(), plen);
        if (r1 >= 0) return static_cast<long>(start) + r1;
        const unsigned wrap = std::min<unsigned>(0x10000, start + static_cast<unsigned>(plen) - 1);
        const int r2 = mamute_find(img.data(), static_cast<int>(wrap), pat.data(), mask.data(), plen);
        return r2;
    };

    if (!text_mode) {
        std::vector<uint8_t> pat, mask;
        for (size_t k = 1; k < f.size(); ++k) {
            unsigned v = 0;
            if (f[k].empty()) {
                pat.push_back(0);
                mask.push_back(0x00);
            } else if (ParseHex(f[k], v, 2)) {
                pat.push_back(static_cast<uint8_t>(v));
                mask.push_back(0xFF);
            } else {
                return "BYTE INVALIDO: '" + f[k] + "' (00-FF)";
            }
        }
        if (pat.empty() || std::all_of(mask.begin(), mask.end(), [](uint8_t x) { return x == 0; })) return "SINTAXE: SH [<end>],<byte>[,<byte>...]  (ao menos um byte exato)";
        const long at = first(pat, mask);
        if (at < 0) return "NAO ENCONTRADO";
        last_find_ = static_cast<int>(at);
        return "ACHADO EM " + Hex(static_cast<unsigned>(at), 4);
    }
    if (text.size() < 2) return "O TEXTO PRECISA DE 2 OU MAIS CARACTERES";
    // texto: em cada deslocamento d, o byte cru e' (caractere - d); acha o menor endereco (na ordem da busca), empate = menor |d|
    long best = -1, best_dist = 0x7FFFFFFF;
    int best_shift = 0;
    for (int step = 0; step <= 0x80 * 2; ++step) {
        const int d = (step & 1) ? -((step + 1) / 2) : (step / 2); // 0, -1, +1, -2, +2 ...
        if (d < -0x7F || d > 0x80) continue;
        std::vector<uint8_t> pat, mask(text.size(), 0xFF);
        for (const char c : text) pat.push_back(static_cast<uint8_t>((static_cast<unsigned char>(c) - d) & 0xFF));
        const long at = first(pat, mask);
        if (at < 0) continue;
        const long dist = (at - static_cast<long>(start) + 0x10000) % 0x10000;
        if (dist < best_dist) {
            best_dist = dist;
            best = at;
            best_shift = d;
        }
    }
    if (best < 0) return "NAO ENCONTRADO";
    last_find_ = static_cast<int>(best);
    return "ACHADO EM " + Hex(static_cast<unsigned>(best), 4) + " DESLOC " + Signed(best_shift);
}

std::string Monitor::CmdMs(const std::string &rest) {
    // "end,[desloc],'texto" -- tudo depois do apostrofo e' texto (virgulas inclusive)
    const size_t q = rest.find('\'');
    if (q == std::string::npos) return "SINTAXE: MS <end>,[<desloc>],'<texto>";
    std::vector<std::string> f = Fields(rest.substr(0, q) + ",");
    while (!f.empty() && f.back().empty() && f.size() > 2) f.pop_back();
    unsigned a = 0;
    int shift = 0;
    if (f.empty() || !ParseHex(f[0], a) || (f.size() > 1 && !ParseShift(f[1], shift))) return "SINTAXE: MS <end>,[<desloc>],'<texto>";
    const std::string text = rest.substr(q + 1);
    if (text.empty()) return "SINTAXE: MS <end>,[<desloc>],'<texto>";
    IMemory &m = Mem();
    unsigned ok = 0;
    for (size_t k = 0; k < text.size(); ++k)
        if (m.Write(static_cast<uint16_t>(a + k), static_cast<uint8_t>((static_cast<unsigned char>(text[k]) - shift) & 0xFF))) ++ok;
    std::string out = "GRAVADOS " + std::to_string(ok) + " CARACTERES EM " + Hex(a, 4) + " DESLOC " + Signed(shift);
    if (ok < text.size()) out += " - " + std::to_string(text.size() - ok) + " IGNORADOS (NAO E' RAM)";
    return out;
}

std::string Monitor::CmdC(const std::vector<std::string> &f) {
    unsigned m = 0;
    if (f.empty() || !ParseHex(f[0], m, 1) || mamute_dump_width(static_cast<int>(m)) == 0) return "SINTAXE: C <modo 0-3>";
    dump_mode_ = static_cast<int>(m);
    static const char *const names[] = {"HEXA+ASCII, 4 BYTES/LINHA", "HEXA+ASCII, 16 BYTES/LINHA", "HEXA, 8 BYTES/LINHA + CHECKSUM (SOMA + ENDERECO)", "HEXA, 8 BYTES/LINHA + CHECKSUM (SOMA)"};
    return "MODO " + std::to_string(m) + ": " + names[m];
}

std::string Monitor::WriteListing(const std::string &path, const std::string &title, const std::string &body) {
    std::ofstream f(path, std::ios::trunc);
    if (!f) return "NAO FOI POSSIVEL GRAVAR " + path;
    f << title << "\n\n" << body << "\n";
    const size_t lines = static_cast<size_t>(std::count(body.begin(), body.end(), '\n')) + 1;
    return "GRAVADO " + path + " - " + std::to_string(lines) + " LINHAS";
}

std::string Monitor::CmdDump(const std::vector<std::string> &f, bool vram, bool to_file) {
    const char *name = vram ? "V" : (to_file ? "P" : "D");
    const int digits = vram ? 5 : 4;
    const unsigned limit = vram ? static_cast<unsigned>(Mem().VramSize()) - 1 : 0xFFFF;
    unsigned a = 0, b = 0;
    if (f.empty() || !ParseHex(f[0], a, digits)) return std::string("SINTAXE: ") + name + " <ini>[,<fim>]" + (to_file ? "[,<arquivo>]" : "");
    b = a + 15;
    if (f.size() > 1 && !f[1].empty()) {
        if (!ParseHex(f[1], b, digits)) return std::string("SINTAXE: ") + name + " <ini>[,<fim>]";
    }
    if (b < a) return "O FIM NAO PODE SER MENOR QUE O INICIO";
    if (a > limit || b > limit) return "ENDERECO PASSA DE " + Hex(limit, digits);
    IMemory &m = Mem();
    const int width = mamute_dump_width(dump_mode_);
    std::string out;
    char line[160];
    for (unsigned x = a; x <= b; x += static_cast<unsigned>(width)) {
        unsigned char buf[16];
        const int n = static_cast<int>(std::min<unsigned>(static_cast<unsigned>(width), b - x + 1));
        for (int k = 0; k < n; ++k) buf[k] = vram ? m.ReadVram(x + static_cast<unsigned>(k)) : m.Read(static_cast<uint16_t>(x + static_cast<unsigned>(k)));
        mamute_dump_line(dump_mode_, x, buf, n, line, sizeof(line));
        if (!out.empty()) out += '\n';
        out += line;
    }
    if (!to_file) return out;
    std::string path = "mamute.txt";
    if (f.size() > 2 && !f[2].empty()) path = f[2];
    return WriteListing(path, std::string("MAMUTE - DESPEJO ") + Hex(a, digits) + "-" + Hex(b, digits) + " (MODO " + std::to_string(dump_mode_) + ")", out);
}

std::string Monitor::CmdT(const std::vector<std::string> &f) {
    unsigned a = 0, b = 0, d = 0;
    if (f.size() < 3 || !ParseHex(f[0], a) || !ParseHex(f[1], b) || !ParseHex(f[2], d)) return "SINTAXE: T <ini>,<fim>,<dest>";
    if (b < a) return "O FIM NAO PODE SER MENOR QUE O INICIO";
    if (d + (b - a) > 0xFFFF) return "DESTINO PASSA DE FFFF";
    IMemory &m = Mem();
    std::vector<uint8_t> buf; // le tudo antes de gravar: origem e destino podem se sobrepor (memmove)
    for (unsigned x = a; x <= b; ++x) buf.push_back(m.Read(static_cast<uint16_t>(x)));
    unsigned ok = 0;
    for (size_t k = 0; k < buf.size(); ++k)
        if (m.Write(static_cast<uint16_t>(d + k), buf[k])) ++ok;
    std::string out = "COPIADOS " + std::to_string(ok) + " BYTES DE " + Hex(a, 4) + "-" + Hex(b, 4) + " PARA " + Hex(d, 4);
    if (ok < buf.size()) out += " - " + std::to_string(buf.size() - ok) + " IGNORADOS (NAO E' RAM)";
    return out;
}

std::string Monitor::CmdF(const std::vector<std::string> &f) {
    unsigned a = 0, b = 0, v = 0;
    if (f.size() < 3 || !ParseHex(f[0], a) || !ParseHex(f[1], b) || !ParseHex(f[2], v, 2)) return "SINTAXE: F <ini>,<fim>,<byte>";
    if (b < a) return "O FIM NAO PODE SER MENOR QUE O INICIO";
    IMemory &m = Mem();
    unsigned ok = 0;
    for (unsigned x = a; x <= b; ++x)
        if (m.Write(static_cast<uint16_t>(x), static_cast<uint8_t>(v))) ++ok;
    std::string out = "PREENCHIDOS " + std::to_string(ok) + " BYTES (" + Hex(a, 4) + "-" + Hex(b, 4) + ") COM " + Hex(v, 2);
    if (ok < b - a + 1) out += " - " + std::to_string(b - a + 1 - ok) + " IGNORADOS (NAO E' RAM)";
    return out;
}

std::string Monitor::CmdList(const std::vector<std::string> &f, bool to_file) {
    unsigned a = 0, b = 0;
    bool by_count = true;
    const bool have_a = !f.empty() && !f[0].empty();
    if (have_a) {
        if (!ParseHex(f[0], a)) return std::string("SINTAXE: ") + (to_file ? "LP" : "L") + " [<ini>[,<fim>]]";
    } else {
        if (last_list_ < 0) return "L SEM ENDERECO SO' DEPOIS DE UM L/LP ANTERIOR";
        a = static_cast<unsigned>(last_list_);
    }
    if (f.size() > 1 && !f[1].empty()) {
        if (!ParseHex(f[1], b)) return std::string("SINTAXE: ") + (to_file ? "LP" : "L") + " [<ini>[,<fim>]]";
        if (b < a) return "O FIM NAO PODE SER MENOR QUE O INICIO";
        by_count = false;
    }
    IMemory &m = Mem();
    std::string out;
    unsigned x = a;
    for (int n = 0; (by_count ? n < 10 : x <= b) && n < 4096; ++n) {
        const z80::debug::DisasmResult d = z80::debug::Disassemble([&](uint16_t p) { return m.Read(p); }, static_cast<uint16_t>(x));
        std::string bytes;
        for (uint16_t k = 0; k < 4; ++k) bytes += k < d.length ? Hex(m.Read(static_cast<uint16_t>(x + k)), 2) + " " : "   ";
        if (!out.empty()) out += '\n';
        out += Hex(x, 4) + "  " + bytes + " " + d.text;
        x += d.length;
        if (x > 0xFFFF) break;
    }
    last_list_ = static_cast<int>(x & 0xFFFF);
    if (!to_file) return out;
    std::string path = "mamute.lst";
    if (f.size() > 2 && !f[2].empty()) path = f[2];
    return WriteListing(path, "MAMUTE - DISASSEMBLY A PARTIR DE " + Hex(a, 4), out);
}

std::string Monitor::CmdX(const std::vector<std::string> &f) {
    Z80State &s = Cpu().State();
    auto all = [&]() {
        return "AF=" + Hex(s.af.w, 4) + " BC=" + Hex(s.bc.w, 4) + " DE=" + Hex(s.de.w, 4) + " HL=" + Hex(s.hl.w, 4) + "\n" + "IX=" + Hex(s.ix.w, 4) + " IY=" + Hex(s.iy.w, 4) +
               " SP=" + Hex(s.sp.w, 4) + " PC=" + Hex(s.pc.w, 4) + "\n" + "AF'=" + Hex(s.af_alt.w, 4) + " BC'=" + Hex(s.bc_alt.w, 4) + " DE'=" + Hex(s.de_alt.w, 4) +
               " HL'=" + Hex(s.hl_alt.w, 4);
    };
    if (f.empty()) return all();
    struct R16 { const char *n; Z80Pair *p; };
    const R16 r16[] = {{"AF", &s.af}, {"BC", &s.bc}, {"DE", &s.de}, {"HL", &s.hl}, {"IX", &s.ix}, {"IY", &s.iy}, {"SP", &s.sp}, {"PC", &s.pc},
                       {"AF'", &s.af_alt}, {"BC'", &s.bc_alt}, {"DE'", &s.de_alt}, {"HL'", &s.hl_alt}};
    struct R8 { const char *n; uint8_t *p; };
    const R8 r8[] = {{"A", &s.af.b.hi}, {"F", &s.af.b.lo}, {"B", &s.bc.b.hi}, {"C", &s.bc.b.lo}, {"D", &s.de.b.hi}, {"E", &s.de.b.lo}, {"H", &s.hl.b.hi}, {"L", &s.hl.b.lo}};
    const std::string r = Upper(f[0]);
    for (const R16 &x : r16) {
        if (r != x.n) continue;
        if (f.size() == 1) return r + "=" + Hex(x.p->w, 4);
        unsigned v = 0;
        if (!ParseHex(f[1], v, 4)) return "VALOR INVALIDO (0000-FFFF)";
        x.p->w = static_cast<uint16_t>(v);
        return r + "=" + Hex(v, 4);
    }
    for (const R8 &x : r8) {
        if (r != x.n) continue;
        if (f.size() == 1) return r + "=" + Hex(*x.p, 2);
        unsigned v = 0;
        if (!ParseHex(f[1], v, 2)) return "VALOR INVALIDO (00-FF)";
        *x.p = static_cast<uint8_t>(v);
        return r + "=" + Hex(v, 2);
    }
    return "REGISTRADOR DESCONHECIDO: " + r + " (AF BC DE HL IX IY SP PC AF' BC' DE' HL' A F B C D E H L)";
}

std::string Monitor::CmdG(const std::vector<std::string> &f) {
    unsigned a = 0;
    bool has = false;
    std::vector<uint16_t> bps;
    if (!f.empty() && !f[0].empty()) {
        if (!ParseHex(f[0], a)) return "SINTAXE: G [<ini>[,<bp1>[,<bp2>]]]";
        has = true;
    }
    for (size_t k = 1; k < f.size() && k < 3; ++k) {
        unsigned b = 0;
        if (f[k].empty()) continue;
        if (!ParseHex(f[k], b)) return "SINTAXE: G [<ini>[,<bp1>[,<bp2>]]]";
        bps.push_back(static_cast<uint16_t>(b));
    }
    std::string out = Cpu().Go(has, static_cast<uint16_t>(a), bps);
    if (!online()) out += "\n" + CmdX({});
    return out;
}

std::string Monitor::CmdLoad(const std::vector<std::string> &f) {
    if (f.empty() || f[0].empty()) return "SINTAXE: LOAD <arquivo>[,<slot 0-3>[,<end>]]";
    const std::string path = f[0];
    std::ifstream in(path, std::ios::binary);
    if (!in) return "NAO FOI POSSIVEL ABRIR " + path;
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    std::string ext = Upper(path.size() > 4 ? path.substr(path.size() - 4) : path);
    if (ext == ".CAS") return ".CAS AINDA NAO E' SUPORTADO";
    // slot: o informado; senao o primeiro com RAM (offline) ou o slot ativo da pagina 1 (online)
    int slot = -1;
    if (f.size() > 1 && !f[1].empty()) {
        unsigned s = 0;
        if (!ParseHex(f[1], s, 1) || s > 3) return "O SLOT VAI DE 0 A 3";
        slot = static_cast<int>(s);
    }
    IMemory &m = Mem();
    if (slot < 0) {
        slot = m.PageMap()[1];
        bool found = false;
        for (int s = 0; s < 4 && !online() && !found; ++s)
            for (int p = 0; p < 4 && !found; ++p)
                if (m.Cell(s, p) == CellType::Ram) {
                    slot = s;
                    found = true;
                }
    }
    unsigned addr = 0;
    size_t skip = 0;
    CellType kind = CellType::Ram;
    if (ext == ".ROM") {
        if (data.size() > 0x8000) return "ROM COM MAIS DE 32 KB NAO E' SUPORTADA";
        addr = 0x4000;
        kind = CellType::Rom;
    } else if (f.size() > 2 && !f[2].empty()) {
        if (!ParseHex(f[2], addr)) return "ENDERECO INVALIDO: " + f[2];
    } else if (data.size() >= 7 && data[0] == 0xFE) {
        addr = data[1] | (data[2] << 8); // cabecalho do BSAVE: FE ini fim exec
        skip = 7;
    } else {
        return "ARQUIVO SEM CABECALHO BLOAD: INFORME O ENDERECO  (LOAD <arquivo>,<slot>,<end>)";
    }
    const size_t n = data.size() - skip;
    if (addr + n > 0x10000) return "O ARQUIVO PASSA DE FFFF";
    unsigned ok = 0;
    for (size_t k = 0; k < n; ++k)
        if (m.WriteSlot(slot, static_cast<uint16_t>(addr + k), data[skip + k])) ++ok;
    if (!online()) {
        for (unsigned p = addr >> 14; p <= (addr + (n ? n - 1 : 0)) >> 14 && p < 4; ++p) offline_.SetCell(slot, static_cast<int>(p), kind);
    }
    std::string out = "CARREGADO NO SLOT " + std::to_string(slot) + " EM " + Hex(addr, 4) + " - TAMANHO " + Hex(static_cast<unsigned>(n), 4) + " - FIM " +
                      Hex(static_cast<unsigned>(addr + (n ? n - 1 : 0)) & 0xFFFF, 4);
    if (ok < n) out += " - " + std::to_string(n - ok) + " IGNORADOS (NAO E' RAM)";
    return out;
}

std::string Monitor::CmdSave(const std::vector<std::string> &f) {
    unsigned a = 0, b = 0, e = 0;
    if (f.size() < 3 || f[0].empty() || !ParseHex(f[1], a) || !ParseHex(f[2], b) || b < a) return "SINTAXE: SAVE <arquivo>,<ini>,<fim>[,<exec>[,BIN|ROM|RAW]]";
    e = a;
    if (f.size() > 3 && !f[3].empty() && !ParseHex(f[3], e)) return "SINTAXE: SAVE <arquivo>,<ini>,<fim>[,<exec>[,BIN|ROM|RAW]]";
    const std::string path = f[0];
    // formato: BIN = cabecalho BSAVE (FE ini fim exec); ROM = igual com AB (formato do Mamute original); RAW = so' os bytes.
    // Sem o formato, .ROM vira RAW (uma imagem de cartucho de verdade) e o resto BIN.
    std::string ext = Upper(path.size() > 4 ? path.substr(path.size() - 4) : path);
    std::string fmt = ext == ".ROM" ? "RAW" : "BIN";
    if (f.size() > 4 && !f[4].empty()) fmt = Upper(f[4]);
    if (fmt != "BIN" && fmt != "ROM" && fmt != "RAW") return "FORMATO: BIN, ROM OU RAW";
    IMemory &m = Mem();
    std::vector<uint8_t> out;
    if (fmt != "RAW") {
        out.push_back(fmt == "BIN" ? 0xFE : 0xAB);
        for (const unsigned v : {a, b, e}) {
            out.push_back(static_cast<uint8_t>(v & 0xFF));
            out.push_back(static_cast<uint8_t>(v >> 8));
        }
    }
    for (unsigned x = a; x <= b; ++x) out.push_back(m.Read(static_cast<uint16_t>(x)));
    std::ofstream o(path, std::ios::binary | std::ios::trunc);
    if (!o) return "NAO FOI POSSIVEL GRAVAR " + path;
    o.write(reinterpret_cast<const char *>(out.data()), static_cast<std::streamsize>(out.size()));
    return "SALVO \"" + path + "\" - " + Hex(a, 4) + "-" + Hex(b, 4) + " - TAMANHO " + Hex(b - a + 1, 4) + " - FORMATO " + fmt;
}

std::string Monitor::CmdCfg(const std::string &rest) {
    // o subcomando e' a primeira palavra (separada por espaco); o resto segue as regras normais de campos
    std::vector<std::string> f;
    if (!rest.empty()) {
        const size_t sp = rest.find_first_of(" \t");
        f.push_back(rest.substr(0, sp));
        if (sp != std::string::npos) {
            const std::vector<std::string> more = Fields(rest.substr(sp + 1));
            f.insert(f.end(), more.begin(), more.end());
        }
    }
    if (f.empty()) {
        std::string out = std::string("CONFIGURACAO INICIAL (mamute.cfg)\nABRE EM: ") + (cfg_.start_online ? "ONLINE" : "OFFLINE") + "   VRAM: " + std::to_string(cfg_.vram_kb) +
                          " KB   FORMATO DE DESPEJO: " + std::to_string(cfg_.dump_mode) + "\nSLOT PAGINA ENDERECOS   TIPO   ARQUIVO";
        for (int s = 0; s < 4; ++s)
            for (int p = 0; p < 4; ++p) {
                out += "\n  " + std::to_string(s) + "     " + std::to_string(p) + "    " + Hex(static_cast<unsigned>(p * 0x4000), 4) + "-" + Hex(static_cast<unsigned>(p * 0x4000 + 0x3FFF), 4) + "   " +
                       CellName(cfg_.cell[s][p]);
                if (!cfg_.file[s][p].empty()) out += "   " + cfg_.file[s][p] + (cfg_.half[s][p] ? " (ultimos 16 KB)" : "");
            }
        return out;
    }
    const std::string sub = Upper(f[0]);
    if (sub == "SAVE") {
        if (config_path_.empty()) return "SEM CAMINHO DE CONFIGURACAO";
        return SaveConfig(config_path_, cfg_) ? "GRAVADO " + config_path_ : "NAO FOI POSSIVEL GRAVAR " + config_path_;
    }
    if (sub == "LOAD") {
        if (config_path_.empty()) return "SEM CAMINHO DE CONFIGURACAO";
        cfg_ = LoadConfig(config_path_);
        dump_mode_ = cfg_.dump_mode;
        const std::string w = offline_.Reconfigure(cfg_);
        return "CARREGADO " + config_path_ + " (A MEMORIA OFFLINE FOI REINICIADA)" + (w.empty() ? "" : "\n" + Trim(w));
    }
    if (sub == "ONLINE" && f.size() > 1) {
        cfg_.start_online = f[1] == "1";
        return std::string("ABRE EM ") + (cfg_.start_online ? "ONLINE" : "OFFLINE") + " (CFG SAVE grava)";
    }
    if (sub == "VRAM" && f.size() > 1) {
        const int v = std::atoi(f[1].c_str());
        if (v != 16 && v != 128 && v != 192) return "VRAM: 16, 128 OU 192 (KB)";
        cfg_.vram_kb = v;
        offline_.Reconfigure(cfg_);
        return "VRAM " + std::to_string(v) + " KB (A MEMORIA OFFLINE FOI REINICIADA)";
    }
    if (sub == "DUMP" && f.size() > 1) {
        unsigned m = 0;
        if (!ParseHex(f[1], m, 1) || mamute_dump_width(static_cast<int>(m)) == 0) return "DUMP: 0-3";
        cfg_.dump_mode = static_cast<int>(m);
        dump_mode_ = cfg_.dump_mode;
        return "FORMATO INICIAL DE DESPEJO " + std::to_string(m);
    }
    if (sub == "SET") {
        unsigned s = 0, p = 0;
        CellType t;
        if (f.size() < 4 || !ParseHex(f[1], s, 1) || !ParseHex(f[2], p, 1) || s > 3 || p > 3 || !ParseCellType(f[3], t))
            return "SINTAXE: CFG SET <slot>,<pagina>,<VAZIO|RAM|ROM|BASIC>[,<arquivo>[,HALF]]";
        cfg_.cell[s][p] = t;
        cfg_.file[s][p] = f.size() > 4 ? f[4] : "";
        cfg_.half[s][p] = f.size() > 5 && Upper(f[5]) == "HALF";
        const std::string w = offline_.Reconfigure(cfg_);
        return "SLOT " + std::to_string(s) + " PAGINA " + std::to_string(p) + ": " + CellName(t) + " (A MEMORIA OFFLINE FOI REINICIADA; CFG SAVE grava)" + (w.empty() ? "" : "\n" + Trim(w));
    }
    return "SINTAXE: CFG [SET s,p,tipo[,arq[,HALF]] | VRAM n | ONLINE 0|1 | DUMP n | SAVE | LOAD]";
}

} // namespace mamute
