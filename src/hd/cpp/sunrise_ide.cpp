// fwMSX -- interface Sunrise ATA-IDE. Ver sunrise_ide.h e doc/hd-spec.md.
#include "sunrise_ide.h"

namespace hd {

namespace {

// O numero do banco esta' nos bits 7-3 do controle com a ordem invertida: o bit 7 vale 1, o bit 3 vale 16.
int ReverseBank(uint8_t control) {
    int bank = 0;
    for (int i = 0; i < 5; ++i)
        if (control & (0x80 >> i)) bank |= 1 << i;
    return bank;
}

} // namespace

SunriseIde::SunriseIde(std::vector<uint8_t> rom, HdImage *master, HdImage *slave)
    : rom_(std::move(rom)), master_(master), slave_(slave) {
    if (rom_.empty()) rom_.assign(0x4000, 0xFF);
    if (rom_.size() % 0x4000) rom_.resize((rom_.size() / 0x4000 + 1) * 0x4000, 0xFF);
    Reset();
}

void SunriseIde::Reset() {
    control_ = 0xFF;
    selected_ = 0;
    soft_reset_ = false;
    master_.Reset();
    slave_.Reset();
}

int SunriseIde::bank() const {
    const int banks = static_cast<int>(rom_.size() / 0x4000);
    int b = ReverseBank(control_);
    if (b >= banks) b &= banks - 1; // o numero de bancos e' potencia de 2 nas ROMs reais; senao so' evita sair da ROM
    if (b >= banks) b = 0;
    return b;
}

uint8_t SunriseIde::ReadRegister(int reg) {
    if (reg == 14) reg = 7; // estado alternativo = estado
    IdeDisk &d = disk(selected_);
    if (soft_reset_) return reg == 7 ? 0xFF : 0x7F; // ocupado
    if (!d.present()) return 0x7F; // sem disco ligado o barramento flutua: o Nextor reconhece o 0x7F como "ninguem"
    if (reg == 0) return static_cast<uint8_t>(d.ReadData() & 0xFF);
    uint8_t v = d.ReadReg(reg);
    if (reg == 6 && d.present()) v = static_cast<uint8_t>((v & 0xEF) | (selected_ ? 0x10 : 0x00)); // o bit DEV e' da interface
    return v;
}

void SunriseIde::WriteRegister(int reg, uint8_t value) {
    if (soft_reset_) {
        if (reg == 14 && !(value & 0x04)) soft_reset_ = false; // so' sair do SRST
        return;
    }
    if (reg == 0) {
        disk(selected_).WriteData(static_cast<uint16_t>((value << 8) | value));
        return;
    }
    if (reg == 14) {
        if (value & 0x04) { // SRST
            soft_reset_ = true;
            master_.Reset();
            slave_.Reset();
        }
        return;
    }
    if (reg == 6) selected_ = (value & 0x10) ? 1 : 0;
    disk(selected_).WriteReg(reg, value);
}

bool SunriseIde::CartRead(uint16_t addr, MemMapMapperType, uint8_t &value) {
    if (regs_enabled()) {
        if ((addr & 0x3E00) == 0x3C00) { // dados
            ++reg_accesses_;
            if (soft_reset_) {
                value = 0x7F;
                return true;
            }
            if ((addr & 1) == 0) {
                const uint16_t w = disk(selected_).ReadData();
                read_latch_ = static_cast<uint8_t>(w >> 8);
                value = static_cast<uint8_t>(w & 0xFF);
            } else {
                value = read_latch_;
            }
            return true;
        }
        if ((addr & 0x3F00) == 0x3E00) { // registradores
            ++reg_accesses_;
            value = ReadRegister(addr & 0x0F);
            return true;
        }
    }
    if (addr >= 0x4000 && addr < 0x8000) {
        ++rom_reads_;
        value = rom_[static_cast<size_t>(bank()) * 0x4000 + (addr & 0x3FFF)];
        return true;
    }
    return false;
}

bool SunriseIde::CartWrite(uint16_t addr, uint8_t value, MemMapMapperType) {
    if ((addr & 0xBF04) == 0x0104) { // controle
        ++control_writes_;
        control_ = value;
        return true;
    }
    if (regs_enabled()) {
        if ((addr & 0x3E00) == 0x3C00) { // dados
            ++reg_accesses_;
            if (soft_reset_) return true;
            if ((addr & 1) == 0) write_latch_ = value;
            else disk(selected_).WriteData(static_cast<uint16_t>((value << 8) | write_latch_));
            return true;
        }
        if ((addr & 0x3F00) == 0x3E00) {
            ++reg_accesses_;
            WriteRegister(addr & 0x0F, value);
            return true;
        }
    }
    return false;
}

} // namespace hd
