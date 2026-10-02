#include "audio_output.h"

#include <atomic>
#include <cstring>

// So' o necessario do miniaudio: dispositivo de saida (sem decodificador,
// gerador, engine nem gerenciador de recursos).
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MA_NO_ENGINE
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "ring_buffer.h"

namespace audio {

namespace {

// Tamanho do buffer circular: ~370 ms a 44100 Hz -- folga para o jitter entre
// o relogio da janela e o do dispositivo, sem muita latencia (o excedente e'
// descartado, nao acumulado).
constexpr size_t kRingCapacity = 16384;
// O dispositivo so' comeca a tocar com este tanto ja' guardado (~33 ms), e volta
// a esperar se esvaziar -- evita estalos de underrun na partida.
constexpr size_t kPrimeSamples = 1470;

} // namespace

struct AudioOutput::Impl {
    explicit Impl(int rate) : sample_rate(rate), ring(kRingCapacity) {}

    int sample_rate;
    SampleRing ring;
    ma_device device{};
    bool device_open = false;
    bool primed = false; // so' a thread de audio mexe
    std::atomic<float> gain{0.7f};
    std::atomic<bool> flush_request{false};
    std::atomic<uint64_t> consumed{0};
    std::atomic<uint64_t> underruns{0};
    std::atomic<uint64_t> dropped{0};
    std::string name;

    static void Callback(ma_device *dev, void *output, const void *, ma_uint32 frames) {
        Impl *self = static_cast<Impl *>(dev->pUserData);
        int16_t *out = static_cast<int16_t *>(output);
        const size_t want = frames;

        if (self->flush_request.exchange(false)) {
            int16_t discard[256];
            while (self->ring.Pop(discard, 256) > 0) {
            }
            self->primed = false;
        }

        size_t got = 0;
        if (!self->primed && self->ring.size() >= kPrimeSamples) self->primed = true;
        if (self->primed) {
            got = self->ring.Pop(out, want);
            if (got < want) {
                // Esvaziou: completa com silencio e espera encher de novo.
                self->underruns.fetch_add(1, std::memory_order_relaxed);
                self->primed = false;
            }
        }
        if (got < want) std::memset(out + got, 0, (want - got) * sizeof(int16_t));

        const float g = self->gain.load(std::memory_order_relaxed);
        if (g < 0.999f) {
            for (size_t i = 0; i < got; ++i) out[i] = static_cast<int16_t>(out[i] * g);
        }
        self->consumed.fetch_add(got, std::memory_order_relaxed);
    }
};

AudioOutput::AudioOutput(int sample_rate) : impl_(new Impl(sample_rate)) {}

AudioOutput::~AudioOutput() { Stop(); }

bool AudioOutput::Start(std::string &error) {
    if (impl_->device_open) return true;

    ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
    cfg.playback.format = ma_format_s16;
    cfg.playback.channels = 1;
    cfg.sampleRate = static_cast<ma_uint32>(impl_->sample_rate);
    cfg.periodSizeInFrames = 441; // ~10 ms por periodo
    cfg.periods = 3;
    cfg.dataCallback = &Impl::Callback;
    cfg.pUserData = impl_.get();

    if (ma_device_init(nullptr, &cfg, &impl_->device) != MA_SUCCESS) {
        error = "nao foi possivel abrir um dispositivo de audio";
        return false;
    }
    impl_->device_open = true;
    impl_->name = impl_->device.playback.name;

    if (ma_device_start(&impl_->device) != MA_SUCCESS) {
        ma_device_uninit(&impl_->device);
        impl_->device_open = false;
        error = "nao foi possivel iniciar o dispositivo de audio";
        return false;
    }
    return true;
}

void AudioOutput::Stop() {
    if (!impl_ || !impl_->device_open) return;
    ma_device_uninit(&impl_->device); // para o callback antes de devolver
    impl_->device_open = false;
}

bool AudioOutput::running() const { return impl_->device_open; }

void AudioOutput::Push(const int16_t *samples, size_t count) {
    if (!impl_->device_open || count == 0) return;
    const size_t pushed = impl_->ring.Push(samples, count);
    if (pushed < count) impl_->dropped.fetch_add(count - pushed, std::memory_order_relaxed);
}

void AudioOutput::SetGain(float gain) {
    impl_->gain.store(gain < 0.0f ? 0.0f : (gain > 1.0f ? 1.0f : gain), std::memory_order_relaxed);
}

float AudioOutput::gain() const { return impl_->gain.load(std::memory_order_relaxed); }

void AudioOutput::Flush() { impl_->flush_request.store(true); }

uint64_t AudioOutput::consumed_samples() const { return impl_->consumed.load(std::memory_order_relaxed); }
uint64_t AudioOutput::underruns() const { return impl_->underruns.load(std::memory_order_relaxed); }
uint64_t AudioOutput::dropped_samples() const { return impl_->dropped.load(std::memory_order_relaxed); }
std::string AudioOutput::device_name() const { return impl_->name; }

} // namespace audio
