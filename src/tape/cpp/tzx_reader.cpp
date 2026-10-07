#include "tzx_reader.h"

#include <algorithm>
#include <cstring>
#include <fstream>

#include "../core/kcs_codec.h"
#include "cas_format.h"
#include "cas_reader.h"

namespace tape {

namespace {

constexpr uint32_t kZ80Hz = 3579545;

void SinkAppend(void *ctx, uint32_t t) { static_cast<std::vector<uint32_t> *>(ctx)->push_back(t); }

// Cursor com limites -- os campos de cada bloco do TZX (ver
// resource/makeTSX/docs/TZX_format.md) tem tamanho fixo ou dado por um
// campo anterior; `fail()` fica ligado se qualquer leitura passar do fim
// do arquivo, e o chamador para' o parser nesse caso.
class Cursor {
public:
    Cursor(const uint8_t *p, std::size_t n) : p_(p), n_(n) {}

    bool ok(std::size_t need) const { return pos_ + need <= n_; }
    bool fail() const { return fail_; }
    bool eof() const { return pos_ >= n_; }
    std::size_t pos() const { return pos_; }

    uint8_t u8() {
        if (!ok(1)) { fail_ = true; return 0; }
        return p_[pos_++];
    }
    uint16_t u16() {
        if (!ok(2)) { fail_ = true; return 0; }
        const uint16_t v = static_cast<uint16_t>(p_[pos_] | (p_[pos_ + 1] << 8));
        pos_ += 2;
        return v;
    }
    uint32_t u24() {
        if (!ok(3)) { fail_ = true; return 0; }
        const uint32_t v = static_cast<uint32_t>(p_[pos_] | (p_[pos_ + 1] << 8) | (p_[pos_ + 2] << 16));
        pos_ += 3;
        return v;
    }
    uint32_t u32() {
        if (!ok(4)) { fail_ = true; return 0; }
        const uint32_t v = static_cast<uint32_t>(p_[pos_] | (p_[pos_ + 1] << 8) | (p_[pos_ + 2] << 16) | (static_cast<uint32_t>(p_[pos_ + 3]) << 24));
        pos_ += 4;
        return v;
    }
    const uint8_t *ptr(std::size_t len) {
        if (!ok(len)) { fail_ = true; return nullptr; }
        const uint8_t *r = p_ + pos_;
        pos_ += len;
        return r;
    }
    void skip(std::size_t len) {
        if (!ok(len)) { fail_ = true; pos_ = n_; return; }
        pos_ += len;
    }

private:
    const uint8_t *p_;
    std::size_t n_;
    std::size_t pos_ = 0;
    bool fail_ = false;
};

// Pausa entre blocos (TZX_format.md, secao 2, "Pause block"): um pulso
// curto no nivel oposto para terminar a borda, depois silencio (nivel
// baixo) pelo resto do tempo. `ms`==0 e' ignorado (nao muda o nivel).
void EmitPauseMs(std::vector<uint32_t> &pulses, uint32_t ms) {
    if (ms == 0) return;
    constexpr uint32_t kEdgeFinishTStates = 3580; // ~1ms a 3.58 MHz
    const uint64_t total = (static_cast<uint64_t>(ms) * kZ80Hz) / 1000;
    pulses.push_back(kEdgeFinishTStates);
    if (total > kEdgeFinishTStates) pulses.push_back(static_cast<uint32_t>(total - kEdgeFinishTStates));
}

// Blocos #10/#11/#14 (convencao ZX: piloto + 2 pulsos de sync opcionais +
// dados com 1 periodo completo -- 2 pulsos -- por bit, MSb primeiro, sem
// bits de inicio/fim). usedBitsLastByte trunca o ultimo byte.
void EmitZxDataBlock(uint16_t pilot_len, uint32_t pilot_count, uint16_t sync1, uint16_t sync2, uint16_t zero_len,
                      uint16_t one_len, uint8_t used_bits_last_byte, const uint8_t *data, std::size_t size,
                      std::vector<uint32_t> &pulses) {
    for (uint32_t i = 0; i < pilot_count; ++i) pulses.push_back(pilot_len);
    if (sync1) pulses.push_back(sync1);
    if (sync2) pulses.push_back(sync2);
    const KcsByteFraming cfg{0, 0, 0, 0, 1, 2, 2, zero_len, one_len};
    for (std::size_t i = 0; i + 1 < size; ++i) kcs_emit_byte(&cfg, data[i], &SinkAppend, &pulses);
    if (size > 0) {
        const uint8_t last = data[size - 1];
        const int used = (used_bits_last_byte == 0 || used_bits_last_byte > 8) ? 8 : used_bits_last_byte;
        for (int b = 0; b < used; ++b) {
            const int bit = (last >> (7 - b)) & 1;
            pulses.push_back(bit ? one_len : zero_len);
            pulses.push_back(bit ? one_len : zero_len);
        }
    }
}

} // namespace

bool LoadTzxImage(const std::string &path, TapeImage &out, std::string &error) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) {
        error = "nao foi possivel abrir '" + path + "'";
        return false;
    }
    const std::streamsize size = f.tellg();
    f.seekg(0);
    std::vector<uint8_t> file(static_cast<std::size_t>(size));
    if (size > 0 && !f.read(reinterpret_cast<char *>(file.data()), size)) {
        error = "nao foi possivel ler '" + path + "'";
        return false;
    }

    static const char kSignature[8] = {'Z', 'X', 'T', 'a', 'p', 'e', '!', 0x1A};
    if (file.size() < 10 || std::memcmp(file.data(), kSignature, 8) != 0) {
        error = "'" + path + "' nao e' um TZX/TSX valido (assinatura 'ZXTape!' ausente)";
        return false;
    }

    out = TapeImage{};
    out.from_tsx = true;
    auto mark_skipped = [&](const std::string &what) {
        if (std::find(out.skipped_blocks.begin(), out.skipped_blocks.end(), what) == out.skipped_blocks.end())
            out.skipped_blocks.push_back(what);
    };

    Cursor cur(file.data(), file.size());
    cur.skip(10); // cabecalho (assinatura + versao)

    while (!cur.eof() && !cur.fail()) {
        const uint8_t id = cur.u8();
        if (cur.fail()) break;
        switch (id) {
        case 0x10: { // Standard Speed Data Block
            const uint16_t pause = cur.u16();
            const uint16_t len = cur.u16();
            const uint8_t *data = cur.ptr(len);
            if (cur.fail() || !data) break;
            const uint32_t pilot_count = (len > 0 && data[0] < 128) ? 8063 : 3223;
            EmitZxDataBlock(2168, pilot_count, 667, 735, 855, 1710, 8, data, len, out.pulses);
            EmitPauseMs(out.pulses, pause);
            break;
        }
        case 0x11: { // Turbo Speed Data Block
            const uint16_t pilot = cur.u16();
            const uint16_t sync1 = cur.u16();
            const uint16_t sync2 = cur.u16();
            const uint16_t zero = cur.u16();
            const uint16_t one = cur.u16();
            const uint16_t pilot_count = cur.u16();
            const uint8_t used_bits = cur.u8();
            const uint16_t pause = cur.u16();
            const uint32_t len = cur.u24();
            const uint8_t *data = cur.ptr(len);
            if (cur.fail() || !data) break;
            EmitZxDataBlock(pilot, pilot_count, sync1, sync2, zero, one, used_bits, data, len, out.pulses);
            EmitPauseMs(out.pulses, pause);
            break;
        }
        case 0x12: { // Pure Tone
            const uint16_t pulse_len = cur.u16();
            const uint16_t count = cur.u16();
            for (uint16_t i = 0; i < count; ++i) out.pulses.push_back(pulse_len);
            break;
        }
        case 0x13: { // Pulse sequence
            const uint8_t count = cur.u8();
            for (uint8_t i = 0; i < count; ++i) out.pulses.push_back(cur.u16());
            break;
        }
        case 0x14: { // Pure Data Block
            const uint16_t zero = cur.u16();
            const uint16_t one = cur.u16();
            const uint8_t used_bits = cur.u8();
            const uint16_t pause = cur.u16();
            const uint32_t len = cur.u24();
            const uint8_t *data = cur.ptr(len);
            if (cur.fail() || !data) break;
            EmitZxDataBlock(0, 0, 0, 0, zero, one, used_bits, data, len, out.pulses);
            EmitPauseMs(out.pulses, pause);
            break;
        }
        case 0x15: // Direct recording -- nao reproduzido (ver doc/tape-spec.md, "Limites")
            cur.skip(5); // T-states/amostra (2) + pausa (2) + bits usados (1)
            cur.skip(cur.u24());
            mark_skipped("15 (gravacao direta)");
            break;
        case 0x18: // CSW recording
            cur.skip(cur.u32());
            mark_skipped("18 (CSW)");
            break;
        case 0x19: // Generalized Data Block
            cur.skip(cur.u32());
            mark_skipped("19 (bloco generalizado)");
            break;
        case 0x20: { // Pause/Stop
            const uint16_t ms = cur.u16();
            if (ms != 0) EmitPauseMs(out.pulses, ms);
            break;
        }
        case 0x21: // Group start
            cur.skip(cur.u8());
            break;
        case 0x22: // Group end
            break;
        case 0x23: // Jump to block
            cur.skip(2);
            mark_skipped("23 (salto)");
            break;
        case 0x24: // Loop start
            cur.skip(2);
            mark_skipped("24/25 (laco)");
            break;
        case 0x25: // Loop end
            break;
        case 0x26: { // Call sequence
            const uint16_t n = cur.u16();
            cur.skip(static_cast<std::size_t>(n) * 2);
            mark_skipped("26/27 (chamada)");
            break;
        }
        case 0x27: // Return from sequence
            break;
        case 0x28: // Select block
            cur.skip(cur.u16());
            mark_skipped("28 (selecao)");
            break;
        case 0x2A: // Stop tape if 48K
            cur.skip(cur.u32());
            break;
        case 0x2B: // Set signal level
            cur.skip(cur.u32());
            mark_skipped("2B (nivel de sinal)");
            break;
        case 0x30: // Text description
            cur.skip(cur.u8());
            break;
        case 0x31: // Message block
            cur.u8();
            cur.skip(cur.u8());
            break;
        case 0x32: // Archive info
            cur.skip(cur.u16());
            break;
        case 0x33: // Hardware type
            cur.skip(static_cast<std::size_t>(cur.u8()) * 3);
            break;
        case 0x35: // Custom info block
            cur.skip(10);
            cur.skip(cur.u32());
            break;
        case 0x4B: { // Kansas City Standard (MSX) -- ver doc/tape-spec.md
            const uint32_t block_len = cur.u32();
            if (block_len < 12) { cur.skip(block_len); break; }
            const uint16_t pause = cur.u16();
            const uint16_t pilot = cur.u16();
            const uint16_t pilot_count = cur.u16();
            const uint16_t zero = cur.u16();
            const uint16_t one = cur.u16();
            const uint8_t bit_cfg = cur.u8();
            const uint8_t byte_cfg = cur.u8();
            const uint32_t n = block_len - 12;
            const uint8_t *data = cur.ptr(n);
            if (cur.fail() || !data) break;

            // Decodificacao dos bits de configuracao -- conferida contra
            // resource/openMSX_TSXadv/Contrib/tsx/TsxParser.cc (so' leitura,
            // GPL, nenhum codigo copiado).
            auto decode = [](uint8_t x) { return x ? x : 16; };
            KcsByteFraming cfg{};
            cfg.zero_pulses = decode(static_cast<uint8_t>(bit_cfg >> 4));
            cfg.one_pulses = decode(static_cast<uint8_t>(bit_cfg & 0x0F));
            cfg.start_bits = (byte_cfg & 0xC0) >> 6;
            cfg.start_value = (byte_cfg & 0x20) >> 5;
            cfg.stop_bits = (byte_cfg & 0x18) >> 3;
            cfg.stop_value = (byte_cfg & 0x04) >> 2;
            cfg.msb_first = byte_cfg & 0x01;
            cfg.zero_len = zero;
            cfg.one_len = one;

            for (uint16_t i = 0; i < pilot_count; ++i) out.pulses.push_back(pilot);
            for (uint32_t i = 0; i < n; ++i) kcs_emit_byte(&cfg, data[i], &SinkAppend, &out.pulses);
            EmitPauseMs(out.pulses, pause);

            // fast_bytes: reconstroi o equivalente a um .CAS (cabecalho de 8
            // bytes + os MESMOS dados, ver cas_format.h) -- o bloco #4B ja'
            // guarda `data[N]` como bytes puros, nao pulsos (bitCfg/byteCfg
            // so' dizem como GERAR os pulsos de reproducao).
            out.fast_bytes.insert(out.fast_bytes.end(), kCasHeader.begin(), kCasHeader.end());
            out.fast_bytes.insert(out.fast_bytes.end(), data, data + n);
            break;
        }
        case 0x5A: // "Glue" block (90 dec, 'Z') -- 9 bytes fixos
            cur.skip(9);
            break;
        default:
            error = "'" + path + "': bloco TZX desconhecido (ID " + std::to_string(static_cast<int>(id)) +
                    "h) -- fora da lista do TZX 1.20, sem como saber o tamanho dele";
            return false;
        }
    }
    if (cur.fail()) {
        error = "'" + path + "': arquivo truncado ou corrompido (leitura alem do fim)";
        return false;
    }

    out.files = ScanCasFiles(out.fast_bytes);
    return true;
}

} // namespace tape
