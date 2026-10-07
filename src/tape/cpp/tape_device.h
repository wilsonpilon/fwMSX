// fwMSX -- motor de fita: guarda a imagem carregada e decide entre os
// dois modos de carregamento (ver doc/tape-spec.md):
//  - Rapido: gancho de BIOS "ED FE" em TAPION/TAPIN/TAPIOF (00E1h/00E4h/
//    00E7h), igual ao PatchZ80() do fMSX (resource/fMSX/fMSX/Patch.c),
//    so' que sobre um buffer em memoria em vez de um FILE*.
//  - Normal: pulsos de verdade avancados ciclo a ciclo (como o PSG/SCC/FM,
//    ver Machine::RunFrame), lidos pela porta de verdade (PSG R14 bit 7) e
//    ouvidos de verdade (saida de audio ao vivo).
// Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../../memmap/cpp/memory_system.h"
#include "../../memmap/cpp/slot_memory_bus.h"
#include "../core/tape_pulse.h"
#include "tape_image.h"

namespace tape {

enum class TapeMode { Fast, Normal };

// Taxa de amostragem do audio sintetizado (onda quadrada do sinal de
// fita) -- mesma taxa do PSG/SCC/FM (psg::kSampleRate), repetida aqui
// para nao criar uma dependencia cruzada so' por uma constante.
constexpr int kSampleRate = 44100;

class TapeEngine : public memmap::TapeBiosHook {
public:
    explicit TapeEngine(memmap::MemorySystem &memory) : memory_(memory) { CaptureOriginalVectors(); }

    // Insere/ejeta/rebobina a fita (.cas, .tsx ou .tzx pela extensao).
    bool Insert(const std::string &path, std::string &error);
    void Eject();
    void Rewind();
    bool inserted() const { return inserted_; }
    const std::string &path() const { return path_; }
    const std::vector<TapeFileEntry> &files() const { return image_.files; }
    const std::vector<std::string> &skipped_blocks() const { return image_.skipped_blocks; }

    // Modo de carregamento -- troca o patch da BIOS (sem reiniciar a
    // maquina, ver ApplyFastPatch()).
    void SetMode(TapeMode mode);
    TapeMode mode() const { return mode_; }

    // PPI (porta C, AAh, bit 4): rele do motor -- ligado/desligado pela
    // Machine quando o bit muda (nunca pelo proprio PPI, que nao sabe nada
    // de fita). Ver doc/tape-spec.md.
    void SetMotor(bool on) { motor_on_ = on; }
    bool motor_on() const { return motor_on_; }

    // Avanca `z80_cycles` (chamado a cada instrucao, como PsgDevice::Advance) --
    // so' tem efeito no modo Normal, com motor ligado e fita inserida.
    void Advance(int z80_cycles);

    // Nivel atual do sinal (0/1) para o bit 7 do R14 do PSG (CASRD).
    int CassetteInLevel() const { return cassette_in_level_; }

    // Audio ao vivo (onda quadrada do sinal), mesmo padrao do PsgDevice.
    void EnableLive(bool on) {
        live_ = on;
        if (!on) live_buf_.clear();
    }
    void TakeLive(std::vector<int16_t> &out) {
        out.insert(out.end(), live_buf_.begin(), live_buf_.end());
        live_buf_.clear();
    }

    // Posicao/duracao em segundos (barra de progresso da janela "Fita K7").
    double position_seconds() const;
    double duration_seconds() const;

    // memmap::TapeBiosHook -- ver slot_memory_bus.h.
    bool OnTapeBiosCall(uint16_t trap_pc, z80::Z80Cpu &cpu) override;

private:
    void CaptureOriginalVectors();
    void ApplyFastPatch(bool on);

    memmap::MemorySystem &memory_;

    bool inserted_ = false;
    std::string path_;
    TapeImage image_;
    TapeMode mode_ = TapeMode::Fast;

    // Modo rapido: cursor no fluxo "fast_bytes" (como o CasStream do fMSX).
    std::size_t fast_pos_ = 0;

    // Modo normal: cursor de pulsos + reamostragem para audio.
    TapePulseCursor pulse_cursor_{};
    bool motor_on_ = false;
    int cassette_in_level_ = 0;
    uint32_t cycle_acc_ = 0;
    bool live_ = false;
    std::vector<int16_t> live_buf_;

    bool vectors_captured_ = false;
    uint8_t original_vectors_[6] = {}; // 00E1-E2 (TAPION), 00E4-E5 (TAPIN), 00E7-E8 (TAPIOF)
};

} // namespace tape
