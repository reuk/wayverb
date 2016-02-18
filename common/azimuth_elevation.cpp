#include "azimuth_elevation.h"

#include <cmath>

Vec3f sphere(float cosz, float sinz, float theta) {
    return Vec3f(cosz * cos(theta), sinz, cosz * sin(theta));
}

Vec3f point_on_sphere(float az, float el) {
    return sphere(cos(el), sin(el), az);
}

Vec3f sphere_point(float z, float theta) {
    return sphere(std::sqrt(1 - z * z), z, theta);
}
