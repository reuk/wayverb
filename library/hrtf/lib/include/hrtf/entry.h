#pragma once

#include <array>

namespace hrtf {

template <size_t bands>
struct entry final {
    int azimuth;
    int elevation;
    std::array<std::array<double, bands>, 2> energy;
};

template <size_t bands>
constexpr auto make_entry(
        int azimuth,
        int elevation,
        const std::array<std::array<double, bands>, 2>& energy) {
    return entry<bands>{azimuth, elevation, energy};
}

}  // namespace hrtf
