// Wrapper C++ de orquestracao do motor Z80 -- design proprio
// (BSD-3-Clause), delega a execucao ao motor em C (src/z80/core/). Ver
// doc/z80-core-spec.md, secao 3.3.
#pragma once

#include <cstdint>

#include "../core/z80_bus.h"
#include "../common/z80_state.h"
#include "z80_bus.h"

namespace z80 {

class Z80Cpu {
public:
    explicit Z80Cpu(IBus &bus);

    void reset();

    // Executa ate consumir `cycles` ciclos; devolve o saldo restante
    // (possivelmente negativo) -- ver z80_core.h/z80_run().
    int run(int cycles);

    void interrupt(uint16_t vector);

    // Acessores minimos para um futuro debugger (Fase 4) -- sem
    // disassembler/breakpoints nesta fase.
    uint16_t pc() const { return state_.pc.w; }
    uint16_t sp() const { return state_.sp.w; }
    uint16_t af() const { return state_.af.w; }
    uint16_t bc() const { return state_.bc.w; }
    uint16_t de() const { return state_.de.w; }
    uint16_t hl() const { return state_.hl.w; }
    uint16_t ix() const { return state_.ix.w; }
    uint16_t iy() const { return state_.iy.w; }
    uint8_t iff() const { return state_.iff; }

    void set_pc(uint16_t v) { state_.pc.w = v; }
    void set_sp(uint16_t v) { state_.sp.w = v; }
    void set_af(uint16_t v) { state_.af.w = v; }
    void set_bc(uint16_t v) { state_.bc.w = v; }
    void set_de(uint16_t v) { state_.de.w = v; }
    void set_hl(uint16_t v) { state_.hl.w = v; }
    void set_ix(uint16_t v) { state_.ix.w = v; }
    void set_iy(uint16_t v) { state_.iy.w = v; }

    Z80State &state() { return state_; }
    const Z80State &state() const { return state_; }

private:
    static uint8_t trampoline_read(void *ctx, uint16_t addr);
    static void trampoline_write(void *ctx, uint16_t addr, uint8_t value);
    static uint8_t trampoline_in(void *ctx, uint16_t port);
    static void trampoline_out(void *ctx, uint16_t port, uint8_t value);
    static void trampoline_patch(void *ctx, Z80State *state);
    static void trampoline_jump(void *ctx, uint16_t pc);

    IBus &bus_;
    Z80State state_{};
    Z80Bus c_bus_{};
};

} // namespace z80
