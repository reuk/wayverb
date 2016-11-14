#pragma once

#include "glm/glm.hpp"

#include <tuple>

namespace wayverb {
namespace core {

struct az_el final {
    constexpr az_el() = default;
    constexpr az_el(float both)
            : azimuth{both}
            , elevation{both} {}
    constexpr az_el(float azimuth, float elevation)
            : azimuth{azimuth}
            , elevation{elevation} {}

    float azimuth{0};
    float elevation{0};
};

az_el& operator+=(az_el& a, const az_el& b);
az_el& operator-=(az_el& a, const az_el& b);
az_el& operator*=(az_el& a, const az_el& b);
az_el& operator/=(az_el& a, const az_el& b);

az_el operator+(const az_el& a, const az_el& b);
az_el operator-(const az_el& a, const az_el& b);
az_el operator*(const az_el& a, const az_el& b);
az_el operator/(const az_el& a, const az_el& b);

float compute_azimuth(const glm::vec3& pointing);
float compute_elevation(const glm::vec3& pointing);
az_el compute_azimuth_elevation(const glm::vec3& pointing);

glm::vec3 compute_pointing(const az_el& azel);

constexpr auto to_tuple(az_el& x) {
    return std::tie(x.azimuth, x.elevation);
}

constexpr auto to_tuple(const az_el&x) {
    return std::tie(x.azimuth, x.elevation);
}

constexpr auto operator==(const az_el& a, const az_el& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr auto operator!=(const az_el& a, const az_el& b) {
    return !(a == b);
}

}  // namespace core
}  // namespace wayverb
