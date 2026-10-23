// fwMSX -- a interface Sunrise ATA-IDE (a do Nextor) como dispositivo de slot de cartucho. Codigo ORIGINAL do fwMSX (BSD-3-Clause); o mapa de
// enderecos e' o do hardware (documentado pela Sunrise e pelo Nextor), conferido com o openMSX so' como referencia. Ver doc/hd-spec.md.
//
// O cartucho tem uma ROM do Nextor de 128 KB (8 bancos de 16 KB) visivel em 4000h-7FFFh e dois discos IDE (mestre e escravo):
//   escrita em 4104h (A15=0, A13-A8=000001, A2=1)  registrador de controle: bit 0 liga os registradores IDE no lugar da ROM;
//                                                  bits 7-3 = o numero do banco, com os bits invertidos (bit 7 e' o menos significativo)
//   7C00h-7DFFh                                    dados (palavra de 16 bits: o byte par le/escreve o baixo, o impar o alto, com trava)
//   7E00h-7EFFh                                    registradores do ATA (endereco & 0Fh; 0Eh = estado alternativo / controle do dispositivo)
// A janela dos registradores aparece tambem em 3C00h/BC00h por espelhamento, como na placa.
#pragma once

#include <cstdint>
#include <vector>

#include "../../memmap/cpp/slot_memory_bus.h"
#include "ide_disk.h"

namespace hd {

class SunriseIde : public memmap::SlotCartIo {
public:
    // `rom`: o Nextor (16 KB a 128 KB, multiplo de 16 KB; o numero de bancos tem de ser potencia de 2). Os discos ficam com o dono.
    SunriseIde(std::vector<uint8_t> rom, HdImage *master, HdImage *slave);

    bool CartRead(uint16_t addr, MemMapMapperType mapper, uint8_t &value) override;
    bool CartWrite(uint16_t addr, uint8_t value, MemMapMapperType mapper) override;

    void Reset();
    int bank() const;
    bool regs_enabled() const { return (control_ & 1) != 0; }
    IdeDisk &disk(int unit) { return unit ? slave_ : master_; }
    // Contadores de diagnostico (aparecem em `ide`): leituras da ROM, escritas no controle e acessos aos registradores/dados.
    uint32_t rom_reads() const { return rom_reads_; }
    uint32_t control_writes() const { return control_writes_; }
    uint32_t reg_accesses() const { return reg_accesses_; }

private:
    uint8_t ReadRegister(int reg);
    void WriteRegister(int reg, uint8_t value);

    std::vector<uint8_t> rom_;
    IdeDisk master_;
    IdeDisk slave_;
    uint8_t control_ = 0xFF;   // no power-on o controle vale FFh: registradores IDE ligados e o ultimo banco (a ROM do Nextor sobe nele)
    int selected_ = 0;
    bool soft_reset_ = false;
    uint8_t read_latch_ = 0;
    uint8_t write_latch_ = 0;
    uint32_t rom_reads_ = 0, control_writes_ = 0, reg_accesses_ = 0;
};

} // namespace hd
