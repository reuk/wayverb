#include "LoadContext.hpp"

LoadContext::LoadContext(GLAudioThumbnailBase& o,
                         std::unique_ptr<AudioFormatReader>&& m)
        : owner(o)
        , audio_format_reader(std::move(m))
        , samples_read(0)
        , channels(audio_format_reader->numChannels)
        , length_in_samples(audio_format_reader->lengthInSamples)
        , sample_rate(audio_format_reader->sampleRate)
        , keep_reading(true)
        , thread([this] {
            const auto buffer_size = 4096;
            AudioSampleBuffer buffer(channels, buffer_size);
            owner.reset(channels, sample_rate, length_in_samples);
            for (; !is_fully_loaded() && keep_reading;
                 samples_read += buffer_size) {
                buffer.clear();
                assert(audio_format_reader);
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
