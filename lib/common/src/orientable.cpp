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

template <typename Op>
az_el zip(const az_el& a, const az_el& b, Op op) {
    auto ret = a;
    return inplace_zip(ret, b, op);
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

az_el& operator+=(az_el& a, float b) { return a += az_el{b, b}; }
az_el& operator-=(az_el& a, float b) { return a -= az_el{b, b}; }
az_el& operator*=(az_el& a, float b) { return a *= az_el{b, b}; }
az_el& operator/=(az_el& a, float b) { return a /= az_el{b, b}; }

az_el operator+(const az_el& a, const az_el& b) {
    return zip(a, b, std::plus<>());
}
az_el operator-(const az_el& a, const az_el& b) {
    return zip(a, b, std::minus<>());
}
az_el operator*(const az_el& a, const az_el& b) {
    return zip(a, b, std::multiplies<>());
}
az_el operator/(const az_el& a, const az_el& b) {
    return zip(a, b, std::divides<>());
}

az_el operator+(const az_el& a, float b) { return a + az_el{b, b}; }
az_el operator-(const az_el& a, float b) { return a - az_el{b, b}; }
az_el operator*(const az_el& a, float b) { return a * az_el{b, b}; }
az_el operator/(const az_el& a, float b) { return a / az_el{b, b}; }

az_el operator+(float a, const az_el& b) { return az_el{a, a} + b; }
az_el operator-(float a, const az_el& b) { return az_el{a, a} - b; }
az_el operator*(float a, const az_el& b) { return az_el{a, a} * b; }
az_el operator/(float a, const az_el& b) { return az_el{a, a} / b; }

//----------------------------------------------------------------------------//

float compute_azimuth(const glm::vec3& pointing) {
    return std::atan2(pointing.x, pointing.z);
}
float compute_elevation(const glm::vec3& pointing) {
    return glm::asin(pointing.y);
}
az_el compute_azimuth_elevation(const glm::vec3& pointing) {
    return az_el{compute_azimuth(pointing), compute_elevation(pointing)};
}
glm::vec3 compute_pointing(const az_el& azel) {
    return glm::vec3(glm::sin(azel.azimuth) * glm::cos(azel.elevation),
                     glm::sin(azel.elevation),
                     glm::cos(azel.azimuth) * glm::cos(azel.elevation));
}

//----------------------------------------------------------------------------//

glm::vec3 orientable::get_pointing() const { return pointing; }

void orientable::set_pointing(const glm::vec3& u) {
    pointing = glm::normalize(u);
}

float orientable::get_azimuth() const { return compute_azimuth(pointing); }
float orientable::get_elevation() const { return compute_elevation(pointing); }
az_el orientable::get_azimuth_elevation() const {
    return compute_azimuth_elevation(pointing);
}

void orientable::set_azimuth(float u) {
    set_azimuth_elevation(az_el{u, get_elevation()});
}
void orientable::set_elevation(float u) {
    set_azimuth_elevation(az_el{get_azimuth(), u});
}
void orientable::set_azimuth_elevation(const az_el& azel) {
    pointing = compute_pointing(azel);
}

glm::mat4 orientable::get_matrix() const {
    auto z_axis = pointing;
    auto x_axis = glm::normalize(glm::cross(pointing, glm::vec3(0, -1, 0)));
    auto y_axis = glm::normalize(glm::cross(z_axis, x_axis));
    return glm::mat4(glm::vec4(x_axis, 0),
                     glm::vec4(y_axis, 0),
                     glm::vec4(z_axis, 0),
                     glm::vec4(0, 0, 0, 1));
}
