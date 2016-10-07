#pragma once

#include <string>
#include <vector>

namespace audio_file {

namespace detail {

template <typename It>
auto interleave(It begin, It end) {
    const auto channels{std::distance(begin, end)};
    const auto frames{begin->size()};

    for (auto it{begin}; it != end; ++it) {
        if (it->size() != frames) {
            throw std::runtime_error{"all channels must have equal length"};
        }
    }

    std::vector<typename std::decay_t<decltype(*begin)>::value_type>
            interleaved(channels * frames);
    for (auto i{0ul}; i != channels; ++i) {
        for (auto j{0ul}; j != frames; ++j) {
            interleaved[j * channels + i] = begin[i][j];
        }
    }
    return interleaved;
}

template <typename It>
auto deinterleave(It begin, It end, size_t channels) {
    const auto in_size{std::distance(begin, end)};
    if (in_size % channels) {
        throw std::runtime_error{
                "input size must be divisible by number of channels"};
    }
    const auto frames{in_size / channels};
    using value_type = std::decay_t<decltype(*begin)>;
    std::vector<std::vector<value_type>> deinterleaved(
            channels, std::vector<value_type>(frames));

    for (auto i{0ul}; begin != end; ++begin, ++i) {
        const auto channel{i % channels};
        const auto sample{i / channels};
        deinterleaved[channel][sample] = *begin;
    }
    return deinterleaved;
}

int get_file_format(const std::string& fname);
int get_file_depth(int bitDepth);

template <typename T>
void write_interleaved(const std::string& name,
                       T data,
                       size_t num,
                       int channels,
                       int sr,
                       int bd,
                       int ftype);

}  // namespace detail

template <typename T,
          typename AllocIn = std::allocator<T>,
          typename AllocOut = std::allocator<std::vector<T, AllocIn>>>
struct audio_file final {
    std::vector<std::vector<T, AllocIn>, AllocOut> signal;
    double sample_rate;
};

template <typename T, typename AllocIn, typename AllocOut>
auto make_audio_file(std::vector<std::vector<T, AllocIn>, AllocOut> signal,
                     double sample_rate) {
    return audio_file<T, AllocIn, AllocOut>{std::move(signal), sample_rate};
}

template <typename T, typename Alloc>
auto make_audio_file(std::vector<T, Alloc> signal, double sample_rate) {
    return make_audio_file(
            std::vector<std::vector<T, Alloc>>{std::move(signal)}, sample_rate);
}

audio_file<double> read(const std::string& fname);

template <typename value_type, typename AllocIn, typename AllocOut>
void write(const std::string& fname,
           const audio_file<value_type, AllocIn, AllocOut>& audio,
           size_t bit_depth) {
    const auto format{detail::get_file_format(fname)};
    const auto depth{detail::get_file_depth(bit_depth)};
    const auto interleaved{
            detail::interleave(audio.signal.begin(), audio.signal.end())};
    detail::write_interleaved(fname,
                              interleaved.data(),
                              interleaved.size(),
                              audio.signal.size(),
                              audio.sample_rate,
                              format,
                              depth);
}

}  // namespace audio_file
