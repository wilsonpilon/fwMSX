// fwMSX -- adaptador do Ym2413State (motor em C, ver src/fm/core/ym2413_state.h)
// para z80::IBus. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/fm-spec.md.
//
// Portas do MSX-MUSIC e do FM-PAC: 7Ch grava o numero do registrador (latch),
// 7Dh grava o dado nesse registrador. A leitura de 7Ch devolve o status, que
// aqui e' sempre 0 (os timers do OPLL nao sao emulados ainda).
#pragma once

#include <cstdint>
#include <vector>

#include "../../z80/cpp/z80_bus.h"
#include "../core/ym2413_state.h"

namespace fm {

// Taxa de saida das amostras geradas.
constexpr int kSampleRate = 44100;

class FmDevice : public z80::IBus {
public:
    FmDevice() { Reset(); }

    uint8_t read(uint16_t) override { return 0xFF; }
    void write(uint16_t, uint8_t) override {}

    uint8_t in(uint16_t port) override { return (port & 0xFF) == 0x7C ? 0x00 : 0xFF; }

    void out(uint16_t port, uint8_t value) override {
        switch (port & 0xFF) {
        case 0x7C: state_.latch = value & 0x3F; break;
        case 0x7D: ym2413_write_reg(&state_, state_.latch, value); break;
        default: break;
        }
    }

    // Avanca `z80_cycles` ciclos de Z80 (chamado pela maquina a cada instrucao,
    // como o PSG e o SCC). Em modo ao vivo, acumula as amostras geradas; senao
    // so' o estado avanca.
    void Advance(int z80_cycles) {
        if (!live_) {
            ym2413_advance(&state_, z80_cycles, kSampleRate, nullptr, 0);
            return;
        }
        const int max = static_cast<int>(static_cast<int64_t>(z80_cycles) * kSampleRate / YM2413_Z80_CLOCK) + 2;
        scratch_.resize(static_cast<size_t>(max));
        const int n = ym2413_advance(&state_, z80_cycles, kSampleRate, scratch_.data(), max);
        live_buf_.insert(live_buf_.end(), scratch_.begin(), scratch_.begin() + n);
        // Ninguem drenando: guarda no maximo 1 s e descarta o mais antigo.
        if (live_buf_.size() > static_cast<size_t>(kSampleRate))
            live_buf_.erase(live_buf_.begin(), live_buf_.end() - kSampleRate);
    }

    void EnableLive(bool on) {
        live_ = on;
        if (!on) live_buf_.clear();
    }
    bool live() const { return live_; }
    void TakeLive(std::vector<int16_t> &out) {
        out.insert(out.end(), live_buf_.begin(), live_buf_.end());
        live_buf_.clear();
    }

    // Reset de maquina: o chip volta ao estado de ligado (sem nota tocando).
    void Reset() { ym2413_reset(&state_); }

    Ym2413State &state() { return state_; }
    const Ym2413State &state() const { return state_; }

private:
    Ym2413State state_{};
    bool live_ = false;
    std::vector<int16_t> live_buf_;
    std::vector<int16_t> scratch_;
};

} // namespace fm
