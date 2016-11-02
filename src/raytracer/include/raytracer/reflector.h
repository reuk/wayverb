#pragma once

#include "raytracer/program.h"

#include "core/cl/geometry.h"
#include "core/cl/include.h"
#include "core/geo/geometric.h"
#include "core/spatial_division/scene_buffers.h"

#include "utilities/aligned/vector.h"
#include "utilities/map_to_vector.h"

#include "glm/glm.hpp"

namespace wayverb {
namespace raytracer {

template <typename It>  /// Iterator over directions
auto get_rays_from_directions(It begin, It end, const glm::vec3& source) {
    return util::map_to_vector(begin, end, [&](const auto& i) {
        return core::geo::ray{source, i};
    });
}

class reflector final {
public:
    template <typename It>
    reflector(const core::compute_context& cc,
              const glm::vec3& receiver,
              It b,
              It e)
            : cc_{cc}
            , queue_{cc.context, cc.device}
            , kernel_{program{cc}.get_kernel()}
            , receiver_{core::to_cl_float3(receiver)}
            , rays_(std::distance(b, e))
            , ray_buffer_{core::load_to_buffer(
                      cc.context,
                      util::map_to_vector(
                              b,
                              e,
                              [](const auto& i) { return core::convert(i); }),
                      false)}
            , reflection_buffer_{cc.context,
                                 CL_MEM_READ_WRITE,
                                 rays_ * sizeof(reflection)}
            , rng_buffer_{cc.context,
                          CL_MEM_READ_WRITE,
                          rays_ * 2 * sizeof(cl_float)} {
        program{cc_}.get_init_reflections_kernel()(
                cl::EnqueueArgs{queue_, cl::NDRange{rays_}},
                reflection_buffer_);
    }

    util::aligned::vector<reflection> run_step(
            const core::scene_buffers& buffers);

    util::aligned::vector<core::ray> get_rays() const;
    util::aligned::vector<reflection> get_reflections() const;
    util::aligned::vector<cl_float> get_rng() const;

private:
    using kernel_t = decltype(std::declval<program>().get_kernel());

    core::compute_context cc_;
    cl::CommandQueue queue_;
    kernel_t kernel_;
    cl_float3 receiver_;
    size_t rays_;

    cl::Buffer ray_buffer_;
    cl::Buffer reflection_buffer_;

    cl::Buffer rng_buffer_;
};

}  // namespace raytracer
}  // namespace wayverb
