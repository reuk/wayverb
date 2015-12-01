#include "filters.h"
#include "filters_common.h"
#include "hrtf.h"

#include "sinc.h"

#include <numeric>
#include <iostream>

void filter(FilterType ft,
            std::vector<std::vector<std::vector<float>>>& data,
            float sr,
            float lo_cutoff) {
    std::unique_ptr<Bandpass> bp;

    switch (ft) {
        case FILTER_TYPE_WINDOWED_SINC:
            bp = std::make_unique<BandpassWindowedSinc>(
                data.front().front().size());
            break;
        case FILTER_TYPE_BIQUAD_ONEPASS:
            bp = std::make_unique<OnepassBandpassBiquad>();
            break;
        case FILTER_TYPE_BIQUAD_TWOPASS:
            bp = std::make_unique<TwopassBandpassBiquad>();
            break;
        case FILTER_TYPE_LINKWITZ_RILEY:
            bp = std::make_unique<LinkwitzRiley>();
            break;
    }

    for (auto& channel : data) {
        for (auto i = 0u; i != channel.size(); ++i) {
            bp->setParams(HrtfData::EDGES[i], HrtfData::EDGES[i + 1], sr);
            bp->filter(channel[i]);
        }
    }
}
