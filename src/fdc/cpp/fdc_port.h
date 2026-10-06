// fwMSX -- controladora de disquete WD2793 acessada por PORTAS, no estilo Microsol (DDX 3.0,
// CDX-2). Codigo ORIGINAL do fwMSX (BSD-3-Clause), com a convencao de bits conferida no
// openMSX (src/fdc/MicrosolFDC.cc, por Ricardo Bittencourt). Ver doc/fdc-spec.md, secao 6.
//
// Mesmo motor (fdc_state) do acesso por memoria; so' muda o enderecamento. Registradores
// a partir de `base` (padrao D0h), 5 portas:
//   base+0  leitura: status      escrita: comando
//   base+1  trilha (leitura/escrita)
//   base+2  setor  (leitura/escrita)
//   base+3  dados  (leitura/escrita)
//   base+4  leitura: bit 7 = IRQ, bit 6 = DRQ (0 quando ha' dado pedido, ativo em zero)
//           escrita: controle do drive
//             bit 0 = drive A   bit 1 = drive B   (so' um por vez)
//             bit 4 = lado (1 = lado 1)
//             bit 5 = motor ligado
//             bit 6 = estados de espera (nao modelado)
//             bit 7 = densidade: 1 = dupla (nao modelado: o formato e' o do drive)
//
// O driver (ddx_3.0.rom, cdx-2.rom) fica numa ROM de 16KB em 4000h-7FFFh do slot, como o
// DISK.ROM, e chama estas portas. O MSX-DOS sobe por esse modo quando o driver e' o certo.
#pragma once

#include <cstdint>

#include "../../z80/cpp/z80_bus.h"
#include "../core/fdc_state.h"

namespace fdc {

class PortFdcDevice : public z80::IBus {
public:
    static constexpr int kPorts = 5;  // base .. base+4

    explicit PortFdcDevice(uint8_t base) : base_(base) { fdc_reset(&fdc_); }

    uint8_t read(uint16_t) override { return 0xFF; }
    void write(uint16_t, uint8_t) override {}

    uint8_t in(uint16_t port) override {
        const int off = offset(port);
        if (off >= 0 && off < 4) return fdc_read(&fdc_, static_cast<uint8_t>(off));
        if (off == 4) {
            // Como o MicrosolFDC do openMSX: 7Fh + IRQ (bit 7) e DRQ invertido (bit 6).
            const uint8_t lines = fdc_read(&fdc_, FDC_REG_READY);
            uint8_t value = 0x7F;
            if (lines & FDC_IRQ) value |= 0x80;
            if (lines & FDC_DRQ) value &= static_cast<uint8_t>(~0x40);
            return value;
        }
        return 0xFF;
    }

    void out(uint16_t port, uint8_t value) override {
        const int off = offset(port);
        if (off >= 0 && off < 4) {
            fdc_write(&fdc_, static_cast<uint8_t>(off), value);
        } else if (off == 4) {
            // Drive: A (bit 0) ou B (bit 1); se nenhum estiver marcado, o drive atual continua.
            // Lado: bit 4 (FDC_S_SIDE e' o inverso, como no 7FFCh do DISK.ROM).
            const uint8_t drive = (value & 0x01) ? 0 : (value & 0x02) ? 1 : fdc_.drive;
            const bool side1 = (value & 0x10) != 0;
            fdc_write(&fdc_, FDC_REG_SYSTEM, static_cast<uint8_t>(drive | FDC_S_DENSITY | (side1 ? 0 : FDC_S_SIDE)));
        }
    }

    uint8_t base() const { return base_; }
    Fdc &fdc() { return fdc_; }
    const Fdc &fdc() const { return fdc_; }

private:
    int offset(uint16_t port) const { return static_cast<int>(port & 0xFF) - base_; }

    uint8_t base_;
    Fdc fdc_{};
};

} // namespace fdc
