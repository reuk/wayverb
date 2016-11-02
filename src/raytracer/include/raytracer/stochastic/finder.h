#pragma once

#include "program.h"

#include "raytracer/cl/structs.h"

#include "core/cl/common.h"
#include "core/conversions.h"
#include "core/model/parameters.h"
#include "core/pressure_intensity.h"
#include "core/spatial_division/scene_buffers.h"

#include "utilities/aligned/vector.h"

namespace wayverb {
namespace raytracer {
namespace stochastic {

class finder final {
public:
    finder(const core::compute_context& cc,
           const core::model::parameters& params,
           float receiver_radius,
           size_t rays);

    struct results final {
        util::aligned::vector<impulse<core::simulation_bands>> specular;
        util::aligned::vector<impulse<core::simulation_bands>> stochastic;
    };

    template <typename It>
    auto process(It b, It e, const core::scene_buffers& scene_buffers) {
        //  copy the current batch of reflections to the device
        cl::copy(queue_, b, e, reflections_buffer_);

        //  get the kernel and run it
        kernel_(cl::EnqueueArgs(queue_, cl::NDRange(rays_)),
                reflections_buffer_,
                receiver_,
                receiver_radius_,
                scene_buffers.get_triangles_buffer(),
                scene_buffers.get_vertices_buffer(),
                scene_buffers.get_surfaces_buffer(),
                stochastic_path_buffer_,
                stochastic_output_buffer_,
                specular_output_buffer_);

        const auto read_out_impulses = [&](const auto& buffer) {
            auto raw = core::read_from_buffer<impulse<core::simulation_bands>>(
                    queue_, buffer);
            raw.erase(std::remove_if(begin(raw),
                                     end(raw),
                                     [](const auto& impulse) {
                                         return !impulse.distance;
                                     }),
                      end(raw));
            return raw;
        };

        return results{read_out_impulses(specular_output_buffer_),
                       read_out_impulses(stochastic_output_buffer_)};
    }

private:
    using kernel_t = decltype(std::declval<program>().get_kernel());

    core::compute_context cc_;
    cl::CommandQueue queue_;
    kernel_t kernel_;
    cl_float3 receiver_;
    cl_float receiver_radius_;
    size_t rays_;

    cl::Buffer reflections_buffer_;
    cl::Buffer stochastic_path_buffer_;
    cl::Buffer stochastic_output_buffer_;
    cl::Buffer specular_output_buffer_;
};

}  // namespace stochastic
}  // namespace raytracer
}  // namespace wayverb
