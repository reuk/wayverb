#pragma once

#include "waveguide/postprocessor/directional_receiver.h"

#include "utilities/aligned/vector.h"
#include "utilities/range.h"

namespace wayverb {
namespace waveguide {

struct band final {
    util::aligned::vector<postprocessor::directional_receiver::output>
            directional;
    double sample_rate;
};

struct bandpass_band final {
    band band;
    util::range<double> valid_hz;
};

}  // namespace waveguide
}  // namespace wayverb
