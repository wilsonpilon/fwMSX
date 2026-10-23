// fwMSX -- disco ATA/IDE. Ver ide_disk.h e doc/hd-spec.md.
#include "ide_disk.h"

#include <cstring>

namespace hd {

namespace {

// Escreve `text` (preenchido com espacos ate' `bytes`) na area do IDENTIFY, com o par de bytes trocado (ASM).
void PutIdentifyString(uint8_t *buf, int word, int words, const std::string &text) {
    std::string padded = text;
    padded.resize(static_cast<size_t>(words) * 2, ' ');
    hd_swap_pairs(buf + word * 2, reinterpret_cast<const uint8_t *>(padded.data()), padded.size());
}

void Put16(uint8_t *buf, int word, uint32_t value) {
    buf[word * 2] = static_cast<uint8_t>(value & 0xFF);
    buf[word * 2 + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

} // namespace

void IdeDisk::Reset() {
    error_ = 0x01; // o diagnostico passou
    status_ = kDrdy | kDsc;
    feature_ = 0;
    count_ = 1;
    sec_ = 1;
    cyl_lo_ = 0;
    cyl_hi_ = 0;
    head_ = 0;
    mode_ = Mode::None;
    index_ = 0;
    remaining_ = 0;
}

uint8_t IdeDisk::ReadReg(int reg) {
    switch (reg) {
    case 1: return error_;
    case 2: return count_;
    case 3: return sec_;
    case 4: return cyl_lo_;
    case 5: return cyl_hi_;
    case 6: return head_;
    case 7: return status_;
    default: return 0x7F;
    }
}

void IdeDisk::WriteReg(int reg, uint8_t value) {
    switch (reg) {
    case 1: feature_ = value; break;
    case 2: count_ = value; break;
    case 3: sec_ = value; break;
    case 4: cyl_lo_ = value; break;
    case 5: cyl_hi_ = value; break;
    case 6: head_ = value; break;
    case 7:
        // um comando novo cancela a transferencia que estava no meio
        mode_ = Mode::None;
        status_ = static_cast<uint8_t>(status_ & ~(kDrq | kErr));
        Execute(value);
        break;
    default: break;
    }
}

void IdeDisk::Fail(uint8_t error) {
    error_ = error;
    status_ = kDrdy | kDsc | kErr;
    mode_ = Mode::None;
}

bool IdeDisk::CurrentLba(uint32_t &lba) const {
    if (head_ & 0x40) { // LBA
        lba = static_cast<uint32_t>(sec_) | (static_cast<uint32_t>(cyl_lo_) << 8) | (static_cast<uint32_t>(cyl_hi_) << 16) | (static_cast<uint32_t>(head_ & 0x0F) << 24);
        return true;
    }
    // CHS com a geometria do IDENTIFY (16 cabecas x 63 setores)
    if (sec_ == 0) return false;
    const uint32_t cyl = static_cast<uint32_t>(cyl_lo_) | (static_cast<uint32_t>(cyl_hi_) << 8);
    const uint32_t head = head_ & 0x0F;
    lba = (cyl * HD_HEADS + head) * HD_SECTORS_PER_TRACK + (sec_ - 1u);
    return true;
}

void IdeDisk::StoreLba(uint32_t lba) {
    sec_ = static_cast<uint8_t>(lba & 0xFF);
    cyl_lo_ = static_cast<uint8_t>((lba >> 8) & 0xFF);
    cyl_hi_ = static_cast<uint8_t>((lba >> 16) & 0xFF);
    head_ = static_cast<uint8_t>((head_ & 0xF0) | ((lba >> 24) & 0x0F));
}

void IdeDisk::Identify() {
    std::memset(buffer_, 0, sizeof buffer_);
    const uint32_t total = image_->sectors();
    const uint32_t cylinders = total / (HD_HEADS * HD_SECTORS_PER_TRACK);
    const uint32_t cyl16 = cylinders > 16383u ? 16383u : cylinders;
    Put16(buffer_, 0, 0x0040);          // disco fixo
    Put16(buffer_, 1, cyl16);
    Put16(buffer_, 3, HD_HEADS);
    Put16(buffer_, 6, HD_SECTORS_PER_TRACK);
    PutIdentifyString(buffer_, 10, 10, "FWMSX0001");
    PutIdentifyString(buffer_, 23, 4, "1.0");
    PutIdentifyString(buffer_, 27, 20, "fwMSX hard disk");
    Put16(buffer_, 47, 0x8001);         // 1 setor por interrupcao no multiplo
    Put16(buffer_, 49, 0x0200);         // LBA suportado
    Put16(buffer_, 53, 0x0001);         // palavras 54-58 validas
    Put16(buffer_, 54, cyl16);
    Put16(buffer_, 55, HD_HEADS);
    Put16(buffer_, 56, HD_SECTORS_PER_TRACK);
    const uint32_t chs_total = cyl16 * HD_HEADS * HD_SECTORS_PER_TRACK;
    Put16(buffer_, 57, chs_total & 0xFFFF);
    Put16(buffer_, 58, chs_total >> 16);
    Put16(buffer_, 60, total & 0xFFFF);
    Put16(buffer_, 61, total >> 16);
    index_ = 0;
    remaining_ = 1;
    mode_ = Mode::Read;
    lba_ = 0xFFFFFFFFu; // nao e' um setor do disco: LoadNextSector nao e' chamado (remaining_ = 1)
    status_ = kDrdy | kDsc | kDrq;
}

bool IdeDisk::LoadNextSector() {
    if (!image_->ReadSector(lba_, buffer_)) return false;
    ++sectors_read_;
    index_ = 0;
    return true;
}

void IdeDisk::FinishTransfer() {
    mode_ = Mode::None;
    status_ = kDrdy | kDsc;
    count_ = 0;
}

void IdeDisk::Execute(uint8_t cmd) {
    if (!image_) return;
    if (cmd >= 0x10 && cmd <= 0x1F) { // recalibrar
        error_ = 0;
        status_ = kDrdy | kDsc;
        return;
    }
    switch (cmd) {
    case 0x20: // READ SECTORS
    case 0x21: {
        uint32_t lba = 0;
        const int n = count_ == 0 ? 256 : count_;
        if (!CurrentLba(lba) || static_cast<uint64_t>(lba) + n > image_->sectors()) {
            Fail(kIdnf);
            return;
        }
        lba_ = lba;
        remaining_ = n;
        if (!LoadNextSector()) {
            Fail(kUnc);
            return;
        }
        error_ = 0;
        mode_ = Mode::Read;
        status_ = kDrdy | kDsc | kDrq;
        return;
    }
    case 0x30: // WRITE SECTORS
    case 0x31: {
        uint32_t lba = 0;
        const int n = count_ == 0 ? 256 : count_;
        if (!CurrentLba(lba) || static_cast<uint64_t>(lba) + n > image_->sectors()) {
            Fail(kIdnf);
            return;
        }
        if (image_->read_only()) {
            Fail(kAbrt);
            return;
        }
        lba_ = lba;
        remaining_ = n;
        index_ = 0;
        error_ = 0;
        mode_ = Mode::Write;
        status_ = kDrdy | kDsc | kDrq;
        return;
    }
    case 0x40: // READ VERIFY
    case 0x41:
    case 0x70: // SEEK
    case 0xE0: // STANDBY IMMEDIATE
    case 0xE1: // IDLE IMMEDIATE
    case 0xE2: // STANDBY
    case 0xE3: // IDLE
    case 0xE5: // CHECK POWER MODE
    case 0xE7: // FLUSH CACHE (as escritas ja' foram ao arquivo)
    case 0xEA: // FLUSH CACHE EXT
    case 0x91: // INITIALIZE DEVICE PARAMETERS
        error_ = 0;
        status_ = kDrdy | kDsc;
        if (cmd == 0xE5) count_ = 0xFF; // ativo
        return;
    case 0x90: // EXECUTE DEVICE DIAGNOSTIC
        Reset();
        return;
    case 0xEF: // SET FEATURES: aceita (modo de transferencia, cache...) sem mudar nada
        error_ = 0;
        status_ = kDrdy | kDsc;
        return;
    case 0xEC: // IDENTIFY DEVICE
        error_ = 0;
        Identify();
        return;
    case 0xF8: // READ NATIVE MAX ADDRESS
        error_ = 0;
        StoreLba(image_->sectors() - 1u);
        status_ = kDrdy | kDsc;
        return;
    default:
        last_unsupported_ = cmd;
        Fail(kAbrt);
        return;
    }
}

uint16_t IdeDisk::ReadData() {
    if (!image_ || mode_ != Mode::Read) return 0x7F7F;
    const uint16_t word = static_cast<uint16_t>(buffer_[index_ * 2] | (buffer_[index_ * 2 + 1] << 8));
    if (++index_ < 256) return word;
    // o setor acabou
    if (--remaining_ <= 0) {
        FinishTransfer();
        return word;
    }
    ++lba_;
    if (!LoadNextSector()) Fail(kUnc);
    return word;
}

void IdeDisk::WriteData(uint16_t value) {
    if (!image_ || mode_ != Mode::Write) return;
    buffer_[index_ * 2] = static_cast<uint8_t>(value & 0xFF);
    buffer_[index_ * 2 + 1] = static_cast<uint8_t>(value >> 8);
    if (++index_ < 256) return;
    if (!image_->WriteSector(lba_, buffer_)) {
        Fail(kUnc);
        return;
    }
    ++sectors_written_;
    index_ = 0;
    ++lba_;
    if (--remaining_ <= 0) FinishTransfer();
}

} // namespace hd
