// fwMSX -- a controladora de disquete como dispositivo mapeado em memoria no
// slot do DiskROM. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver
// doc/fdc-spec.md.
#pragma once

#include <cstdint>

#include "../../memmap/cpp/slot_memory_bus.h"
#include "../core/fdc_state.h"

namespace fdc {

// Os registradores do WD2793 aparecem DENTRO da ROM do DiskROM, enquanto o
// slot dela esta visivel (mapeamento padrao, de MSX.c do fMSX):
//   7FF8h/BFF8h  status (leitura) / comando (escrita)
//   7FF9h/BFF9h  trilha          7FFAh/BFFAh  setor
//   7FFBh/BFFBh  dados
//   7FFCh/BFFCh  lado (escrita: bit 0)
//   7FFDh/BFFDh  drive (escrita: bit 0)
//   7FFFh/BFFFh  leitura: DRQ (40h) / IRQ (80h) -- a ROM fica sondando isto
class FdcDevice : public memmap::SlotMmio {
public:
    FdcDevice() { fdc_reset(&fdc_); }

    bool MmioRead(uint16_t addr, uint8_t &value) override {
        switch (addr & 0x3FFF) {
        case 0x3FF8: case 0x3FF9: case 0x3FFA: case 0x3FFB:
            value = fdc_read(&fdc_, static_cast<uint8_t>(addr & 3));
            return true;
        case 0x3FFF:
            value = fdc_read(&fdc_, FDC_REG_READY);
            return true;
        default:
            return false; // o resto e' ROM normal
        }
    }

    bool MmioWrite(uint16_t addr, uint8_t value) override {
        switch (addr & 0x3FFF) {
        case 0x3FF8: case 0x3FF9: case 0x3FFA: case 0x3FFB:
            fdc_write(&fdc_, static_cast<uint8_t>(addr & 3), value);
            return true;
        case 0x3FFC: // lado: [xxxxxxxS]
            fdc_write(&fdc_, FDC_REG_SYSTEM, static_cast<uint8_t>(fdc_.drive | FDC_S_DENSITY | ((value & 1) ? 0 : FDC_S_SIDE)));
            return true;
        case 0x3FFD: // drive: [xxxxxxxD]
            fdc_write(&fdc_, FDC_REG_SYSTEM, static_cast<uint8_t>((value & 1) | FDC_S_DENSITY | (fdc_.side ? 0 : FDC_S_SIDE)));
            return true;
        default:
            return false;
        }
    }

    Fdc &fdc() { return fdc_; }
    const Fdc &fdc() const { return fdc_; }

private:
    Fdc fdc_{};
};

} // namespace fdc
