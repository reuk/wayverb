#pragma once

#include "utilities/range.h"

#include <array>

namespace hrtf_data {

struct entry final {
    static constexpr size_t bands{8};

    int azimuth;
    int elevation;
    std::array<std::array<double, bands>, 2> energy;
};

}  // namespace hrtf_data
