#include "core/az_el.h"
#include "core/almost_equal.h"

#include "glm/gtx/transform.hpp"

#include <functional>

namespace wayverb {
namespace core {
namespace {

template <typename Op>
az_el& inplace_zip(az_el& a, const az_el& b, Op op) {
    a.azimuth = op(a.azimuth, b.azimuth);
    a.elevation = op(a.elevation, b.elevation);
    return a;
}

}  // namespace

az_el& operator+=(az_el& a, const az_el& b) {
    return inplace_zip(a, b, std::plus<>());
}
az_el& operator-=(az_el& a, const az_el& b) {
    return inplace_zip(a, b, std::minus<>());
}
az_el& operator*=(az_el& a, const az_el& b) {
    return inplace_zip(a, b, std::multiplies<>());
}
az_el& operator/=(az_el& a, const az_el& b) {
    return inplace_zip(a, b, std::divides<>());
}

az_el operator+(const az_el& a, const az_el& b) {
    auto copy = a;
    return copy += b;
}
az_el operator-(const az_el& a, const az_el& b) {
    auto copy = a;
    return copy -= b;
}
az_el operator*(const az_el& a, const az_el& b) {
    auto copy = a;
    return copy *= b;
}
az_el operator/(const az_el& a, const az_el& b) {
    auto copy = a;
    return copy /= b;
}

////////////////////////////////////////////////////////////////////////////////

float compute_azimuth(const glm::vec3& pointing) {
    return std::atan2(pointing.x, pointing.z);
}

float compute_elevation(const glm::vec3& pointing) {
    return std::asin(pointing.y);
}

az_el compute_azimuth_elevation(const glm::vec3& pointing) {
    az_el ret{compute_azimuth(pointing), compute_elevation(pointing)};
    if (almost_equal(ret.elevation, static_cast<float>(-M_PI / 2), 10) ||
        almost_equal(ret.elevation, static_cast<float>(M_PI / 2), 10)) {
        ret.azimuth = 0;
    }
    return ret;
}

glm::vec3 compute_pointing(const az_el& azel) {
    return glm::vec3(std::sin(azel.azimuth) * std::cos(azel.elevation),
                     std::sin(azel.elevation),
                     std::cos(azel.azimuth) * std::cos(azel.elevation));
}

}  // namespace core
}  // namespace wayverb
