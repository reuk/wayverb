#pragma once

#include "hrtf/meta.h"

#include <array>

namespace hrtf_data {

struct entry final {
    int azimuth;
    int elevation;
    std::array<std::array<double, bands>, 2> energy;
};

}  // namespace hrtf_data
