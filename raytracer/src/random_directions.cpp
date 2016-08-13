#include "raytracer/random_directions.h"

namespace raytracer {

aligned::vector<cl_float3> get_random_directions(size_t num) {
    aligned::vector<cl_float3> ret(num);
    std::default_random_engine engine{std::random_device()()};

    for (auto& i : ret) {
        const direction_rng rng(engine);
        i = to_cl_float3(sphere_point(rng.get_z(), rng.get_theta()));
    }

    return ret;
}

}  // namespace raytracer
