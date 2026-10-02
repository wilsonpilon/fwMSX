// Teste do audio ao vivo: buffer circular, PSG em modo "live", a maquina
// completa gerando amostras e o dispositivo de audio de verdade (se houver) --
// ver doc/audio-spec.md.
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

#include "../../src/audio/audio_output.h"
#include "../../src/audio/ring_buffer.h"
#include "../../src/machine/machine.h"
#include "../../src/psg/cpp/psg_device.h"

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

void ProgramTone(psg::PsgDevice &dev) {
    dev.out(0xA0, 7);
    dev.out(0xA1, 0xF8);
    dev.out(0xA0, 0);
    dev.out(0xA1, 254);
    dev.out(0xA0, 8);
    dev.out(0xA1, 15);
}

} // namespace

int main() {
    // --- 1. Buffer circular ---------------------------------------------------
    {
        audio::SampleRing r(1000);
        check(r.capacity() == 1024, "capacidade arredondada para potencia de 2 (1000 -> 1024)");
        const int16_t in[5] = {1, 2, 3, 4, 5};
        int16_t out[8] = {};
        check(r.Push(in, 5) == 5 && r.size() == 5, "Push/size");
        check(r.Pop(out, 3) == 3 && out[0] == 1 && out[1] == 2 && out[2] == 3 && r.size() == 2, "Pop devolve em ordem FIFO");
        check(r.Pop(out, 8) == 2 && out[0] == 4 && out[1] == 5 && r.Pop(out, 8) == 0, "Pop de mais do que ha' devolve so' o que ha'");

        // Dar a volta no buffer varias vezes sem perder a ordem.
        audio::SampleRing small(8);
        bool ordered = true;
        int16_t next_in = 0, next_out = 0;
        for (int round = 0; round < 1000 && ordered; ++round) {
            int16_t chunk[5];
            for (int16_t &c : chunk) c = next_in++;
            small.Push(chunk, 5);
            int16_t got[5];
            const size_t n = small.Pop(got, 5);
            for (size_t i = 0; i < n; ++i) ordered = ordered && got[i] == next_out++;
        }
        check(ordered, "dar a volta no buffer 1000 vezes mantem a ordem");

        audio::SampleRing full(4);
        const int16_t big[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        check(full.Push(big, 10) == 4 && full.size() == 4 && full.Push(big, 1) == 0, "buffer cheio: o excedente e' descartado, nunca bloqueia");
    }

    // --- 2. Buffer circular com duas threads ---------------------------------
    {
        audio::SampleRing r(4096);
        constexpr int kTotal = 2000000;
        std::atomic<bool> bad{false};
        std::thread consumer([&] {
            int expected = 0;
            int16_t buf[256];
            while (expected < kTotal && !bad) {
                const size_t n = r.Pop(buf, 256);
                for (size_t i = 0; i < n; ++i) {
                    if (buf[i] != static_cast<int16_t>(expected & 0x7FFF)) bad = true;
                    ++expected;
                }
                if (n == 0) std::this_thread::yield();
            }
        });
        int sent = 0;
        while (sent < kTotal && !bad) {
            int16_t chunk[100];
            const int n = std::min(100, kTotal - sent);
            for (int i = 0; i < n; ++i) chunk[i] = static_cast<int16_t>((sent + i) & 0x7FFF);
            const size_t pushed = r.Push(chunk, static_cast<size_t>(n));
            sent += static_cast<int>(pushed); // so' avanca o que coube (como um produtor que reenvia)
            if (pushed == 0) std::this_thread::yield();
        }
        consumer.join();
        check(!bad, "2 milhoes de amostras entre duas threads: nenhuma perdida, duplicada ou fora de ordem");
    }

    // --- 3. PSG em modo ao vivo --------------------------------------------------
    {
        psg::PsgDevice dev;
        ProgramTone(dev);
        dev.Advance(100000);
        std::vector<int16_t> out;
        dev.TakeLive(out);
        check(out.empty() && !dev.live(), "sem EnableLive(), nenhuma amostra ao vivo e' acumulada");

        dev.EnableLive(true);
        for (int i = 0; i < 100; ++i) dev.Advance(PSG_Z80_CLOCK / 100); // 1 s
        dev.TakeLive(out);
        check(out.size() >= 44099 && out.size() <= 44101, "1 s de ciclos ao vivo = ~44100 amostras (" + std::to_string(out.size()) + ")");
        int16_t peak = 0;
        for (int16_t v : out) peak = std::max(peak, v);
        check(peak == 10922, "as amostras ao vivo sao o tom do PSG (pico 10922)");
        std::vector<int16_t> again;
        dev.TakeLive(again);
        check(again.empty(), "TakeLive() esvazia o buffer");

        // Ninguem drenando: nao cresce sem limite (guarda ate' 1 s).
        for (int i = 0; i < 300; ++i) dev.Advance(PSG_Z80_CLOCK / 100); // 3 s
        dev.TakeLive(again);
        check(again.size() == 44100, "sem consumidor, o buffer ao vivo guarda no maximo 1 s (" + std::to_string(again.size()) + ")");

        // Gravacao e ao vivo ao mesmo tempo, sem interferir um no outro.
        dev.StartRecording();
        for (int i = 0; i < 10; ++i) dev.Advance(PSG_Z80_CLOCK / 100);
        std::vector<int16_t> live;
        dev.TakeLive(live);
        check(!live.empty() && live.size() == dev.capture().size(), "gravacao + ao vivo juntos: as duas recebem as mesmas amostras");
        dev.EnableLive(false);
        dev.Advance(PSG_Z80_CLOCK / 10);
        again.clear();
        dev.TakeLive(again);
        check(again.empty(), "EnableLive(false) para de acumular");
    }

#ifdef FWMSX_SOURCE_DIR
    // --- 4. Maquina completa: o PSG ao vivo acompanha os quadros ----------------
    {
        machine::MachineConfig config;
        config.bios_path = std::string(FWMSX_SOURCE_DIR) + "/resource/fMSX/ROMs/MSX.ROM";
        std::string error;
        std::unique_ptr<machine::Machine> m = machine::Machine::Create(config, error);
        if (!m) {
            std::printf("[SKIP] BIOS real nao encontrada (%s)\n", error.c_str());
        } else {
            m->psg().EnableLive(true);
            std::vector<int16_t> live;
            for (int i = 0; i < 120; ++i) {
                m->RunFrame();
                m->psg().TakeLive(live); // uma vez por quadro, como a janela
            }
            const double expected = 120.0 * 44100.0 / machine::Machine::kFrameRate;
            check(std::fabs(static_cast<double>(live.size()) - expected) < 200.0,
                  "120 quadros geram ~" + std::to_string(static_cast<int>(expected)) + " amostras (" + std::to_string(live.size()) + ")");
        }
    }
#endif

    // --- 5. Dispositivo de audio de verdade -------------------------------------------
    {
        audio::AudioOutput out(44100);
        check(!out.running() && out.consumed_samples() == 0, "antes do Start(): parado, nada consumido");
        std::string error;
        if (!out.Start(error)) {
            std::printf("[SKIP] sem dispositivo de audio nesta maquina: %s\n", error.c_str());
        } else {
            check(out.running() && !out.device_name().empty(), "Start(): dispositivo aberto -- '" + out.device_name() + "'");
            out.SetGain(0.0f); // o teste nao precisa tocar nada de audivel
            check(out.gain() == 0.0f, "SetGain(0.0) = mudo");
            out.SetGain(5.0f);
            check(out.gain() == 1.0f, "SetGain limita em 1.0");
            out.SetGain(0.0f);

            // Alimenta em tempo real como a janela faz: a cada volta empurra as
            // amostras que o tempo decorrido ja' "gerou" (nao dorme um tanto
            // fixo -- o sleep do Windows tem granularidade de ~15 ms). Dura 1 s
            // e confere que o dispositivo consome no ritmo de 44100 Hz.
            std::vector<int16_t> chunk;
            const auto start = std::chrono::steady_clock::now();
            const uint64_t consumed_before = out.consumed_samples();
            size_t produced = 0;
            while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(1000)) {
                const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
                const size_t due = static_cast<size_t>(elapsed * 44100.0);
                chunk.clear();
                for (; produced < due; ++produced)
                    chunk.push_back(static_cast<int16_t>(8000 * std::sin(2.0 * 3.14159265 * 440.0 * static_cast<double>(produced) / 44100.0)));
                out.Push(chunk.data(), chunk.size());
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
            const double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
            const uint64_t consumed = out.consumed_samples() - consumed_before;
            const double rate = consumed / secs;
            check(rate > 44100 * 0.8 && rate < 44100 * 1.15,
                  "o dispositivo consome em tempo real, ~44100 amostras/s (medido: " + std::to_string(static_cast<int>(rate)) + ")");
            check(out.underruns() <= 3, "alimentado no ritmo, quase sem underruns (" + std::to_string(out.underruns()) + ")");
            check(out.dropped_samples() < 2000, "alimentado no ritmo, quase nada descartado (" + std::to_string(out.dropped_samples()) + ")");

            // Produtor MUITO mais rapido que o dispositivo: descarta, nao trava nem cresce.
            const uint64_t dropped_before = out.dropped_samples();
            chunk.assign(735, 1000);
            for (int i = 0; i < 100; ++i) out.Push(chunk.data(), chunk.size());
            check(out.dropped_samples() > dropped_before, "produtor mais rapido que o dispositivo: o excedente e' descartado (sem travar)");

            // Flush: o que sobrou e' jogado fora e o dispositivo volta a esperar.
            out.Flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            const uint64_t after_flush = out.consumed_samples();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            check(out.consumed_samples() - after_flush < 200, "Flush(): o dispositivo fica em silencio ate' chegarem amostras novas");

            out.Stop();
            check(!out.running(), "Stop() fecha o dispositivo");
            out.Stop(); // idempotente
        }
    }

    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
