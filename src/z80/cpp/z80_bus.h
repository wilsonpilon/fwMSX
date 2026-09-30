// Interface C++ do barramento do Z80 -- design proprio (BSD-3-Clause),
// nao adaptado do fMSX. Ver doc/z80-core-spec.md, secao 3.3.
#pragma once

#include <cstdint>

namespace z80 {

class Z80Cpu;

// Implementada pela camada de maquina (fora do escopo desta fase --
// ver doc/z80-core-spec.md, secao 6, Fase 4). Uma unica instancia de
// IBus e' convertida, uma vez, para o Z80Bus (struct de ponteiros de
// funcao) que o motor em C entende -- ver z80_cpu.cpp.
class IBus {
public:
    virtual ~IBus() = default;

    virtual uint8_t read(uint16_t addr) = 0;
    virtual void write(uint16_t addr, uint8_t value) = 0;
    virtual uint8_t in(uint16_t port) = 0;
    virtual void out(uint16_t port, uint8_t value) = 0;

    // Gancho do opcode especial "ED FE" (patch de BIOS -- ver
    // doc/z80-core-spec.md, secao 2). Default: nao faz nada, para que um
    // IBus simples (ex.: o smoke test) nao seja obrigado a implementar.
    virtual void on_bios_patch(Z80Cpu &cpu) { (void)cpu; }

    // Gancho opcional chamado em todo JP/JR/CALL/RST/RET. Default:
    // nao faz nada.
    virtual void on_jump(uint16_t pc) { (void)pc; }

    // Gancho opcional (Fase 3) para acelerar LDIR/LDDR via Assembly. Ver
    // o contrato completo em src/z80/core/z80_bus.h (campo `ram_ptr` de
    // Z80Bus) -- este metodo e' convertido para aquele campo em
    // Z80Cpu::Z80Cpu(). Default: nullptr (sem caminho rapido, sempre
    // correto, so mais lento).
    virtual uint8_t *ram_ptr(uint16_t addr, uint16_t len) {
        (void)addr;
        (void)len;
        return nullptr;
    }
};

} // namespace z80
