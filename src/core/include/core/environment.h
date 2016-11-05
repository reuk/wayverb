#pragma once

namespace wayverb {
namespace core {

struct environment final {
    double speed_of_sound{340.0};
    double acoustic_impedance{400.0};
};

constexpr auto get_ambient_density(const environment& s) {
    return s.acoustic_impedance / s.speed_of_sound;
}

}  // namespace core
}  // namespace wayverb
