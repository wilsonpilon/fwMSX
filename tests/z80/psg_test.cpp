// Teste do PSG AY-3-8910 (portas A0h-A2h) -- ver doc/psg-spec.md.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "../../src/psg/cpp/psg_device.h"
#include "../../src/psg/cpp/wav_writer.h"
#include "../../src/psg/core/psg_state.h"
#include "../../src/z80/debug/z80_debug_session.h"
#include "../../src/z80/debug/z80_debug_shell_startup.h"

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

bool Contains(const std::string &haystack, const std::string &needle) {
    return haystack.find(needle) != std::string::npos;
}

// Gera `seconds` segundos de audio a 44100 Hz a partir do estado atual.
std::vector<int16_t> Render(PsgState &p, double seconds) {
    const int cycles = static_cast<int>(PSG_Z80_CLOCK * seconds);
    std::vector<int16_t> out(static_cast<size_t>(seconds * 44100) + 8);
    int total = 0;
    // Em lotes de ate' 10000 ciclos (a sessao avanca por instrucao, lotes menores).
    for (int done = 0; done < cycles; done += 10000) {
        const int n = std::min(10000, cycles - done);
        total += psg_advance(&p, n, 44100, out.data() + total, static_cast<int>(out.size()) - total);
    }
    out.resize(static_cast<size_t>(total));
    return out;
}

// Quantas vezes o sinal cruza o ponto medio de baixo para cima.
int RisingEdges(const std::vector<int16_t> &s) {
    int32_t lo = 32767, hi = -32768;
    for (int16_t v : s) {
        lo = std::min<int32_t>(lo, v);
        hi = std::max<int32_t>(hi, v);
    }
    const int32_t mid = (lo + hi) / 2;
    int edges = 0;
    for (size_t i = 1; i < s.size(); ++i)
        if (s[i - 1] <= mid && s[i] > mid) ++edges;
    return edges;
}

// Passos de envelope com periodo 1: 1 passo = 2 ticks de gerador = 32 ciclos de Z80.
void Ticks(PsgState &p, int steps) { psg_advance(&p, steps * 2 * PSG_CYCLES_PER_TICK, 44100, nullptr, 0); }

// Configura o envelope para andar 1 passo por tick e liga o envelope no canal A.
void SetupEnvelope(PsgState &p, int shape) {
    psg_write_reg(&p, 11, 1);
    psg_write_reg(&p, 12, 0);
    psg_write_reg(&p, 8, 0x10);
    psg_write_reg(&p, 13, static_cast<uint8_t>(shape));
}

} // namespace

int main() {
    // --- 1. Reset e mascaras de registrador ----------------------------------
    {
        PsgState p;
        psg_reset(&p);
        check(p.r[7] == 0xFD && p.r[14] == 0xFF && p.r[0] == 0 && p.r[8] == 0, "reset: R7=FDh, R14=FFh, resto zerado (RegInit do fMSX)");
        psg_write_reg(&p, 1, 0xFF);
        psg_write_reg(&p, 6, 0xFF);
        psg_write_reg(&p, 8, 0xFF);
        psg_write_reg(&p, 13, 0xFF);
        check(p.r[1] == 0x0F && p.r[6] == 0x1F && p.r[8] == 0x1F && p.r[13] == 0x0F, "mascaras: R1/R3/R5 4 bits, R6 5 bits, R8-R10 5 bits, R13 4 bits");
        psg_write_reg(&p, 7, 0xAB);
        psg_write_reg(&p, 16, 0x55); // fora de faixa: ignorado
        check(p.r[7] == 0xAB, "R7 guarda os 8 bits; registrador invalido (16) e' ignorado");
    }

    // --- 2. Tabela de volume (Fortran) ---------------------------------------
    {
        PsgState p;
        psg_reset(&p);
        psg_write_reg(&p, 7, 0xF8);   // so' tom (ruido desligado nos 3 canais)
        psg_write_reg(&p, 0, 1);      // periodo 1: tom ligado e rapido
        std::vector<int> peaks;
        for (int vol = 0; vol <= 15; ++vol) {
            psg_write_reg(&p, 8, static_cast<uint8_t>(vol));
            const std::vector<int16_t> s = Render(p, 0.01);
            peaks.push_back(*std::max_element(s.begin(), s.end()));
        }
        bool mono = true;
        for (int i = 1; i <= 15; ++i) mono = mono && peaks[i] >= peaks[i - 1];
        check(peaks[0] <= 1 && mono, "volume 0 silencia; volume sobe monotonicamente ate o 15");
        // Periodo 1 a 44100 Hz fica no limite do filtro de caixa, entao o pico
        // nao chega ao maximo -- a relacao exata e' conferida via periodo longo.
        psg_write_reg(&p, 0, 0xFF);
        psg_write_reg(&p, 1, 0x0F);
        psg_write_reg(&p, 8, 15);
        std::vector<int16_t> s15 = Render(p, 0.5);
        psg_write_reg(&p, 8, 13);
        std::vector<int16_t> s13 = Render(p, 0.5);
        const int p15 = *std::max_element(s15.begin(), s15.end());
        const int p13 = *std::max_element(s13.begin(), s13.end());
        check(p15 == 10922, "volume 15 = 10922 (32767/3: tres canais nunca estouram 16 bits)");
        check(std::abs(p13 * 2 - p15) <= 4, "cada 2 passos de volume = metade da amplitude (3 dB por passo)");
    }

    // --- 3. Frequencia do tom -------------------------------------------------
    {
        PsgState p;
        psg_reset(&p);
        psg_write_reg(&p, 7, 0xF8);
        psg_write_reg(&p, 0, 254); // 1789772 / (16 * 254) = 440.4 Hz
        psg_write_reg(&p, 8, 15);
        const std::vector<int16_t> s = Render(p, 1.0);
        const int edges = RisingEdges(s);
        check(std::abs(edges - 440) <= 2, "periodo 254 -> ~440 Hz (" + std::to_string(edges) + " ciclos em 1 s)");
        check(std::fabs(psg_tone_hz(&p, 0) - 440.4) < 0.1, "psg_tone_hz(): 440.4 Hz");
        check(s.size() >= 44099 && s.size() <= 44101, "1 s de ciclos de Z80 produz 44100 amostras (" + std::to_string(s.size()) + ")");

        // Periodo 0 conta como 1 (como o hardware); canal A com tom desligado no mixer calla.
        psg_write_reg(&p, 7, 0xF9);
        check(psg_tone_hz(&p, 0) == 0.0, "tom desligado no mixer: psg_tone_hz() = 0");
        psg_write_reg(&p, 8, 0);
        psg_write_reg(&p, 7, 0xF8);
        const std::vector<int16_t> quiet = Render(p, 0.05);
        check(*std::max_element(quiet.begin(), quiet.end()) == 0 && *std::min_element(quiet.begin(), quiet.end()) == 0,
              "volume 0 -> silencio absoluto");
    }

    // --- 4. Ruido -------------------------------------------------------------
    {
        PsgState p;
        psg_reset(&p);
        psg_write_reg(&p, 7, 0xF7); // canal A: so' ruido
        psg_write_reg(&p, 6, 4);
        psg_write_reg(&p, 8, 15);
        const std::vector<int16_t> s = Render(p, 0.5);
        const int16_t lo = *std::min_element(s.begin(), s.end());
        const int16_t hi = *std::max_element(s.begin(), s.end());
        int transitions = 0;
        const int mid = (lo + hi) / 2;
        for (size_t i = 1; i < s.size(); ++i)
            if ((s[i - 1] > mid) != (s[i] > mid)) ++transitions;
        check(lo < 2000 && hi > 8000 && transitions > 1000, "ruido: oscila entre silencio e nivel alto de forma irregular (" + std::to_string(transitions) + " transicoes)");
        // O LFSR e' deterministico: dois PSGs iguais geram exatamente o mesmo ruido.
        PsgState q;
        psg_reset(&q);
        psg_write_reg(&q, 7, 0xF7);
        psg_write_reg(&q, 6, 4);
        psg_write_reg(&q, 8, 15);
        check(Render(q, 0.5) == s, "ruido: LFSR deterministico (mesma semente, mesma sequencia)");
    }

    // --- 5. Envelope: as 16 formas de R13 ------------------------------------
    {
        struct Case {
            int shape;
            int after5;  // nivel 5 ticks depois do disparo
            int after20; // nivel 20 ticks depois (ciclo de 16 ja' terminou)
            int after21; // 21 ticks depois (5 passos no 2o ciclo)
            const char *what;
        };
        const Case cases[] = {
            {0x00, 10, 0, 0, "forma 0 (decay, depois 0)"},
            {0x03, 10, 0, 0, "forma 3 (decay, depois 0)"},
            {0x04, 5, 0, 0, "forma 4 (attack, depois 0)"},
            {0x07, 5, 0, 0, "forma 7 (attack, depois 0)"},
            {0x08, 10, 11, 10, "forma 8 (decay repetido -- serra)"},
            {0x09, 10, 0, 0, "forma 9 (decay, depois 0)"},
            {0x0A, 10, 4, 5, "forma 10 (triangulo: decay, attack, ...)"},
            {0x0B, 10, 15, 15, "forma 11 (decay, depois 15)"},
            {0x0C, 5, 4, 5, "forma 12 (attack repetido -- serra)"},
            {0x0D, 5, 15, 15, "forma 13 (attack, depois 15)"},
            {0x0E, 5, 11, 10, "forma 14 (triangulo: attack, decay, ...)"},
            {0x0F, 5, 0, 0, "forma 15 (attack, depois 0)"},
        };
        for (const Case &c : cases) {
            PsgState p;
            psg_reset(&p);
            SetupEnvelope(p, c.shape);
            const int l0 = psg_channel_level(&p, 0);
            Ticks(p, 5);
            const int l5 = psg_channel_level(&p, 0);
            Ticks(p, 15); // 20 ticks desde o disparo
            const int l20 = psg_channel_level(&p, 0);
            Ticks(p, 1);
            const int l21 = psg_channel_level(&p, 0);
            const bool start_ok = (c.shape & 4) ? l0 == 0 : l0 == 15;
            check(start_ok && l5 == c.after5 && l20 == c.after20 && l21 == c.after21,
                  std::string("envelope ") + c.what + ": niveis " + std::to_string(l0) + "," + std::to_string(l5) + "," +
                      std::to_string(l20) + "," + std::to_string(l21));
        }
    }

    // --- 6. Portas A0h-A2h (PsgDevice) ---------------------------------------
    {
        psg::PsgDevice dev;
        dev.out(0xA0, 0x07);
        dev.out(0xA1, 0xB8);
        check(dev.state().r[7] == 0xB8, "OUT (A0h) seleciona o registrador, OUT (A1h) escreve nele");
        dev.out(0xA0, 0x07);
        check(dev.in(0xA2) == 0xB8, "IN (A2h) le o registrador selecionado");
        dev.out(0xA0, 0xF2); // so' os 4 bits baixos contam no latch
        dev.out(0xA1, 0x12);
        check(dev.state().latch == 2 && dev.state().r[2] == 0x12, "latch usa so' 4 bits (F2h -> R2)");
        dev.out(0xA0, 14);
        check(dev.in(0xA2) == 0x7F, "R14 (joystick) sem nada plugado le 7Fh");
        dev.out(0xA0, 15);
        dev.out(0xA1, 0xFF);
        check(dev.in(0xA2) == 0xF0, "R15 so' devolve os 4 bits altos");
        dev.Reset();
        check(dev.state().r[7] == 0xFD, "Reset() de maquina volta os registradores ao estado inicial");
    }

    // --- 6b. Joystick em R14/R15 ---------------------------------------------
    {
        psg::PsgDevice dev;
        auto read_r14 = [&] {
            dev.out(0xA0, 14);
            return dev.in(0xA2);
        };
        check(read_r14() == 0x7F, "joystick: nada pressionado le 7Fh (porta A)");
        psg_set_joystick(&dev.state(), 0, PSG_JOY_UP | PSG_JOY_FIRE_A);
        check(read_r14() == 0x7F - 0x01 - 0x10, "joystick A: cima + fogo A = bits 0 e 4 em 0 (logica invertida): " + std::to_string(read_r14()));
        psg_set_joystick(&dev.state(), 0, PSG_JOY_LEFT | PSG_JOY_RIGHT | PSG_JOY_DOWN | PSG_JOY_FIRE_B);
        check(read_r14() == (0x7F & ~(0x04 | 0x08 | 0x02 | 0x20)), "joystick A: esquerda+direita+baixo+fogo B");
        // O bit 6 de R15 escolhe a porta: B nao foi tocada.
        dev.out(0xA0, 15);
        dev.out(0xA1, 0x40);
        check(read_r14() == 0x7F, "R15 bit 6 = 1 seleciona a porta B (nada pressionado la')");
        psg_set_joystick(&dev.state(), 1, PSG_JOY_UP);
        check(read_r14() == 0x7E, "joystick B: cima = 7Eh");
        // Bit 4/5 de R15 desliga as linhas do joystick A/B (le tudo solto).
        dev.out(0xA0, 15);
        dev.out(0xA1, 0x60);
        check(read_r14() == 0x7F, "R15 bit 5 = 1 desliga as linhas da porta B (le 7Fh mesmo pressionado)");
        psg_set_joystick(&dev.state(), 1, 0xFF);
        check((dev.state().joy[1] & 0xC0) == 0, "psg_set_joystick guarda so' os 6 bits validos");
        psg_set_joystick(&dev.state(), 2, PSG_JOY_UP); // porta invalida: ignorada
        // Reset de maquina nao solta o joystick (e' o mundo externo, como as teclas).
        psg_set_joystick(&dev.state(), 0, PSG_JOY_FIRE_A);
        dev.Reset();
        dev.out(0xA0, 14);
        check(dev.in(0xA2) == 0x7F - 0x10 && dev.state().r[7] == 0xFD, "Reset() zera os registradores mas mantem o joystick");
    }

    // --- 7. Gravacao de amostras + WAV ----------------------------------------
    {
        psg::PsgDevice dev;
        dev.out(0xA0, 7);
        dev.out(0xA1, 0xF8);
        dev.out(0xA0, 0);
        dev.out(0xA1, 254);
        dev.out(0xA0, 8);
        dev.out(0xA1, 15);
        dev.Advance(100000);
        check(dev.capture().empty(), "sem gravacao ligada, nenhuma amostra e' acumulada");
        dev.StartRecording();
        for (int i = 0; i < 100; ++i) dev.Advance(PSG_Z80_CLOCK / 100); // 1 s em 100 lotes
        check(dev.capture().size() >= 44099 && dev.capture().size() <= 44101, "gravando 1 s de ciclos acumula ~44100 amostras");
        check(std::abs(RisingEdges(dev.capture()) - 440) <= 2, "o audio gravado tem ~440 Hz");

        const std::string path = (std::getenv("TEMP") ? std::string(std::getenv("TEMP")) : std::string("/tmp")) + "/fwmsx_psg_test.wav";
        std::string error;
        const bool ok = psg::WriteWav(path, dev.capture(), psg::kSampleRate, error);
        std::ifstream f(path, std::ios::binary);
        const std::vector<char> bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        auto u32 = [&](size_t off) { return static_cast<uint32_t>(static_cast<uint8_t>(bytes[off])) | (static_cast<uint32_t>(static_cast<uint8_t>(bytes[off + 1])) << 8) |
                                            (static_cast<uint32_t>(static_cast<uint8_t>(bytes[off + 2])) << 16) | (static_cast<uint32_t>(static_cast<uint8_t>(bytes[off + 3])) << 24); };
        check(ok && bytes.size() == 44 + dev.capture().size() * 2 && std::string(bytes.begin(), bytes.begin() + 4) == "RIFF" &&
                  std::string(bytes.begin() + 8, bytes.begin() + 16) == "WAVEfmt " && u32(24) == 44100 && u32(40) == dev.capture().size() * 2,
              "WriteWav: cabecalho RIFF/WAVE PCM mono 16 bits, 44100 Hz, tamanho do bloco de dados certo");
        std::remove(path.c_str());
        std::string bad_error;
        check(!psg::WriteWav("/nao/existe/x.wav", dev.capture(), 44100, bad_error) && !bad_error.empty(), "WriteWav em caminho invalido: false + mensagem");
    }

    // --- 8. Sessao de depuracao: --psg e comandos -----------------------------
    {
        z80::debug::Z80DebugShellStartup startup = z80::debug::BuildZ80DebugShellStartup({"--slots", "--psg"});
        check(startup.use_slots && startup.use_psg && startup.psg_device && startup.composite_bus && !startup.use_vdp && !startup.use_ppi,
              "startup: '--slots --psg' monta PSG + barramento composto (sem VDP/PPI)");
        z80::debug::Z80DebugSession session(startup.Bus(), startup.memory_system.get(), nullptr, nullptr, startup.psg_device.get());

        // LD A,7 / OUT (A0h),A / LD A,F8h / OUT (A1h),A / LD A,0Fh / OUT (A0h),A / LD A,0 / OUT (A1h),A / HALT
        session.ProcessCommand({"poke", "0", "0x3E"});
        const uint8_t program[] = {0x3E, 0x07, 0xD3, 0xA0, 0x3E, 0xF8, 0xD3, 0xA1, 0x3E, 0x0F, 0xD3, 0xA0, 0x3E, 0x00, 0xD3, 0xA1, 0x76};
        for (size_t i = 0; i < sizeof(program); ++i)
            session.ProcessCommand({"poke", std::to_string(i), std::to_string(program[i])});
        session.ProcessCommand({"psgrec", "start"});
        session.ProcessCommand({"run", "200"});
        std::string r = session.ProcessCommand({"psgregs"});
        check(startup.psg_device->state().r[7] == 0xF8 && Contains(r, "R0-R15: 00 00 00 00 00 00 00 F8"),
              "programa Z80 com OUT (A0h)/(A1h) chega no PSG; psgregs mostra R7=F8");
        check(Contains(r, "Canal A: tom ON") && Contains(r, "Gravacao: LIGADA"), "psgregs: decodifica canais e estado da gravacao");

        r = session.ProcessCommand({"run", "100000"});
        check(startup.psg_device->capture().size() > 1000, "run com gravacao ligada acumula amostras (" + std::to_string(startup.psg_device->capture().size()) + ")");
        check(Contains(session.ProcessCommand({"psgpoke", "8", "0x0F"}), "R8 = 0F") && startup.psg_device->state().r[8] == 0x0F, "psgpoke escreve num registrador");
        check(Contains(session.ProcessCommand({"psgpoke", "16", "0"}), "invalido") && Contains(session.ProcessCommand({"psgpoke", "1"}), "uso:"), "psgpoke: erros de uso claros");
        check(Contains(session.ProcessCommand({"psgrec", "stop"}), "parada"), "psgrec stop");
        const std::string path = (std::getenv("TEMP") ? std::string(std::getenv("TEMP")) : std::string("/tmp")) + "/fwmsx_psg_sess.wav";
        r = session.ProcessCommand({"psgrec", "save", path});
        check(Contains(r, "WAV salvo"), "psgrec save: " + r);
        std::remove(path.c_str());
        check(Contains(session.ProcessCommand({"psgrec", "clear"}), "descartada") && Contains(session.ProcessCommand({"psgrec", "save", path}), "nada gravado"),
              "psgrec clear descarta; save sem gravacao avisa");
        session.ProcessCommand({"reset"});
        check(startup.psg_device->state().r[7] == 0xFD, "reset da sessao tambem reseta o PSG");

        z80::debug::Z80DebugSession no_psg(startup.Bus(), startup.memory_system.get());
        check(Contains(no_psg.ProcessCommand({"psgregs"}), "requer") && Contains(no_psg.ProcessCommand({"psgrec", "start"}), "requer"),
              "sem --psg, os comandos de PSG pedem a flag em vez de travar");
        z80::debug::Z80DebugShellStartup bad = z80::debug::BuildZ80DebugShellStartup({"--psg"});
        check(!bad.use_psg && Contains(bad.psg_error, "--slots"), "'--psg' sem '--slots' e' ignorado com aviso");
        z80::debug::Z80DebugShellStartup all = z80::debug::BuildZ80DebugShellStartup({"--slots", "--psg", "--vdp", "--ppi"});
        check(all.use_psg && all.use_vdp && all.use_ppi && !all.boot_rom_requested, "'--slots --psg --vdp --ppi': '--psg' logo apos '--slots' nao vira caminho de ROM");
    }

#ifdef FWMSX_SOURCE_DIR
    // --- 9. BIOS real: o PSG e' inicializado e o BEEP soa --------------------
    {
        const std::string rom = std::string(FWMSX_SOURCE_DIR) + "/resource/fMSX/ROMs/MSX.ROM";
        z80::debug::Z80DebugShellStartup s = z80::debug::BuildZ80DebugShellStartup({"--slots", rom, "--vdp", "--ppi", "--psg"});
        if (!s.boot_rom_loaded) {
            std::printf("[SKIP] BIOS real: '%s' nao encontrada\n", rom.c_str());
        } else {
            z80::debug::Z80DebugSession session(s.Bus(), s.memory_system.get(), s.vdp_device.get(), s.ppi_device.get(), s.psg_device.get());
            auto run = [&](int cycles) {
                for (int done = 0; done < cycles; done += 100000) session.ProcessCommand({"run", "100000"});
            };
            run(100000000);
            const PsgState &ps = s.psg_device->state();
            check(ps.r[7] == 0xB8, "BIOS real: GICINI programa o mixer do PSG (R7=B8h: tom ligado, ruido desligado, I/O em saida) -- R7=" + std::to_string(ps.r[7]));
            check(ps.r[8] == 0 && ps.r[9] == 0 && ps.r[10] == 0, "BIOS real: volumes dos 3 canais zerados apos a inicializacao");

            // Digita BEEP + ENTER no BASIC e grava o que sai. O BEEP da BIOS toca o
            // canal A em 1316 Hz (periodo 85) com volume 7.
            s.psg_device->StartRecording();
            for (const char *key : {"b", "e", "e", "p"}) {
                session.ProcessCommand({"keydown", key});
                run(1500000);
                session.ProcessCommand({"keyup", key});
                run(1500000);
            }
            session.ProcessCommand({"keydown", "enter"});
            double beep_hz = 0.0;
            int beep_level = 0;
            for (int i = 0; i < 100; ++i) { // 2M de ciclos em fatias, olhando o canal A
                session.ProcessCommand({"run", "20000"});
                if (psg_channel_level(&ps, 0) > 0 && beep_hz == 0.0) {
                    beep_hz = psg_tone_hz(&ps, 0);
                    beep_level = psg_channel_level(&ps, 0);
                }
            }
            session.ProcessCommand({"keyup", "enter"});
            run(20000000);
            const std::vector<int16_t> &cap = s.psg_device->capture();
            int peak = 0;
            for (int16_t v : cap) peak = std::max(peak, static_cast<int>(v));
            check(std::fabs(beep_hz - 1316.0) < 1.0 && beep_level == 7,
                  "BIOS real + BASIC: 'BEEP' programa o canal A em 1316 Hz, volume 7 (" + std::to_string(beep_hz) + " Hz, volume " +
                      std::to_string(beep_level) + ")");
            check(peak == 683, "BIOS real + BASIC: o BEEP gravado tem pico 683 (volume 7 = 10922/16) -- pico " + std::to_string(peak));
            check(RisingEdges(cap) > 30, "BIOS real + BASIC: o BEEP gravado e' uma onda de verdade (" + std::to_string(RisingEdges(cap)) + " ciclos)");
        }
    }
#endif

    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
