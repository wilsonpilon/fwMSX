#include "z80_cpu.h"

#include "../core/z80_core.h"

namespace z80 {

Z80Cpu::Z80Cpu(IBus &bus) : bus_(bus) {
    state_.user_data = this;
    c_bus_.ctx = this;
    c_bus_.read = &Z80Cpu::trampoline_read;
    c_bus_.write = &Z80Cpu::trampoline_write;
    c_bus_.in = &Z80Cpu::trampoline_in;
    c_bus_.out = &Z80Cpu::trampoline_out;
    c_bus_.patch = &Z80Cpu::trampoline_patch;
    c_bus_.jump = &Z80Cpu::trampoline_jump;
    c_bus_.ram_ptr = &Z80Cpu::trampoline_ram_ptr;

    // Estado de reset ja' na construcao. Alem do estado inicial dos
    // registradores, isto garante z80_tables_init() (tabelas de flag em
    // Fortran): antes, quem nao chamasse reset() explicitamente rodava com
    // as tabelas de Sinal/Zero/Paridade ZERADAS -- toda instrucao que
    // consulta a tabela (AND/OR/XOR/INC/DEC/CP/...) produzia flags erradas,
    // e foi isso que fazia a BIOS real ficar presa (ver doc/ppi-spec.md).
    z80_reset(&state_, &c_bus_);
}

void Z80Cpu::reset() { z80_reset(&state_, &c_bus_); }

int Z80Cpu::run(int cycles) { return z80_run(&state_, &c_bus_, cycles); }

void Z80Cpu::interrupt(uint16_t vector) { z80_interrupt(&state_, &c_bus_, vector); }

uint8_t Z80Cpu::trampoline_read(void *ctx, uint16_t addr) {
    return static_cast<Z80Cpu *>(ctx)->bus_.read(addr);
}

void Z80Cpu::trampoline_write(void *ctx, uint16_t addr, uint8_t value) {
    static_cast<Z80Cpu *>(ctx)->bus_.write(addr, value);
}

uint8_t Z80Cpu::trampoline_in(void *ctx, uint16_t port) {
    return static_cast<Z80Cpu *>(ctx)->bus_.in(port);
}

void Z80Cpu::trampoline_out(void *ctx, uint16_t port, uint8_t value) {
    static_cast<Z80Cpu *>(ctx)->bus_.out(port, value);
}

void Z80Cpu::trampoline_patch(void *ctx, Z80State *state) {
    (void)state;
    Z80Cpu *self = static_cast<Z80Cpu *>(ctx);
    self->bus_.on_bios_patch(*self);
}

void Z80Cpu::trampoline_jump(void *ctx, uint16_t pc) {
    static_cast<Z80Cpu *>(ctx)->bus_.on_jump(pc);
}

uint8_t *Z80Cpu::trampoline_ram_ptr(void *ctx, uint16_t addr, uint16_t len) {
    return static_cast<Z80Cpu *>(ctx)->bus_.ram_ptr(addr, len);
}

} // namespace z80
