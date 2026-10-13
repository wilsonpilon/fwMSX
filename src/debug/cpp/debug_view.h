// fwMSX -- "Z80 Debugger" da janela do emulador: modelo e desenho (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause), no
// estilo do depurador do fMSX (resource/fMSX/Z80/ConDebug.c: ENTER passo, setas, G, D, M, S, J, R, C, Q). Ver
// doc/debug-spec.md, secao "Depurador na janela".
//
// Aqui NAO ha' OpenGL nem ImGui: o modelo recebe teclas ja' traduzidas (Key), age sobre a Machine e desenha numa
// grade de caracteres (Grid, 44 x 32). A grade vira pixels com a FONTE DO PROPRIO MSX (a tabela de caracteres da
// BIOS, CGTABL, apontada pelo endereco 0004h da ROM), 6 x 8 pixels por caractere, como a tela de texto do MSX --
// por isso os bytes de um dump aparecem com os graficos e simbolos especiais do MSX. O que a janela faz e' so' ligar
// as teclas do ImGui a Press()/TypeChar() e mostrar o resultado de RenderRgba() numa textura.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../../machine/machine.h"

namespace dbgui {

constexpr int kCols = 44;
constexpr int kRows = 32;
constexpr int kHeaderRows = 6; // titulo, 4 linhas de registradores e uma linha em branco
constexpr int kFooterRows = 4; // mensagem/pergunta e 3 linhas de ajuda
constexpr int kBodyRows = kRows - kHeaderRows - kFooterRows;
constexpr int kMemBytesPerRow = 8;

enum class ViewMode { Disasm, Memory, Stack };

enum class Key {
    Enter,     // executa a proxima instrucao
    Up,        // instrucao anterior / linha acima
    Down,      // instrucao seguinte / linha abaixo
    Left,      // uma pagina para cima
    Right,     // uma pagina para baixo
    Go,        // G: vai para um endereco (pergunta)
    Disasm,    // D: visao de desmontagem (a padrao)
    Memory,    // M: dump de memoria (8 bytes por linha, com os caracteres do MSX)
    Stack,     // S: pilha
    Jump,      // J: o PC passa a ser o cursor
    RunTo,     // R: roda ate' o cursor
    Continue,  // C: continua a execucao
    Quit,      // Q: encerra o emulador
    Break,     // B: liga/desliga um breakpoint no cursor
    Escape,    // fecha o painel (a maquina continua parada); numa pergunta, cancela
    Backspace  // apaga um caractere da pergunta
};

enum class Action {
    None,
    Close,        // fecha o painel (C e R deixam a maquina rodando; Esc a deixa parada)
    QuitEmulator  // Q
};

// Estilos de cor das celulas (a paleta fica em RenderRgba).
enum Style : uint8_t { kNormal = 0, kTitle, kReg, kCursor, kPc, kBreakMark, kDim, kPrompt, kStyleCount };

struct Cell {
    uint8_t ch = ' ';  // codigo do caractere NA FONTE DO MSX (0-255)
    uint8_t style = kNormal;
};

struct Grid {
    std::vector<Cell> cells = std::vector<Cell>(static_cast<size_t>(kCols) * kRows);
    Cell &at(int x, int y) { return cells[static_cast<size_t>(y) * kCols + x]; }
    const Cell &at(int x, int y) const { return cells[static_cast<size_t>(y) * kCols + x]; }
    // Escreve `text` a partir de (x, y) com `style`, cortando no fim da linha.
    void Put(int x, int y, const std::string &text, Style style = kNormal);
    // Pinta o estilo da linha inteira (a barra do cursor, o titulo).
    void PaintRow(int y, Style style);
};

// A fonte de 256 caracteres de 8 linhas (so' os 6 bits de cima de cada linha contam, como no MSX).
struct Font {
    uint8_t glyph[256][8] = {};
    bool valid = false;
};

// Copia a fonte da BIOS (CGTABL, o endereco esta' em 0004h-0005h da ROM principal). `valid` fica falso se a tabela
// nao parece uma fonte (ponteiro fora da ROM, ou 'A' em branco).
bool LoadFontFromMachine(machine::Machine &m, Font &font);

// Desenha a grade em pixels RGBA (6 x 8 por celula), como uma tela de texto do MSX: largura kCols*6, altura kRows*8.
// Com `font.valid` falso, devolve false e nao toca em `rgba` (a janela usa um texto comum no lugar).
bool RenderRgba(const Grid &grid, const Font &font, std::vector<uint32_t> &rgba, int &width, int &height);

// O mesmo conteudo da grade como texto comum (caracteres nao imprimiveis viram '.'): fallback e testes.
std::string GridRowText(const Grid &grid, int y);

class DebugView {
public:
    // Abre o painel: para a maquina (se ainda nao parou), poe o cursor no PC e mostra a desmontagem.
    void Enter(machine::Machine &m);

    // Uma tecla de comando. Devolve o que a janela deve fazer.
    Action Press(Key key, machine::Machine &m);

    // Um caractere digitado enquanto a pergunta de endereco (G) esta' aberta: so' digitos hexadecimais.
    void TypeChar(char c);
    bool prompt_active() const { return prompt_; }

    // Desenha o painel inteiro.
    void Render(machine::Machine &m, Grid &grid) const;

    ViewMode mode() const { return mode_; }
    uint16_t cursor() const { return cursor_; }
    uint16_t top() const { return top_; }
    uint16_t stack_cursor() const { return stack_cursor_; }
    const std::string &message() const { return message_; }

private:
    void StepOne(machine::Machine &m);
    void EnsureVisible(machine::Machine &m);
    void CommitPrompt(machine::Machine &m);
    void Move(machine::Machine &m, int rows);

    ViewMode mode_ = ViewMode::Disasm;
    uint16_t cursor_ = 0;       // desmontagem e dump: endereco da linha do cursor
    uint16_t top_ = 0;          // desmontagem: primeiro endereco mostrado
    uint16_t mem_top_ = 0;      // dump: primeiro endereco mostrado (multiplo de 8)
    uint16_t stack_top_ = 0;    // pilha: primeiro endereco mostrado
    uint16_t stack_cursor_ = 0; // pilha: endereco da palavra do cursor
    bool prompt_ = false;
    std::string prompt_text_;
    std::string message_;
};

// Endereco da instrucao ANTERIOR a `addr` (heuristica: a maior das ate' 4 posicoes anteriores cuja instrucao termina
// exatamente em `addr`; senao addr-1). A desmontagem de tras para frente nao e' unica no Z80.
uint16_t PreviousInstruction(machine::Machine &m, uint16_t addr);
// Comprimento da instrucao em `addr`.
uint16_t InstructionLength(machine::Machine &m, uint16_t addr);

} // namespace dbgui
