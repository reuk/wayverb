#include "filters.h"
#include "filters_common.h"
#include "hrtf.h"

#include "sinc.h"

#include <iostream>
#include <numeric>

namespace filter {

void run(FilterType ft,
         std::vector<std::vector<std::vector<float>>>& data,
         float sr,
         float lo_cutoff) {
    std::unique_ptr<Bandpass> bp;

    switch (ft) {
        case FilterType::windowed_sinc:
            bp = std::make_unique<BandpassWindowedSinc>(
                data.front().front().size());
            break;
        case FilterType::biquad_onepass:
            bp = std::make_unique<BandpassBiquad>();
            break;
        case FilterType::biquad_twopass:
            bp = std::make_unique<TwopassFilterWrapper<BandpassBiquad>>();
            break;
        case FilterType::linkwitz_riley:
            bp = std::make_unique<LinkwitzRileyBandpass>();
            break;
    }

    for (auto& channel : data) {
        for (auto i = 0u; i != channel.size(); ++i) {
            bp->setParams(HrtfData::EDGES[i], HrtfData::EDGES[i + 1], sr);
            bp->filter(channel[i]);
        }
    }
}
}  // namespace filter
