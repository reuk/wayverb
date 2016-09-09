#include "common/azimuth_elevation.h"

#include <cmath>

glm::vec3 sphere(double cosz, double sinz, double theta) {
    return glm::vec3(cosz * cos(theta), sinz, cosz * sin(theta));
}

glm::vec3 point_on_sphere(double az, double el) {
    return sphere(cos(el), sin(el), az);
}

glm::vec3 sphere_point(double z, double theta) {
    return sphere(std::sqrt(1 - z * z), z, theta);
}

aligned::vector<glm::vec3> get_random_directions(size_t num) {
    aligned::vector<glm::vec3> ret;
    ret.reserve(num);
    std::default_random_engine engine{std::random_device()()};

    for (auto i = 0u; i != num; ++i) {
        const direction_rng rng(engine);
        ret.emplace_back(sphere_point(rng.get_z(), rng.get_theta()));
    }
    return ret;
}

