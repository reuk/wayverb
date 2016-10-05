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

az_el& operator+=(az_el& a, float b);
az_el& operator-=(az_el& a, float b);
az_el& operator*=(az_el& a, float b);
az_el& operator/=(az_el& a, float b);

az_el operator+(const az_el& a, const az_el& b);
az_el operator-(const az_el& a, const az_el& b);
az_el operator*(const az_el& a, const az_el& b);
az_el operator/(const az_el& a, const az_el& b);

az_el operator+(const az_el& a, float b);
az_el operator-(const az_el& a, float b);
az_el operator*(const az_el& a, float b);
az_el operator/(const az_el& a, float b);

az_el operator+(float a, const az_el& b);
az_el operator-(float a, const az_el& b);
az_el operator*(float a, const az_el& b);
az_el operator/(float a, const az_el& b);

float compute_azimuth(const glm::vec3& pointing);
float compute_elevation(const glm::vec3& pointing);
az_el compute_azimuth_elevation(const glm::vec3& pointing);

glm::vec3 compute_pointing(const az_el& azel);

//----------------------------------------------------------------------------//

class orientable {
public:
    orientable() = default;
    orientable(const orientable&) = default;
    orientable& operator=(const orientable&) = default;
    orientable(orientable&&) noexcept = default;
    orientable& operator=(orientable&&) noexcept = default;

protected:
    ~orientable() noexcept = default;

public:
    glm::vec3 get_pointing() const;
    void set_pointing(const glm::vec3& u);

    float get_azimuth() const;
    float get_elevation() const;
    az_el get_azimuth_elevation() const;

    void set_azimuth(float u);
    void set_elevation(float u);
    void set_azimuth_elevation(const az_el& azel);

    glm::mat4 get_matrix() const;

private:
    glm::vec3 pointing{0, 0, 1};
};
