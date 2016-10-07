#pragma once

#include "raytracer/program.h"

#include "common/cl/geometry.h"
#include "common/cl/include.h"
#include "common/geo/geometric.h"
#include "common/spatial_division/scene_buffers.h"

#include "utilities/aligned/vector.h"
#include "utilities/map_to_vector.h"

#include "glm/glm.hpp"

namespace raytracer {

template <typename It>  /// Iterator over directions
aligned::vector<geo::ray> get_rays_from_directions(It begin,
                                                   It end,
                                                   const glm::vec3& source) {
    return map_to_vector(begin, end, [&](const auto& i) {
        return geo::ray{source, i};
    });
}

aligned::vector<geo::ray> get_random_rays(const glm::vec3& source, size_t num);

class reflector final {
public:
    reflector(const compute_context& cc,
              const glm::vec3& receiver,
              const aligned::vector<geo::ray>& rays);

    aligned::vector<reflection> run_step(const scene_buffers& buffers);

    aligned::vector<ray> get_rays() const;
    aligned::vector<reflection> get_reflections() const;
    aligned::vector<cl_float> get_rng() const;

private:
    using kernel_t = decltype(std::declval<program>().get_kernel());

    compute_context cc;
    cl::CommandQueue queue;
    kernel_t kernel;
    cl_float3 receiver;
    size_t rays;

    cl::Buffer ray_buffer;
    cl::Buffer reflection_buffer;

    cl::Buffer rng_buffer;
};

}  // namespace raytracer
