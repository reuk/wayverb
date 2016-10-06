#pragma once

#include <array>

namespace hrtf {

template <size_t bands>
struct entry final {
    int azimuth;
    int elevation;
    std::array<float, bands> energy;
};

}  // namespace hrtf
