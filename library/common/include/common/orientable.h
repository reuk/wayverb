#pragma once

#include "glm/glm.hpp"

struct az_el final {
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

//----------------------------------------------------------------------------//

/// Invariant: pointing_ is a unit vector.
class orientable final {
    orientable() = default;
    orientable(const glm::vec3& pointing);

    glm::vec3 get_pointing() const;
    void set_pointing(const glm::vec3& u);

    glm::mat4 get_matrix() const;

private:
    glm::vec3 pointing_{0, 0, 1};
};
