// fwMSX -- cartao SD em modo SPI. Ver sd_card.h e doc/hd-spec.md.
#include "sd_card.h"

#include <cstring>

namespace hd {

namespace {

// Poe `value` nos bits [hi:lo] de um registrador de 128 bits guardado do bit 127 (byte 0, bit 7) ao bit 0 (byte 15, bit 0), como o CSD.
void SetBits(uint8_t *reg, int hi, int lo, uint32_t value) {
    for (int bit = lo; bit <= hi; ++bit) {
        const int idx = 15 - bit / 8;
        const uint8_t mask = static_cast<uint8_t>(1u << (bit % 8));
        if ((value >> (bit - lo)) & 1u) reg[idx] |= mask;
        else reg[idx] = static_cast<uint8_t>(reg[idx] & ~mask);
    }
}

} // namespace

SdCard::SdCard(HdImage *image) : image_(image) {
    sdhc_ = image_ && image_->sectors() > 4194304u;
}

void SdCard::Select(bool selected) {
    selected_ = selected;
    if (!selected_) {
        // /CS alto: o cartao esquece o comando pela metade e a transferencia em curso
        cmd_len_ = 0;
        if (state_ != State::Command) state_ = State::Command;
        out_.clear();
        out_pos_ = 0;
        multi_read_ = false;
    }
}

// `lead` bytes FFh antes da resposta (o Ncr do cartao real: 1 a 8 bytes). O driver do MegaSD descarta dois bytes depois do CRC do comando, entao a
// resposta tem de vir depois deles; dois FFh deixam margem.
void SdCard::Respond(std::vector<uint8_t> bytes, int lead) {
    out_.assign(static_cast<size_t>(lead), 0xFF);
    out_.insert(out_.end(), bytes.begin(), bytes.end());
    out_pos_ = 0;
}

void SdCard::QueueReadBlock(uint32_t sector) {
    uint8_t block[HD_SECTOR_SIZE] = {};
    const bool ok = image_->ReadSector(sector, block);
    if (ok) ++sectors_read_;
    out_.push_back(ok ? 0xFE : 0x08);  // token de dados, ou "erro de leitura" (fora da faixa)
    if (ok) {
        out_.insert(out_.end(), block, block + HD_SECTOR_SIZE);
        out_.push_back(0x00);  // CRC16 (o host nao confere)
        out_.push_back(0x00);
    }
}

void SdCard::BuildCsd(uint8_t *csd) const {
    std::memset(csd, 0, 16);
    const uint32_t sectors = image_->sectors();
    if (sdhc_) {  // CSD versao 2: capacidade = (C_SIZE + 1) * 512 KB
        SetBits(csd, 127, 126, 1);
        SetBits(csd, 119, 112, 0x0E);
        SetBits(csd, 103, 96, 0x32);
        SetBits(csd, 95, 84, 0x5B5);
        SetBits(csd, 83, 80, 9);
        SetBits(csd, 69, 48, sectors / 1024u - 1u);
        SetBits(csd, 46, 46, 1);
        SetBits(csd, 45, 39, 0x7F);
        SetBits(csd, 28, 26, 2);
        SetBits(csd, 25, 22, 9);
    } else {  // CSD versao 1: capacidade = (C_SIZE + 1) * 2^(C_SIZE_MULT + 2) * 2^READ_BL_LEN
        const bool big = sectors > 2097152u;  // acima de 1 GB os blocos de leitura sao de 1024 bytes
        const uint32_t read_bl_len = big ? 10 : 9;
        SetBits(csd, 119, 112, 0x0E);
        SetBits(csd, 103, 96, 0x32);
        SetBits(csd, 95, 84, 0x5B5);
        SetBits(csd, 83, 80, read_bl_len);
        const uint32_t units = sectors >> (big ? 10 : 9);  // com C_SIZE_MULT = 7 (x512)
        SetBits(csd, 73, 62, (units ? units : 1u) - 1u);
        SetBits(csd, 49, 47, 7);
        SetBits(csd, 46, 46, 1);
        SetBits(csd, 45, 39, 0x7F);
        SetBits(csd, 28, 26, 2);
        SetBits(csd, 25, 22, read_bl_len);
    }
    csd[15] |= 1;  // bit 0 sempre 1 (o CRC7 nao e' conferido)
}

void SdCard::Execute() {
    const uint8_t cmd = cmd_[0] & 0x3F;
    const uint32_t arg = (static_cast<uint32_t>(cmd_[1]) << 24) | (static_cast<uint32_t>(cmd_[2]) << 16) | (static_cast<uint32_t>(cmd_[3]) << 8) | cmd_[4];
    const bool was_app = app_cmd_;
    app_cmd_ = false;
    const uint8_t r1_idle = idle_ ? 0x01 : 0x00;
    multi_read_ = false;

    if (was_app) {
        switch (cmd) {
        case 41:  // SD_SEND_OP_COND: sai do idle depois de algumas consultas
            if (idle_ && ++init_polls_ >= 2) idle_ = false;
            Respond({static_cast<uint8_t>(idle_ ? 0x01 : 0x00)});
            return;
        case 22:  // SEND_NUM_WR_BLOCKS
            Respond({0x00, 0xFE, 0, 0, 0, 1, 0, 0});
            return;
        case 23:  // SET_WR_BLK_ERASE_COUNT
            Respond({0x00});
            return;
        default:
            break;  // cai nos comandos normais
        }
    }
    switch (cmd) {
    case 0:  // GO_IDLE_STATE
        idle_ = true;
        init_polls_ = 0;
        Respond({0x01});
        return;
    case 8:  // SEND_IF_COND: cartao SD versao 2 (eco do padrao de verificacao)
        Respond({r1_idle, 0x00, 0x00, cmd_[3], cmd_[4]});
        return;
    case 55:  // APP_CMD
        app_cmd_ = true;
        Respond({r1_idle});
        return;
    case 58: {  // READ_OCR: bit 31 = pronto, bit 30 = SDHC
        uint8_t hi = idle_ ? 0x00 : 0x80;
        if (sdhc_) hi |= 0x40;
        Respond({r1_idle, hi, 0xFF, 0x80, 0x00});
        return;
    }
    case 59:  // CRC_ON_OFF
    case 16:  // SET_BLOCKLEN
        Respond({r1_idle});
        return;
    case 13:  // SEND_STATUS
        Respond({r1_idle, 0x00});
        return;
    case 9:   // SEND_CSD
    case 10:  // SEND_CID
    {
        std::vector<uint8_t> bytes = {r1_idle, 0xFE};
        uint8_t reg[16];
        if (cmd == 9) {
            BuildCsd(reg);
        } else {
            std::memset(reg, 0, sizeof reg);
            reg[0] = 0xFE;  // fabricante
            std::memcpy(reg + 3, "FWMSX", 5);
            reg[8] = 0x10;
            reg[15] = 1;
        }
        bytes.insert(bytes.end(), reg, reg + 16);
        bytes.push_back(0x00);
        bytes.push_back(0x00);
        Respond(std::move(bytes));
        return;
    }
    case 17:  // READ_SINGLE_BLOCK
    case 18: {  // READ_MULTIPLE_BLOCK
        const uint32_t sector = SectorFromArg(arg);
        if (sector >= image_->sectors()) {
            Respond({0x40});  // erro de parametro
            return;
        }
        Respond({0x00});
        QueueReadBlock(sector);
        if (cmd == 18) {
            multi_read_ = true;
            next_sector_ = sector + 1;
        }
        return;
    }
    case 12:  // STOP_TRANSMISSION: o byte seguinte e' de enchimento, depois R1
        Respond({0xFF, 0x00});
        return;
    case 24:  // WRITE_BLOCK
    case 25: {  // WRITE_MULTIPLE_BLOCK
        const uint32_t sector = SectorFromArg(arg);
        if (sector >= image_->sectors() || image_->read_only()) {
            Respond({static_cast<uint8_t>(image_->read_only() ? 0x20 : 0x40)});
            return;
        }
        write_sector_ = sector;
        multi_write_ = cmd == 25;
        state_ = cmd == 24 ? State::WriteToken : State::MultiWriteToken;
        Respond({0x00});
        return;
    }
    case 23:  // SET_BLOCK_COUNT (MMC)
    case 32:  // ERASE_WR_BLK_START_ADDR
    case 33:  // ERASE_WR_BLK_END_ADDR
        Respond({r1_idle});
        return;
    case 38:  // ERASE: aceita sem apagar
        Respond({0x00, 0xFF});
        return;
    default:
        last_unsupported_ = cmd;
        Respond({static_cast<uint8_t>(r1_idle | 0x04)});  // comando ilegal
        return;
    }
}

uint8_t SdCard::Transfer(uint8_t out) {
    if (!image_ || !selected_) return 0xFF;

    // 1. O que o cartao envia neste byte: a fila de resposta/dados, ou FFh.
    uint8_t in = 0xFF;
    if (out_pos_ < out_.size()) {
        in = out_[out_pos_++];
        if (out_pos_ >= out_.size()) {
            out_.clear();
            out_pos_ = 0;
            if (multi_read_) {  // leitura multipla: o proximo bloco ja' vai para a fila
                if (next_sector_ < image_->sectors()) QueueReadBlock(next_sector_++);
                else multi_read_ = false;
            }
        }
    }

    // 2. O que o host enviou neste byte.
    switch (state_) {
    case State::Command:
        if (cmd_len_ == 0) {
            if ((out & 0xC0) == 0x40) cmd_[cmd_len_++] = out;  // bit de partida 01
        } else {
            cmd_[cmd_len_++] = out;
            if (cmd_len_ == 6) {
                cmd_len_ = 0;
                Execute();
            }
        }
        break;
    case State::WriteToken:
    case State::MultiWriteToken:
        if (out == 0xFE || (state_ == State::MultiWriteToken && out == 0xFC)) {
            wbuf_.clear();
            state_ = State::WriteData;
        } else if (state_ == State::MultiWriteToken && out == 0xFD) {  // token de parada
            state_ = State::Command;
            Respond({0xFF, 0x00, 0xFF}, 0);
        }
        break;
    case State::WriteData:
        wbuf_.push_back(out);
        if (wbuf_.size() == HD_SECTOR_SIZE + 2) {  // 512 bytes + CRC16
            const bool multi = multi_write_;
            const bool ok = image_->WriteSector(write_sector_, wbuf_.data());
            if (ok) ++sectors_written_;
            ++write_sector_;
            Respond({static_cast<uint8_t>(ok ? 0x05 : 0x0D), 0x00, 0xFF}, 0);  // aceito / erro de gravacao, ocupado, pronto
            state_ = multi ? State::MultiWriteToken : State::Command;
        }
        break;
    }
    return in;
}

} // namespace hd
