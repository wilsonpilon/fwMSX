// fwMSX -- barramento de depuracao do nucleo Z80 (Fase 4). Codigo
// ORIGINAL do fwMSX (BSD-3-Clause), nao adaptado do fMSX -- ver
// doc/z80-core-spec.md, secao 6 (Fase 4).
//
// RAM plana de 64KB sem nenhum mapeamento de maquina (sem VDP, sem
// bank-switch, sem portas de I/O reais) -- serve so para exercitar o
// Z80Cpu isoladamente via os comandos de depuracao (Z80DebugSession),
// ate existir uma maquina MSX de verdade para o core rodar contra.
#pragma once

#include <array>
#include <cstdint>

#include "../cpp/z80_bus.h"

namespace z80::debug {

class FlatMemoryBus : public z80::IBus {
public:
    std::array<uint8_t, 0x10000> ram{};

    uint8_t read(uint16_t addr) override { return ram[addr]; }
    void write(uint16_t addr, uint8_t value) override { ram[addr] = value; }

    // Sem dispositivo de I/O nenhum ainda -- portas sempre leem 0 e
    // escritas nao tem efeito. Isso muda quando houver uma maquina de
    // verdade (VDP/PSG/etc.) por tras da sessao de depuracao.
    uint8_t in(uint16_t) override { return 0; }
    void out(uint16_t, uint8_t) override {}

    // Habilita o caminho rapido de LDIR/LDDR (Fase 3) durante a
    // depuracao, ja que a RAM aqui e' sempre plana. addr+len pode
    // estourar 16 bits (regiao cruzando 0xFFFF) -- nesse caso recusamos
    // o caminho rapido (devolvendo nullptr) e o core cai pro loop
    // byte-a-byte, que sempre funciona.
    uint8_t *ram_ptr(uint16_t addr, uint16_t len) override {
        if (static_cast<uint32_t>(addr) + len > 0x10000u) return nullptr;
        return &ram[addr];
    }

    uint8_t &at(uint16_t addr) { return ram[addr]; }
    uint8_t at(uint16_t addr) const { return ram[addr]; }
};

} // namespace z80::debug
