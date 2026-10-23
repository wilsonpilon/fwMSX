// fwMSX -- dispositivos de E/S do OCM. Ver ocm_devices.h e doc/ocm-spec.md.
#include "ocm_devices.h"

namespace ocm {

uint8_t KanjiDevice::in(uint16_t port) {
    switch (port & 0xFF) {
    case 0xD9: {
        const uint32_t at = kanji_next(&a1_);
        return at < jis1_.size() ? jis1_[at] : 0xFF;
    }
    case 0xDB: {
        const uint32_t at = kanji_next(&a2_);
        return at < jis2_.size() ? jis2_[at] : 0xFF;
    }
    default: return 0xFF;
    }
}

void KanjiDevice::out(uint16_t port, uint8_t value) {
    switch (port & 0xFF) {
    case 0xD8: kanji_write_low(&a1_, value); break;
    case 0xD9: kanji_write_high(&a1_, value); break;
    case 0xDA: kanji_write_low(&a2_, value); break;
    case 0xDB: kanji_write_high(&a2_, value); break;
    default: break;
    }
}

uint8_t SwitchedIo::in(uint16_t port) {
    const int reg = port & 0x0F;
    if (reg == 0) return static_cast<uint8_t>(ocm_swio_read40(sel_));
    return static_cast<uint8_t>(ocm_swio_regs(sel_, reg));
}

void SwitchedIo::out(uint16_t port, uint8_t value) {
    const int reg = port & 0x0F;
    if (reg == 0) sel_ = value;
    else last_[reg] = value;
}

} // namespace ocm
