// fwMSX -- adaptador do PsgState (motor em C, ver src/psg/core/psg_state.h)
// para z80::IBus. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver
// doc/psg-spec.md.
#pragma once

#include <cstdint>
#include <vector>

#include "../../z80/cpp/z80_bus.h"
#include "../core/psg_state.h"

namespace psg {

// Taxa de saida das amostras geradas/gravadas.
constexpr int kSampleRate = 44100;

// Dispositivo SOMENTE DE PORTA (A0h-A2h): A0h = latch do registrador,
// A1h = escrita, A2h = leitura. Registrar com
// z80::CompositeBus::RegisterPortRange(0xA0, 0xA2, ...).
class PsgDevice : public z80::IBus {
public:
    PsgDevice() { psg_reset(&state_); }

    uint8_t read(uint16_t) override { return 0; }
    void write(uint16_t, uint8_t) override {}

    uint8_t in(uint16_t port) override { return (port & 0xFF) == 0xA2 ? psg_read_data(&state_) : 0xFF; }

    void out(uint16_t port, uint8_t value) override {
        switch (port & 0xFF) {
        case 0xA0: psg_write_latch(&state_, value); break;
        case 0xA1: psg_write_data(&state_, value); break;
        default: break;
        }
    }

    // Avanca `z80_cycles` ciclos de Z80 (chamado pela sessao a cada
    // instrucao executada, como DriveVdp). Quando gravando (capture) e/ou
    // em modo ao vivo (live), acumula as amostras geradas; senao so' o
    // estado avanca.
    void Advance(int z80_cycles) {
        if (!recording_ && !live_) {
            psg_advance(&state_, z80_cycles, kSampleRate, nullptr, 0);
            return;
        }
        const int max = static_cast<int>(static_cast<int64_t>(z80_cycles) * kSampleRate / PSG_Z80_CLOCK) + 2;
        scratch_.resize(static_cast<size_t>(max));
        const int n = psg_advance(&state_, z80_cycles, kSampleRate, scratch_.data(), max);
        if (recording_) capture_.insert(capture_.end(), scratch_.begin(), scratch_.begin() + n);
        if (live_) {
            live_buf_.insert(live_buf_.end(), scratch_.begin(), scratch_.begin() + n);
            // Ninguem drenando (ex.: janela pausada/sem audio): nao cresce sem
            // limite -- guarda no maximo 1 s e descarta o mais antigo.
            if (live_buf_.size() > static_cast<size_t>(kSampleRate))
                live_buf_.erase(live_buf_.begin(), live_buf_.end() - kSampleRate);
        }
    }

    // Modo ao vivo (saida de audio): as amostras geradas ficam num buffer que
    // o consumidor esvazia com TakeLive() (normalmente uma vez por quadro).
    void EnableLive(bool on) {
        live_ = on;
        if (!on) live_buf_.clear();
    }
    bool live() const { return live_; }
    // Move as amostras ao vivo acumuladas para `out` (acrescenta) e zera o buffer.
    void TakeLive(std::vector<int16_t> &out) {
        out.insert(out.end(), live_buf_.begin(), live_buf_.end());
        live_buf_.clear();
    }

    // Reset de maquina (ResetMSX() do fMSX): registradores voltam ao
    // estado inicial; uma gravacao em andamento continua.
    void Reset() { psg_reset(&state_); }

    void StartRecording() { recording_ = true; }
    void StopRecording() { recording_ = false; }
    bool recording() const { return recording_; }
    void ClearCapture() { capture_.clear(); }
    const std::vector<int16_t> &capture() const { return capture_; }

    PsgState &state() { return state_; }
    const PsgState &state() const { return state_; }

private:
    PsgState state_{};
    bool recording_ = false;
    bool live_ = false;
    std::vector<int16_t> capture_;
    std::vector<int16_t> live_buf_;
    std::vector<int16_t> scratch_;
};

} // namespace psg
