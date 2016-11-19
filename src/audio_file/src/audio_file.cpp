#include "audio_file/audio_file.h"

#include "sndfile.hh"

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace audio_file {

namespace {

constexpr auto get_sndfile_format(format f) {
    switch (f) {
        case format::wav: return SF_FORMAT_WAV;
        case format::aiff: return SF_FORMAT_AIFF;
    }
}

constexpr auto get_sndfile_bit_depth(bit_depth b) {
    switch (b) {
        case bit_depth::pcm16: return SF_FORMAT_PCM_16;
        case bit_depth::pcm24: return SF_FORMAT_PCM_24;
        case bit_depth::pcm32: return SF_FORMAT_PCM_32;
        case bit_depth::float32: return SF_FORMAT_FLOAT;
    }
}

}  // namespace

template <typename T>
void write_interleaved(const char* name,
                       const T* data,
                       size_t num,
                       int channels,
                       int sr,
                       format format,
                       bit_depth bit_depth) {
    const auto fmt =
            get_sndfile_format(format) | get_sndfile_bit_depth(bit_depth);
    if (!SndfileHandle::formatCheck(fmt, channels, sr)) {
        throw std::runtime_error(
                "looks like libsndfile can't write with those parameters");
    }

    SndfileHandle outfile{name, SFM_WRITE, fmt, channels, sr};
    const auto written = outfile.write(data, num);
    if (!written) {
        throw std::runtime_error("failed to write audio file");
    }
}

template void write_interleaved<short>(const char* name,
                                       const short* begin,
                                       size_t num,
                                       int channels,
                                       int sr,
                                       format format,
                                       bit_depth bit_depth);

template void write_interleaved<int>(const char* name,
                                     const int* begin,
                                     size_t num,
                                     int channels,
                                     int sr,
                                     format format,
                                     bit_depth bit_depth);

template void write_interleaved<float>(const char* name,
                                       const float* begin,
                                       size_t num,
                                       int channels,
                                       int sr,
                                       format format,
                                       bit_depth bit_depth);

template void write_interleaved<double>(const char* name,
                                        const double* begin,
                                        size_t num,
                                        int channels,
                                        int sr,
                                        format format,
                                        bit_depth bit_depth);

audio_file<double> read(const char* fname) {
    {
        std::ifstream is{fname};
        if (!is.good()) {
            throw std::runtime_error{"audio_file::read: unable to open file"};
        }
    }
    SndfileHandle infile{fname, SFM_READ};
    const auto channels = infile.channels();
    std::vector<double> interleaved(infile.frames() * channels);
    infile.readf(interleaved.data(), infile.frames());
    return {deinterleave(interleaved.begin(), interleaved.end(), channels),
            static_cast<double>(infile.samplerate())};
}

}  // namespace audio_file
