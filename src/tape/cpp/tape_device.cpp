#include "tape_device.h"

#include <algorithm>
#include <cctype>
#include <cstring>

#include "../../z80/common/z80_state.h"
#include "../../z80/cpp/z80_cpu.h"
#include "../asm/pulse_sum.h"
#include "cas_format.h"
#include "cas_reader.h"
#include "tzx_reader.h"

namespace tape {

namespace {

// Enderecos fixos da BIOS do MSX (jump table de pagina 0, mesmos valores
// conferidos em resource/fMSX/fMSX/Patch.c): TAPION, TAPIN e TAPIOF. So'
// o lado de LEITURA -- TAPOON/TAPOUT/TAPOOF (gravacao) nao sao tocados,
// fora de escopo (ver doc/tape-spec.md).
constexpr uint16_t kTapionAddr = 0x00E1;
constexpr uint16_t kTapinAddr = 0x00E4;
constexpr uint16_t kTapiofAddr = 0x00E7;
constexpr int kZ80Clock = 3579545;

bool EndsWithCi(const std::string &s, const char *suffix) {
    const std::size_t n = std::strlen(suffix);
    if (s.size() < n) return false;
    return std::equal(s.end() - static_cast<std::ptrdiff_t>(n), s.end(), suffix,
                       [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == b; });
}

} // namespace

void TapeEngine::CaptureOriginalVectors() {
    if (vectors_captured_) return;
    original_vectors_[0] = memory_.PeekSlot(0, 0, kTapionAddr);
    original_vectors_[1] = memory_.PeekSlot(0, 0, kTapionAddr + 1);
    original_vectors_[2] = memory_.PeekSlot(0, 0, kTapinAddr);
    original_vectors_[3] = memory_.PeekSlot(0, 0, kTapinAddr + 1);
    original_vectors_[4] = memory_.PeekSlot(0, 0, kTapiofAddr);
    original_vectors_[5] = memory_.PeekSlot(0, 0, kTapiofAddr + 1);
    vectors_captured_ = true;
}

void TapeEngine::ApplyFastPatch(bool on) {
    static const uint8_t kEdFe[2] = {0xED, 0xFE};
    if (on) {
        memory_.PatchRomBytes(0, 0, kTapionAddr, kEdFe, 2);
        memory_.PatchRomBytes(0, 0, kTapinAddr, kEdFe, 2);
        memory_.PatchRomBytes(0, 0, kTapiofAddr, kEdFe, 2);
    } else {
        memory_.PatchRomBytes(0, 0, kTapionAddr, &original_vectors_[0], 2);
        memory_.PatchRomBytes(0, 0, kTapinAddr, &original_vectors_[2], 2);
        memory_.PatchRomBytes(0, 0, kTapiofAddr, &original_vectors_[4], 2);
    }
}

void TapeEngine::SetMode(TapeMode mode) {
    mode_ = mode;
    ApplyFastPatch(mode == TapeMode::Fast);
}

bool TapeEngine::Insert(const std::string &path, std::string &error) {
    TapeImage img;
    const bool is_tsx = EndsWithCi(path, ".tsx") || EndsWithCi(path, ".tzx");
    if (!(is_tsx ? LoadTzxImage(path, img, error) : LoadCasImage(path, img, error))) return false;
    image_ = std::move(img);
    path_ = path;
    inserted_ = true;
    Rewind();
    return true;
}

void TapeEngine::Eject() {
    inserted_ = false;
    path_.clear();
    image_ = TapeImage{};
    Rewind();
}

void TapeEngine::Rewind() {
    fast_pos_ = 0;
    tape_cursor_set(&pulse_cursor_, image_.pulses.empty() ? nullptr : image_.pulses.data(),
                     static_cast<uint32_t>(image_.pulses.size()));
    cassette_in_level_ = 0;
    cycle_acc_ = 0;
}

void TapeEngine::Advance(int z80_cycles) {
    if (mode_ != TapeMode::Normal || !motor_on_ || !inserted_ || image_.pulses.empty()) return;
    cassette_in_level_ = tape_cursor_advance(&pulse_cursor_, z80_cycles);
    if (!live_) return;
    // Reamostragem simples (acumulador de ciclos -> amostras em
    // kSampleRate): e' uma onda quadrada, sem envelope, entao so' precisa
    // do nivel atual -- diferente do PSG (psg_advance), que mistura 3
    // canais com volume e ruido.
    cycle_acc_ += static_cast<uint32_t>(z80_cycles) * static_cast<uint32_t>(kSampleRate);
    while (cycle_acc_ >= static_cast<uint32_t>(kZ80Clock)) {
        cycle_acc_ -= static_cast<uint32_t>(kZ80Clock);
        live_buf_.push_back(cassette_in_level_ ? static_cast<int16_t>(6000) : static_cast<int16_t>(-6000));
    }
    if (live_buf_.size() > static_cast<std::size_t>(kSampleRate)) {
        live_buf_.erase(live_buf_.begin(), live_buf_.end() - kSampleRate);
    }
}

double TapeEngine::position_seconds() const {
    return static_cast<double>(tape_sum_cycles(image_.pulses.data(), pulse_cursor_.index)) / kZ80Clock;
}

double TapeEngine::duration_seconds() const {
    return static_cast<double>(tape_sum_cycles(image_.pulses.data(), static_cast<uint32_t>(image_.pulses.size()))) /
           kZ80Clock;
}

bool TapeEngine::OnTapeBiosCall(uint16_t trap_pc, z80::Z80Cpu &cpu) {
    // Mesma logica de TAPION/TAPIN/TAPIOF do fMSX (Patch.c), sobre o fluxo
    // "fast_bytes" em memoria em vez de um FILE* (fseek/ftell/fread) -- ver
    // doc/tape-spec.md.
    uint16_t af = cpu.af();
    switch (trap_pc) {
    case kTapionAddr: { // TAPION: alinha a 8 bytes e procura o proximo cabecalho.
        af |= Z80_C_FLAG;
        if (inserted_ && !image_.fast_bytes.empty()) {
            const std::vector<uint8_t> &b = image_.fast_bytes;
            if (fast_pos_ % kCasHeader.size()) fast_pos_ += kCasHeader.size() - (fast_pos_ % kCasHeader.size());
            bool found = false;
            while (fast_pos_ + kCasHeader.size() <= b.size()) {
                if (std::equal(kCasHeader.begin(), kCasHeader.end(), b.begin() + static_cast<std::ptrdiff_t>(fast_pos_))) {
                    fast_pos_ += kCasHeader.size();
                    found = true;
                    break;
                }
                fast_pos_ += kCasHeader.size();
            }
            if (found) af &= static_cast<uint16_t>(~Z80_C_FLAG);
            else fast_pos_ = 0;
        }
        break;
    }
    case kTapinAddr: { // TAPIN: le o proximo byte do fluxo.
        af |= Z80_C_FLAG;
        if (inserted_ && fast_pos_ < image_.fast_bytes.size()) {
            const uint8_t byte = image_.fast_bytes[fast_pos_++];
            af = static_cast<uint16_t>(((af & 0x00FF)) | (static_cast<uint16_t>(byte) << 8));
            af &= static_cast<uint16_t>(~Z80_C_FLAG);
        } else {
            fast_pos_ = 0;
        }
        break;
    }
    case kTapiofAddr: // TAPIOF: sempre sucesso (so' desliga o motor/porta, ja' simulado).
        af &= static_cast<uint16_t>(~Z80_C_FLAG);
        break;
    default:
        return false; // nao e' um dos nossos 3 enderecos (nao deveria acontecer)
    }
    cpu.set_af(af);
    return true;
}

} // namespace tape
