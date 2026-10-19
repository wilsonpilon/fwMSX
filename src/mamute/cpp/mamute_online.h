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
    uint8_t ReadSlot(int slot, uint16_t addr) override { return m_.memory().PeekSlot(slot & 3, Sec(slot), addr); }
    bool WriteSlot(int slot, uint16_t addr, uint8_t value) override;
    uint8_t ReadSub(int slot, int sub, uint16_t addr) override { return m_.memory().PeekSlot(slot & 3, sub ? (sub & 3) : Sec(slot), addr); }
    bool WriteSub(int slot, int sub, uint16_t addr, uint8_t value) override;
    CellType Cell(int slot, int page) override;
    std::array<int, 4> PageMap() override;
    bool SetPageMap(const std::array<int, 4> &) override { return false; } // o mapeamento e' o da maquina
    size_t VramSize() override;
    uint8_t ReadVram(uint32_t addr) override;
    bool WriteVram(uint32_t addr, uint8_t value) override;

private:
    // O sub-slot de um slot "puro" (#3, sem -sub): o que a CPU enxerga agora naquele slot; 0 se ele nao esta' em nenhuma pagina.
    // Num MSX com o slot 3 expandido, a RAM nao esta' no sub-slot 0, e o monitor nao deve obrigar a digitar #3-2.
    int Sec(int slot) const {
        const auto view = m_.memory().CurrentView();
        for (const auto &pv : view)
            if (pv.primary == (slot & 3)) return pv.secondary;
        return 0;
    }
    machine::Machine &m_;
};

// A CPU da maquina emulada: X mostra/edita os registradores dela e G muda o PC, poe breakpoints e solta.
class MachineExec : public IExec {
public:
    explicit MachineExec(machine::Machine &m) : m_(m) {}
    Z80State &State() override { return m_.cpu().state(); }
    void StepOne() override {
        if (!m_.debugger().stopped()) m_.debugger().Stop("mamute"); // o trace so' faz sentido com a maquina parada
        m_.StepInstruction();
    }
    std::string Go(bool has_addr, uint16_t addr, const std::vector<uint16_t> &bps) override;

private:
    machine::Machine &m_;
};

} // namespace mamute
