// fwMSX -- stub de audio para builds com FWMSX_AUDIO=OFF -- ver audio_output.h.
#include "audio_output.h"

namespace audio {

struct AudioOutput::Impl {};

AudioOutput::AudioOutput(int) : impl_(new Impl()) {}
AudioOutput::~AudioOutput() = default;

bool AudioOutput::Start(std::string &error) {
    error = "audio nao foi compilado nesta build (recompile com -DFWMSX_AUDIO=ON)";
    return false;
}
void AudioOutput::Stop() {}
bool AudioOutput::running() const { return false; }
void AudioOutput::Push(const int16_t *, size_t) {}
void AudioOutput::SetGain(float) {}
float AudioOutput::gain() const { return 0.0f; }
void AudioOutput::Flush() {}
uint64_t AudioOutput::consumed_samples() const { return 0; }
uint64_t AudioOutput::underruns() const { return 0; }
uint64_t AudioOutput::dropped_samples() const { return 0; }
std::string AudioOutput::device_name() const { return ""; }

} // namespace audio
