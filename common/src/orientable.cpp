#include "common/orientable.h"

#include "glm/gtx/transform.hpp"

#include <functional>

namespace {

template <typename Op>
AzEl& inplace_zip(AzEl& a, const AzEl& b, Op op) {
    a.azimuth   = op(a.azimuth, b.azimuth);
    a.elevation = op(a.elevation, b.elevation);
    return a;
}

template <typename Op>
AzEl zip(const AzEl& a, const AzEl& b, Op op) {
    auto ret = a;
    return inplace_zip(ret, b, op);
}

}  // namespace

AzEl& operator+=(AzEl& a, const AzEl& b) {
    return inplace_zip(a, b, std::plus<>());
}
AzEl& operator-=(AzEl& a, const AzEl& b) {
    return inplace_zip(a, b, std::minus<>());
}
AzEl& operator*=(AzEl& a, const AzEl& b) {
    return inplace_zip(a, b, std::multiplies<>());
}
AzEl& operator/=(AzEl& a, const AzEl& b) {
    return inplace_zip(a, b, std::divides<>());
}

AzEl& operator+=(AzEl& a, float b) { return a += AzEl{b, b}; }
AzEl& operator-=(AzEl& a, float b) { return a -= AzEl{b, b}; }
AzEl& operator*=(AzEl& a, float b) { return a *= AzEl{b, b}; }
AzEl& operator/=(AzEl& a, float b) { return a /= AzEl{b, b}; }

AzEl operator+(const AzEl& a, const AzEl& b) {
    return zip(a, b, std::plus<>());
}
AzEl operator-(const AzEl& a, const AzEl& b) {
    return zip(a, b, std::minus<>());
}
AzEl operator*(const AzEl& a, const AzEl& b) {
    return zip(a, b, std::multiplies<>());
}
AzEl operator/(const AzEl& a, const AzEl& b) {
    return zip(a, b, std::divides<>());
}

AzEl operator+(const AzEl& a, float b) { return a + AzEl{b, b}; }
AzEl operator-(const AzEl& a, float b) { return a - AzEl{b, b}; }
AzEl operator*(const AzEl& a, float b) { return a * AzEl{b, b}; }
AzEl operator/(const AzEl& a, float b) { return a / AzEl{b, b}; }

AzEl operator+(float a, const AzEl& b) { return AzEl{a, a} + b; }
AzEl operator-(float a, const AzEl& b) { return AzEl{a, a} - b; }
AzEl operator*(float a, const AzEl& b) { return AzEl{a, a} * b; }
AzEl operator/(float a, const AzEl& b) { return AzEl{a, a} / b; }

//----------------------------------------------------------------------------//

float compute_azimuth(const glm::vec3& pointing) {
    return std::atan2(pointing.x, pointing.z);
}
float compute_elevation(const glm::vec3& pointing) {
    return glm::asin(pointing.y);
}
AzEl compute_azimuth_elevation(const glm::vec3& pointing) {
    return AzEl{compute_azimuth(pointing), compute_elevation(pointing)};
}
glm::vec3 compute_pointing(const AzEl& azel) {
    return glm::vec3(glm::sin(azel.azimuth) * glm::cos(azel.elevation),
                     glm::sin(azel.elevation),
                     glm::cos(azel.azimuth) * glm::cos(azel.elevation));
}

//----------------------------------------------------------------------------//

glm::vec3 Orientable::get_pointing() const { return pointing; }

void Orientable::set_pointing(const glm::vec3& u) {
    pointing = glm::normalize(u);
}

float Orientable::get_azimuth() const { return compute_azimuth(pointing); }
float Orientable::get_elevation() const { return compute_elevation(pointing); }
AzEl Orientable::get_azimuth_elevation() const {
    return compute_azimuth_elevation(pointing);
}

void Orientable::set_azimuth(float u) {
    set_azimuth_elevation(AzEl{u, get_elevation()});
}
void Orientable::set_elevation(float u) {
    set_azimuth_elevation(AzEl{get_azimuth(), u});
}
void Orientable::set_azimuth_elevation(const AzEl& azel) {
    pointing = compute_pointing(azel);
}

glm::mat4 Orientable::get_matrix() const {
    auto z_axis = pointing;
    auto x_axis = glm::normalize(glm::cross(pointing, glm::vec3(0, -1, 0)));
    auto y_axis = glm::normalize(glm::cross(z_axis, x_axis));
    return glm::mat4(glm::vec4(x_axis, 0),
                     glm::vec4(y_axis, 0),
                     glm::vec4(z_axis, 0),
                     glm::vec4(0, 0, 0, 1));
}