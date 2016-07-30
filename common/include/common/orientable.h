#pragma once

#include "glm/glm.hpp"

struct AzEl final {
    float azimuth{0};
    float elevation{0};
};

AzEl& operator+=(AzEl& a, const AzEl& b);
AzEl& operator-=(AzEl& a, const AzEl& b);
AzEl& operator*=(AzEl& a, const AzEl& b);
AzEl& operator/=(AzEl& a, const AzEl& b);

AzEl& operator+=(AzEl& a, float b);
AzEl& operator-=(AzEl& a, float b);
AzEl& operator*=(AzEl& a, float b);
AzEl& operator/=(AzEl& a, float b);

AzEl operator+(const AzEl& a, const AzEl& b);
AzEl operator-(const AzEl& a, const AzEl& b);
AzEl operator*(const AzEl& a, const AzEl& b);
AzEl operator/(const AzEl& a, const AzEl& b);

AzEl operator+(const AzEl& a, float b);
AzEl operator-(const AzEl& a, float b);
AzEl operator*(const AzEl& a, float b);
AzEl operator/(const AzEl& a, float b);

AzEl operator+(float a, const AzEl& b);
AzEl operator-(float a, const AzEl& b);
AzEl operator*(float a, const AzEl& b);
AzEl operator/(float a, const AzEl& b);

float compute_azimuth(const glm::vec3& pointing);
float compute_elevation(const glm::vec3& pointing);
AzEl compute_azimuth_elevation(const glm::vec3& pointing);

glm::vec3 compute_pointing(const AzEl& azel);

//----------------------------------------------------------------------------//

class Orientable {
public:
    Orientable()                  = default;
    Orientable(const Orientable&) = default;
    Orientable& operator=(const Orientable&) = default;
    Orientable(Orientable&&) noexcept        = default;
    Orientable& operator=(Orientable&&) noexcept = default;

protected:
    ~Orientable() noexcept = default;

public:
    glm::vec3 get_pointing() const;
    void set_pointing(const glm::vec3& u);

    float get_azimuth() const;
    float get_elevation() const;
    AzEl get_azimuth_elevation() const;

    void set_azimuth(float u);
    void set_elevation(float u);
    void set_azimuth_elevation(const AzEl& azel);

    glm::mat4 get_matrix() const;

private:
    glm::vec3 pointing{0, 0, 1};
};