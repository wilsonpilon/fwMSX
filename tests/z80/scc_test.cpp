// Teste do chip de som SCC (cartuchos Konami5/Gen8, 9800h-98FFh) -- ver
// doc/scc-spec.md.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>

#include "../../src/memmap/cpp/memory_system.h"
#include "../../src/memmap/cpp/slot_memory_bus.h"
#include "../../src/memmap/core/slot_state.h"
#include "../../src/scc/core/scc_state.h"
#include "../../src/scc/cpp/scc_device.h"

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    if (cond) {
        std::printf("[PASS] %s\n", what.c_str());
    } else {
        std::printf("[FAIL] %s\n", what.c_str());
        ++g_failures;
    }
}

// Cartucho de 64KB (8 bancos de 8KB), cada banco preenchido com o proprio
// indice, em slot 1:0 visivel na pagina 2 (8000h-BFFFh).
struct Cart {
    memmap::MemorySystem mem;
    scc::SccDevice dev;
    memmap::SlotMemoryBus bus;

    explicit Cart(MemMapMapperType mapper, int size = 0x10000) : bus(mem) {
        std::vector<uint8_t> rom(static_cast<size_t>(size));
        for (size_t i = 0; i < rom.size(); ++i) rom[i] = static_cast<uint8_t>(i / 0x2000);
        std::string error;
        mem.LoadRom(1, 0, rom.data(), rom.size(), &error, mapper);
        memmap_switch_primary(&mem.state(), 0x10);
        bus.AttachCart(1, 0, &dev);
    }
};

// Escreve um quadrado de 32 amostras (16 altas, 16 baixas) no canal `ch`.
void WriteSquare(Cart &c, int ch) {
    for (int i = 0; i < 32; ++i) c.bus.write(static_cast<uint16_t>(0x9800 + ch * 0x20 + i), i < 16 ? 0x7F : 0x80);
}

// Ativa o chip (protocolo Konami5: 3Fh em 9000h) e configura um canal.
void SetupTone(Cart &c, int ch, int period, int volume) {
    c.bus.write(0x9000, 0x3F);
    WriteSquare(c, ch);
    c.bus.write(static_cast<uint16_t>(0x9880 + 2 * ch), static_cast<uint8_t>(period & 0xFF));
    c.bus.write(static_cast<uint16_t>(0x9881 + 2 * ch), static_cast<uint8_t>((period >> 8) & 0x0F));
    c.bus.write(static_cast<uint16_t>(0x988A + ch), static_cast<uint8_t>(volume));
    c.bus.write(0x988F, static_cast<uint8_t>(1 << ch));
}

// Gera `seconds` de audio ao vivo do dispositivo, em lotes como a maquina faz.
std::vector<int16_t> Render(scc::SccDevice &dev, double seconds) {
    dev.EnableLive(true);
    const int cycles = static_cast<int>(SCC_Z80_CLOCK * seconds);
    for (int done = 0; done < cycles; done += 100) dev.Advance(std::min(100, cycles - done));
    std::vector<int16_t> out;
    dev.TakeLive(out);
    dev.EnableLive(false);
    return out;
}

int SignChanges(const std::vector<int16_t> &s) {
    int changes = 0;
    for (size_t i = 1; i < s.size(); ++i)
        if ((s[i - 1] < 0) != (s[i] < 0)) ++changes;
    return changes;
}

int32_t Peak(const std::vector<int16_t> &s) {
    int32_t peak = 0;
    for (int16_t v : s) peak = std::max<int32_t>(peak, std::abs(static_cast<int32_t>(v)));
    return peak;
}

void TestProtocolKonami5() {
    Cart c(MEMMAP_MAPPER_KONAMI5);
    c.bus.write(0x9800, 0x55);
    check(c.bus.read(0x9800) != 0x55, "K5 desligado: escrita em 9800h nao chega ao chip (cai na ROM)");

    c.bus.write(0x9000, 0x3F);
    check(c.dev.enabled(), "K5: 3Fh em 9000h liga o SCC");
    check(c.bus.read(0x8000) == 7, "K5: a mesma escrita em 9000h ainda troca o banco da pagina 2");

    c.bus.write(0x9800, 0x7F);
    check(c.bus.read(0x9800) == 0x7F, "K5 ligado: leitura de 9800h devolve a onda escrita");
    check(c.bus.read(0x9880) == 0xFF, "K5 ligado: registrador 9880h (so escrita) le 0FFh");

    c.bus.write(0x8800, 0x00);
    check(c.dev.enabled(), "K5: escrita em 8800h nao mexe no enable (so 9000h)");

    c.bus.write(0x9000, 0x00);
    check(!c.dev.enabled(), "K5: outro valor em 9000h desliga o SCC");
}

void TestProtocolGen8() {
    Cart c(MEMMAP_MAPPER_GEN8);
    c.bus.write(0x8800, 0x3F);
    check(c.dev.enabled(), "Gen8: 3Fh em 8800h (faixa 8000h-9FFFh) liga o SCC");
    c.bus.write(0x9000, 0x00);
    check(!c.dev.enabled(), "Gen8: 0 em 9000h desliga o SCC");
}

void TestPlainRomIgnoresScc() {
    Cart c(MEMMAP_MAPPER_NONE, 0x4000);
    c.bus.write(0x9000, 0x3F);
    check(!c.dev.enabled(), "ROM plana (sem mapper) nao responde ao protocolo do SCC");
}

void TestToneFrequencyAndLevel() {
    Cart c(MEMMAP_MAPPER_KONAMI5);
    SetupTone(c, 0, 100, 15);
    const std::vector<int16_t> out = Render(c.dev, 1.0);

    check(out.size() == 44100 || out.size() == 44099 || out.size() == 44101,
          "Advance: 1 s de ciclos de Z80 rende ~44100 amostras");

    // Quadrado de 32 amostras, periodo 100: 2 trocas de sinal por ciclo de onda,
    // que dura 32*100 ciclos de Z80 -> clock/(16*100) trocas por segundo.
    const double expected = SCC_Z80_CLOCK / (16.0 * 100.0);
    const int changes = SignChanges(out);
    check(std::abs(changes - expected) < expected * 0.01,
          "frequencia: trocas de sinal batem com clock/(16*periodo) (+-1%)");

    const int32_t peak = Peak(out);
    check(peak >= 6400 && peak <= 6600, "nivel 15: amplitude de pico ~6502");

    // Volume linear: nivel 7 vale ~7/15 do nivel 15.
    Cart c7(MEMMAP_MAPPER_KONAMI5);
    SetupTone(c7, 0, 100, 7);
    const int32_t peak7 = Peak(Render(c7.dev, 0.25));
    const double ratio = static_cast<double>(peak7) / peak;
    check(std::abs(ratio - 7.0 / 15.0) < 0.02, "volume linear: nivel 7 e' ~7/15 do nivel 15");
}

void TestMixerAndVolumeSilence() {
    Cart c(MEMMAP_MAPPER_KONAMI5);
    SetupTone(c, 0, 100, 15);
    c.bus.write(0x988F, 0x00);
    check(Peak(Render(c.dev, 0.1)) == 0, "mixer com bit do canal zerado: canal silenciado");

    SetupTone(c, 0, 100, 0);
    check(Peak(Render(c.dev, 0.1)) == 0, "volume 0: canal silenciado");
}

void TestRegisterMirror() {
    Cart c(MEMMAP_MAPPER_KONAMI5);
    c.bus.write(0x9000, 0x3F);
    c.bus.write(0x9880, 0x34);
    check(c.dev.state().r[0xB0] == 0x34 && c.dev.state().r[0xA0] == 0x34,
          "frequencia escrita em A0h tambem fica espelhada em B0h (regra do fMSX)");
}

// Teste diferencial do kernel em Assembly contra a referencia em C.
void TestAsmKernelMatchesReference() {
    std::mt19937 rng(0x5CC);
    bool all_equal = true;
    for (int iter = 0; iter < 2000 && all_equal; ++iter) {
        int8_t wave[32];
        for (auto &w : wave) w = static_cast<int8_t>(rng() & 0xFF);
        const int n = static_cast<int>(rng() % 300) + 1;
        const uint32_t step = static_cast<uint32_t>(rng());
        const int32_t level = static_cast<int32_t>(rng() % 6554);
        const uint32_t start_phase = static_cast<uint32_t>(rng());

        std::vector<int32_t> acc_asm(static_cast<size_t>(n)), acc_ref(static_cast<size_t>(n));
        for (int i = 0; i < n; ++i) acc_asm[i] = acc_ref[i] = static_cast<int32_t>(rng() % 20000) - 10000;

        uint32_t phase_asm = start_phase, phase_ref = start_phase;
        scc_render_channel(acc_asm.data(), n, wave, &phase_asm, step, level);
        scc_render_channel_ref(acc_ref.data(), n, wave, &phase_ref, step, level);

        all_equal = acc_asm == acc_ref && phase_asm == phase_ref;
    }
    check(all_equal, "Assembly (render_channel.asm) bate com a referencia em C em 2000 casos aleatorios");
}

} // namespace

int main() {
    TestProtocolKonami5();
    TestProtocolGen8();
    TestPlainRomIgnoresScc();
    TestToneFrequencyAndLevel();
    TestMixerAndVolumeSilence();
    TestRegisterMirror();
    TestAsmKernelMatchesReference();

    if (g_failures == 0) std::printf("\nTodos os testes do SCC passaram.\n");
    else std::printf("\n%d falha(s) no SCC.\n", g_failures);
    return g_failures == 0 ? 0 : 1;
}
