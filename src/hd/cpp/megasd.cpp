// fwMSX -- interface MegaSD. Ver megasd.h e doc/hd-spec.md.
#include "megasd.h"

#include <algorithm>

namespace hd {

MegaSd::MegaSd(std::vector<uint8_t> rom, SdCard *card) : ram_(0x100000, 0), card_(card) {
    std::copy(rom.begin(), rom.begin() + static_cast<std::ptrdiff_t>(std::min(rom.size(), ram_.size())), ram_.begin());
    Reset();
}

void MegaSd::Reset() {
    for (uint8_t &b : bank_) b = 0;
    latch_ = 0xFF;
    power_on_ = true;
    data_enabled_ = true;
    low_speed_ = false;
    status_read_ = false;
    card_->Select(false);
}

uint8_t MegaSd::SdRead(uint16_t offset) {
    ++sd_accesses_;
    if (offset < 0x1800) {  // dados: devolve a troca anterior e inicia outra com FFh
        status_read_ = false;
        const uint8_t value = latch_;
        if (data_enabled_) {
            card_->Select((offset & 0x1000) != 0 ? false : true);
            latch_ = card_->Transfer(0xFF);
        }
        return value;
    }
    if (offset < 0x1C00) {  // estado: bit 7 = ocupado (as trocas aqui terminam na hora), bit 0 = acabou de ligar
        status_read_ = true;
        return static_cast<uint8_t>(power_on_ ? 0x01 : 0x00);
    }
    status_read_ = false;
    return latch_;
}

void MegaSd::SdWrite(uint16_t offset, uint8_t value) {
    ++sd_accesses_;
    if (offset < 0x1800) {  // dados
        if (data_enabled_) {
            card_->Select((offset & 0x1000) != 0 ? false : true);
            latch_ = card_->Transfer(value);
        }
        return;
    }
    low_speed_ = (value & 0x80) != 0;
    data_enabled_ = (value & 0x01) == 0;
    power_on_ = false;
}

bool MegaSd::CartRead(uint16_t addr, MemMapMapperType, uint8_t &value) {
    const int window = (addr - 0x4000) >> 13;
    if (window == 0 && sd_mode()) {
        value = SdRead(static_cast<uint16_t>(addr & 0x1FFF));
        return true;
    }
    ++rom_reads_;
    value = ram_[RamOffset(window, addr)];
    return true;
}

bool MegaSd::CartWrite(uint16_t addr, uint8_t value, MemMapMapperType) {
    const int window = (addr - 0x4000) >> 13;
    if (window == 1) {  // 6000h-7FFFh: registradores dos bancos
        const int reg = (addr >> 11) & 3;
        bank_[reg] = reg == 1 ? static_cast<uint8_t>(value & 0x7F) : value;
        return true;
    }
    if (window == 0 && sd_mode()) {
        SdWrite(static_cast<uint16_t>(addr & 0x1FFF), value);
        return true;
    }
    if (bank_[window] & 0x80) ram_[RamOffset(window, addr)] = value;  // RAM gravavel; sem o bit 7 e' ROM
    return true;
}

} // namespace hd
