// fwMSX -- adaptador do VdpState (motor "digital" em C, ver
// src/vdp/core/vdp_state.h) para z80::IBus. Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver doc/vdp-spec.md, secao 3.3.
#pragma once

#include <cstdint>

#include "../../z80/cpp/z80_bus.h"
#include "../core/vdp_state.h"

namespace vdp {

// Dispositivo SOMENTE DE PORTA (98h-9Bh) -- nunca deve ser registrado
// como o dispositivo de MEMORIA de um z80::CompositeBus (esse papel e'
// do memmap::SlotMemoryBus). `read`/`write`/`on_bios_patch`/`ram_ptr`
// tem implementacoes-padrao inofensivas so' para satisfazer a interface
// IBus por completo -- na pratica, nunca sao chamadas dado como o
// CompositeBus e' montado (ver z80_debug_shell_startup.cpp).
class VdpDevice : public z80::IBus {
public:
    VdpDevice() { vdp_reset(&state_); }

    uint8_t read(uint16_t) override { return 0; }
    void write(uint16_t, uint8_t) override {}

    uint8_t in(uint16_t port) override { return vdp_in(&state_, port); }
    void out(uint16_t port, uint8_t value) override { vdp_out(&state_, port, value); }

    // Avanca a maquina de estados um "meio-scanline" -- ver
    // vdp_step_scanline() para o que isso cobre (e nao cobre) nesta
    // fase. Nao faz parte de IBus; a orquestracao (Z80DebugSession::
    // CmdRun/CmdStep) chama isto diretamente.
    VdpStepResult Step() { return vdp_step_scanline(&state_); }

    void Reset() { vdp_reset_keep_model(&state_); }
    // MSX1 (16KB, TMS9918) ou MSX2 (128KB, V9938): ver vdp_set_model().
    void SetModel(int model) { vdp_set_model(&state_, model); }

    VdpState &state() { return state_; }
    const VdpState &state() const { return state_; }

private:
    VdpState state_{};
};

} // namespace vdp
