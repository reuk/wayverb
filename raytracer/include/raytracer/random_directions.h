#pragma once

#include "common/aligned/vector.h"
#include "common/azimuth_elevation.h"
#include "common/conversions.h"

#include <random>

namespace raytracer {

class direction_rng {
public:
    template <typename T>
    direction_rng(T& engine)
            : z(std::uniform_real_distribution<float>(-1, 1)(engine))
            , theta(std::uniform_real_distribution<float>(-M_PI,
                                                          M_PI)(engine)) {}

    float get_z() const;
    float get_theta() const;

private:
    float z;      //  -1    to 1
    float theta;  //  -M_PI to M_PI
};

aligned::vector<cl_float3> get_random_directions(size_t num);

}  // namespace raytracer
