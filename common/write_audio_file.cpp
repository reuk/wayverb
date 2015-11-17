#include "write_audio_file.h"

#include <unordered_map>
#include <sstream>

using namespace std;

void write_sndfile(const string& fname,
                   const vector<vector<float>>& outdata,
                   float sr,
                   unsigned long bd,
                   unsigned long ftype) {
    vector<float> interleaved(outdata.size() * outdata[0].size());

    for (auto i = 0u; i != outdata.size(); ++i)
        for (auto j = 0u; j != outdata[i].size(); ++j)
            interleaved[j * outdata.size() + i] = outdata[i][j];

    SndfileHandle outfile(fname, SFM_WRITE, ftype | bd, outdata.size(), sr);
    outfile.write(interleaved.data(), interleaved.size());
}

unsigned long get_file_format(const string& fname) {
    unordered_map<string, decltype(SF_FORMAT_AIFF)> ftypeTable{
        {"aif", SF_FORMAT_AIFF},
        {"aiff", SF_FORMAT_AIFF},
        {"wav", SF_FORMAT_WAV}};

    auto extension = fname.substr(fname.find_last_of(".") + 1);
    auto ftypeIt = ftypeTable.find(extension);
    if (ftypeIt == ftypeTable.end()) {
        stringstream ss;
        ss << "Invalid output file extension - valid extensions are: ";
        for (const auto& i : ftypeTable)
            ss << i.first << " ";
        throw runtime_error(ss.str());
    }
    return ftypeIt->second;
}

unsigned long get_file_depth(unsigned long bitDepth) {
    unordered_map<unsigned long, decltype(SF_FORMAT_PCM_16)> depthTable{
        {16, SF_FORMAT_PCM_16}, {24, SF_FORMAT_PCM_24}};

    auto depthIt = depthTable.find(bitDepth);
    if (depthIt == depthTable.end()) {
        stringstream ss;
        ss << "Invalid bitdepth - valid bitdepths are: ";
        for (const auto& i : depthTable)
            ss << i.first << " ";
        throw runtime_error(ss.str());
    }
    return depthIt->second;
}
