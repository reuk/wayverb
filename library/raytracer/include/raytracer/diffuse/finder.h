#pragma once

#include "program.h"

#include "raytracer/cl/structs.h"

#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/model/parameters.h"
#include "common/pressure_intensity.h"
#include "common/spatial_division/scene_buffers.h"

#include "utilities/aligned/vector.h"

namespace raytracer {
namespace diffuse {

class finder final {
public:
    finder(const compute_context& cc,
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
            , specular_output_buffer_{cc.context,
                                      CL_MEM_READ_WRITE,
                                      sizeof(impulse<8>) * rays} {
        //  see schroder2011 5.54
        const auto dist{glm::distance(params.source, params.receiver)};
        const auto sin_y{receiver_radius / std::max(receiver_radius, dist)};
        const auto cos_y{std::sqrt(1 - sin_y * sin_y)};
        const auto starting_intensity{2.0 / (rays * dist * dist * (1 - cos_y))};

        program{cc_}.get_init_diffuse_path_info_kernel()(
                cl::EnqueueArgs{queue_, cl::NDRange{rays_}},
                diffuse_path_buffer_,
                make_volume_type(starting_intensity),
                to_cl_float3(params.source));
    }

    struct results final {
        aligned::vector<impulse<8>> specular;
        aligned::vector<impulse<8>> diffuse;
    };

    /// Find diffuse impulses, given a range of `reflection` data.
    /// Calculates in terms of pressure.
    /// *Does* account for distance travelled.
    template <typename It>
    auto process(It b, It e, const scene_buffers& scene_buffers) {
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
                diffuse_path_buffer_,
                diffuse_output_buffer_,
                specular_output_buffer_);

        const auto read_out_impulses{[&](const auto& buffer) {
            auto raw{read_from_buffer<impulse<8>>(queue_, buffer)};
            raw.erase(std::remove_if(begin(raw),
                                     end(raw),
                                     [](const auto& impulse) {
                                         return !impulse.distance;
                                     }),
                      end(raw));
            return raw;
        }};

        return results{read_out_impulses(specular_output_buffer_),
                       read_out_impulses(diffuse_output_buffer_)};
    }

private:
    using kernel_t = decltype(std::declval<program>().get_kernel());

    compute_context cc_;
    cl::CommandQueue queue_;
    kernel_t kernel_;
    cl_float3 receiver_;
    cl_float receiver_radius_;
    size_t rays_;

    cl::Buffer reflections_buffer_;
    cl::Buffer diffuse_path_buffer_;
    cl::Buffer diffuse_output_buffer_;
    cl::Buffer specular_output_buffer_;
};

}  // namespace diffuse
}  // namespace raytracer
