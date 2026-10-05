// fwMSX -- adaptador do SccState (motor em C, ver src/scc/core/scc_state.h)
// para o slot de cartucho. Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver
// doc/scc-spec.md.
//
// Protocolo de ativacao, como o MSX.c do fMSX: o chip liga quando um cartucho
// Konami5 recebe 3Fh em 9000h, ou um Gen8 recebe 3Fh em 8000h-9FFFh, e desliga
// com qualquer outro valor. Ligado, 9800h-98FFh vai para o chip (leitura e
// escrita); a escrita nesse intervalo nao chega ao mapper.
#pragma once

#include <cstdint>
#include <vector>

#include "../../memmap/cpp/slot_memory_bus.h"
#include "../core/scc_state.h"

namespace scc {

// Taxa de saida das amostras geradas.
constexpr int kSampleRate = 44100;

class SccDevice : public memmap::SlotCartIo {
public:
    SccDevice() { Reset(); }

    bool CartRead(uint16_t addr, MemMapMapperType mapper, uint8_t &value) override {
        if (!Responds(mapper) || !enabled_ || !InWindow(addr)) return false;
        value = scc_read(&state_, static_cast<uint8_t>(addr), (addr & 0x2000) != 0);
        return true;
    }

    bool CartWrite(uint16_t addr, uint8_t value, MemMapMapperType mapper) override {
        if (!Responds(mapper)) return false;
        if (enabled_ && InWindow(addr)) {
            scc_write(&state_, static_cast<uint8_t>(addr), value, (addr & 0x2000) != 0);
            return true;
        }
        const bool enable_write = mapper == MEMMAP_MAPPER_KONAMI5 ? addr == 0x9000 : (addr >= 0x8000 && addr <= 0x9FFF);
        if (enable_write) enabled_ = value == 0x3F;
        return false;
    }

    // Avanca `z80_cycles` ciclos de Z80 (chamado pela maquina a cada instrucao,
    // como o PSG). Em modo ao vivo, acumula as amostras geradas; senao so' o
    // estado avanca.
    void Advance(int z80_cycles) {
        if (!live_) {
            scc_advance(&state_, z80_cycles, kSampleRate, nullptr, 0);
            return;
        }
        const int max = static_cast<int>(static_cast<int64_t>(z80_cycles) * kSampleRate / SCC_Z80_CLOCK) + 2;
        scratch_.resize(static_cast<size_t>(max));
        const int n = scc_advance(&state_, z80_cycles, kSampleRate, scratch_.data(), max);
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

    // Reset de maquina: o chip desliga e os registradores voltam a zero.
    void Reset() {
        scc_reset(&state_);
        enabled_ = false;
    }

    bool enabled() const { return enabled_; }
    SccState &state() { return state_; }
    const SccState &state() const { return state_; }

private:
    static bool Responds(MemMapMapperType mapper) {
        return mapper == MEMMAP_MAPPER_KONAMI5 || mapper == MEMMAP_MAPPER_GEN8;
    }
    static bool InWindow(uint16_t addr) { return (addr & 0xDF00) == 0x9800; }

    SccState state_{};
    bool enabled_ = false;
    bool live_ = false;
    std::vector<int16_t> live_buf_;
    std::vector<int16_t> scratch_;
};

} // namespace scc
