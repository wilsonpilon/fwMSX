// fwMSX -- Mamute: a memoria que o monitor enxerga (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/mamute-spec.md.
//
// O monitor fala com UMA interface (IMemory) e nao sabe se atras dela esta a memoria PRIVADA do modo offline
// (16 blocos de 16 KB: 4 slots x 4 paginas, como no Mamute original) ou a memoria viva da maquina emulada
// (modo online, MachineMemory em mamute_online.h). E' isso que deixa o mesmo comando (D, SH, L, T...) valer
// nos dois modos.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "mamute_config.h"

namespace mamute {

class IMemory {
public:
    virtual ~IMemory() = default;

    // "online" ou "offline" (aparece no prompt e no STATUS).
    virtual const char *Name() const = 0;

    // Visao da CPU: o que o Z80 enxerga agora, pelo mapeamento ativo (PAGE).
    virtual uint8_t Read(uint16_t addr) = 0;
    // false = o endereco nao e' gravavel (ROM, vazio) -- nada muda, sem erro, como num MSX de verdade.
    virtual bool Write(uint16_t addr, uint8_t value) = 0;

    // Slot fisico (0-3), independente do PAGE. Offline grava mesmo em celula ROM/vazia (e' "inserir um
    // cartucho"); online so' grava onde a maquina deixa.
    virtual uint8_t ReadSlot(int slot, uint16_t addr) = 0;
    virtual bool WriteSlot(int slot, uint16_t addr, uint8_t value) = 0;

    // Tipo da celula (slot, pagina); Unknown quando a memoria nao sabe dizer (online, slot fora de vista).
    virtual CellType Cell(int slot, int page) = 0;

    // Slot ativo em cada uma das 4 paginas. Offline: PAGE troca. Online: so' leitura (SetPageMap = false).
    virtual std::array<int, 4> PageMap() = 0;
    virtual bool SetPageMap(const std::array<int, 4> &map) = 0;

    // VRAM plana (sem PAGE): 16 KB (MSX1) ou 128/192 KB (MSX2/2+).
    virtual size_t VramSize() = 0;
    virtual uint8_t ReadVram(uint32_t addr) = 0;
    virtual bool WriteVram(uint32_t addr, uint8_t value) = 0;
};

// Memoria privada do modo offline. Fica viva enquanto o monitor existir; o que se faz aqui nunca toca a
// maquina emulada (so' PUSH leva um bloco para ela).
class OfflineMemory : public IMemory {
public:
    explicit OfflineMemory(const Config &cfg);

    // Recarrega a configuracao (tipos das celulas, arquivos de ROM, VRAM) e zera tudo. Devolve avisos
    // (arquivo de ROM que nao abriu etc.), um por linha; vazio = sem problemas.
    std::string Reconfigure(const Config &cfg);

    const char *Name() const override { return "offline"; }
    uint8_t Read(uint16_t addr) override { return blocks_[map_[addr >> 14]][addr >> 14][addr & 0x3FFF]; }
    bool Write(uint16_t addr, uint8_t value) override;
    uint8_t ReadSlot(int slot, uint16_t addr) override { return blocks_[slot & 3][(addr >> 14) & 3][addr & 0x3FFF]; }
    bool WriteSlot(int slot, uint16_t addr, uint8_t value) override;
    CellType Cell(int slot, int page) override { return types_[slot & 3][page & 3]; }
    std::array<int, 4> PageMap() override { return map_; }
    bool SetPageMap(const std::array<int, 4> &map) override;
    size_t VramSize() override { return vram_.size(); }
    uint8_t ReadVram(uint32_t addr) override { return addr < vram_.size() ? vram_[addr] : 0xFF; }
    bool WriteVram(uint32_t addr, uint8_t value) override;

    // Muda so' o TIPO de uma celula em memoria (LOAD faz isso: RAM para binario, ROM para .rom).
    void SetCell(int slot, int page, CellType t) { types_[slot & 3][page & 3] = t; }

    // Estado de boot: as 4 paginas no primeiro slot (0-3) que tem RAM em alguma pagina; sem RAM, no slot 0.
    void Boot();

private:
    std::array<std::array<std::vector<uint8_t>, 4>, 4> blocks_;
    CellType types_[4][4]{};
    std::array<int, 4> map_{{0, 0, 0, 0}};
    std::vector<uint8_t> vram_;
};

} // namespace mamute
