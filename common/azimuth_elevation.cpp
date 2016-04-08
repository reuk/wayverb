#include "azimuth_elevation.h"

#include "vec.h"

#include <cmath>

Vec3f sphere(double cosz, double sinz, double theta) {
    return Vec3f(cosz * cos(theta), sinz, cosz * sin(theta));
}

Vec3f point_on_sphere(double az, double el) {
    return sphere(cos(el), sin(el), az);
}

Vec3f sphere_point(double z, double theta) {
    return sphere(std::sqrt(1 - z * z), z, theta);
}
