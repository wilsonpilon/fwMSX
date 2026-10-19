// fwMSX -- Mamute: os paineis interativos (DM, M, S, ZAP, SCR) (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver mamute_panel.h.
#include "mamute_panel.h"

#include "mamute_superx.h"
#include "mamute_target.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <set>

namespace mamute {

const char *const kDefaultSKeys = "1234qwerasdfzxcv";

namespace {

constexpr char kOn = '\x01', kOff = '\x02'; // marcadores do cursor

std::string Hex(unsigned v, int digits) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%0*X", digits, v);
    return buf;
}

char Printable(int b) { return (b >= 0x20 && b < 0x7F) ? static_cast<char>(b) : '.'; }

std::string Signed(int d) { return (d < 0 ? "-" : "+") + Hex(static_cast<unsigned>(d < 0 ? -d : d), 2); }

std::string Mark(const std::string &s) { return std::string(1, kOn) + s + std::string(1, kOff); }

int HexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

std::string Upper(std::string s) {
    for (char &c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

// "c:3" -> '3' (um caractere); false para qualquer outra tecla
bool CharKey(const std::string &key, char &c) {
    if (key == "space") { // o espaco tem nome porque as camadas de cima aparam espacos das pontas da linha
        c = ' ';
        return true;
    }
    if (key.size() == 3 && key[0] == 'c' && key[1] == ':') {
        c = key[2];
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
// A grade de 128 bytes
// ---------------------------------------------------------------------------------------------------------------------
class GridPanel : public Panel {
public:
    enum Kind { DM, M, S, ZAP, XD };

    GridPanel(Kind kind, uint32_t base, int shift, std::string skeys) : kind_(kind), base_(base), shift_(shift), skeys_(std::move(skeys)) {
        if (skeys_.size() != 16) skeys_ = kDefaultSKeys;
    }

    // XD: o alvo (slot/sub-slot/VRAM) em que a grade le e escreve
    void SetTarget(const Target &t) { target_ = t; }

    // ZAP: a imagem toda na memoria
    void SetImage(const std::string &path, std::vector<uint8_t> bytes) {
        path_ = path;
        image_ = std::move(bytes);
    }

    std::string Name() const override {
        static const char *const names[] = {"DM", "M", "S", "ZAP", "XD"};
        return names[kind_];
    }

    std::vector<std::string> Render(IMemory &mem) override {
        cur_mem_ = &mem;
        std::vector<std::string> out;
        const bool zap = kind_ == ZAP;
        const int digits = TargetDigits(target_);
        std::string title;
        switch (kind_) {
        case DM: title = "DM - MEMORIA (" + Upper(mem.Name()) + ")"; break;
        case M: title = "M - EDICAO RAPIDA (" + Upper(mem.Name()) + ")"; break;
        case S: title = "S - EDICAO RAPIDA, TECLADO S (" + Upper(mem.Name()) + ")"; break;
        case ZAP: title = "ZAP - " + BaseName(path_) + (dirty_.empty() ? "" : " *"); break;
        case XD: title = "XD - MEMORIA" + (TargetSuffix(target_).empty() ? std::string() : " " + TargetSuffix(target_)) + " (" + Upper(mem.Name()) + ")" + (ascii_mode_ ? "  DIGITANDO ASCII" : ""); break;
        }
        out.push_back(title);
        for (int row = 0; row < 16; ++row) {
            std::string hex, txt;
            for (int k = 0; k < 8; ++k) {
                const int cell = row * 8 + k;
                const int v = ReadCell(mem, cell);
                std::string h = v < 0 ? "--" : Hex(static_cast<unsigned>(v), 2);
                std::string t = v < 0 ? " " : std::string(1, Printable((v + shift_) & 0xFF));
                const bool here = cell == cursor_ && show_cursor_;
                if (here && !text_pane_) {
                    if (pending_ >= 0) h = std::string(1, "0123456789ABCDEF"[pending_]) + "_";
                    h = Mark(h);
                } else {
                    h += "";
                }
                if (here && text_pane_) t = Mark(t);
                hex += h + (k < 7 ? " " : "");
                txt += t;
            }
            out.push_back((zap ? Hex((base_ + static_cast<uint32_t>(row) * 8) % 512, 3) : Hex(Wrap(base_ + static_cast<uint32_t>(row) * 8), digits)) + "  " + hex + "  " + txt);
        }
        const uint32_t cur = base_ + static_cast<uint32_t>(cursor_);
        if (zap) out.push_back("SETOR: " + Hex(cur / 512, 3) + "  BYTE: " + Hex(cur % 512, 3) + "  DESLOC.: " + Signed(shift_));
        else out.push_back("ENDERECO: " + Hex(Wrap(base_), digits) + "  CURSOR: " + Hex(Wrap(cur), digits) + "  DESLOC.: " + Signed(shift_));
        if (editing_) out.push_back(std::string("EDITANDO (") + (text_pane_ ? "TEXTO" : "HEXA") + "): " + field_ + "_   ENTER confirma, ESC cancela");
        if (!msg_.empty()) out.push_back(msg_);
        if (kind_ == S) {
            std::string legend = "TECLAS S:";
            for (int r = 0; r < 4; ++r) {
                legend += "  ";
                for (int c = 0; c < 4; ++c) legend += std::string(1, skeys_[static_cast<size_t>(r * 4 + c)]);
                legend += "=" + std::string(1, "123456789ABCDEF0"[r * 4]) + std::string(1, "123456789ABCDEF0"[r * 4 + 1]) +
                          std::string(1, "123456789ABCDEF0"[r * 4 + 2]) + std::string(1, "123456789ABCDEF0"[r * 4 + 3]);
            }
            out.push_back(legend);
        }
        std::string help = "setas movem  TAB hexa/texto  PgUp/PgDn +-128 bytes  + - desloc.";
        if (kind_ == DM || kind_ == ZAP) help += "  ENTER edita";
        else help += "  digite 2 digitos hexa  ENTER sai";
        if (kind_ == XD) help += "  \" digita ASCII (ESC sai)  @ repete o byte anterior";
        if (zap) help += "  Ctrl+S grava o setor";
        out.push_back(help + "  ESC sai");
        return out;
    }

    bool Key(IMemory &mem, const std::string &key) override {
        cur_mem_ = &mem;
        msg_.clear();
        char ch = 0;
        const bool is_char = CharKey(key, ch);

        // ---- XD: digitacao ASCII direta (depois de \"): cada tecla impressa grava um byte e o cursor avanca ----
        if (kind_ == XD && ascii_mode_) {
            if (key == "esc") {
                ascii_mode_ = false;
                return true;
            }
            if (is_char) {
                if (!WriteCell(mem, cursor_, static_cast<uint8_t>((static_cast<unsigned char>(ch) - shift_) & 0xFF))) msg_ = "ROM/VAZIO: SOMENTE LEITURA (NADA FOI GRAVADO)";
                Advance(1);
                return true;
            }
        }

        // ---- campo de edicao aberto (DM e ZAP) ----
        if (editing_) {
            if (key == "esc") {
                editing_ = false;
                return true;
            }
            if (key == "bs") {
                if (!field_.empty()) field_.pop_back();
                fresh_ = false;
                return true;
            }
            if (key == "enter") {
                ConfirmField(mem);
                return true;
            }
            if (is_char) {
                if (fresh_) field_.clear();
                fresh_ = false;
                if (!text_pane_) {
                    if (HexValue(ch) >= 0 && field_.size() < 2) field_ += static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                } else if (ch >= 0x20 && field_.size() < 64) {
                    field_ += ch;
                }
            }
            return true;
        }

        if (key == "esc") {
            if (pending_ >= 0) {
                pending_ = -1;
                return true;
            }
            if (kind_ == ZAP && !dirty_.empty() && !warned_) {
                warned_ = true;
                msg_ = "HA' ALTERACOES NAO SALVAS (Ctrl+S grava o setor) - ESC DE NOVO DESCARTA E SAI";
                return true;
            }
            return false;
        }
        warned_ = false;
        if (key == "up") return Move(-8), true;
        if (key == "down") return Move(8), true;
        if (key == "left") return Move(-1), true;
        if (key == "right") return Move(1), true;
        if (key == "pgup") return Jump(-128), true;
        if (key == "pgdn") return Jump(128), true;
        if (key == "tab") {
            text_pane_ = !text_pane_;
            pending_ = -1;
            return true;
        }
        if (key == "save") {
            if (kind_ != ZAP) return true;
            SaveSector();
            return true;
        }
        if (key == "enter") {
            if (kind_ == M || kind_ == S || kind_ == XD) return false; // M, S e XD: RETURN sempre sai
            editing_ = true;
            fresh_ = true;
            const int v = ReadCell(mem, cursor_);
            field_ = text_pane_ || v < 0 ? "" : Hex(static_cast<unsigned>(v), 2);
            if (text_pane_) fresh_ = false;
            return true;
        }
        if (key == "plus") return AdjustShift(1), true;
        if (key == "minus") return AdjustShift(-1), true;
        if (is_char) {
            if (ch == '+') return AdjustShift(1), true;
            if (ch == '-') return AdjustShift(-1), true;
            if (kind_ == XD && ch == '"') {
                ascii_mode_ = true;
                pending_ = -1;
                return true;
            }
            if (kind_ == XD && ch == '@' && !text_pane_) { // repete o byte anterior (o do endereco - 1)
                const int prev = ReadAbs(mem, static_cast<int64_t>(base_) + cursor_ - 1);
                if (!WriteCell(mem, cursor_, static_cast<uint8_t>(prev))) msg_ = "ROM/VAZIO: SOMENTE LEITURA (NADA FOI GRAVADO)";
                Advance(1);
                return true;
            }
            if ((kind_ == M || kind_ == S || kind_ == XD) && !text_pane_) {
                int d = -1;
                if (kind_ == M || kind_ == XD) {
                    d = HexValue(ch);
                } else {
                    const size_t at = skeys_.find(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
                    const size_t at2 = at == std::string::npos ? skeys_.find(static_cast<char>(std::toupper(static_cast<unsigned char>(ch)))) : at;
                    if (at2 != std::string::npos) d = HexValue("123456789ABCDEF0"[at2]);
                }
                if (d >= 0) TypeDigit(mem, d);
            }
        }
        return true;
    }

    void ShowCursor(bool v) override { show_cursor_ = v; }

private:
    static std::string BaseName(const std::string &p) {
        const size_t s = p.find_last_of("/\\");
        return s == std::string::npos ? p : p.substr(s + 1);
    }
    // dobra o endereco dentro do alvo (modulo 64 KB; na VRAM, modulo o tamanho dela)
    uint32_t Wrap(uint32_t a) const { return cur_mem_ ? TargetWrap(*cur_mem_, target_, static_cast<int64_t>(a)) : (a & 0xFFFF); }
    int ReadAbs(IMemory &mem, int64_t addr) { return TRead(mem, target_, TargetWrap(mem, target_, addr)); }

    bool IsZap() const { return kind_ == ZAP; }

    // -1 = fora da imagem (ZAP); a memoria sempre responde
    int ReadCell(IMemory &mem, int cell) {
        const uint32_t i = base_ + static_cast<uint32_t>(cell);
        if (IsZap()) return i < image_.size() ? image_[i] : -1;
        return TRead(mem, target_, Wrap(i));
    }

    bool WriteCell(IMemory &mem, int cell, uint8_t v) {
        const uint32_t i = base_ + static_cast<uint32_t>(cell);
        if (IsZap()) {
            if (i >= image_.size()) return false;
            if (image_[i] != v) {
                image_[i] = v;
                dirty_.insert(i / 512);
            }
            return true;
        }
        return TWrite(mem, target_, Wrap(i), v);
    }

    void Move(int delta) {
        const int c = cursor_ + delta;
        if (c >= 0 && c < 128) cursor_ = c;
        pending_ = -1;
    }

    void Jump(int delta) {
        if (IsZap()) {
            const int64_t b = static_cast<int64_t>(base_) + delta;
            if (b < 0 || b >= static_cast<int64_t>(image_.size())) return;
            base_ = static_cast<uint32_t>(b);
        } else {
            base_ = TargetWrap(*cur_mem_, target_, static_cast<int64_t>(base_) + delta);
        }
        pending_ = -1;
    }

    void AdjustShift(int d) { shift_ = std::max(-0x7F, std::min(0x80, shift_ + d)); }

    // avanca o cursor depois de gravar; passando da grade, a janela anda 128 bytes
    void Advance(int n) {
        cursor_ += n;
        while (cursor_ >= 128) {
            if (IsZap() && base_ + 128 >= image_.size()) {
                cursor_ = 127;
                break;
            }
            Jump(128);
            cursor_ -= 128;
        }
    }

    void TypeDigit(IMemory &mem, int d) {
        if (pending_ < 0) {
            pending_ = d;
            return;
        }
        const uint8_t v = static_cast<uint8_t>(pending_ * 16 + d);
        pending_ = -1;
        if (!WriteCell(mem, cursor_, v)) msg_ = "ROM/VAZIO: SOMENTE LEITURA (NADA FOI GRAVADO)";
        Advance(1);
    }

    void ConfirmField(IMemory &mem) {
        editing_ = false;
        if (field_.empty()) return;
        bool ok = true;
        if (!text_pane_) {
            ok = WriteCell(mem, cursor_, static_cast<uint8_t>(HexValue(field_[0]) * (field_.size() == 2 ? 16 : 0) + HexValue(field_.back())));
            Advance(1);
        } else {
            for (size_t k = 0; k < field_.size(); ++k) {
                if (!WriteCell(mem, cursor_ + static_cast<int>(k), static_cast<uint8_t>((static_cast<unsigned char>(field_[k]) - shift_) & 0xFF))) ok = false;
            }
            Advance(static_cast<int>(field_.size()));
        }
        if (!ok) msg_ = IsZap() ? "FORA DA IMAGEM" : "ROM/VAZIO: SOMENTE LEITURA (NADA FOI GRAVADO)";
    }

    void SaveSector() {
        const size_t sector = (base_ + static_cast<uint32_t>(cursor_)) / 512;
        if (!dirty_.count(sector)) {
            msg_ = "SETOR " + Hex(static_cast<unsigned>(sector), 3) + " SEM ALTERACOES";
            return;
        }
        std::fstream f(path_, std::ios::in | std::ios::out | std::ios::binary);
        if (!f) {
            msg_ = "NAO FOI POSSIVEL GRAVAR " + path_;
            return;
        }
        f.seekp(static_cast<std::streamoff>(sector * 512));
        const size_t n = std::min<size_t>(512, image_.size() - sector * 512);
        f.write(reinterpret_cast<const char *>(image_.data() + sector * 512), static_cast<std::streamsize>(n));
        if (!f) {
            msg_ = "ERRO AO GRAVAR " + path_;
            return;
        }
        dirty_.erase(sector);
        msg_ = "SETOR " + Hex(static_cast<unsigned>(sector), 3) + " GRAVADO";
    }

    Kind kind_;
    uint32_t base_;
    int cursor_ = 0;
    bool text_pane_ = false;
    int shift_;
    std::string skeys_;
    bool editing_ = false, fresh_ = false;
    std::string field_;
    int pending_ = -1;
    std::string msg_;
    bool show_cursor_ = true;
    // XD: o alvo, e a digitacao ASCII direta
    Target target_;
    bool ascii_mode_ = false;
    IMemory *cur_mem_ = nullptr; // a memoria da chamada em curso (o painel nao guarda o IMemory: o adaptador da maquina e' temporario)
    // ZAP
    std::string path_;
    std::vector<uint8_t> image_;
    std::set<size_t> dirty_;
    bool warned_ = false;
};

// ---------------------------------------------------------------------------------------------------------------------
// SCR: a memoria como uma tela de 256x192
// ---------------------------------------------------------------------------------------------------------------------
class ScreenPanel : public Panel {
public:
    ScreenPanel(uint16_t addr, int dx, int dy, int mode, int zoom) : addr_(addr), dx_(std::max(1, dx)), dy_(std::max(1, dy)), mode_(mode), zoom_(zoom == 1 ? 1 : 2) {}

    std::string Name() const override { return "SCR"; }

    std::vector<std::string> Render(IMemory &mem) override {
        std::vector<std::string> out;
        out.push_back("SCR - " + Hex(addr_, 4) + "," + Hex(static_cast<unsigned>(dx_), 2) + "," + Hex(static_cast<unsigned>(dy_), 2) + "," + std::to_string(mode_) + " (" +
                      Upper(mem.Name()) + ")" + (label_ ? "  ENDERECO BASE: " + Hex(addr_, 4) : ""));
        // um caractere braille cobre 2x4 pontos; no zoom 2 cada ponto e' um bloco 2x2 de pixels (OU)
        const int cols = zoom_ == 1 ? 128 : 64, rows = zoom_ == 1 ? 48 : 24;
        const int step = zoom_ == 1 ? 1 : 2; // pixels por ponto
        const int fcols = zoom_ == 1 ? 8 : 4, frows = zoom_ == 1 ? 4 : 2; // a moldura de 16x16 pixels, em caracteres
        static const int kBit[4][2] = {{0, 3}, {1, 4}, {2, 5}, {6, 7}}; // [linha do ponto][coluna do ponto]
        for (int r = 0; r < rows; ++r) {
            std::string line;
            bool inside = false;
            for (int c = 0; c < cols; ++c) {
                int bits = 0;
                for (int j = 0; j < 4; ++j) {
                    for (int i = 0; i < 2; ++i) {
                        bool lit = false;
                        for (int yy = 0; yy < step && !lit; ++yy)
                            for (int xx = 0; xx < step && !lit; ++xx) lit = Pixel(mem, (c * 2 + i) * step + xx, (r * 4 + j) * step + yy);
                        if (lit) bits |= 1 << kBit[j][i];
                    }
                }
                const bool in_frame = frame_ && c < fcols && r < frows;
                if (in_frame && !inside) line += kOn;
                if (!in_frame && inside) line += kOff;
                inside = in_frame;
                Utf8(0x2800 + static_cast<unsigned>(bits), line);
            }
            if (inside) line += kOff;
            out.push_back(line);
        }
        if (edit_) {
            out.push_back("EDICAO 16x16 (" + std::string(ro_ ? "SOMENTE LEITURA - NADA E' GRAVADO" : "gravando pixel a pixel") + ")");
            for (int y = 0; y < 16; ++y) {
                std::string line;
                for (int x = 0; x < 16; ++x) {
                    std::string cell = Pixel(mem, x, y) ? "##" : ". ";
                    line += (x == px_ && y == py_) ? Mark(cell) : cell;
                }
                out.push_back(line);
            }
            out.push_back("setas movem  ESPACO inverte o pixel  I inverte tudo  L apaga tudo  ENTER sai  ESC cancela tudo");
        } else {
            out.push_back("setas: esq/dir = 1 byte, cima/baixo = 1 azulejo (" + Hex(static_cast<unsigned>(dx_ * dy_ * 8), 4) + " bytes)  TAB moldura  E endereco  Z zoom  ENTER edita  ESC sai");
        }
        return out;
    }

    bool Key(IMemory &mem, const std::string &key) override {
        char ch = 0;
        const bool is_char = CharKey(key, ch);
        const char lower = is_char ? static_cast<char>(std::tolower(static_cast<unsigned char>(ch))) : 0;
        if (edit_) {
            if (key == "esc") {
                for (const auto &p : snapshot_) mem.Write(p.first, p.second);
                edit_ = false;
                return true;
            }
            if (key == "enter") {
                edit_ = false;
                return true;
            }
            if (key == "up") py_ = std::max(0, py_ - 1);
            else if (key == "down") py_ = std::min(15, py_ + 1);
            else if (key == "left") px_ = std::max(0, px_ - 1);
            else if (key == "right") px_ = std::min(15, px_ + 1);
            else if (is_char && ch == ' ') FlipPixel(mem, px_, py_);
            else if (is_char && lower == 'i') {
                for (const auto &p : snapshot_) Put(mem, p.first, static_cast<uint8_t>(~mem.Read(p.first)));
            } else if (is_char && lower == 'l') {
                for (const auto &p : snapshot_) Put(mem, p.first, 0);
            }
            return true;
        }
        if (key == "esc") return false;
        const int tile = dx_ * dy_ * 8;
        if (key == "left") addr_ = static_cast<uint16_t>(addr_ - 1);
        else if (key == "right") addr_ = static_cast<uint16_t>(addr_ + 1);
        else if (key == "up") addr_ = static_cast<uint16_t>(addr_ - tile);
        else if (key == "down") addr_ = static_cast<uint16_t>(addr_ + tile);
        else if (key == "tab") frame_ = !frame_;
        else if (key == "enter") {
            edit_ = true;
            px_ = py_ = 0;
            ro_ = false;
            snapshot_.clear();
            for (int cy = 0; cy < 2; ++cy)
                for (int cx = 0; cx < 2; ++cx)
                    for (int b = 0; b < 8; ++b) {
                        const uint16_t a = static_cast<uint16_t>(CharAddr(cx, cy) + b);
                        snapshot_.emplace_back(a, mem.Read(a));
                    }
        } else if (is_char && lower == 'e') label_ = !label_;
        else if (is_char && lower == 'z') zoom_ = zoom_ == 1 ? 2 : 1;
        return true;
    }

private:
    static void Utf8(unsigned cp, std::string &out) {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }

    // o endereco do caractere (cx, cy) da tela de 32x24: azulejos de dx x dy caracteres, lidos por linha (modo 0) ou por coluna (modo 1)
    uint16_t CharAddr(int cx, int cy) const {
        const int per_row = (32 + dx_ - 1) / dx_;
        const int tile = (cy / dy_) * per_row + cx / dx_;
        const int ix = cx % dx_, iy = cy % dy_;
        const int idx = mode_ == 0 ? iy * dx_ + ix : ix * dy_ + iy;
        return static_cast<uint16_t>(addr_ + (tile * dx_ * dy_ + idx) * 8);
    }

    bool Pixel(IMemory &mem, int x, int y) const {
        if (x < 0 || x >= 256 || y < 0 || y >= 192) return false;
        const uint8_t b = mem.Read(static_cast<uint16_t>(CharAddr(x / 8, y / 8) + (y % 8)));
        return (b >> (7 - (x % 8))) & 1;
    }

    void Put(IMemory &mem, uint16_t a, uint8_t v) {
        if (!mem.Write(a, v)) ro_ = true;
    }

    void FlipPixel(IMemory &mem, int x, int y) {
        const uint16_t a = static_cast<uint16_t>(CharAddr(x / 8, y / 8) + (y % 8));
        Put(mem, a, static_cast<uint8_t>(mem.Read(a) ^ (0x80 >> (x % 8))));
    }

    uint16_t addr_;
    int dx_, dy_, mode_, zoom_;
    bool frame_ = false, label_ = false, edit_ = false, ro_ = false;
    int px_ = 0, py_ = 0;
    std::vector<std::pair<uint16_t, uint8_t>> snapshot_;
};

} // namespace

std::unique_ptr<Panel> MakeMemoryGrid(char kind, uint16_t addr, int shift, const std::string &skeys) {
    const GridPanel::Kind k = kind == 'M' ? GridPanel::M : kind == 'S' ? GridPanel::S : GridPanel::DM;
    return std::make_unique<GridPanel>(k, addr, shift, skeys);
}

std::unique_ptr<Panel> MakeXdPanel(const SxAddr &a, int shift) {
    auto p = std::make_unique<GridPanel>(GridPanel::XD, a.addr, shift, kDefaultSKeys);
    p->SetTarget(a.t);
    return p;
}

std::unique_ptr<Panel> MakeDiskGrid(const std::string &path, uint32_t sector, int shift, std::string &error) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        error = "NAO FOI POSSIVEL ABRIR " + path;
        return nullptr;
    }
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (static_cast<uint64_t>(sector) * 512 >= bytes.size()) {
        error = "O SETOR " + Hex(sector, 1) + " ESTA' FORA DA IMAGEM (" + std::to_string(bytes.size() / 512) + " SETORES)";
        return nullptr;
    }
    auto p = std::make_unique<GridPanel>(GridPanel::ZAP, sector * 512, shift, kDefaultSKeys);
    p->SetImage(path, std::move(bytes));
    return p;
}

std::unique_ptr<Panel> MakeScreenPanel(uint16_t addr, int dx, int dy, int mode, int zoom) {
    return std::make_unique<ScreenPanel>(addr, dx, dy, mode, zoom);
}

std::vector<std::string> PlainLines(const std::vector<std::string> &lines) {
    std::vector<std::string> out;
    for (const std::string &l : lines) {
        std::string s;
        for (const char c : l) s += c == kOn ? '[' : c == kOff ? ']' : c;
        out.push_back(std::move(s));
    }
    return out;
}

} // namespace mamute
