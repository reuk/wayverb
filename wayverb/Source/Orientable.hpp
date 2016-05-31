#pragma once

#include "glm/glm.hpp"

class Orientable {
public:
    Orientable() = default;
    Orientable(const Orientable&) = default;
    Orientable& operator=(const Orientable&) = default;
    Orientable(Orientable&&) noexcept = default;
    Orientable& operator=(Orientable&&) noexcept = default;
    virtual ~Orientable() noexcept = default;

    //  relative to global axes I guess
    struct AzEl {
        float azimuth{0};
        float elevation{0};
    };

    static float compute_azimuth(const glm::vec3& pointing);
    static float compute_elevation(const glm::vec3& pointing);
    static AzEl compute_azimuth_elevation(const glm::vec3& pointing);

    static glm::vec3 compute_pointing(const AzEl& azel);

    glm::vec3 get_pointing() const;
    void set_pointing(const glm::vec3& u);

    float get_azimuth() const;
    float get_elevation() const;
    AzEl get_azimuth_elevation() const;

    void set_azimuth(float u);
    void set_elevation(float u);
    void set_azimuth_elevation(const AzEl& azel);

    virtual glm::mat4 get_matrix() const;

private:
    glm::vec3 pointing{0, 0, 1};
};