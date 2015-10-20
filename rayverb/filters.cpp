#include "filters.h"
#include "filters_common.h"

#include "sinc.h"

#include <numeric>
#include <iostream>

using namespace std;

void filter
(   FilterType ft
,   vector <vector <vector <float>>> & data
,   float sr
,   float lo_cutoff
)
{
    unique_ptr<Bandpass> bp;

    switch (ft) {
    case FILTER_TYPE_WINDOWED_SINC:
        bp = make_unique<BandpassWindowedSinc>(data.front().front().size());
        break;
    case FILTER_TYPE_BIQUAD_ONEPASS:
        bp = make_unique<OnepassBandpassBiquad>();
        break;
    case FILTER_TYPE_BIQUAD_TWOPASS:
        bp = unique_ptr<TwopassBandpassBiquad>();
        break;
    case FILTER_TYPE_LINKWITZ_RILEY:
        bp = unique_ptr<LinkwitzRiley>();
        break;
    }

    for (auto & channel : data) {
        const vector <float> EDGES
            {lo_cutoff, 175, 350, 700, 1400, 2800, 5600, 11200, 20000};

        for (auto i = 0u; i != channel.size(); ++i) {
            bp->setParams (EDGES [i], EDGES [i + 1], sr);
            bp->filter (channel [i]);
        }
    }
}
