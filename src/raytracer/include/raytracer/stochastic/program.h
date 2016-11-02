#pragma once

#include "raytracer/cl/structs.h"

#include "core/program_wrapper.h"
#include "core/spatial_division/scene_buffers.h"

namespace wayverb {
namespace raytracer {
namespace stochastic {

class program final {
public:
    program(const core::compute_context& cc);

    auto get_kernel() const {
        return program_wrapper_.get_kernel<cl::Buffer,  // reflections
                                           cl_float3,   // receiver
                                           cl_float,    // receiver radius
                                           cl::Buffer,  // triangles
                                           cl::Buffer,  // vertices
                                           cl::Buffer,  // surfaces
                                           cl::Buffer,  // stochastic path info
                                           cl::Buffer,  // stochastic output
                                           cl::Buffer   // intersected output
                                           >("stochastic");
    }

    auto get_init_stochastic_path_info_kernel() const {
        return program_wrapper_.get_kernel<cl::Buffer,        // buffer
                                           core::bands_type,  // initial energy
                                           cl_float3  // initial position
                                           >("init_stochastic_path_info");
    }

private:
    core::program_wrapper program_wrapper_;
};

}  // namespace stochastic
}  // namespace raytracer
}  // namespace wayverb
