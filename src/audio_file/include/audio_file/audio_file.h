#pragma once

#include <string>
#include <vector>

namespace audio_file {

enum class bit_depth { pcm16 = 1, pcm24, pcm32, float32 };

constexpr auto get_description(bit_depth b) {
    switch (b) {
        case bit_depth::pcm16: return "signed 16-bit pcm";
        case bit_depth::pcm24: return "signed 24-bit pcm";
        case bit_depth::pcm32: return "signed 32-bit pcm";
        case bit_depth::float32: return "32-bit float";
    }
}

enum class format { wav = 1, aif };

constexpr auto get_extension(format f) {
    switch (f) {
        case format::wav: return "wav";
        case format::aif: return "aif";
    }
}

/// Given a 2d iterable structure, produce a single interleaved std::vector.
template <typename It>
auto interleave(It b, It e) {
    using std::cbegin;
    using std::cend;

    using value_type = std::decay_t<decltype(*cbegin(*b))>;
    using return_type = std::vector<value_type>;

    const auto channels = std::distance(b, e);
    if (channels <= 0) {
        return return_type{};
    }

    const auto frames = std::distance(cbegin(*b), cend(*b));
    if (frames <= 0) {
        return return_type{};
    }

    for (auto it = b; it != e; ++it) {
        if (std::distance(cbegin(*it), cend(*it)) != frames) {
            throw std::runtime_error{"All channels must have equal length."};
        }
    }

    return_type interleaved(channels * frames);
    for (auto i = 0ul; i != channels; ++i) {
        for (auto j = 0ul; j != frames; ++j) {
            interleaved[j * channels + i] = b[i][j];
        }
    }
    return interleaved;
}

/// Given a single iterable array of elements, and a channel number, produce
/// a 2d vector laid out by [channel][sample].
template <typename It>
auto deinterleave(It b, It e, size_t channels) {
    if (channels == 0) {
        throw std::runtime_error{"Channels must be non-zero."};
    }
    const auto in_size = std::distance(b, e);
    if (in_size % channels) {
        throw std::runtime_error{
                "Input size must be divisible by number of channels."};
    }

    const auto frames = in_size / channels;
    using value_type = std::decay_t<decltype(*b)>;
    std::vector<std::vector<value_type>> deinterleaved(
            channels, std::vector<value_type>(frames));

    for (auto i = 0ul; b != e; ++b, ++i) {
        const auto channel = i % channels;
        const auto sample = i / channels;
        deinterleaved[channel][sample] = *b;
    }

    return deinterleaved;
}

template <typename T>
void write_interleaved(const char* fname,
                       const T* data,
                       size_t num,
                       int channels,
                       int sr,
                       format format,
                       bit_depth bit_depth);

template <typename T>
void write(const char* fname,
           const T* data,
           size_t num,
           int sr,
           format format,
           bit_depth bit_depth) {
    write_interleaved(fname, data, num, 1, sr, format, bit_depth);
}

template <typename T>
void write(const char* fname,
           const T& data,
           int sr,
           format format,
           bit_depth bit_depth) {
    write(fname, data.data(), data.size(), sr, format, bit_depth);
}

template <typename T>
struct audio_file final {
    std::vector<std::vector<T>> signal;
    double sample_rate;
};

audio_file<double> read(const char* fname);

template <typename It>
void write(const char* fname,
           It b,
           It e,
           int sample_rate,
           format format,
           bit_depth bit_depth) {
    const auto channels = std::distance(b, e);
    if (channels <= 0) {
        return;
    }
    const auto interleaved = interleave(b, e);
    write_interleaved(fname,
                      interleaved.data(),
                      interleaved.size(),
                      channels,
                      sample_rate,
                      format,
                      bit_depth);
}

}  // namespace audio_file
