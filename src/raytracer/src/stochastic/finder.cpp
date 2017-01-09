#include "raytracer/stochastic/finder.h"

namespace wayverb {
namespace raytracer {
namespace stochastic {

float compute_ray_energy(size_t total_rays,
                         const glm::vec3& source,
                         const glm::vec3& receiver,
                         float receiver_radius) {
    const auto dist = glm::distance(source, receiver);
    const auto sin_y = receiver_radius / std::max(receiver_radius, dist);
    const auto cos_y = std::sqrt(1 - sin_y * sin_y);
    return compute_ray_energy(total_rays, dist, cos_y);
}

////////////////////////////////////////////////////////////////////////////////

finder::finder(const core::compute_context& cc,
               size_t group_size,
               const glm::vec3& source,
               const glm::vec3& receiver,
               float receiver_radius,
               float starting_energy)
        : cc_{cc}
        , queue_{cc.context, cc.device}
        , kernel_{program{cc}.get_kernel()}
        , receiver_{core::to_cl_float3{}(receiver)}
        , receiver_radius_{receiver_radius}
        , rays_{group_size}
        , reflections_buffer_{cc.context,
                              CL_MEM_READ_WRITE,
                              sizeof(reflection) * group_size}
        , stochastic_path_buffer_{cc.context,
                                  CL_MEM_READ_WRITE,
                                  sizeof(stochastic_path_info) * group_size}
        , stochastic_output_buffer_{cc.context,
                                    CL_MEM_READ_WRITE,
                                    sizeof(impulse<core::simulation_bands>) *
                                            group_size}
        , specular_output_buffer_{
                  cc.context,
                  CL_MEM_READ_WRITE,
                  sizeof(impulse<core::simulation_bands>) * group_size} {
    program{cc_}.get_init_stochastic_path_info_kernel()(
            cl::EnqueueArgs{queue_, cl::NDRange{rays_}},
            stochastic_path_buffer_,
            core::make_bands_type(starting_energy),
            core::to_cl_float3{}(source));
}

}  // namespace stochastic
}  // namespace raytracer
}  // namespace wayverb
