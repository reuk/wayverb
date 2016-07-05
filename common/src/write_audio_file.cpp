#include "common/write_audio_file.h"

#include <sstream>
#include <unordered_map>

void snd::write(const std::string& fname,
                const std::vector<std::vector<float>>& outdata,
                float sr,
                int bd,
                int ftype) {
    std::vector<float> interleaved(outdata.size() * outdata[0].size());

    for (auto i = 0u; i != outdata.size(); ++i) {
        for (auto j = 0u; j != outdata[i].size(); ++j) {
            interleaved[j * outdata.size() + i] = outdata[i][j];
        }
    }

    auto fmt = ftype | bd;
    if (! SndfileHandle::formatCheck(fmt, outdata.size(), sr)) {
        throw std::runtime_error(
                "looks like libsndfile can't write with those parameters");
    }

    SndfileHandle outfile(fname, SFM_WRITE, fmt, outdata.size(), sr);
    auto written = outfile.write(interleaved.data(), interleaved.size());
    if (!written) {
        throw std::runtime_error("failed to write audio file");
    }
}

void snd::write(const std::string& fname,
                const std::vector<std::vector<float>>& signal,
                double sample_rate,
                size_t bit_depth) {
    auto format = get_file_format(fname);
    auto depth = get_file_depth(bit_depth);
    write(fname, signal, sample_rate, depth, format);
}

int snd::get_file_format(const std::string& fname) {
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

int snd::get_file_depth(int bitDepth) {
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
