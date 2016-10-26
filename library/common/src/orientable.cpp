#include "common/orientable.h"

#include "glm/gtx/transform.hpp"

#include <functional>

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
    return az_el{compute_azimuth(pointing), compute_elevation(pointing)};
}
glm::vec3 compute_pointing(const az_el& azel) {
    return glm::vec3(std::sin(azel.azimuth) * std::cos(azel.elevation),
                     std::sin(azel.elevation),
                     std::cos(azel.azimuth) * std::cos(azel.elevation));
}

////////////////////////////////////////////////////////////////////////////////

orientable::orientable(const glm::vec3& pointing)
        : pointing_{glm::normalize(pointing)} {}

glm::vec3 orientable::get_pointing() const { return pointing_; }

void orientable::set_pointing(const glm::vec3& u) {
    pointing_ = glm::normalize(u);
}

glm::mat4 orientable::get_matrix() const {
    const auto z_axis = pointing_;
    const auto x_axis =
            glm::normalize(glm::cross(pointing_, glm::vec3(0, -1, 0)));
    const auto y_axis = glm::normalize(glm::cross(z_axis, x_axis));
    return glm::mat4(glm::vec4(x_axis, 0),
                     glm::vec4(y_axis, 0),
                     glm::vec4(z_axis, 0),
                     glm::vec4(0, 0, 0, 1));
}
