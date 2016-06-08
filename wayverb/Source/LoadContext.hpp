#pragma once

#include "GLAudioThumbnail.h"

#include <thread>

class LoadContext {
public:
    LoadContext(GLAudioThumbnailBase& o,
                std::unique_ptr<AudioFormatReader>&& m,
                int buffer_size = 4096);
    virtual ~LoadContext() noexcept;

    bool is_fully_loaded() const;

private:
    GLAudioThumbnailBase& owner;
    std::unique_ptr<AudioFormatReader> audio_format_reader;
    int samples_read{0};

public:
    const int channels;
    const int length_in_samples;
    const double sample_rate;

private:
    std::atomic_bool keep_reading{true};
    std::thread thread;
};