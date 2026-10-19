// fwMSX -- Mamute: os paineis do SUPER-X (XA, XI, XH, XIR, XPP, XTP, XTR) (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver mamute_superx.h e doc/mamute-spec.md, "Fase 4". Cada um era uma janela no Mamute do Paleobasic; aqui e' um MODELO sem tela (veja
// mamute_panel.h): recebe teclas por nome e devolve as linhas.
#include <algorithm>
#include <cctype>
#include <regex>

#include "../../z80/debug/z80_disasm.h"
#include "mamute_superx.h"
#include "mamute_util.h"

namespace mamute {

using util::Hex;
using util::Printable;
using util::Upper;

namespace {

constexpr char kOn = '\x01', kOff = '\x02'; // marcadores do cursor (ver mamute_panel.h)

std::string Mark(const std::string &s) { return std::string(1, kOn) + s + std::string(1, kOff); }

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// "c:3" -> '3'; o espaco tem nome porque as camadas de cima aparam espacos das pontas da linha
bool CharKey(const std::string &key, char &c) {
    if (key == "space") {
        c = ' ';
        return true;
    }
    if (key.size() == 3 && key[0] == 'c' && key[1] == ':') {
        c = key[2];
        return true;
    }
    return false;
}

std::string Where(const Target &t) { return TargetSuffix(t).empty() ? std::string() : " " + TargetSuffix(t); }

// ---------------------------------------------------------------------------------------------------------------------
// A busca dos paineis de texto: com ou sem distincao de maiusculas, e como expressao regular ou texto simples.
// ---------------------------------------------------------------------------------------------------------------------
struct Finder {
    std::string query;
    bool match_case = false;
    bool use_regex = false;

    // 1 = achou, 0 = nao, -1 = expressao regular invalida
    int Match(const std::string &hay) const {
        if (query.empty()) return 0;
        if (use_regex) {
            try {
                const std::regex re(query, match_case ? std::regex::ECMAScript : (std::regex::ECMAScript | std::regex::icase));
                return std::regex_search(hay, re) ? 1 : 0;
            } catch (const std::regex_error &) {
                return -1;
            }
        }
        return match_case ? (hay.find(query) != std::string::npos ? 1 : 0) : (Lower(hay).find(Lower(query)) != std::string::npos ? 1 : 0);
    }
    std::string Flags() const { return std::string(match_case ? "[Case]" : "[case]") + " " + (use_regex ? "[Regex]" : "[regex]"); }
};

// ---------------------------------------------------------------------------------------------------------------------
// XA: 256 bytes como texto ASCII, 16 x 16; qualquer tecla impressa grava o byte e o cursor avanca
// ---------------------------------------------------------------------------------------------------------------------
class XaPanel : public Panel {
public:
    explicit XaPanel(const SxAddr &a) : base_(a.addr), t_(a.t) {}
    std::string Name() const override { return "XA"; }

    std::vector<std::string> Render(IMemory &mem) override {
        cur_mem_ = &mem;
        const int digits = TargetDigits(t_);
        std::vector<std::string> out;
        out.push_back("XA - ASCII" + Where(t_) + " (" + Upper(mem.Name()) + ")");
        for (int row = 0; row < 16; ++row) {
            std::string txt;
            for (int col = 0; col < 16; ++col) {
                const std::string ch(1, Printable(TRead(mem, t_, TargetWrap(mem, t_, static_cast<int64_t>(base_) + row * 16 + col))));
                txt += (row == row_ && col == col_ && show_cursor_) ? Mark(ch) : ch;
            }
            out.push_back(Hex(TargetWrap(mem, t_, static_cast<int64_t>(base_) + row * 16), digits) + "  " + txt);
        }
        const uint32_t cur = TargetWrap(mem, t_, static_cast<int64_t>(base_) + row_ * 16 + col_);
        out.push_back("ENDERECO: " + Hex(base_, digits) + "  CURSOR: " + Hex(cur, digits) + "  BYTE: " + Hex(TRead(mem, t_, cur), 2));
        if (!msg_.empty()) out.push_back(msg_);
        out.push_back("setas movem  PgUp/PgDn +-256 bytes  qualquer tecla imprimivel grava o byte  ENTER/ESC sai");
        return out;
    }

    bool Key(IMemory &mem, const std::string &key) override {
        cur_mem_ = &mem;
        msg_.clear();
        char ch = 0;
        if (key == "esc" || key == "enter") return false;
        if (key == "up") Move(-16);
        else if (key == "down") Move(16);
        else if (key == "left") Move(-1);
        else if (key == "right") Move(1);
        else if (key == "pgup") base_ = TargetWrap(mem, t_, static_cast<int64_t>(base_) - 256);
        else if (key == "pgdn") base_ = TargetWrap(mem, t_, static_cast<int64_t>(base_) + 256);
        else if (CharKey(key, ch) && ch >= 0x20) { // aqui nao ha' o atalho "@" nem deslocamento: o texto e' o byte cru
            const uint32_t at = TargetWrap(mem, t_, static_cast<int64_t>(base_) + row_ * 16 + col_);
            if (!TWrite(mem, t_, at, static_cast<uint8_t>(ch))) msg_ = "ROM/VAZIO: SOMENTE LEITURA (NADA FOI GRAVADO)";
            Move(1);
        }
        return true;
    }
    void ShowCursor(bool v) override { show_cursor_ = v; }

private:
    void Move(int delta) {
        int pos = row_ * 16 + col_ + delta;
        if (pos < 0) pos = 0;
        if (pos > 255) {
            base_ = TargetWrap(*cur_mem_, t_, static_cast<int64_t>(base_) + 256); // passou do fim da tela: rola uma tela
            pos -= 256;
        }
        row_ = pos / 16;
        col_ = pos % 16;
    }

    uint32_t base_;
    Target t_;
    int row_ = 0, col_ = 0;
    bool show_cursor_ = true;
    std::string msg_;
    IMemory *cur_mem_ = nullptr;
};

// ---------------------------------------------------------------------------------------------------------------------
// XI: o disassembly rolando (so' leitura)
// ---------------------------------------------------------------------------------------------------------------------
class XiPanel : public Panel {
public:
    explicit XiPanel(const SxAddr &a) : base_(a.addr), t_(a.t) {}
    std::string Name() const override { return "XI"; }

    std::vector<std::string> Render(IMemory &mem) override {
        const int digits = TargetDigits(t_);
        std::vector<std::string> out;
        out.push_back("XI - DISASSEMBLY" + Where(t_) + " (" + Upper(mem.Name()) + ")");
        uint32_t a = base_;
        for (int n = 0; n < kRows; ++n) {
            const DisasmLine d = DisasmAt(mem, t_, a);
            out.push_back(FormatDisasm(d, digits));
            a = TargetWrap(mem, t_, static_cast<int64_t>(a) + d.len);
        }
        next_ = a;
        width_ = static_cast<int>((static_cast<int64_t>(next_) - base_ + TargetMax(mem, t_) + 1) % (static_cast<int64_t>(TargetMax(mem, t_)) + 1));
        out.push_back("ENDERECO BASE: " + Hex(base_, digits) + "  PROXIMO: " + Hex(next_, digits));
        out.push_back("cima/baixo = 1 byte (acha o alinhamento)  PgDn = proxima tela  PgUp = volta a largura da tela  ESC/ENTER sai");
        return out;
    }

    bool Key(IMemory &mem, const std::string &key) override {
        if (key == "esc" || key == "enter") return false;
        if (key == "up") base_ = TargetWrap(mem, t_, static_cast<int64_t>(base_) - 1);
        else if (key == "down") base_ = TargetWrap(mem, t_, static_cast<int64_t>(base_) + 1);
        else if (key == "pgdn") {
            Render(mem); // recalcula `next_` a partir da base atual
            base_ = next_;
        } else if (key == "pgup") {
            Render(mem);
            // nao da' para decodificar Z80 "para tras" de forma confiavel: volta a mesma largura em bytes que a tela atual cobre
            base_ = TargetWrap(mem, t_, static_cast<int64_t>(base_) - std::max(1, width_));
        }
        return true;
    }

private:
    static constexpr int kRows = 20;
    uint32_t base_;
    Target t_;
    uint32_t next_ = 0;
    int width_ = 0;
};

// ---------------------------------------------------------------------------------------------------------------------
// XH: 4 caracteres (ou sprites) 8x8 consecutivos, pixel a pixel: 2 em cima, 2 embaixo
// ---------------------------------------------------------------------------------------------------------------------
class XhPanel : public Panel {
public:
    explicit XhPanel(const SxAddr &a) : base_(a.addr), t_(a.t) {}
    std::string Name() const override { return "XH"; }

    std::vector<std::string> Render(IMemory &mem) override {
        const int digits = TargetDigits(t_);
        std::vector<std::string> out;
        out.push_back("XH - CARACTERES/SPRITES" + Where(t_) + " (" + Upper(mem.Name()) + ")");
        out.push_back("    0123456789ABCDEF");
        for (int y = 0; y < 16; ++y) {
            std::string row;
            for (int x = 0; x < 16; ++x) {
                const std::string px = Pixel(mem, x, y) ? "0" : "-";
                row += (x == cx_ && y == cy_ && show_cursor_) ? Mark(px) : px;
            }
            const int line = y % 8, left = (y / 8) * 2;
            const uint32_t a = Addr(mem, left * 8 + line);
            out.push_back("    " + row + "  " + Hex(a, digits) + " : " + Hex(TRead(mem, t_, a), 2) + ":" + Hex(TRead(mem, t_, Addr(mem, (left + 1) * 8 + line)), 2) + " " + std::to_string(line));
        }
        out.push_back("MINIATURA (os 4 caracteres montados, 16x16 pixels):");
        // um caractere braille cobre 2 x 4 pontos
        static const int kBit[4][2] = {{0, 3}, {1, 4}, {2, 5}, {6, 7}};
        for (int r = 0; r < 4; ++r) {
            std::string line = "    ";
            for (int c = 0; c < 8; ++c) {
                int bits = 0;
                for (int j = 0; j < 4; ++j)
                    for (int i = 0; i < 2; ++i)
                        if (Pixel(mem, c * 2 + i, r * 4 + j)) bits |= 1 << kBit[j][i];
                const unsigned cp = 0x2800 + static_cast<unsigned>(bits);
                line += static_cast<char>(0xE0 | (cp >> 12));
                line += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                line += static_cast<char>(0x80 | (cp & 0x3F));
            }
            out.push_back(line);
        }
        out.push_back("ENDERECO: " + Hex(base_, digits) + "  CURSOR: " + std::to_string(cx_) + "," + std::to_string(cy_));
        if (!msg_.empty()) out.push_back(msg_);
        out.push_back("setas movem  ESPACO inverte o pixel  I inverte o caractere  L apaga  F preenche  B apaga os 4  PgUp/PgDn +-32 bytes  ESC sai");
        return out;
    }

    bool Key(IMemory &mem, const std::string &key) override {
        msg_.clear();
        char ch = 0;
        if (key == "esc" || key == "enter") return false;
        if (key == "up") cy_ = std::max(0, cy_ - 1);
        else if (key == "down") cy_ = std::min(15, cy_ + 1);
        else if (key == "left") cx_ = std::max(0, cx_ - 1);
        else if (key == "right") cx_ = std::min(15, cx_ + 1);
        else if (key == "pgup") base_ = TargetWrap(mem, t_, static_cast<int64_t>(base_) - 32);
        else if (key == "pgdn") base_ = TargetWrap(mem, t_, static_cast<int64_t>(base_) + 32);
        else if (CharKey(key, ch)) {
            const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            const int which = (cy_ / 8) * 2 + cx_ / 8; // o caractere sob o cursor
            if (c == ' ') {
                const int line = cy_ % 8;
                Put(mem, which * 8 + line, static_cast<uint8_t>(Byte(mem, which * 8 + line) ^ (0x80 >> (cx_ % 8))));
            } else if (c == 'i') {
                for (int b = 0; b < 8; ++b) Put(mem, which * 8 + b, static_cast<uint8_t>(~Byte(mem, which * 8 + b)));
            } else if (c == 'l') {
                for (int b = 0; b < 8; ++b) Put(mem, which * 8 + b, 0);
            } else if (c == 'f') {
                for (int b = 0; b < 8; ++b) Put(mem, which * 8 + b, 0xFF);
            } else if (c == 'b') {
                for (int b = 0; b < 32; ++b) Put(mem, b, 0);
            }
        }
        return true;
    }
    void ShowCursor(bool v) override { show_cursor_ = v; }

private:
    uint32_t Addr(IMemory &mem, int off) const { return TargetWrap(mem, t_, static_cast<int64_t>(base_) + off); }
    uint8_t Byte(IMemory &mem, int off) const { return TRead(mem, t_, Addr(mem, off)); }
    bool Pixel(IMemory &mem, int x, int y) const { return (Byte(mem, ((y / 8) * 2 + x / 8) * 8 + y % 8) >> (7 - x % 8)) & 1; }
    void Put(IMemory &mem, int off, uint8_t v) {
        if (!TWrite(mem, t_, Addr(mem, off), v)) msg_ = "ROM/VAZIO: SOMENTE LEITURA (NADA FOI GRAVADO)";
    }

    uint32_t base_;
    Target t_;
    int cx_ = 0, cy_ = 0;
    bool show_cursor_ = true;
    std::string msg_;
};

// ---------------------------------------------------------------------------------------------------------------------
// XIR: as notas em memoria, uma de cada vez, com busca (com ou sem Case, com ou sem expressao regular)
// ---------------------------------------------------------------------------------------------------------------------
class NotesPanel : public Panel {
public:
    NotesPanel(std::vector<Note> *notes, int first) : notes_(notes), idx_(first < 0 ? 0 : first) {}
    std::string Name() const override { return "XIR"; }

    std::vector<std::string> Render(IMemory &) override {
        std::vector<std::string> out;
        const int total = static_cast<int>(notes_->size());
        if (idx_ >= total) idx_ = total ? total - 1 : 0;
        out.push_back("XIR - NOTAS (" + std::string(total ? std::to_string(idx_ + 1) : "0") + "/" + std::to_string(total) + ")");
        if (!total) {
            out.push_back("NENHUMA NOTA NA MEMORIA (XIL carrega um arquivo, XIM adiciona uma)");
        } else {
            const Note &n = (*notes_)[static_cast<size_t>(idx_)];
            out.push_back("ENDERECO: " + Hex(n.addr, 4));
            out.push_back(std::string("SLOT    : ") + NoteSlotName(n.slot));
            out.push_back(std::string("TIPO    : ") + NoteTypeName(n.type));
            out.push_back("TEXTO   : " + n.text);
        }
        out.push_back("BUSCAR  : " + (in_query_ ? Mark(f_.query + "_") : f_.query) + "   " + f_.Flags());
        if (!msg_.empty()) out.push_back(msg_);
        out.push_back(in_query_ ? "digite o texto  ENTER busca a proxima  ESC volta" : "cima/esq anterior  baixo/dir proxima  Home/End  PgUp/PgDn +-10  / busca  c Case  r Regex  n proxima  ESC sai");
        return out;
    }

    bool Key(IMemory &, const std::string &key) override {
        msg_.clear();
        const int total = static_cast<int>(notes_->size());
        char ch = 0;
        if (in_query_) {
            if (key == "esc" || key == "tab") in_query_ = false;
            else if (key == "enter") Search();
            else if (key == "bs") {
                if (!f_.query.empty()) f_.query.pop_back();
            } else if (CharKey(key, ch) && ch >= 0x20 && f_.query.size() < 60) f_.query += ch;
            return true;
        }
        if (key == "esc" || key == "enter") return false;
        if (key == "up" || key == "left") idx_ = std::max(0, idx_ - 1);
        else if (key == "down" || key == "right") idx_ = std::min(std::max(0, total - 1), idx_ + 1);
        else if (key == "home") idx_ = 0;
        else if (key == "end") idx_ = std::max(0, total - 1);
        else if (key == "pgup") idx_ = std::max(0, idx_ - 10);
        else if (key == "pgdn") idx_ = std::min(std::max(0, total - 1), idx_ + 10);
        else if (key == "tab") in_query_ = true;
        else if (CharKey(key, ch)) {
            const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            if (c == '/') in_query_ = true;
            else if (c == 'c') f_.match_case = !f_.match_case;
            else if (c == 'r') f_.use_regex = !f_.use_regex;
            else if (c == 'n') Search();
        }
        return true;
    }

private:
    // a PROXIMA nota depois da atual, com a volta completa pela lista; casa contra "ENDERECO SLOT TIPO TEXTO"
    void Search() {
        const int total = static_cast<int>(notes_->size());
        if (!total || f_.query.empty()) {
            msg_ = "NAO ENCONTRADO";
            return;
        }
        for (int k = 1; k <= total; ++k) {
            const int i = (idx_ + k) % total;
            const Note &n = (*notes_)[static_cast<size_t>(i)];
            const int m = f_.Match(Hex(n.addr, 4) + " " + NoteSlotName(n.slot) + " " + NoteTypeName(n.type) + " " + n.text);
            if (m < 0) {
                msg_ = "?EXPRESSAO REGULAR INVALIDA";
                return;
            }
            if (m > 0) {
                idx_ = i;
                return;
            }
        }
        msg_ = "NAO ENCONTRADO";
    }

    std::vector<Note> *notes_;
    int idx_;
    Finder f_;
    bool in_query_ = false;
    std::string msg_;
};

// ---------------------------------------------------------------------------------------------------------------------
// XPP: o painel de portas de E/S monitoradas
// ---------------------------------------------------------------------------------------------------------------------
class PortsPanel : public Panel {
public:
    explicit PortsPanel(PortTable *p) : p_(p) {}
    std::string Name() const override { return "XPP"; }

    std::vector<std::string> Render(IMemory &) override {
        std::vector<std::string> out;
        out.push_back("XPP - PORTAS DE E/S");
        out.push_back("PORTA  ENTRADA  SAIDA");
        const std::vector<int> ports = List();
        if (cursor_ >= static_cast<int>(ports.size())) cursor_ = ports.empty() ? 0 : static_cast<int>(ports.size()) - 1;
        if (ports.empty()) out.push_back("(nenhuma porta monitorada: A adiciona, XPI/XPO tambem criam)");
        for (size_t k = 0; k < ports.size(); ++k) {
            const int p = ports[k];
            std::string row = Hex(static_cast<unsigned>(p), 2) + (p_->changed[p] ? "*" : " ") + "    " + Hex(p_->in[p], 2) + "      " + Hex(p_->out[p], 2);
            out.push_back(static_cast<int>(k) == cursor_ ? Mark(row) : row);
        }
        if (mode_ != Mode::Nav) out.push_back(std::string(mode_ == Mode::Add ? "PORTA" : mode_ == Mode::SetIn ? "ENTRADA" : "SAIDA") + " (hexa): " + field_ + "_   ENTER confirma, ESC cancela");
        if (!msg_.empty()) out.push_back(msg_);
        out.push_back("* = a porta sofreu um OUT (ou XPO)  ENTRADA = o ultimo OUT  SAIDA = o que um IN le");
        out.push_back("cima/baixo escolhem  A adiciona  D remove  S define a SAIDA  E define a ENTRADA  C limpa as marcas  ESC sai");
        return out;
    }

    bool Key(IMemory &, const std::string &key) override {
        msg_.clear();
        char ch = 0;
        if (mode_ != Mode::Nav) {
            if (key == "esc") mode_ = Mode::Nav;
            else if (key == "bs") {
                if (!field_.empty()) field_.pop_back();
            } else if (key == "enter") {
                Confirm();
            } else if (CharKey(key, ch) && std::isxdigit(static_cast<unsigned char>(ch)) && field_.size() < 2) {
                field_ += static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
            }
            return true;
        }
        if (key == "esc") return false;
        if (key == "up") cursor_ = std::max(0, cursor_ - 1);
        else if (key == "down") ++cursor_; // o Render prende no fim da lista
        else if (CharKey(key, ch)) {
            const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            const std::vector<int> ports = List();
            const bool have = cursor_ >= 0 && cursor_ < static_cast<int>(ports.size());
            if (c == 'a') Begin(Mode::Add);
            else if (c == 'c') std::fill(std::begin(p_->changed), std::end(p_->changed), false);
            else if (c == 'd' && have) {
                p_->mon[ports[static_cast<size_t>(cursor_)]] = false;
                p_->changed[ports[static_cast<size_t>(cursor_)]] = false;
            } else if (c == 's' && have) Begin(Mode::SetOut);
            else if (c == 'e' && have) Begin(Mode::SetIn);
        }
        return true;
    }

private:
    enum class Mode { Nav, Add, SetOut, SetIn };

    std::vector<int> List() const {
        std::vector<int> v;
        for (int p = 0; p < 256; ++p)
            if (p_->mon[p]) v.push_back(p);
        return v;
    }
    void Begin(Mode m) {
        mode_ = m;
        field_.clear();
    }
    void Confirm() {
        uint32_t v = 0;
        if (field_.empty() || !util::ParseHexN(field_, v, 2)) {
            mode_ = Mode::Nav;
            return;
        }
        const std::vector<int> ports = List();
        if (mode_ == Mode::Add) {
            p_->Add(static_cast<int>(v));
        } else if (cursor_ >= 0 && cursor_ < static_cast<int>(ports.size())) {
            const int p = ports[static_cast<size_t>(cursor_)];
            if (mode_ == Mode::SetOut) p_->out[p] = static_cast<uint8_t>(v);
            else p_->in[p] = static_cast<uint8_t>(v);
        }
        mode_ = Mode::Nav;
    }

    PortTable *p_;
    int cursor_ = 0;
    Mode mode_ = Mode::Nav;
    std::string field_;
    std::string msg_;
};

// ---------------------------------------------------------------------------------------------------------------------
// XTP: um visualizador de texto com rolagem e busca (para os arquivos de dentro do disco)
// ---------------------------------------------------------------------------------------------------------------------
class TextPanel : public Panel {
public:
    TextPanel(std::string title, std::vector<std::string> lines) : title_(std::move(title)), lines_(std::move(lines)) {}
    std::string Name() const override { return "XTP"; }

    std::vector<std::string> Render(IMemory &) override {
        std::vector<std::string> out;
        out.push_back("XTP - " + title_);
        const int total = static_cast<int>(lines_.size());
        for (int r = 0; r < kRows; ++r) {
            const int i = top_ + r;
            if (i >= total) break;
            const std::string &l = lines_[static_cast<size_t>(i)];
            out.push_back(left_ < static_cast<int>(l.size()) ? l.substr(static_cast<size_t>(left_), kCols) : std::string());
        }
        out.push_back("LINHA " + std::to_string(total ? top_ + 1 : 0) + " DE " + std::to_string(total) + "   COLUNA " + std::to_string(left_ + 1));
        out.push_back("BUSCAR  : " + (in_query_ ? Mark(f_.query + "_") : f_.query) + "   " + f_.Flags());
        if (!msg_.empty()) out.push_back(msg_);
        out.push_back(in_query_ ? "digite o texto  ENTER busca a proxima  ESC volta"
                               : "setas rolam  PgUp/PgDn 30 linhas  Home/End inicio/fim  / busca  c Case  r Regex  n proxima  ESC sai");
        return out;
    }

    bool Key(IMemory &, const std::string &key) override {
        msg_.clear();
        const int total = static_cast<int>(lines_.size());
        const int last = std::max(0, total - kRows);
        char ch = 0;
        if (in_query_) {
            if (key == "esc" || key == "tab") in_query_ = false;
            else if (key == "enter") Search();
            else if (key == "bs") {
                if (!f_.query.empty()) f_.query.pop_back();
            } else if (CharKey(key, ch) && ch >= 0x20 && f_.query.size() < 60) f_.query += ch;
            return true;
        }
        if (key == "esc" || key == "enter") return false;
        if (key == "up") top_ = std::max(0, top_ - 1);
        else if (key == "down") top_ = std::min(last, top_ + 1);
        else if (key == "left") left_ = std::max(0, left_ - 8);
        else if (key == "right") left_ += 8;
        else if (key == "pgup") top_ = std::max(0, top_ - 30);
        else if (key == "pgdn") top_ = std::min(last, top_ + 30);
        else if (key == "home") top_ = 0, left_ = 0;
        else if (key == "end") top_ = last;
        else if (key == "tab") in_query_ = true;
        else if (CharKey(key, ch)) {
            const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            if (c == '/') in_query_ = true;
            else if (c == 'c') f_.match_case = !f_.match_case;
            else if (c == 'r') f_.use_regex = !f_.use_regex;
            else if (c == 'n') Search();
        }
        return true;
    }

private:
    static constexpr int kRows = 22, kCols = 76;

    // a proxima linha depois da primeira da tela, com a volta ao inicio do arquivo
    void Search() {
        const int total = static_cast<int>(lines_.size());
        if (!total || f_.query.empty()) {
            msg_ = "NAO ENCONTRADO";
            return;
        }
        for (int k = 1; k <= total; ++k) {
            const int i = (top_ + k) % total;
            const int m = f_.Match(lines_[static_cast<size_t>(i)]);
            if (m < 0) {
                msg_ = "?EXPRESSAO REGULAR INVALIDA";
                return;
            }
            if (m > 0) {
                top_ = std::min(i, std::max(0, total - kRows));
                return;
            }
        }
        msg_ = "NAO ENCONTRADO";
    }

    std::string title_;
    std::vector<std::string> lines_;
    int top_ = 0, left_ = 0;
    Finder f_;
    bool in_query_ = false;
    std::string msg_;
};

// ---------------------------------------------------------------------------------------------------------------------
// XTR: o trace passo a passo (ENTER executa a proxima instrucao, ESC interrompe)
// ---------------------------------------------------------------------------------------------------------------------
class TracePanel : public Panel {
public:
    TracePanel(std::function<std::vector<std::string>()> step, std::vector<std::string> first) : step_(std::move(step)), log_(std::move(first)) {}
    std::string Name() const override { return "XTR"; }

    std::vector<std::string> Render(IMemory &) override {
        std::vector<std::string> out;
        out.push_back("XTR - TRACE" + std::string(done_ ? " (A CPU PAROU)" : ""));
        const size_t from = log_.size() > kRows ? log_.size() - kRows : 0;
        out.insert(out.end(), log_.begin() + static_cast<std::ptrdiff_t>(from), log_.end());
        out.push_back(done_ ? "ESC sai" : "ENTER executa a proxima instrucao  ESC interrompe");
        return out;
    }

    bool Key(IMemory &, const std::string &key) override {
        if (key == "esc") return false;
        if (key == "enter" && !done_) {
            const std::vector<std::string> lines = step_();
            if (lines.empty()) {
                done_ = true;
                log_.push_back("CPU HALTADA (HALT)");
            } else {
                log_.insert(log_.end(), lines.begin(), lines.end());
            }
        }
        return true;
    }

private:
    static constexpr size_t kRows = 21;
    std::function<std::vector<std::string>()> step_;
    std::vector<std::string> log_;
    bool done_ = false;
};

} // namespace

// ---------------------------------------------------------------------------------------------------------------------
const char *NoteSlotName(int slot) {
    static const char *const n[] = {"GERAL", "MAIN", "SUB", "FDC", "RAM"};
    return slot >= 0 && slot <= 4 ? n[slot] : "?";
}

const char *NoteTypeName(int type) {
    static const char *const n[] = {"GERAL", "BIOS", "WORK", "DATA", "PORT", "MATH", "KEY", "HOOK"};
    return type >= 0 && type <= 7 ? n[type] : "?";
}

std::string FormatNote(const Note &n) { return Hex(n.addr, 4) + " [" + NoteSlotName(n.slot) + "/" + NoteTypeName(n.type) + "] " + n.text; }

DisasmLine DisasmAt(IMemory &mem, const Target &t, uint32_t addr) {
    // o disassembler le' por enderecos de 16 bits; na VRAM (ate' 192 KB) a janela de 64 KB e' a do endereco pedido
    const uint32_t hi = addr & ~0xFFFFu;
    const z80::debug::DisasmResult r = z80::debug::Disassemble([&](uint16_t p) { return TRead(mem, t, TargetWrap(mem, t, static_cast<int64_t>(hi) + p)); }, static_cast<uint16_t>(addr & 0xFFFF));
    DisasmLine d;
    d.addr = addr;
    d.len = r.length < 1 ? 1 : r.length;
    for (int k = 0; k < d.len; ++k) d.bytes += (k ? " " : "") + Hex(TRead(mem, t, TargetWrap(mem, t, static_cast<int64_t>(addr) + k)), 2);
    d.text = r.text;
    return d;
}

std::string FormatDisasm(const DisasmLine &d, int digits) {
    std::string b = d.bytes;
    while (b.size() < 11) b += ' ';
    return Hex(d.addr, digits) + "  " + b + "  " + d.text;
}

std::unique_ptr<Panel> MakeXaPanel(const SxAddr &a) { return std::make_unique<XaPanel>(a); }
std::unique_ptr<Panel> MakeXiPanel(const SxAddr &a) { return std::make_unique<XiPanel>(a); }
std::unique_ptr<Panel> MakeXhPanel(const SxAddr &a) { return std::make_unique<XhPanel>(a); }
std::unique_ptr<Panel> MakeNotesPanel(std::vector<Note> *notes, int first) { return std::make_unique<NotesPanel>(notes, first); }
std::unique_ptr<Panel> MakePortsPanel(PortTable *ports) { return std::make_unique<PortsPanel>(ports); }
std::unique_ptr<Panel> MakeTextPanel(const std::string &title, const std::vector<std::string> &lines) { return std::make_unique<TextPanel>(title, lines); }
std::unique_ptr<Panel> MakeTracePanel(std::function<std::vector<std::string>()> step, std::vector<std::string> first) {
    return std::make_unique<TracePanel>(std::move(step), std::move(first));
}

} // namespace mamute
