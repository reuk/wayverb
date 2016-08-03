#include "common/hrtf_utils.h"
#include "common/filters_common.h"
#include "common/hrtf.h"

/// Sum a collection of vectors of the same length into a single vector
aligned::vector<float> mixdown(
        const aligned::vector<aligned::vector<float>>& data) {
    aligned::vector<float> ret(data.front().size(), 0);
    for (const auto& i : data)
        proc::transform(ret, i.begin(), ret.begin(), std::plus<float>());
    return ret;
}

aligned::vector<aligned::vector<float>> mixdown(
        const aligned::vector<aligned::vector<aligned::vector<float>>>& data) {
    aligned::vector<aligned::vector<float>> ret(data.size());
    proc::transform(data, ret.begin(), [](auto i) { return mixdown(i); });
    return ret;
}

aligned::vector<aligned::vector<float>> multiband_filter(
        const aligned::vector<aligned::vector<float>> bands,
        const aligned::vector<double>& band_edges,
        double sample_rate) {
    aligned::vector<aligned::vector<float>> ret;
    ret.reserve(bands.size());
    filter::LinkwitzRileyBandpass bandpass;
    for (auto i = 0u; i != bands.size() && i != hrtf_data::edges.size() - 1 &&
                      hrtf_data::edges[i] < sample_rate * 0.5;
         ++i) {
        //  set bandpass parameters
        bandpass.set_params(
                hrtf_data::edges[i], hrtf_data::edges[i + 1], sample_rate);

        //  filter the band
        ret.push_back(filter::run_one_pass(
                bandpass, bands[i].begin(), bands[i].end()));
    }
    return ret;
}

aligned::vector<float> multiband_filter_and_mixdown(
        const aligned::vector<aligned::vector<float>>& bands,
        double sample_rate) {
    return mixdown(
            multiband_filter(bands,
                             aligned::vector<double>(hrtf_data::edges.begin(),
                                                     hrtf_data::edges.end()),
                             sample_rate));
}
