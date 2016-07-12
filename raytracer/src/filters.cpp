#include "raytracer/filters.h"

#include "common/filters_common.h"
#include "common/hrtf.h"
#include "common/sinc.h"

#include <iostream>
#include <numeric>

namespace filter {

template<FilterType ft> struct get_filter_trait;

template<> struct get_filter_trait<FilterType::windowed_sinc> {
    using filter = BandpassWindowedSinc;

    static filter construct(size_t signal_length) {
        return filter(signal_length);
    }

    template<typename It>
    static auto run(filter& f, It begin, It end) {
        return run_one_pass(f, begin, end);
    }
};

template<> struct get_filter_trait<FilterType::biquad_onepass> {
    using filter = BandpassBiquad;

    static filter construct(size_t signal_length) {
        return filter();
    }

    template<typename It>
    static auto run(filter& f, It begin, It end) {
        return run_one_pass(f, begin, end);
    }
};

template<> struct get_filter_trait<FilterType::biquad_twopass> {
    using filter = BandpassBiquad;

    static filter construct(size_t signal_length) {
        return filter();
    }

    template<typename It>
    static auto run(filter& f, It begin, It end) {
        return run_two_pass(f, begin, end);
    }
};

template<> struct get_filter_trait<FilterType::linkwitz_riley> {
    using filter = LinkwitzRileyBandpass;

    static filter construct(size_t signal_length) {
        return filter();
    }

    template<typename It>
    static auto run(filter& f, It begin, It end) {
        return run_one_pass(f, begin, end);
    }
};

template <FilterType ft>
std::vector<std::vector<std::vector<float>>> run_filter(
        const std::vector<std::vector<std::vector<float>>>& data,
        float sr,
        float lo_cutoff) {
    auto filter = get_filter_trait<ft>::construct(data.front().front().size());

    std::vector<std::vector<std::vector<float>>> ret;
    ret.reserve(data.size());

    for (const auto& channel : data) {
        std::vector<std::vector<float>> ret_channel;
        ret_channel.reserve(channel.size());
        for (auto i = 0u; i != channel.size(); ++i) {
            filter.set_params(HrtfData::EDGES[i], HrtfData::EDGES[i + 1], sr);
            ret_channel.push_back(
                    filter.filter(channel[i].begin(), channel[i].end()));
        }

        ret.push_back(ret_channel);
    }

    return ret;
}

std::vector<std::vector<std::vector<float>>> run(
        FilterType ft,
        const std::vector<std::vector<std::vector<float>>>& data,
        float sr,
        float lo_cutoff) {
    switch (ft) {
        case FilterType::windowed_sinc:
            return run_filter<FilterType::windowed_sinc>(data, sr, lo_cutoff);
        case FilterType::biquad_onepass:
            return run_filter<FilterType::biquad_onepass>(data, sr, lo_cutoff);
        case FilterType::biquad_twopass:
            return run_filter<FilterType::biquad_twopass>(data, sr, lo_cutoff);
        case FilterType::linkwitz_riley:
            return run_filter<FilterType::linkwitz_riley>(data, sr, lo_cutoff);
    }
}
}  // namespace filter
