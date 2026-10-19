// fwMSX -- Mamute: os paineis interativos (DM, M, S, ZAP, SCR) (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md, "Fase 3".
//
// No Mamute do Paleobasic eram janelas separadas. Aqui um painel e' um MODELO sem tela nenhuma: recebe teclas por nome ("up", "tab", "c:3"...) e
// devolve as linhas de texto para desenhar. Quem hospeda decide como mostrar -- a TUI abre uma caixa e manda as teclas de verdade; os testes
// (e quem quiser) mandam as teclas por comando (`mon @key up`). O cursor vem marcado com \x01 ... \x02 (a TUI inverte as cores; no resto vira [ ]).
//
// Nomes de tecla: up down left right pgup pgdn home end tab enter esc bs plus minus save (Ctrl+S) e c:<caractere>.
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "mamute_memory.h"

namespace mamute {

class Panel {
public:
    virtual ~Panel() = default;
    virtual std::string Name() const = 0; // "DM", "M", "S", "ZAP", "SCR"
    // As linhas do painel (a primeira e' o titulo). Os marcadores \x01..\x02 envolvem o que esta' sob o cursor.
    virtual std::vector<std::string> Render(IMemory &mem) = 0;
    // Aplica uma tecla; false = o painel fecha.
    virtual bool Key(IMemory &mem, const std::string &key) = 0;
    // As versoes de linha (DM, ZAP...) mostram a grade sem o cursor.
    virtual void ShowCursor(bool) {}
};

// DM, M e S sobre a memoria (vista da CPU) e ZAP sobre uma imagem de disco: a grade de 128 bytes (16 linhas de 8), hexa e texto.
//   DM / ZAP  RETURN abre o campo de edicao do byte (ou do texto); RETURN confirma, ESC cancela.
//   M / S     digitar dois digitos hexa grava direto (S usa as teclas do `skeys`); o texto e' so' leitura; RETURN fecha.
std::unique_ptr<Panel> MakeMemoryGrid(char kind /* 'D', 'M' ou 'S' */, uint16_t addr, int shift, const std::string &skeys);
// ZAP: abre a imagem (o arquivo inteiro vai para a memoria; Ctrl+S grava o setor do cursor). nullptr + error se nao abrir.
std::unique_ptr<Panel> MakeDiskGrid(const std::string &path, uint32_t sector, int shift, std::string &error);

// SCR: a memoria como uma tela de 256x192 pixels (caracteres 8x8 de 8 bytes), desenhada em braille.
std::unique_ptr<Panel> MakeScreenPanel(uint16_t addr, int dx, int dy, int mode /* 0 horizontal, 1 vertical */, int zoom /* 1 = 128x48 colunas, 2 = 64x24 */);

// As teclas S padrao: 16 caracteres para os digitos 1 2 3 4 5 6 7 8 9 A B C D E F 0 (o teclado numerico do manual do Mamute).
extern const char *const kDefaultSKeys;

// Desenha o resultado de Render para quem nao tem como inverter cores: troca \x01 por '[' e \x02 por ']'.
std::vector<std::string> PlainLines(const std::vector<std::string> &lines);

} // namespace mamute
