#include "LoadContext.hpp"

LoadContext::LoadContext(GLAudioThumbnailBase& o,
                         std::unique_ptr<AudioFormatReader>&& m,
                         int buffer_size)
        : owner(o)
        , audio_format_reader(std::move(m))
        , channels(audio_format_reader->numChannels)
        , length_in_samples(audio_format_reader->lengthInSamples)
        , sample_rate(audio_format_reader->sampleRate)
        , thread([this, buffer_size] {
            AudioSampleBuffer buffer(channels, buffer_size);
            owner.reset(channels, sample_rate, length_in_samples);
            for (; !is_fully_loaded() && keep_reading;
                 samples_read += buffer_size) {
                buffer.clear();
                audio_format_reader->read(
                        &buffer,
                        0,
                        std::min(buffer_size, length_in_samples - samples_read),
                        samples_read,
                        true,
                        true);
                owner.addBlock(samples_read, buffer, 0, buffer_size);
            }
        }) {
}

LoadContext::~LoadContext() noexcept {
    keep_reading = false;
    thread.join();
}

bool LoadContext::is_fully_loaded() const {
    return samples_read >= length_in_samples;
}
