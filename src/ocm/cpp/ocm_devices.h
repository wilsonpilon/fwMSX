// fwMSX -- dispositivos de E/S da maquina OCM (1chipMSX / MSX++): a ROM de Kanji (portas D8h-DBh) e as portas comutadas (40h-4Fh). Codigo ORIGINAL do
// fwMSX (BSD-3-Clause). O mapa vem do OCM-PLD (emsx_top.vhd, swioports.vhd; lidos so' como referencia). Ver doc/ocm-spec.md.
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../../z80/cpp/z80_bus.h"
#include "../core/kanji_core.h"

namespace ocm {

// Kanji ROM: nivel 1 em D8h/D9h (JIS1, 128 KB) e nivel 2 em DAh/DBh (JIS2, 128 KB). Sem a ROM do nivel 2 as portas DAh/DBh leem FFh.
class KanjiDevice : public z80::IBus {
public:
    KanjiDevice(std::vector<uint8_t> jis1, std::vector<uint8_t> jis2) : jis1_(std::move(jis1)), jis2_(std::move(jis2)) {
        kanji_reset(&a1_);
        kanji_reset(&a2_);
    }

    uint8_t read(uint16_t) override { return 0xFF; }
    void write(uint16_t, uint8_t) override {}

    uint8_t in(uint16_t port) override;
    void out(uint16_t port, uint8_t value) override;

private:
    std::vector<uint8_t> jis1_, jis2_;
    KanjiAddr a1_{}, a2_{};
};

// Portas comutadas do MSX (40h-4Fh): a 40h escolhe o dispositivo pelo ID (D4h = OCM/MSX++, 08h = Panasonic) e devolve o complemento do ID quando ele existe.
class SwitchedIo : public z80::IBus {
public:
    uint8_t read(uint16_t) override { return 0xFF; }
    void write(uint16_t, uint8_t) override {}

    uint8_t in(uint16_t port) override;
    void out(uint16_t port, uint8_t value) override;
    uint8_t selected() const { return sel_; }

private:
    uint8_t sel_ = 0xFF;   // nenhum dispositivo
    uint8_t last_[16] = {};
};

} // namespace ocm

extern "C" {
int32_t ocm_swio_read40(int32_t sel);          // Fortran
int32_t ocm_swio_regs(int32_t sel, int32_t reg); // Fortran
void ocm_copy(uint8_t *dst, const uint8_t *src, size_t count); // Assembly
}
