// fwMSX -- um disco ATA/IDE (so' disco fixo, PIO, LBA e CHS) sobre uma imagem HdImage. Codigo ORIGINAL do fwMSX (BSD-3-Clause), escrito a
// partir da especificacao ATA (o openMSX foi lido so' para conferir quais comandos o Nextor usa). Tudo termina na hora: nao ha' BSY
// entre o comando e o resultado, e uma leitura/escrita de setores e' sempre por PIO, um setor de 256 palavras por vez. Ver doc/hd-spec.md.
#pragma once

#include <cstdint>
#include <string>

#include "hd_image.h"

namespace hd {

class IdeDisk {
public:
    // `image` fica com o dono (a Machine); nullptr = nao ha' disco nesta posicao.
    explicit IdeDisk(HdImage *image = nullptr) : image_(image) { Reset(); }

    bool present() const { return image_ != nullptr; }

    void Reset();

    // reg: 1 erro/features, 2 contagem, 3 LBA 7:0, 4 LBA 15:8, 5 LBA 23:16, 6 dispositivo/cabeca (+LBA 27:24), 7 estado/comando.
    uint8_t ReadReg(int reg);
    void WriteReg(int reg, uint8_t value);
    uint16_t ReadData();
    void WriteData(uint16_t value);

    // O ultimo comando que o disco nao conhece (0 = nenhum): ajuda a ver o que o driver pede.
    uint8_t last_unsupported() const { return last_unsupported_; }
    uint32_t sectors_read() const { return sectors_read_; }
    uint32_t sectors_written() const { return sectors_written_; }

    // bits do registrador de estado
    static constexpr uint8_t kErr = 0x01, kDrq = 0x08, kDsc = 0x10, kDrdy = 0x40, kBsy = 0x80;
    // bits do registrador de erro
    static constexpr uint8_t kAbrt = 0x04, kIdnf = 0x10, kUnc = 0x40;

private:
    enum class Mode { None, Read, Write };

    void Execute(uint8_t cmd);
    void Fail(uint8_t error);
    bool CurrentLba(uint32_t &lba) const;
    void StoreLba(uint32_t lba);
    void Identify();
    bool LoadNextSector();   // leitura: poe o proximo setor no buffer
    void FinishTransfer();

    HdImage *image_ = nullptr;
    uint8_t error_ = 0, feature_ = 0, count_ = 0, sec_ = 0, cyl_lo_ = 0, cyl_hi_ = 0, head_ = 0, status_ = 0;
    Mode mode_ = Mode::None;
    uint8_t buffer_[HD_SECTOR_SIZE] = {};
    int index_ = 0;          // palavra atual do buffer (0 a 255)
    uint32_t lba_ = 0;       // setor em transferencia
    int remaining_ = 0;      // setores que faltam (contando o que esta' no buffer)
    uint8_t last_unsupported_ = 0;
    uint32_t sectors_read_ = 0, sectors_written_ = 0;
};

} // namespace hd
