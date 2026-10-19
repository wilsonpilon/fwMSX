// fwMSX -- Mamute: as telas (DM, M, S, ZAP, SCR) dentro do monitor (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md, "Fase 3".
//
// Dois jeitos de usar: com PAINEIS ligados (a TUI) o comando abre um painel que recebe as teclas de verdade (`@key`); sem eles (console, scripts,
// o TCP) e' uma versao de LINHA: o comando mostra a grade/tela uma vez e as mudancas sao feitas por argumentos.
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>

#include "mamute_monitor.h"

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

bool ParseHexN(const std::string &str, unsigned &out, int max_digits) {
    const std::string s = Trim(str);
    if (s.empty() || static_cast<int>(s.size()) > max_digits) return false;
    unsigned v = 0;
    for (const char c : s) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
        v = v * 16 + static_cast<unsigned>(std::isdigit(static_cast<unsigned char>(c)) ? c - '0' : std::toupper(static_cast<unsigned char>(c)) - 'A' + 10);
    }
    out = v;
    return true;
}

bool ParseShiftN(const std::string &str, int &out) {
    std::string t = Trim(str);
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
    if (!ParseHexN(t, v, 2)) return false;
    const int d = sign * static_cast<int>(v);
    if (d < -0x7F || d > 0x80) return false;
    out = d;
    return true;
}

std::string Join(const std::vector<std::string> &v) {
    std::string out;
    for (const std::string &l : v) out += (out.empty() ? "" : "\n") + l;
    return out;
}

} // namespace

std::string Monitor::PanelText() {
    const std::vector<std::string> lines = panel_->Render(Mem());
    if (!panels_) return Join(PlainLines(lines));
    return "action=show panel=mamute title=" + panel_->Name() + "\n" + Join(lines);
}

std::string Monitor::OpenPanel(std::unique_ptr<Panel> panel) {
    panel_ = std::move(panel);
    return PanelText();
}

std::string Monitor::CmdInternal(const std::string &line) {
    // "@panels on|off" -- o hospedeiro avisa se sabe desenhar paineis (a TUI liga)
    // "@key <tecla>"   -- uma tecla para o painel aberto (a resposta e' o painel de novo, ou action=close)
    // "@panel"         -- o painel aberto, como esta' agora
    std::string w, rest;
    {
        const std::string t = Trim(line);
        const size_t sp = t.find_first_of(" \t");
        w = t.substr(0, sp);
        rest = sp == std::string::npos ? std::string() : Trim(t.substr(sp + 1));
    }
    if (w == "@panels") {
        panels_ = Upper(rest) == "ON";
        if (!panels_) panel_.reset();
        return "";
    }
    if (w == "@key") {
        if (!panel_) return "action=close panel=mamute";
        if (!panel_->Key(Mem(), rest)) {
            panel_.reset();
            return "action=close panel=mamute";
        }
        return PanelText();
    }
    if (w == "@panel") return panel_ ? PanelText() : "action=close panel=mamute";
    return "COMANDO INTERNO DESCONHECIDO: " + w;
}

std::string Monitor::CmdS(const std::vector<std::string> &f) {
    unsigned a = 0;
    if (f.empty() || !ParseHexN(f[0], a, 4)) return "SINTAXE: S <end>[,<teclas>]  (as teclas do teclado S: pares de teclas = bytes; CFG SKEYS muda o teclado)";
    if (panels_ && f.size() == 1) return OpenPanel(MakeMemoryGrid('S', static_cast<uint16_t>(a), 0, cfg_.skeys));
    if (f.size() == 1) return CmdDm(f);
    // linha: cada duas teclas formam um byte (a mesma tabela do painel: 1234 qwer asdf zxcv = 1234 5678 9ABC DEF0)
    std::string keys;
    for (size_t k = 1; k < f.size(); ++k) keys += f[k];
    std::vector<uint8_t> bytes;
    int pending = -1;
    for (const char c : keys) {
        if (std::isspace(static_cast<unsigned char>(c))) continue;
        size_t at = cfg_.skeys.find(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        if (at == std::string::npos) at = cfg_.skeys.find(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
        if (at == std::string::npos) return std::string("TECLA FORA DO TECLADO S: '") + c + "' (" + cfg_.skeys + ")";
        const int d = "123456789ABCDEF0"[at] <= '9' ? "123456789ABCDEF0"[at] - '0' : "123456789ABCDEF0"[at] - 'A' + 10;
        if (pending < 0) pending = d;
        else {
            bytes.push_back(static_cast<uint8_t>(pending * 16 + d));
            pending = -1;
        }
    }
    if (pending >= 0) return "NUMERO IMPAR DE TECLAS: FALTA O SEGUNDO DIGITO DO ULTIMO BYTE";
    if (bytes.empty()) return "SINTAXE: S <end>,<teclas>";
    unsigned ok = 0;
    for (size_t k = 0; k < bytes.size(); ++k)
        if (Mem().Write(static_cast<uint16_t>(a + k), bytes[k])) ++ok;
    std::string out = "GRAVADOS " + std::to_string(ok) + " BYTES EM " + [&] {
        char b[8];
        std::snprintf(b, sizeof(b), "%04X", a);
        return std::string(b);
    }();
    if (ok < bytes.size()) out += " - " + std::to_string(bytes.size() - ok) + " IGNORADOS (NAO E' RAM)";
    return out;
}

std::string Monitor::CmdZap(const std::vector<std::string> &f) {
    unsigned sector = 0;
    int shift = 0;
    if (f.size() < 2 || f[0].empty() || !ParseHexN(f[1], sector, 6) || (f.size() > 2 && !ParseShiftN(f[2], shift))) return "SINTAXE: ZAP <arquivo.dsk>,<setor>[,<desloc>]  (setor em hexa)";
    std::string error;
    std::unique_ptr<Panel> p = MakeDiskGrid(f[0], sector, shift, error);
    if (!p) return error;
    if (panels_) return OpenPanel(std::move(p));
    p->ShowCursor(false);
    return Join(PlainLines(p->Render(Mem()))) + "\n(ZAPW arq,setor,byte,b1,b2... grava; o painel interativo vive na TUI)";
}

std::string Monitor::CmdZapW(const std::vector<std::string> &f) {
    unsigned sector = 0, at = 0;
    if (f.size() < 4 || f[0].empty() || !ParseHexN(f[1], sector, 6) || !ParseHexN(f[2], at, 3) || at >= 512) return "SINTAXE: ZAPW <arquivo.dsk>,<setor>,<byte 0-1FF>,<b1>[,<b2>...]";
    std::vector<uint8_t> bytes;
    for (size_t k = 3; k < f.size(); ++k) {
        unsigned v = 0;
        if (!ParseHexN(f[k], v, 2)) return "BYTE INVALIDO: '" + f[k] + "' (00-FF)";
        bytes.push_back(static_cast<uint8_t>(v));
    }
    std::fstream io(f[0], std::ios::in | std::ios::out | std::ios::binary);
    if (!io) return "NAO FOI POSSIVEL ABRIR " + f[0];
    io.seekg(0, std::ios::end);
    const uint64_t size = static_cast<uint64_t>(io.tellg());
    const uint64_t pos = static_cast<uint64_t>(sector) * 512 + at;
    if (pos + bytes.size() > size) return "FORA DA IMAGEM (" + std::to_string(size / 512) + " SETORES)";
    io.seekp(static_cast<std::streamoff>(pos));
    io.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!io) return "ERRO AO GRAVAR " + f[0];
    char b[64];
    std::snprintf(b, sizeof(b), "GRAVADOS %zu BYTES NO SETOR %X, BYTE %03X", bytes.size(), sector, at);
    return b;
}

std::string Monitor::CmdScr(const std::vector<std::string> &f) {
    unsigned a = 0, dx = 0, dy = 0, mode = 0, zoom = 2;
    if (f.size() < 3 || !ParseHexN(f[0], a, 4) || !ParseHexN(f[1], dx, 2) || !ParseHexN(f[2], dy, 2) || dx < 1 || dy < 1 || dx > 32 || dy > 24)
        return "SINTAXE: SCR <end>,<dx>,<dy>[,<modo 0|1>[,<zoom 1|2>]]  (dx 1-20 e dy 1-18 em hexa: o azulejo de dx x dy caracteres)";
    if (f.size() > 3 && !f[3].empty() && (!ParseHexN(f[3], mode, 1) || mode > 1)) return "O MODO E' 0 (HORIZONTAL) OU 1 (VERTICAL)";
    if (f.size() > 4 && !f[4].empty() && (!ParseHexN(f[4], zoom, 1) || (zoom != 1 && zoom != 2))) return "O ZOOM E' 1 (128x48 COLUNAS) OU 2 (64x24)";
    std::unique_ptr<Panel> p = MakeScreenPanel(static_cast<uint16_t>(a), static_cast<int>(dx), static_cast<int>(dy), static_cast<int>(mode), static_cast<int>(zoom));
    if (panels_) return OpenPanel(std::move(p));
    return Join(PlainLines(p->Render(Mem())));
}

} // namespace mamute
