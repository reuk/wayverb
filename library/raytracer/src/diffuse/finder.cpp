#include "raytracer/diffuse/finder.h"

namespace raytracer {
namespace diffuse {

finder::finder(const compute_context& cc,
               const model::parameters& params,
               float receiver_radius,
               size_t rays)
        : cc_{cc}
        , queue_{cc.context, cc.device}
        , kernel_{program{cc}.get_kernel()}
        , receiver_{to_cl_float3(params.receiver)}
        , receiver_radius_{receiver_radius}
        , rays_{rays}
        , reflections_buffer_{cc.context,
                              CL_MEM_READ_WRITE,
                              sizeof(reflection) * rays}
        , diffuse_path_buffer_{cc.context,
                               CL_MEM_READ_WRITE,
                               sizeof(diffuse_path_info) * rays}
        , diffuse_output_buffer_{cc.context,
                                 CL_MEM_READ_WRITE,
                                 sizeof(impulse<8>) * rays}
        , specular_output_buffer_{
                  cc.context, CL_MEM_READ_WRITE, sizeof(impulse<8>) * rays} {
    //  see schroder2011 5.54
    const auto dist{glm::distance(params.source, params.receiver)};
    const auto sin_y{receiver_radius / std::max(receiver_radius, dist)};
    const auto cos_y{std::sqrt(1 - sin_y * sin_y)};

    //  The extra factor of 4pi here is because
    //  image-source intensity = 1 / 4pir^2 instead of just 1 / r^2
    const auto starting_intensity{2.0 / (4 * M_PI * rays * dist * dist * (1 - cos_y))};

    program{cc_}.get_init_diffuse_path_info_kernel()(
            cl::EnqueueArgs{queue_, cl::NDRange{rays_}},
            diffuse_path_buffer_,
            make_volume_type(starting_intensity),
            to_cl_float3(params.source));
}

}  // namespace diffuse
}  // namespace raytracer
