// fwMSX -- "Z80 Debugger" da janela do emulador. Ver debug_view.h e doc/debug-spec.md.
#include "debug_view.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>

#include "../../z80/debug/z80_disasm.h"

extern "C" void dbg_format_flags(int f, char *buf); // src/debug/fortran/dbg_fmt.f90

namespace dbgui {

namespace {

std::string Hex(unsigned v, int digits) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%0*X", digits, v);
    return buf;
}

uint16_t Sum(uint16_t a, int delta) { return static_cast<uint16_t>(a + delta); }

z80::debug::DisasmResult Dis(machine::Machine &m, uint16_t addr) {
    return z80::debug::Disassemble([&](uint16_t a) { return m.ReadMemory(a); }, addr);
}

bool HasBreak(machine::Machine &m, uint16_t addr) {
    for (const dbg::Breakpoint &b : m.debugger().breaks()) {
        if (b.addr == addr) return true;
    }
    return false;
}

const char *ModeName(ViewMode mode) {
    switch (mode) {
    case ViewMode::Disasm: return "Disassembly";
    case ViewMode::Memory: return "Memory dump";
    case ViewMode::Stack: return "Stack";
    }
    return "";
}

// Paleta no espirito do MSX (fundo azul escuro, texto claro): {frente, fundo} em 0xRRGGBB.
struct Colors {
    uint32_t fg, bg;
};
const Colors kPalette[kStyleCount] = {
    {0xE0E0E0, 0x2020E0}, // kNormal
    {0x2020E0, 0xE0E0E0}, // kTitle
    {0xE0E060, 0x2020E0}, // kReg
    {0x000000, 0x40C0E0}, // kCursor
    {0xFFFF40, 0x2020E0}, // kPc
    {0xFF5050, 0x2020E0}, // kBreakMark
    {0x9090FF, 0x2020E0}, // kDim
    {0x000000, 0xE0E060}, // kPrompt
};

uint32_t Rgba(uint32_t rgb) {
    return 0xFF000000u | ((rgb & 0xFF) << 16) | (rgb & 0xFF00) | ((rgb >> 16) & 0xFF); // bytes R,G,B,A na memoria
}

} // namespace

void Grid::Put(int x, int y, const std::string &text, Style style) {
    if (y < 0 || y >= kRows) return;
    for (size_t i = 0; i < text.size(); ++i) {
        const int cx = x + static_cast<int>(i);
        if (cx < 0 || cx >= kCols) continue;
        Cell &c = at(cx, y);
        c.ch = static_cast<uint8_t>(text[i]);
        c.style = style;
    }
}

void Grid::PaintRow(int y, Style style) {
    if (y < 0 || y >= kRows) return;
    for (int x = 0; x < kCols; ++x) at(x, y).style = style;
}

bool LoadFontFromMachine(machine::Machine &m, Font &font) {
    font.valid = false;
    auto rd = [&](unsigned a) { return m.memory().PeekSlot(0, 0, static_cast<uint16_t>(a)); };
    const unsigned p = rd(4) | (static_cast<unsigned>(rd(5)) << 8); // CGTABL: endereco da fonte na ROM principal
    if (p < 8 || p + 256 * 8 > 0x8000) return false;
    for (int c = 0; c < 256; ++c) {
        for (int r = 0; r < 8; ++r) font.glyph[c][r] = rd(p + c * 8 + r);
    }
    bool a_has_pixels = false;
    for (int r = 0; r < 8; ++r) a_has_pixels = a_has_pixels || font.glyph[0x41][r] != 0;
    font.valid = a_has_pixels;
    return font.valid;
}

bool RenderRgba(const Grid &grid, const Font &font, std::vector<uint32_t> &rgba, int &width, int &height) {
    if (!font.valid) return false;
    width = kCols * 6;
    height = kRows * 8;
    rgba.assign(static_cast<size_t>(width) * height, 0);
    for (int cy = 0; cy < kRows; ++cy) {
        for (int cx = 0; cx < kCols; ++cx) {
            const Cell &cell = grid.at(cx, cy);
            const Colors &col = kPalette[cell.style < kStyleCount ? cell.style : kNormal];
            const uint32_t fg = Rgba(col.fg), bg = Rgba(col.bg);
            for (int r = 0; r < 8; ++r) {
                const uint8_t bits = font.glyph[cell.ch][r];
                for (int b = 0; b < 6; ++b) { // so' os 6 bits de cima: a celula do texto do MSX tem 6 pixels
                    rgba[static_cast<size_t>(cy * 8 + r) * width + cx * 6 + b] = (bits & (0x80 >> b)) ? fg : bg;
                }
            }
        }
    }
    return true;
}

std::string GridRowText(const Grid &grid, int y) {
    std::string s;
    for (int x = 0; x < kCols; ++x) {
        const uint8_t c = grid.at(x, y).ch;
        s += (c >= 32 && c < 127) ? static_cast<char>(c) : '.';
    }
    return s;
}

uint16_t InstructionLength(machine::Machine &m, uint16_t addr) { return std::max<uint16_t>(1, Dis(m, addr).length); }

uint16_t PreviousInstruction(machine::Machine &m, uint16_t addr) {
    for (int n = 4; n >= 1; --n) {
        const uint16_t start = Sum(addr, -n);
        if (Sum(start, InstructionLength(m, start)) == addr) return start;
    }
    return Sum(addr, -1);
}

// ---- DebugView -----------------------------------------------------------------------------------------

void DebugView::Enter(machine::Machine &m) {
    if (!m.debugger().stopped()) m.debugger().Stop("manual");
    const uint16_t pc = m.cpu().state().pc.w;
    mode_ = ViewMode::Disasm;
    cursor_ = pc;
    top_ = pc;
    for (int i = 0; i < 3; ++i) top_ = PreviousInstruction(m, top_); // algum contexto acima do PC
    mem_top_ = static_cast<uint16_t>(pc & ~(kMemBytesPerRow - 1));
    stack_cursor_ = stack_top_ = m.cpu().state().sp.w;
    prompt_ = false;
    prompt_text_.clear();
    message_.clear();
}

void DebugView::EnsureVisible(machine::Machine &m) {
    if (mode_ == ViewMode::Disasm) {
        // O cursor esta' entre as instrucoes mostradas? Senao, ou desce o topo ate' ele aparecer ou recomeca nele.
        for (int guard = 0; guard < 4096; ++guard) {
            uint16_t a = top_;
            for (int i = 0; i < kBodyRows; ++i) {
                if (a == cursor_) return;
                a = Sum(a, InstructionLength(m, a));
            }
            const uint16_t ahead = static_cast<uint16_t>(cursor_ - top_);
            if (ahead < 0x100) top_ = Sum(top_, InstructionLength(m, top_)); // esta' adiante: rola uma instrucao
            else break;                                                     // esta' atras ou longe: recomeca nele
        }
        top_ = cursor_;
    } else if (mode_ == ViewMode::Memory) {
        const unsigned bytes = kBodyRows * kMemBytesPerRow;
        const uint16_t ahead = static_cast<uint16_t>(cursor_ - mem_top_);
        if (ahead >= bytes) {
            mem_top_ = ahead < 0x8000 ? static_cast<uint16_t>((cursor_ - (bytes - kMemBytesPerRow)) & ~(kMemBytesPerRow - 1))
                                      : static_cast<uint16_t>(cursor_ & ~(kMemBytesPerRow - 1));
        }
    } else {
        const unsigned bytes = kBodyRows * 2;
        const uint16_t ahead = static_cast<uint16_t>(stack_cursor_ - stack_top_);
        if (ahead >= bytes) stack_top_ = ahead < 0x8000 ? Sum(stack_cursor_, -static_cast<int>(bytes - 2)) : stack_cursor_;
    }
}

void DebugView::Move(machine::Machine &m, int rows) {
    if (mode_ == ViewMode::Disasm) {
        for (int i = 0; i < std::abs(rows); ++i) cursor_ = rows > 0 ? Sum(cursor_, InstructionLength(m, cursor_)) : PreviousInstruction(m, cursor_);
    } else if (mode_ == ViewMode::Memory) {
        cursor_ = Sum(cursor_, rows * kMemBytesPerRow);
    } else {
        stack_cursor_ = Sum(stack_cursor_, rows * 2);
    }
    EnsureVisible(m);
}

void DebugView::StepOne(machine::Machine &m) {
    const size_t mark = m.debugger().EventCount();
    m.StepInstruction();
    const uint16_t pc = m.cpu().state().pc.w;
    message_.clear();
    if (m.debugger().TakeWatchHit(pc)) message_ = "Watchpoint disparou";
    m.debugger().TruncateEvents(mark); // o painel ja' mostra o que aconteceu
    cursor_ = pc;
    stack_top_ = stack_cursor_ = m.cpu().state().sp.w;
    EnsureVisible(m);
}

void DebugView::CommitPrompt(machine::Machine &m) {
    prompt_ = false;
    if (prompt_text_.empty()) return;
    const uint16_t addr = static_cast<uint16_t>(std::strtoul(prompt_text_.c_str(), nullptr, 16));
    prompt_text_.clear();
    if (mode_ == ViewMode::Stack) {
        stack_cursor_ = stack_top_ = addr;
    } else if (mode_ == ViewMode::Memory) {
        cursor_ = addr;
        mem_top_ = static_cast<uint16_t>(addr & ~(kMemBytesPerRow - 1));
    } else {
        cursor_ = addr;
        top_ = addr;
        for (int i = 0; i < 2; ++i) top_ = PreviousInstruction(m, top_);
        EnsureVisible(m);
    }
    message_ = "Endereco " + Hex(addr, 4);
}

void DebugView::TypeChar(char c) {
    if (!prompt_) return;
    if (std::isxdigit(static_cast<unsigned char>(c)) && prompt_text_.size() < 4) prompt_text_ += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

Action DebugView::Press(Key key, machine::Machine &m) {
    if (prompt_) {
        if (key == Key::Enter) CommitPrompt(m);
        else if (key == Key::Escape) {
            prompt_ = false;
            prompt_text_.clear();
        } else if (key == Key::Backspace && !prompt_text_.empty()) {
            prompt_text_.pop_back();
        }
        return Action::None;
    }
    message_.clear();
    switch (key) {
    case Key::Enter: StepOne(m); break;
    case Key::Up: Move(m, -1); break;
    case Key::Down: Move(m, 1); break;
    case Key::Left: Move(m, -kBodyRows); break;
    case Key::Right: Move(m, kBodyRows); break;
    case Key::Go:
        prompt_ = true;
        prompt_text_.clear();
        break;
    case Key::Disasm:
        mode_ = ViewMode::Disasm;
        EnsureVisible(m);
        break;
    case Key::Memory:
        mode_ = ViewMode::Memory;
        mem_top_ = static_cast<uint16_t>(cursor_ & ~(kMemBytesPerRow - 1));
        break;
    case Key::Stack:
        mode_ = ViewMode::Stack;
        stack_cursor_ = stack_top_ = m.cpu().state().sp.w;
        break;
    case Key::Jump:
        m.cpu().set_pc(cursor_);
        message_ = "PC = " + Hex(cursor_, 4);
        break;
    case Key::RunTo:
        m.debugger().SetUntil(cursor_);
        m.debugger().Resume();
        return Action::Close;
    case Key::Continue:
        m.debugger().Resume();
        return Action::Close;
    case Key::Quit: return Action::QuitEmulator;
    case Key::Break:
        if (HasBreak(m, cursor_)) {
            m.debugger().RemoveBreak(cursor_);
            message_ = "Breakpoint removido em " + Hex(cursor_, 4);
        } else {
            m.debugger().AddBreak(cursor_);
            message_ = "Breakpoint em " + Hex(cursor_, 4);
        }
        break;
    case Key::Escape: return Action::Close;
    case Key::Backspace: break;
    }
    return Action::None;
}

void DebugView::Render(machine::Machine &m, Grid &grid) const {
    grid = Grid();
    const Z80State &s = m.cpu().state();
    const uint16_t pc = s.pc.w;

    // Titulo
    grid.PaintRow(0, kTitle);
    grid.Put(1, 0, std::string("Z80 Debugger - ") + ModeName(mode_), kTitle);

    // Registradores (nas primeiras linhas, separados do corpo por uma linha em branco)
    char flags[9];
    dbg_format_flags(s.af.b.lo, flags);
    grid.Put(1, 1, "AF=" + Hex(s.af.w, 4) + " BC=" + Hex(s.bc.w, 4) + " DE=" + Hex(s.de.w, 4) + " HL=" + Hex(s.hl.w, 4), kReg);
    grid.Put(1, 2, "IX=" + Hex(s.ix.w, 4) + " IY=" + Hex(s.iy.w, 4) + " SP=" + Hex(s.sp.w, 4) + " PC=" + Hex(pc, 4), kReg);
    grid.Put(1, 3, "AF'=" + Hex(s.af_alt.w, 4) + " BC'=" + Hex(s.bc_alt.w, 4) + " DE'=" + Hex(s.de_alt.w, 4) + " HL'=" + Hex(s.hl_alt.w, 4), kReg);
    const char *im = (s.iff & Z80_IFF_IM2) ? "2" : (s.iff & Z80_IFF_IM1) ? "1" : "0";
    grid.Put(1, 4, "I=" + Hex(s.i, 2) + " R=" + Hex(s.r, 2) + " IM" + im + " " + ((s.iff & Z80_IFF_1) ? "EI" : "DI") + ((s.iff & Z80_IFF_HALT) ? " HALT" : "     ") +
                       "  F=" + flags,
             kReg);

    // Corpo
    uint16_t da = top_; // proximo endereco da desmontagem
    for (int i = 0; i < kBodyRows; ++i) {
        const int y = kHeaderRows + i;
        if (mode_ == ViewMode::Disasm) {
            // endereco  bytes  instrucao com os parametros ("3E 05" -> LD A,05h)
            const uint16_t a = da;
            const z80::debug::DisasmResult d = Dis(m, a);
            const uint16_t len = std::max<uint16_t>(1, d.length);
            std::string bytes;
            for (uint16_t k = 0; k < 4; ++k) bytes += k < len ? Hex(m.ReadMemory(Sum(a, k)), 2) + " " : "   ";
            if (HasBreak(m, a)) grid.Put(0, y, "*", kBreakMark);
            if (a == pc) grid.Put(1, y, ">", kPc);
            grid.Put(2, y, Hex(a, 4) + ": " + bytes + " " + d.text, kNormal);
            // o rotulo do fonte (asmrun / sym load) na coluna da direita, se couber: "loop" ao lado da instrucao
            std::string label;
            if (d.text.size() <= 14 && m.debugger().symbols().LabelAt(a, label)) grid.Put(36, y, label.substr(0, kCols - 36), kDim);
            if (a == cursor_) grid.PaintRow(y, kCursor);
            da = Sum(a, len);
        } else if (mode_ == ViewMode::Memory) {
            const uint16_t a = Sum(mem_top_, i * kMemBytesPerRow);
            grid.Put(1, y, Hex(a, 4) + ":", kNormal);
            for (int k = 0; k < kMemBytesPerRow; ++k) {
                const uint8_t v = m.ReadMemory(Sum(a, k));
                grid.Put(7 + k * 3, y, Hex(v, 2), kNormal);
                Cell &c = grid.at(32 + k, y); // os mesmos bytes como caracteres do MSX
                c.ch = v;
                c.style = kNormal;
            }
            const uint16_t delta = static_cast<uint16_t>(cursor_ - a);
            if (delta < kMemBytesPerRow) grid.PaintRow(y, kCursor);
        } else {
            const uint16_t a = Sum(stack_top_, i * 2);
            const unsigned v = m.ReadMemory(a) | (static_cast<unsigned>(m.ReadMemory(Sum(a, 1))) << 8);
            const uint16_t off = static_cast<uint16_t>(a - s.sp.w);
            if (a == s.sp.w) grid.Put(0, y, ">", kPc);
            grid.Put(1, y, off < 0x100 ? "SP+" + Hex(off, 2) : std::string("     "), kDim);
            grid.Put(7, y, Hex(a, 4) + ": " + Hex(v, 4), kNormal);
            if (a == stack_cursor_) grid.PaintRow(y, kCursor);
        }
    }

    // Rodape: pergunta ou mensagem, e as teclas
    const int fy = kRows - kFooterRows;
    if (prompt_) {
        grid.PaintRow(fy, kPrompt);
        grid.Put(1, fy, "Ir para o endereco: " + prompt_text_ + "_", kPrompt);
    } else if (!message_.empty()) {
        grid.Put(1, fy, message_, kReg);
    }
    grid.Put(1, fy + 1, "ENTER Passo  Setas Move  Esq/Dir Pagina", kDim);
    grid.Put(1, fy + 2, "G Ir  D Disasm  M Memoria  S Pilha  B Break", kDim);
    grid.Put(1, fy + 3, "J Pular  R Rodar ate  C Continua  Q Sai", kDim);
}

} // namespace dbgui
