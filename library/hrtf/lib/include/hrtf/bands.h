#pragma once

#include "common/cl/traits.h"

#include "frequency_domain/envelope.h"

namespace hrtf {

constexpr auto bands{8};
constexpr range<double> audible_range_hz{20, 20000};

const auto band_edges_hz{
        frequency_domain::band_edge_frequencies<bands>(audible_range_hz)};
const auto band_centres_hz{
        frequency_domain::band_centre_frequencies<bands>(audible_range_hz)};

}  // namespace hrtf
