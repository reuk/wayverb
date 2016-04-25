#include "waveguide/power_function.h"

#include "common/vec.h"

float BasicPowerFunction::operator()(const Vec3f& a, const Vec3f& b) const {
    return (a == b).all() ? 1 : 0;
}

GaussianFunction::GaussianFunction(float standard_deviation)
        : standard_deviation(standard_deviation) {
}

float GaussianFunction::gaussian(const Vec3f& x, float sdev) {
    return 1 / pow(sdev * sqrt(2 * M_PI), 3) *
           pow(M_E, -x.mag_squared() / (2 * pow(sdev, 2)));
}

float GaussianFunction::operator()(const Vec3f& a, const Vec3f& ex) const {
    return gaussian(ex - a, standard_deviation);
}
