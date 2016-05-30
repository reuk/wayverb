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
