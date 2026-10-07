#include "tape_device.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>

#include "../../z80/common/z80_state.h"
#include "../../z80/cpp/z80_cpu.h"
#include "../asm/pulse_sum.h"
#include "cas_format.h"
#include "cas_reader.h"
#include "tsx_writer.h"
#include "tzx_reader.h"

namespace tape {

namespace {

// Enderecos fixos da BIOS do MSX (jump table de pagina 0, mesmos valores
// conferidos em resource/fMSX/fMSX/Patch.c): leitura (TAPION/TAPIN/
// TAPIOF) e gravacao (TAPOON/TAPOUT/TAPOOF).
constexpr uint16_t kTapionAddr = 0x00E1;
constexpr uint16_t kTapinAddr = 0x00E4;
constexpr uint16_t kTapiofAddr = 0x00E7;
constexpr uint16_t kTapoonAddr = 0x00EA;
constexpr uint16_t kTapoutAddr = 0x00ED;
constexpr uint16_t kTapoofAddr = 0x00F0;
constexpr int kZ80Clock = 3579545;

bool EndsWithCi(const std::string &s, const char *suffix) {
    const std::size_t n = std::strlen(suffix);
    if (s.size() < n) return false;
    return std::equal(s.end() - static_cast<std::ptrdiff_t>(n), s.end(), suffix,
                       [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == b; });
}

bool WriteRawFile(const std::string &path, const std::vector<uint8_t> &bytes, std::string &error) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f || !f.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()))) {
        error = "nao foi possivel gravar '" + path + "'";
        return false;
    }
    return true;
}

} // namespace

void TapeEngine::CaptureOriginalVectors() {
    if (vectors_captured_) return;
    const uint16_t addrs[6] = {kTapionAddr, kTapinAddr, kTapiofAddr, kTapoonAddr, kTapoutAddr, kTapoofAddr};
    for (int i = 0; i < 6; ++i) {
        original_vectors_[i * 2] = memory_.PeekSlot(0, 0, addrs[i]);
        original_vectors_[i * 2 + 1] = memory_.PeekSlot(0, 0, static_cast<uint16_t>(addrs[i] + 1));
    }
    vectors_captured_ = true;
}

void TapeEngine::ApplyFastReadPatch(bool on) {
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

void TapeEngine::ApplyWritePatch() {
    // Gravacao e' sempre pelo gancho (ver o comentario no topo de
    // tape_device.h) -- nunca desfeito, independente do modo de leitura.
    static const uint8_t kEdFe[2] = {0xED, 0xFE};
    memory_.PatchRomBytes(0, 0, kTapoonAddr, kEdFe, 2);
    memory_.PatchRomBytes(0, 0, kTapoutAddr, kEdFe, 2);
    memory_.PatchRomBytes(0, 0, kTapoofAddr, kEdFe, 2);
}

void TapeEngine::SetMode(TapeMode mode) {
    mode_ = mode;
    ApplyFastReadPatch(mode == TapeMode::Fast);
}

bool TapeEngine::Insert(const std::string &path, std::string &error) {
    TapeImage img;
    const bool is_tsx = EndsWithCi(path, ".tsx") || EndsWithCi(path, ".tzx");
    if (!(is_tsx ? LoadTzxImage(path, img, error) : LoadCasImage(path, img, error))) return false;
    image_ = std::move(img);
    path_ = path;
    inserted_ = true;
    read_only_ = true; // fita de arquivo: protegida por padrao (ver doc/tape-spec.md, secao 6)
    write_mode_ = TapeWriteMode::AppendAtEnd;
    marked_file_ = kNoMark;
    writing_ = false;
    Rewind();
    return true;
}

bool TapeEngine::NewBlank(const std::string &path, std::string &error) {
    image_ = TapeImage{};
    path_ = path;
    inserted_ = true;
    read_only_ = false; // fita nova: destravada (e' o motivo de criar uma)
    write_mode_ = TapeWriteMode::AppendAtEnd;
    marked_file_ = kNoMark;
    writing_ = false;
    Rewind();
    // Grava o arquivo ja' (so' o cabecalho, sem blocos) para o caminho
    // existir no disco desde ja' -- mesma logica de Persist().
    if (EndsWithCi(path, ".tsx") || EndsWithCi(path, ".tzx")) return WriteTsxFromCas(image_.fast_bytes, path, error);
    return WriteRawFile(path, image_.fast_bytes, error);
}

void TapeEngine::Eject() {
    inserted_ = false;
    path_.clear();
    image_ = TapeImage{};
    read_only_ = true;
    marked_file_ = kNoMark;
    writing_ = false;
    Rewind();
}

void TapeEngine::Rewind() {
    fast_pos_ = 0;
    tape_cursor_set(&pulse_cursor_, image_.pulses.empty() ? nullptr : image_.pulses.data(),
                     static_cast<uint32_t>(image_.pulses.size()));
    cassette_in_level_ = 0;
    cycle_acc_ = 0;
}

bool TapeEngine::SeekToFile(std::size_t index) {
    if (index >= image_.files.size()) return false;
    marked_file_ = index;
    const TapeFileEntry &f = image_.files[index];
    fast_pos_ = f.fast_byte_offset;
    tape_cursor_seek(&pulse_cursor_, static_cast<uint32_t>(f.pulse_index));
    cassette_in_level_ = pulse_cursor_.level;
    return true;
}

void TapeEngine::RebuildFromFastBytes() {
    image_.pulses.clear();
    std::vector<std::pair<std::size_t, std::size_t>> marks;
    SynthesizeCasPulses(image_.fast_bytes, image_.pulses, &marks);
    image_.files = ScanCasFiles(image_.fast_bytes, marks);
    image_.from_tsx = false;
    tape_cursor_set(&pulse_cursor_, image_.pulses.empty() ? nullptr : image_.pulses.data(),
                     static_cast<uint32_t>(image_.pulses.size()));
}

void TapeEngine::Persist() {
    if (path_.empty()) return;
    std::string err; // melhor esforco: uma falha ao gravar no disco nao desfaz a gravacao em memoria
    if (EndsWithCi(path_, ".tsx") || EndsWithCi(path_, ".tzx")) {
        WriteTsxFromCas(image_.fast_bytes, path_, err);
    } else {
        WriteRawFile(path_, image_.fast_bytes, err);
    }
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

long TapeEngine::odometer() const {
    // So' uma simulacao visual (ver doc/tape-spec.md, secao 6): um
    // contador mecanico de verdade acelera conforme a bobina de saida
    // engorda; aqui e' so' proporcional ao tempo de fita consumido.
    return static_cast<long>(position_seconds() * 15.0);
}

bool TapeEngine::OnTapion(uint16_t &af) {
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
    return true;
}

bool TapeEngine::OnTapin(uint16_t &af) {
    af |= Z80_C_FLAG;
    if (inserted_ && fast_pos_ < image_.fast_bytes.size()) {
        const uint8_t byte = image_.fast_bytes[fast_pos_++];
        af = static_cast<uint16_t>((af & 0x00FF) | (static_cast<uint16_t>(byte) << 8));
        af &= static_cast<uint16_t>(~Z80_C_FLAG);
    } else {
        fast_pos_ = 0;
    }
    return true;
}

bool TapeEngine::OnTapoon(uint16_t &af) {
    af |= Z80_C_FLAG;
    writing_ = false;
    if (!inserted_ || read_only_) return true; // fita protegida (ou nenhuma): "Device I/O error"

    switch (write_mode_) {
    case TapeWriteMode::NewTape:
        image_.fast_bytes.clear();
        marked_file_ = kNoMark;
        break;
    case TapeWriteMode::OverwriteAtPoint:
        if (marked_file_ != kNoMark && marked_file_ < image_.files.size()) {
            const std::size_t cut = image_.files[marked_file_].fast_byte_offset;
            if (cut < image_.fast_bytes.size()) image_.fast_bytes.resize(cut);
        }
        break;
    case TapeWriteMode::AppendAtEnd:
        break; // grava depois do que ja' existe
    }

    // Alinha a um multiplo de 8 antes do cabecalho -- mesma regra do
    // TAPOON de verdade (ver o comentario no caso #4B de tzx_reader.cpp).
    if (image_.fast_bytes.size() % kCasHeader.size()) {
        image_.fast_bytes.resize(image_.fast_bytes.size() + (kCasHeader.size() - image_.fast_bytes.size() % kCasHeader.size()), 0);
    }
    image_.fast_bytes.insert(image_.fast_bytes.end(), kCasHeader.begin(), kCasHeader.end());
    writing_ = true;
    af &= static_cast<uint16_t>(~Z80_C_FLAG);
    return true;
}

bool TapeEngine::OnTapout(uint16_t &af) {
    af |= Z80_C_FLAG;
    if (writing_) {
        image_.fast_bytes.push_back(static_cast<uint8_t>((af >> 8) & 0xFF));
        af &= static_cast<uint16_t>(~Z80_C_FLAG);
    }
    return true;
}

bool TapeEngine::OnTapeBiosCall(uint16_t trap_pc, z80::Z80Cpu &cpu) {
    // Mesma logica de TAPION/TAPIN/TAPIOF/TAPOON/TAPOUT/TAPOOF do fMSX
    // (Patch.c), sobre o fluxo "fast_bytes" em memoria em vez de um FILE*
    // (fseek/ftell/fread/fwrite) -- ver doc/tape-spec.md.
    uint16_t af = cpu.af();
    switch (trap_pc) {
    case kTapionAddr:
        OnTapion(af);
        break;
    case kTapinAddr:
        OnTapin(af);
        break;
    case kTapiofAddr: // TAPIOF: sempre sucesso.
        af &= static_cast<uint16_t>(~Z80_C_FLAG);
        break;
    case kTapoonAddr:
        OnTapoon(af);
        break;
    case kTapoutAddr:
        OnTapout(af);
        break;
    case kTapoofAddr: // TAPOOF: sempre sucesso; finaliza a gravacao, se houver.
        if (writing_) {
            writing_ = false;
            RebuildFromFastBytes();
            Persist();
        }
        af &= static_cast<uint16_t>(~Z80_C_FLAG);
        break;
    default:
        return false; // nao e' um dos nossos enderecos (nao deveria acontecer)
    }
    cpu.set_af(af);
    return true;
}

} // namespace tape
