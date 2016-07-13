#include "common/hrtf_utils.h"
#include "common/filters_common.h"
#include "common/hrtf.h"

/// Sum a collection of vectors of the same length into a single vector
std::vector<float> mixdown(const std::vector<std::vector<float>>& data) {
    std::vector<float> ret(data.front().size(), 0);
    for (const auto& i : data)
        proc::transform(ret, i.begin(), ret.begin(), std::plus<float>());
    return ret;
}

std::vector<std::vector<float>> mixdown(
        const std::vector<std::vector<std::vector<float>>>& data) {
    std::vector<std::vector<float>> ret(data.size());
    proc::transform(data, ret.begin(), [](auto i) { return mixdown(i); });
    return ret;
}

std::vector<std::vector<float>> multiband_filter(
        const std::vector<std::vector<float>> bands,
        const std::vector<double>& band_edges,
        double sample_rate) {
    std::vector<std::vector<float>> ret;
    ret.reserve(bands.size());
    filter::LinkwitzRileyBandpass bandpass;
    for (auto i = 0u; i != bands.size() && i != HrtfData::EDGES.size() - 1 &&
                      HrtfData::EDGES[i] < sample_rate * 0.5;
         ++i) {
        //  set bandpass parameters
        bandpass.set_params(
                HrtfData::EDGES[i], HrtfData::EDGES[i + 1], sample_rate);

        //  filter the band
        ret.push_back(filter::run_one_pass(
                bandpass, bands[i].begin(), bands[i].end()));
    }
    return ret;
}

std::vector<float> multiband_filter_and_mixdown(
        const std::vector<std::vector<float>>& bands,
        double sample_rate) {
    return mixdown(multiband_filter(
            bands,
            std::vector<double>(HrtfData::EDGES.begin(), HrtfData::EDGES.end()),
            sample_rate));
}
