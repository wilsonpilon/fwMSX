// fwMSX -- depurador: decorador do barramento do Z80 (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause).
//
// Fica entre o Z80Cpu e o barramento da maquina e entrega ao Debugger cada leitura/escrita de memoria quando
// ha watchpoints. Sem watchpoints o custo e um teste de booleano por acesso. O caminho rapido de LDIR/LDDR
// (ram_ptr) e desligado enquanto houver watchpoints, senao as copias em bloco passariam sem ser vistas.
#pragma once

#include "../../z80/cpp/z80_bus.h"
#include "debugger.h"

namespace dbg {

class DebugBus : public z80::IBus {
public:
    DebugBus(z80::IBus &inner, Debugger &debugger) : inner_(inner), debugger_(debugger) {}

    uint8_t read(uint16_t addr) override {
        const uint8_t v = inner_.read(addr);
        if (debugger_.watching()) debugger_.OnAccess(addr, v, false);
        return v;
    }
    void write(uint16_t addr, uint8_t value) override {
        if (debugger_.watching()) debugger_.OnAccess(addr, value, true);
        inner_.write(addr, value);
    }
    uint8_t in(uint16_t port) override { return inner_.in(port); }
    void out(uint16_t port, uint8_t value) override { inner_.out(port, value); }
    void on_bios_patch(z80::Z80Cpu &cpu) override { inner_.on_bios_patch(cpu); }
    void on_jump(uint16_t pc) override { inner_.on_jump(pc); }
    uint8_t *ram_ptr(uint16_t addr, uint16_t len) override { return debugger_.watching() ? nullptr : inner_.ram_ptr(addr, len); }

private:
    z80::IBus &inner_;
    Debugger &debugger_;
};

} // namespace dbg
