#pragma once

#include "common/azimuth_elevation.h"
#include "common/conversions.h"

#include <random>

class direction_rng {
public:
    template <typename T>
    direction_rng(T& engine)
            : z(std::uniform_real_distribution<float>(-1, 1)(engine))
            , theta(std::uniform_real_distribution<float>(-M_PI,
                                                          M_PI)(engine)) {}

    float get_z() const { return z; }
    float get_theta() const { return theta; }

private:
    float z;      //  -1    to 1
    float theta;  //  -M_PI to M_PI
};

aligned::vector<cl_float3> get_random_directions(size_t num) {
    aligned::vector<cl_float3> ret(num);
    std::default_random_engine engine{std::random_device()()};

    for (auto& i : ret) {
        const direction_rng rng(engine);
        i = to_cl_float3(sphere_point(rng.get_z(), rng.get_theta()));
    }

    return ret;
}
