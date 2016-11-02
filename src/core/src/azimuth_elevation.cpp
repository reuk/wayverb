#include "core/azimuth_elevation.h"

#include <cmath>

namespace wayverb {
namespace core {

glm::vec3 sphere(double cosz, double sinz, double theta) {
    return glm::vec3(cosz * cos(theta), sinz, cosz * sin(theta));
}

glm::vec3 point_on_sphere(double az, double el) {
    return sphere(cos(el), sin(el), az);
}

glm::vec3 sphere_point(double z, double theta) {
    return sphere(std::sqrt(1 - z * z), z, theta);
}

auto sphere_point(const direction_rng& rng) {
    return sphere_point(rng.get_z(), rng.get_theta());
}

util::aligned::vector<glm::vec3> get_random_directions(size_t num) {
    util::aligned::vector<glm::vec3> ret(num);
    std::default_random_engine engine{std::random_device()()};

    std::generate(begin(ret), end(ret), [&] {
        return sphere_point(direction_rng{engine});
    });

    return ret;
}

}  // namespace core
}  // namespace wayverb
