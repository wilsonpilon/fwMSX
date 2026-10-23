// fwMSX -- a interface MegaSD (ESE Artists' Factory; a do OCM) como dispositivo de slot de cartucho: um mapper "ESE-RAM" de 1 MB com bancos de
// 8 KB e, no lugar do banco 0, a porta SPI do cartao SD. Codigo ORIGINAL do fwMSX (BSD-3-Clause); o mapa de enderecos vem do hardware
// (megasd.v do OCM-PLD, lido so' como referencia). A ROM e' a do Nextor com o driver MegaSD/MegaSDHC (128 KB), carregada no inicio da RAM. Ver
// doc/hd-spec.md.
//
//   4000h-5FFFh  janela 0, banco = registrador 0;  6000h-7FFFh janela 1;  8000h-9FFFh janela 2;  A000h-BFFFh janela 3
//   escrita em 6000h-67FFh / 6800h-6FFFh / 7000h-77FFh / 7800h-7FFFh  registradores dos bancos 0 a 3 (o banco 1 tem 7 bits)
//   bit 7 do registrador (bancos 0, 2 e 3)  a janela e' RAM gravavel; com bit 7 = 0 e' ROM (so' leitura)
//   registrador 0 com os bits 7-6 = 01 (40h-7Fh)  a janela 0 vira a interface do cartao SD:
//     4000h-57FFh  registrador de dados: cada acesso troca um byte com o cartao (a leitura devolve o byte da troca anterior); o bit 12 do
//                  endereco e' o /CS (4000h-4FFFh seleciona o cartao, 5000h-57FFh solta)
//     5800h-5FFFh  escrita: modo (bit 7 baixa velocidade; bit 0 = 1 desliga o soquete); leitura de 5800h-5BFFh: estado (bit 7 ocupado,
//                  bit 0 = "acabou de ligar"); leitura de 5C00h-5FFFh: o ultimo byte recebido
#pragma once

#include <cstdint>
#include <vector>

#include "../../memmap/cpp/slot_memory_bus.h"
#include "sd_card.h"

namespace hd {

class MegaSd : public memmap::SlotCartIo {
public:
    // `rom`: o Nextor com o driver (multiplo de 8 KB, ate' 1 MB), copiado para o inicio da RAM. `card` fica com o dono.
    MegaSd(std::vector<uint8_t> rom, SdCard *card);

    bool CartRead(uint16_t addr, MemMapMapperType mapper, uint8_t &value) override;
    bool CartWrite(uint16_t addr, uint8_t value, MemMapMapperType mapper) override;

    void Reset();
    bool sd_mode() const { return (bank_[0] & 0xC0) == 0x40; }
    uint8_t bank(int window) const { return bank_[window & 3]; }
    SdCard &card() { return *card_; }
    uint32_t rom_reads() const { return rom_reads_; }
    uint32_t sd_accesses() const { return sd_accesses_; }

private:
    size_t RamOffset(int window, uint16_t addr) const {
        const uint32_t bank = window == 1 ? (bank_[1] & 0x7Fu) : (bank_[window] & 0x7Fu);
        return (static_cast<size_t>(bank) * 0x2000 + (addr & 0x1FFF)) & (ram_.size() - 1);
    }
    uint8_t SdRead(uint16_t offset);
    void SdWrite(uint16_t offset, uint8_t value);

    std::vector<uint8_t> ram_;   // 1 MB
    SdCard *card_;
    uint8_t bank_[4] = {};
    uint8_t latch_ = 0xFF;       // o ultimo byte recebido do cartao
    bool power_on_ = true;       // "acabou de ligar" ate' a primeira escrita no registrador de modo
    bool data_enabled_ = true;
    bool low_speed_ = false;
    bool status_read_ = false;   // a ultima leitura foi do estado (5800h-5BFFh)
    uint32_t rom_reads_ = 0, sd_accesses_ = 0;
};

} // namespace hd
