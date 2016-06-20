#include "Orientable.hpp"

#include "glm/gtx/transform.hpp"

Orientable::AzEl Orientable::AzEl::operator+(const AzEl& rhs) const {
    return AzEl{azimuth + rhs.azimuth, elevation + rhs.elevation};
}

Orientable::AzEl Orientable::AzEl::operator-(const AzEl& rhs) const {
    return AzEl{azimuth - rhs.azimuth, elevation - rhs.elevation};
}

Orientable::AzEl Orientable::AzEl::operator*(float rhs) const {
    return AzEl{azimuth * rhs, elevation * rhs};
}

Orientable::AzEl Orientable::AzEl::operator/(float rhs) const {
    return AzEl{azimuth / rhs, elevation / rhs};
}

Orientable::AzEl& Orientable::AzEl::operator+=(const AzEl& rhs) {
    return *this = *this + rhs;
}

Orientable::AzEl& Orientable::AzEl::operator-=(const AzEl& rhs) {
    return *this = *this - rhs;
}

Orientable::AzEl& Orientable::AzEl::operator*=(float rhs) {
    return *this = *this * rhs;
}

Orientable::AzEl& Orientable::AzEl::operator/=(float rhs) {
    return *this = *this / rhs;
}

//----------------------------------------------------------------------------//

Orientable::Orientable(Orientable&& rhs) noexcept
        : pointing(rhs.pointing) {
}

Orientable& Orientable::operator=(Orientable&& rhs) noexcept {
    pointing = rhs.pointing;
    return *this;
}

float Orientable::compute_azimuth(const glm::vec3& pointing) {
    return std::atan2(pointing.x, pointing.z);
}
float Orientable::compute_elevation(const glm::vec3& pointing) {
    return glm::asin(pointing.y);
}
Orientable::AzEl Orientable::compute_azimuth_elevation(
        const glm::vec3& pointing) {
    return AzEl{compute_azimuth(pointing), compute_elevation(pointing)};
}
glm::vec3 Orientable::compute_pointing(const AzEl& azel) {
    return glm::vec3(glm::sin(azel.azimuth) * glm::cos(azel.elevation),
                     glm::sin(azel.elevation),
                     glm::cos(azel.azimuth) * glm::cos(azel.elevation));
}

glm::vec3 Orientable::get_pointing() const {
    return pointing;
}

void Orientable::set_pointing(const glm::vec3& u) {
    pointing = glm::normalize(u);
}

float Orientable::get_azimuth() const {
    return compute_azimuth(pointing);
}
float Orientable::get_elevation() const {
    return compute_elevation(pointing);
}
Orientable::AzEl Orientable::get_azimuth_elevation() const {
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