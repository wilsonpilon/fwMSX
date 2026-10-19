// fwMSX -- motor de fita: guarda a imagem carregada e decide entre os
// dois modos de CARREGAMENTO (ver doc/tape-spec.md):
//  - Rapido: gancho de BIOS "ED FE" em TAPION/TAPIN/TAPIOF (00E1h/00E4h/
//    00E7h), igual ao PatchZ80() do fMSX (resource/fMSX/fMSX/Patch.c),
//    so' que sobre um buffer em memoria em vez de um FILE*.
//  - Normal: pulsos de verdade avancados ciclo a ciclo (como o PSG/SCC/FM,
//    ver Machine::RunFrame), lidos pela porta de verdade (PSG R14 bit 7) e
//    ouvidos de verdade (saida de audio ao vivo).
// A GRAVACAO (CSAVE/BSAVE "CAS:") e' sempre pelo gancho de BIOS (TAPOON/
// TAPOUT/TAPOOF, 00EAh/00EDh/00F0h) -- nao existe "modo normal" de
// gravar (precisaria decodificar os pulsos que o programa gera de volta
// em bytes, como um "ripper" de WAV; fora de escopo por enquanto). Ver
// doc/tape-spec.md, secao 6.
// Codigo ORIGINAL do fwMSX (BSD-3-Clause).
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "../../memmap/cpp/memory_system.h"
#include "../../memmap/cpp/slot_memory_bus.h"
#include "../core/tape_pulse.h"
#include "tape_deck.h"
#include "tape_image.h"
#include "tape_io.h"

namespace tape {

enum class TapeMode { Fast, Normal };

// TapeWriteMode (o que o PROXIMO TAPOON faz) e kNoMark estao em tape_deck.h, junto com a fita inserida, a protecao e o ponto
// marcado: o motor herda do TapeDeck e acrescenta o que e' do emulador.

// Taxa de amostragem do audio sintetizado (onda quadrada do sinal de
// fita) -- mesma taxa do PSG/SCC/FM (psg::kSampleRate), repetida aqui
// para nao criar uma dependencia cruzada so' por uma constante.
constexpr int kSampleRate = 44100;

class TapeEngine : public TapeDeck, public memmap::TapeBiosHook {
public:
    explicit TapeEngine(memmap::MemorySystem &memory) : memory_(memory) {
        CaptureOriginalVectors();
        ApplyWritePatch(); // TAPOON/TAPOUT/TAPOOF: sempre pelo gancho, independente do modo de leitura
    }

    // Insert/Eject/NewBlank, a protecao contra gravacao (fitas de arquivo entram travadas, TAPOON falha se travada), o modo de
    // gravacao, AppendFile/ReadFile/SaveAs e o ponto marcado vem do TapeDeck (tape_deck.h).
    void Rewind() override;

    // Marca o arquivo `index` como o "ponto": TAPION busca a partir daqui, nao do inicio (nos dois modos), e
    // TapeWriteMode::OverwriteAtPoint trunca aqui para gravar. false se `index` for invalido.
    bool SeekToFile(std::size_t index) override;

    // Modo de CARREGAMENTO -- troca o patch da BIOS (sem reiniciar a
    // maquina, ver ApplyFastReadPatch()). Nao afeta a gravacao (sempre
    // pelo gancho, ver o comentario no topo do arquivo).
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

    // "Contagiros": um odometro simulado (nao e' fisicamente exato -- um
    // contador de giros de verdade acelera conforme a bobina de saida
    // engorda; aqui e' so' proporcional ao tempo de fita consumido, para
    // dar a mesma sensacao visual). Reinicia em 0 ao rebobinar/inserir/
    // ejetar/criar uma fita nova.
    long odometer() const;

    // memmap::TapeBiosHook -- ver slot_memory_bus.h.
    bool OnTapeBiosCall(uint16_t trap_pc, z80::Z80Cpu &cpu) override;

protected:
    // Fita trocada: religa os cursores (e descarta uma gravacao pela metade). Pulsos mudados: so' refaz o ponteiro do cursor.
    void OnReplaced() override {
        writing_ = false;
        Rewind();
    }
    void OnPulsesChanged() override { RebindPulses(); }

private:
    void RebindPulses();
    void CaptureOriginalVectors();
    void ApplyFastReadPatch(bool on);
    void ApplyWritePatch();
    bool OnTapion(uint16_t &af);
    bool OnTapin(uint16_t &af);
    bool OnTapoon(uint16_t &af);
    bool OnTapout(uint16_t &af);
    void FinalizeWrite();

    memmap::MemorySystem &memory_;

    TapeMode mode_ = TapeMode::Fast;

    // Modo rapido: cursor no fluxo "fast_bytes" (como o CasStream do fMSX).
    std::size_t fast_pos_ = 0;

    // Gravacao (sempre pelo gancho -- ver o comentario no topo do arquivo).
    bool writing_ = false; // entre um TAPOON com sucesso e o TAPOOF correspondente
    // Posicao (em fast_bytes) do cabecalho de 8 bytes e do inicio dos dados
    // da gravacao EM ANDAMENTO -- usados por FinalizeWrite() para gerar os
    // pulsos SO' do que foi escrito de verdade (a contagem exata de
    // TAPOUT), sem adivinhar onde o preenchimento de alinhamento termina
    // (ver doc/tape-spec.md, secao 5/6).
    std::size_t write_header_pos_ = 0;
    std::size_t write_data_start_ = 0;

    // Modo normal: cursor de pulsos + reamostragem para audio.
    TapePulseCursor pulse_cursor_{};
    bool motor_on_ = false;
    int cassette_in_level_ = 0;
    uint32_t cycle_acc_ = 0;
    bool live_ = false;
    std::vector<int16_t> live_buf_;

    bool vectors_captured_ = false;
    // 00E1-E2 (TAPION), 00E4-E5 (TAPIN), 00E7-E8 (TAPIOF),
    // 00EA-EB (TAPOON), 00ED-EE (TAPOUT), 00F0-F1 (TAPOOF).
    uint8_t original_vectors_[12] = {};
};

} // namespace tape
