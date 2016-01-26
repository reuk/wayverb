#include "power_function.h"

float BasicPowerFunction::operator()(const Vec3f& a, const Vec3f& b) const {
    return (a == b).all() ? 1 : 0;
}

InversePowerFunction::InversePowerFunction(float power)
        : power(power) {
}

float InversePowerFunction::operator()(const Vec3f& a, const Vec3f& b) const {
    return power / (a - b).mag();
}

InverseSquarePowerFunction::InverseSquarePowerFunction(float power)
        : power(power) {
}

float InverseSquarePowerFunction::operator()(const Vec3f& a,
                                             const Vec3f& b) const {
    return power / (a - b).mag_squared();
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
