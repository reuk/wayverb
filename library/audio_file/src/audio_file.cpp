#include "audio_file/audio_file.h"

#include "sndfile.hh"

#include <sstream>
#include <unordered_map>

namespace detail {

int get_file_format(const std::string& fname) {
    std::unordered_map<std::string, decltype(SF_FORMAT_AIFF)> ftypeTable{
            {"aif", SF_FORMAT_AIFF},
            {"aiff", SF_FORMAT_AIFF},
            {"wav", SF_FORMAT_WAV}};

    auto extension = fname.substr(fname.find_last_of(".") + 1);
    auto ftypeIt = ftypeTable.find(extension);
    if (ftypeIt == ftypeTable.end()) {
        std::stringstream ss;
        ss << "Invalid output file extension - valid extensions are: ";
        for (const auto& i : ftypeTable) {
            ss << i.first << " ";
        }
        throw std::runtime_error(ss.str());
    }
    return ftypeIt->second;
}

int get_file_depth(int bitDepth) {
    std::unordered_map<int, decltype(SF_FORMAT_PCM_16)> depthTable{
            {16, SF_FORMAT_PCM_16},
            {24, SF_FORMAT_PCM_24},
            {32, SF_FORMAT_PCM_32}};

    auto depthIt = depthTable.find(bitDepth);
    if (depthIt == depthTable.end()) {
        std::stringstream ss;
        ss << "Invalid bitdepth - valid bitdepths are: ";
        for (const auto& i : depthTable) {
            ss << i.first << " ";
        }
        throw std::runtime_error(ss.str());
    }
    return depthIt->second;
}

template <typename T>
void write_interleaved(const std::string& name,
                       T data,
                       size_t num,
                       int channels,
                       int sr,
                       int ftype,
                       int bd) {
    const auto fmt{ftype | bd};
    if (!SndfileHandle::formatCheck(fmt, channels, sr)) {
        throw std::runtime_error(
                "looks like libsndfile can't write with those parameters");
    }

    SndfileHandle outfile{name, SFM_WRITE, fmt, channels, sr};
    const auto written{outfile.write(data, num)};
    if (!written) {
        throw std::runtime_error("failed to write audio file");
    }
}

template void write_interleaved<const short*>(const std::string& name,
                                              const short* begin,
                                              size_t num,
                                              int channels,
                                              int sr,
                                              int ftype,
                                              int bd);

template void write_interleaved<const int*>(const std::string& name,
                                            const int* begin,
                                            size_t num,
                                            int channels,
                                            int sr,
                                            int ftype,
                                            int bd);

template void write_interleaved<const float*>(const std::string& name,
                                              const float* begin,
                                              size_t num,
                                              int channels,
                                              int sr,
                                              int ftype,
                                              int bd);

template void write_interleaved<const double*>(const std::string& name,
                                               const double* begin,
                                               size_t num,
                                               int channels,
                                               int sr,
                                               int ftype,
                                               int bd);

}  // namespace detail

audio_file<double> read(const std::string& fname) {
    SndfileHandle infile{fname, SFM_READ};
    const auto channels{infile.channels()};
    std::vector<double> interleaved(infile.frames() * channels);
    infile.readf(interleaved.data(), infile.frames());
    return make_audio_file(
            detail::deinterleave(
                    interleaved.begin(), interleaved.end(), channels),
            static_cast<double>(infile.samplerate()));
}
