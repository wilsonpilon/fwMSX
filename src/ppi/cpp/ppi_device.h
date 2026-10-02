// fwMSX -- adaptador do PpiState (motor em C, src/ppi/core/ppi_state.h)
// para z80::IBus, ligado ao mapa de memoria. Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver doc/ppi-spec.md.
#pragma once

#include <cstdint>

#include "../../memmap/cpp/memory_system.h"
#include "../../z80/cpp/z80_bus.h"
#include "../core/ppi_state.h"

namespace ppi {

// Dispositivo SOMENTE DE PORTA (A8h-ABh), a registrar com
// z80::CompositeBus::RegisterPortRange(0xA8, 0xAB, ...) NO LUGAR do
// registro de A8h do SlotMemoryBus -- aqui a porta A8h passa a ser a
// porta A do i8255, e o slot primario muda quando o PINO de saida dela
// muda (como PSlot(PPI.Rout[0]) no fMSX), nao a cada escrita crua.
// Consequencia fiel ao hardware/fMSX: ate' o software programar o modo do
// chip (a BIOS escreve 82h em ABh logo no inicio), a porta A esta' em
// modo ENTRADA e OUT (A8h) nao troca slot nenhum.
class PpiDevice : public z80::IBus {
public:
    explicit PpiDevice(memmap::MemorySystem &memory) : memory_(memory) { ppi_reset(&state_); }

    uint8_t read(uint16_t) override { return 0; }
    void write(uint16_t, uint8_t) override {}

    uint8_t in(uint16_t port) override { return ppi_read(&state_, port & 3); }

    void out(uint16_t port, uint8_t value) override {
        ppi_write(&state_, port & 3, value);
        SyncSlot();
    }

    // Reset de maquina: so' o chip (volta ao slot primario 0, como
    // ResetMSX() do fMSX); teclas pressionadas continuam pressionadas.
    void Reset() {
        ppi_reset_chip(&state_);
        applied_slot_reg_ = 0;
        memmap_switch_primary(&memory_.state(), 0);
    }

    PpiState &state() { return state_; }
    const PpiState &state() const { return state_; }

private:
    // if(PPI.Rout[0]!=PSLReg) PSlot(PPI.Rout[0]) do fMSX.
    void SyncSlot() {
        if (state_.rout[0] != applied_slot_reg_) {
            applied_slot_reg_ = state_.rout[0];
            memmap_switch_primary(&memory_.state(), applied_slot_reg_);
        }
    }

    memmap::MemorySystem &memory_;
    PpiState state_{};
    uint8_t applied_slot_reg_ = 0;
};

} // namespace ppi
