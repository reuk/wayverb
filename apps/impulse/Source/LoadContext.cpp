#include "LoadContext.hpp"

#include "raii_thread.hpp"

class LoadContext::Impl final {
public:
    Impl(AudioFormatWriter::ThreadedWriter::IncomingDataReceiver& o,
         std::unique_ptr<AudioFormatReader>&& m)
            : owner(o)
            , audio_format_reader(std::move(m))
            , samples_read(0)
            , channels(audio_format_reader->numChannels)
            , length_in_samples(audio_format_reader->lengthInSamples)
            , sample_rate(audio_format_reader->sampleRate)
            , keep_reading(true)
            , thread([this] {
                const size_t buffer_size = 4096;
                AudioSampleBuffer buffer(channels, buffer_size);
                owner.reset(channels, sample_rate, length_in_samples);
                for (; !is_fully_loaded() && keep_reading;
                     samples_read += buffer_size) {
                    buffer.clear();
                    assert(audio_format_reader);
                    audio_format_reader->read(
                            &buffer,
                            0,
                            std::min(buffer_size,
                                     length_in_samples - samples_read),
                            samples_read,
                            true,
                            true);
                    owner.addBlock(samples_read, buffer, 0, buffer_size);
                }
            }) {
    }

    ~Impl() noexcept {
        keep_reading = false;
    }

    bool is_fully_loaded() const {
        return samples_read >= length_in_samples;
    }

    size_t get_num_channels() const {
        return channels;
    }

    size_t get_length_in_samples() const {
        return length_in_samples;
    }

    double get_sample_rate() const {
        return sample_rate;
    }

private:
    AudioFormatWriter::ThreadedWriter::IncomingDataReceiver& owner;
    std::unique_ptr<AudioFormatReader> audio_format_reader;
    size_t samples_read;

    size_t channels;
    size_t length_in_samples;
    double sample_rate;

    std::atomic_bool keep_reading;
    raii_thread thread;
};

//----------------------------------------------------------------------------//

LoadContext::LoadContext(
        AudioFormatWriter::ThreadedWriter::IncomingDataReceiver& o,
        std::unique_ptr<AudioFormatReader>&& m)
        : pimpl(std::make_unique<Impl>(o, std::move(m))) {
}

LoadContext::~LoadContext() noexcept = default;

bool LoadContext::is_fully_loaded() const {
    return pimpl->is_fully_loaded();
}

size_t LoadContext::get_num_channels() const {
    return pimpl->get_num_channels();
}

size_t LoadContext::get_length_in_samples() const {
    return pimpl->get_length_in_samples();
}

double LoadContext::get_sample_rate() const {
    return pimpl->get_sample_rate();
}