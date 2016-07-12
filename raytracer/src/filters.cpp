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
};

template<> struct get_filter_trait<FilterType::biquad_onepass> {
    using filter = BandpassBiquad;
    static filter construct(size_t signal_length) {
        return filter();
    }
};

template<> struct get_filter_trait<FilterType::biquad_twopass> {
    using filter = TwopassFilterWrapper<BandpassBiquad>;
    static filter construct(size_t signal_length) {
        return filter();
    }
};

template<> struct get_filter_trait<FilterType::linkwitz_riley> {
    using filter = LinkwitzRileyBandpass;
    static filter construct(size_t signal_length) {
        return filter();
    }
};

template<FilterType ft>
void run_filter(std::vector<std::vector<std::vector<float>>>& data,
                float sr,
                float lo_cutoff) {
    auto filter = get_filter_trait<ft>::construct(data.front().front().size());

    for (auto& channel : data) {
        for (auto i = 0u; i != channel.size(); ++i) {
            filter.set_params(HrtfData::EDGES[i], HrtfData::EDGES[i + 1], sr);
            channel[i] = filter.filter(channel[i].begin(), channel[i].end());
        }
    }
}

void run(FilterType ft,
         std::vector<std::vector<std::vector<float>>>& data,
         float sr,
         float lo_cutoff) {
    switch (ft) {
        case FilterType::windowed_sinc:
            run_filter<FilterType::windowed_sinc>(data, sr, lo_cutoff);
            break;
        case FilterType::biquad_onepass:
            run_filter<FilterType::biquad_onepass>(data, sr, lo_cutoff);
            break;
        case FilterType::biquad_twopass:
            run_filter<FilterType::biquad_twopass>(data, sr, lo_cutoff);
            break;
        case FilterType::linkwitz_riley:
            run_filter<FilterType::linkwitz_riley>(data, sr, lo_cutoff);
            break;
    }
}
}  // namespace filter
