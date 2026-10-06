// Teste do chip FM (YM2413 / MSX-MUSIC e FM-PAC) -- ver doc/fm-spec.md.
// Confere as tabelas (Fortran), o kernel de soma (Assembly contra a referencia
// em C), os timbres prontos, a frequencia, o volume, o envelope (ataque,
// sustentacao e liberacao) e as portas 7Ch/7Dh pelo adaptador FmDevice.
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

#include "../../src/fm/core/ym2413_state.h"
#include "../../src/fm/cpp/fm_device.h"

// Simbolos do Fortran e do Assembly, testados diretamente (ver ym2413_state.c).
extern "C" {
void ym2413_build_tables(double *sine, double *db_gain);
void ym2413_accumulate(int32_t *acc, const int32_t *src, int n);
}

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

// Ajusta o canal `ch` com o timbre do usuario (registradores 00h-07h): modulador
// quase mudo (TL alto) e portadora de onda senoidal. So' a portadora importa.
void SetupSine(fm::FmDevice &d, int ch, int block, int fnum, int car_ar, int car_dr, int car_sl, int car_rr,
               int egt, int vol, int mod_tl = 63) {
    d.out(0x7C, 0x00);
    d.out(0x7D, 0x21);                                           // modulador: MULT 1, EGT (segura)
    d.out(0x7C, 0x01);
    d.out(0x7D, static_cast<uint8_t>(0x01 | (egt << 5)));        // portadora: MULT 1, EGT
    d.out(0x7C, 0x02);
    d.out(0x7D, static_cast<uint8_t>(mod_tl));                    // modulador: TL
    d.out(0x7C, 0x03);
    d.out(0x7D, 0x00);                                           // ondas senoidais, FB 0
    d.out(0x7C, 0x04);
    d.out(0x7D, 0xF0);                                           // modulador AR 15, DR 0
    d.out(0x7C, 0x05);
    d.out(0x7D, static_cast<uint8_t>((car_ar << 4) | car_dr));
    d.out(0x7C, 0x06);
    d.out(0x7D, 0x0F);
    d.out(0x7C, 0x07);
    d.out(0x7D, static_cast<uint8_t>((car_sl << 4) | car_rr));
    d.out(0x7C, static_cast<uint8_t>(0x30 + ch));
    d.out(0x7D, static_cast<uint8_t>(vol & 15));                 // timbre 0, volume
    d.out(0x7C, static_cast<uint8_t>(0x10 + ch));
    d.out(0x7D, static_cast<uint8_t>(fnum & 0xFF));
    d.out(0x7C, static_cast<uint8_t>(0x20 + ch));
    d.out(0x7D, static_cast<uint8_t>(0x10 | (block << 1) | ((fnum >> 8) & 1)));  // KEY-ON
}

void KeyOff(fm::FmDevice &d, int ch, int block, int fnum) {
    d.out(0x7C, static_cast<uint8_t>(0x20 + ch));
    d.out(0x7D, static_cast<uint8_t>(0x00 | (block << 1) | ((fnum >> 8) & 1)));
}

// Gera `seconds` de audio a 44100 Hz (um unico Advance: o chip avanca no tempo do Z80).
std::vector<int16_t> Render(fm::FmDevice &d, double seconds) {
    d.EnableLive(true);
    const int cycles = static_cast<int>(seconds * YM2413_Z80_CLOCK);
    d.Advance(cycles);
    std::vector<int16_t> out;
    d.TakeLive(out);
    d.EnableLive(false);
    return out;
}

int Peak(const std::vector<int16_t> &v, size_t from, size_t to) {
    int peak = 0;
    for (size_t i = from; i < to && i < v.size(); ++i) peak = std::max(peak, std::abs(static_cast<int>(v[i])));
    return peak;
}

// Frequencia pelo numero de cruzamentos de zero no trecho [from, to).
double MeasureHz(const std::vector<int16_t> &v, size_t from, size_t to, int rate) {
    int crossings = 0;
    for (size_t i = from + 1; i < to && i < v.size(); ++i)
        if ((v[i - 1] < 0 && v[i] >= 0) || (v[i - 1] >= 0 && v[i] < 0)) ++crossings;
    const double seconds = static_cast<double>(std::min(to, v.size()) - from) / rate;
    return crossings / (2.0 * seconds);
}

void TestTables() {
    double sine[1024], db[1024];
    ym2413_build_tables(sine, db);
    check(std::fabs(sine[0]) < 1e-12, "tabela de seno: fase 0 vale 0");
    check(std::fabs(sine[256] - 1.0) < 1e-9, "tabela de seno: um quarto de ciclo vale 1");
    check(std::fabs(db[0] - 1.0) < 1e-12, "tabela de atenuacao: 0 dB vale 1");
    check(std::fabs(db[4] - std::pow(10.0, -1.0 / 20.0)) < 1e-9, "tabela de atenuacao: 1 dB vale 10^(-1/20)");
}

void TestAccumulateMatchesC() {
    std::mt19937 rng(1234);
    std::uniform_int_distribution<int> dist(-30000, 30000);
    for (int n : {0, 1, 7, 256}) {
        std::vector<int32_t> acc(static_cast<size_t>(n)), src(static_cast<size_t>(n)), ref;
        for (int i = 0; i < n; ++i) {
            acc[static_cast<size_t>(i)] = dist(rng);
            src[static_cast<size_t>(i)] = dist(rng);
        }
        ref = acc;
        for (int i = 0; i < n; ++i) ref[static_cast<size_t>(i)] += src[static_cast<size_t>(i)];
        ym2413_accumulate(acc.data(), src.data(), n);
        check(acc == ref, "kernel de soma (Assembly) igual a referencia em C, n=" + std::to_string(n));
    }
}

void TestBuiltinPatches() {
    const uint8_t *violin = ym2413_builtin_patch(1);
    check(violin != nullptr && violin[0] == 0x61 && violin[3] == 0x17 && violin[5] == 0x7F,
          "timbre pronto 1 (Violino) tem os bytes da tabela");
    const uint8_t *elec = ym2413_builtin_patch(15);
    check(elec != nullptr && elec[7] == 0x23, "timbre pronto 15 (Guitarra eletrica) tem os bytes da tabela");
    check(ym2413_builtin_patch(0) == nullptr && ym2413_builtin_patch(16) == nullptr,
          "timbres fora de 1-15 nao existem (0 e' o do usuario)");
}

void TestPitch() {
    fm::FmDevice d;
    // block 4, fnum 256: 256 * 49716 / 2^15 = 388.4 Hz.
    SetupSine(d, 0, 4, 256, 15, 0, 0, 0x0F, 1, 0);
    const std::vector<int16_t> out = Render(d, 1.0);
    const double hz = MeasureHz(out, 8820, 44100, fm::kSampleRate);
    const double expected = 256.0 * YM2413_NATIVE_RATE / 32768.0;
    check(std::fabs(hz - expected) / expected < 0.02,
          "frequencia: fnum 256 no bloco 4 toca ~388 Hz (medido " + std::to_string(hz) + ")");
}

void TestVolume() {
    fm::FmDevice loud;
    SetupSine(loud, 0, 4, 256, 15, 0, 0, 0x0F, 1, 0);
    const int peak_loud = Peak(Render(loud, 0.5), 8820, 22050);

    fm::FmDevice quiet;
    SetupSine(quiet, 0, 4, 256, 15, 0, 0, 0x0F, 1, 15);
    const int peak_quiet = Peak(Render(quiet, 0.5), 8820, 22050);

    check(peak_loud > 2800, "volume 0 (maximo): pico proximo de FULL_SCALE (" + std::to_string(peak_loud) + ")");
    // volume 15 = 45 dB de atenuacao: ~0,56% da amplitude maxima.
    check(peak_quiet > 10 && peak_quiet < 25,
          "volume 15: 45 dB abaixo (pico " + std::to_string(peak_quiet) + ")");
}

void TestNoAttackIsSilent() {
    fm::FmDevice d;
    SetupSine(d, 0, 4, 256, 0, 0, 0, 0x0F, 1, 0);
    const int peak = Peak(Render(d, 0.5), 0, 22050);
    check(peak == 0, "taxa de ataque 0: a nota nunca sobe (pico " + std::to_string(peak) + ")");
}

void TestRelease() {
    fm::FmDevice d;
    SetupSine(d, 0, 4, 256, 15, 0, 0, 15, 1, 0);
    Render(d, 0.2);
    KeyOff(d, 0, 4, 256);
    // 0,5 s a 0,9 s depois do key-off: a liberacao (RR 15) ja' terminou.
    const int peak_after = Peak(Render(d, 0.9), 22050, 39690);
    check(peak_after == 0, "key-off com liberacao rapida (RR 15): silencio em 0,5 s (pico " +
                               std::to_string(peak_after) + ")");
}

void TestSustainFlag() {
    // Decaimento rapido ate SL=15 (-45 dB). Com EGT=1 a nota fica no nivel de
    // sustentacao; com EGT=0 ela continua caindo ate' zero.
    fm::FmDevice sustained;
    SetupSine(sustained, 0, 4, 256, 15, 15, 15, 15, 1, 0);
    const int peak_sus = Peak(Render(sustained, 0.9), 22050, 39690);

    fm::FmDevice percussive;
    SetupSine(percussive, 0, 4, 256, 15, 15, 15, 15, 0, 0);
    const int peak_perc = Peak(Render(percussive, 0.9), 22050, 39690);

    check(peak_sus > 10, "EGT=1: a nota segura no nivel de sustentacao (pico " + std::to_string(peak_sus) + ")");
    check(peak_perc == 0, "EGT=0: a nota morre apos o decaimento (pico " + std::to_string(peak_perc) + ")");
}

void TestModulatorChangesTone() {
    fm::FmDevice plain, modulated;
    SetupSine(plain, 0, 4, 256, 15, 0, 0, 0x0F, 1, 0, 63);
    SetupSine(modulated, 0, 4, 256, 15, 0, 0, 0x0F, 1, 0, 0);
    const std::vector<int16_t> a = Render(plain, 0.3);
    const std::vector<int16_t> b = Render(modulated, 0.3);
    int64_t diff = 0;
    for (size_t i = 13000; i < a.size() && i < b.size(); ++i) diff += std::abs(a[i] - b[i]);
    check(diff > 100000, "modulador com TL 0 muda a forma de onda da portadora");
}

void TestRhythm() {
    fm::FmDevice d;
    // Frequencia do canal 7 (o bumbo usa o tom dele): fnum 128, bloco 3.
    d.out(0x7C, 0x16);
    d.out(0x7D, 0x80);
    d.out(0x7C, 0x26);
    d.out(0x7D, 0x06);
    // BD (bit 4) com o modo ritmo ligado (bit 5): canal 7 inteiro, sem KEY-ON melodico.
    d.out(0x7C, 0x0E);
    d.out(0x7D, 0x30);
    check(d.state().ch[6].opkey[0] == 1 && d.state().ch[6].opkey[1] == 1,
          "ritmo: BD (bit 4 de 0Eh) liga os dois operadores do canal 7");
    const std::vector<int16_t> out = Render(d, 0.2);
    check(Peak(out, 0, out.size()) > 100, "ritmo: a batida do bumbo sai no audio (pico " +
                                              std::to_string(Peak(out, 0, out.size())) + ")");

    // HH (bit 0) passa a tocar sozinho, e BD solta.
    d.out(0x7D, 0x21);
    check(d.state().ch[6].opkey[0] == 0 && d.state().ch[7].opkey[0] == 1,
          "ritmo: trocar para HH solta o bumbo e liga o modulador do canal 8");

    // KEY-ON melodico do canal 7 nao vale no modo ritmo.
    d.out(0x7C, 0x26);
    d.out(0x7D, 0x10);
    check(d.state().ch[6].key == 0, "ritmo: 20h-28h nao liga os canais 7-9 como melodicos");

    // Sair do modo ritmo solta a bateria.
    d.out(0x7C, 0x0E);
    d.out(0x7D, 0x00);
    check(d.state().ch[6].opkey[0] == 0 && d.state().ch[7].opkey[0] == 0 && d.state().ch[8].opkey[0] == 0,
          "ritmo: desligar o modo (0Eh bit 5 = 0) solta todos os canais de bateria");
}

void TestPortsAndReset() {
    fm::FmDevice d;
    d.out(0x7C, 0x10);
    d.out(0x7D, 0x55);
    check(d.state().reg[0x10] == 0x55 && (d.state().ch[0].fnum & 0xFF) == 0x55,
          "porta 7Ch seleciona o registrador e 7Dh grava o dado");
    check(d.in(0x7C) == 0x00, "leitura de 7Ch devolve status 0 (timers nao emulados)");

    SetupSine(d, 0, 4, 256, 15, 0, 0, 0x0F, 1, 0);
    d.Reset();
    check(d.state().ch[0].key == 0 && d.state().reg[0x20] == 0, "reset: sem nota tocando e registradores em 0");
}

} // namespace

int main() {
    TestTables();
    TestAccumulateMatchesC();
    TestBuiltinPatches();
    TestPitch();
    TestVolume();
    TestNoAttackIsSilent();
    TestRelease();
    TestSustainFlag();
    TestModulatorChangesTone();
    TestRhythm();
    TestPortsAndReset();

    std::printf("\n%s (%d falha(s))\n", g_failures == 0 ? "fmtest OK" : "fmtest FALHOU", g_failures);
    return g_failures == 0 ? 0 : 1;
}
