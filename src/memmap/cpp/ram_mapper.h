// fwMSX -- mapper de RAM do MSX2 (portas FCh-FFh). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause); o comportamento (leitura = valor | ~mascara, escrita
// mascarada pelo numero de segmentos) segue InZ80()/OutZ80() de MSX.c do fMSX.
// Ver doc/msx2-spec.md.
#pragma once

#include <cstdint>

#include "../../z80/cpp/z80_bus.h"
#include "memory_system.h"

namespace memmap {

// FCh = segmento da pagina 0000h-3FFFh, FDh = 4000h, FEh = 8000h, FFh = C000h.
// Cada escrita remapeia, na hora, o segmento de 16KB daquela pagina da RAM do
// slot (primary:secondary) -- tipicamente 3:2 (como o fMSX).
class RamMapperDevice : public z80::IBus {
public:
    RamMapperDevice(MemorySystem &memory, int primary, int secondary)
        : memory_(memory), primary_(primary), secondary_(secondary) {
        Reset();
    }

    uint8_t read(uint16_t) override { return 0; }
    void write(uint16_t, uint8_t) override {}

    uint8_t in(uint16_t port) override {
        // Os bits acima da mascara de segmentos leem 1 (o hardware nao os tem)
        return static_cast<uint8_t>(reg_[port & 3] | ~Mask());
    }

    void out(uint16_t port, uint8_t value) override {
        const int page = port & 3;
        const uint8_t v = static_cast<uint8_t>(value & Mask());
        if (reg_[page] == v) return;
        reg_[page] = v;
        memory_.SetMapperSegment(primary_, secondary_, page, v);
    }

    // Estado de reset: pagina k -> segmento k (0,1,2,3): os 4 primeiros segmentos do mapper.
    void Reset() {
        for (int page = 0; page < 4; ++page) {
            reg_[page] = static_cast<uint8_t>(page & Mask());
            memory_.SetMapperSegment(primary_, secondary_, page, reg_[page]);
        }
    }

    uint8_t segment(int page) const { return reg_[page & 3]; }

private:
    uint8_t Mask() const { return static_cast<uint8_t>(memory_.MapperSegments(primary_, secondary_) - 1); }

    MemorySystem &memory_;
    int primary_;
    int secondary_;
    uint8_t reg_[4] = {};
};

} // namespace memmap
