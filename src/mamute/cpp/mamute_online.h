// fwMSX -- Mamute: o lado ONLINE, sobre a maquina emulada (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
// Ver doc/mamute-spec.md. Fica separado do resto do Mamute porque e' o unico arquivo que precisa da maquina.
#pragma once

#include "../../machine/machine.h"
#include "mamute_exec.h"
#include "mamute_memory.h"

namespace mamute {

// A memoria viva da maquina: o que o monitor escreve aqui aparece no programa em execucao na hora (por exemplo
// uma rotina em C000h e, no BASIC, DEFUSR=&HC000:A=USR(0)).
class MachineMemory : public IMemory {
public:
    explicit MachineMemory(machine::Machine &m) : m_(m) {}

    const char *Name() const override { return "online"; }
    uint8_t Read(uint16_t addr) override { return m_.ReadMemory(addr); }
    bool Write(uint16_t addr, uint8_t value) override;
    uint8_t ReadSlot(int slot, uint16_t addr) override { return m_.memory().PeekSlot(slot & 3, 0, addr); }
    bool WriteSlot(int slot, uint16_t addr, uint8_t value) override;
    CellType Cell(int slot, int page) override;
    std::array<int, 4> PageMap() override;
    bool SetPageMap(const std::array<int, 4> &) override { return false; } // o mapeamento e' o da maquina
    size_t VramSize() override;
    uint8_t ReadVram(uint32_t addr) override;
    bool WriteVram(uint32_t, uint8_t) override { return false; }

private:
    machine::Machine &m_;
};

// A CPU da maquina emulada: X mostra/edita os registradores dela e G muda o PC, poe breakpoints e solta.
class MachineExec : public IExec {
public:
    explicit MachineExec(machine::Machine &m) : m_(m) {}
    Z80State &State() override { return m_.cpu().state(); }
    std::string Go(bool has_addr, uint16_t addr, const std::vector<uint16_t> &bps) override;

private:
    machine::Machine &m_;
};

} // namespace mamute
