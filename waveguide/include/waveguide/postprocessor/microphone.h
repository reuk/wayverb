#pragma once

#include "waveguide/waveguide.h"

namespace waveguide {
namespace postprocessor {

class microphone final {
public:
    //  node pressure and intensity are passed to the callback
    using output_callback = std::function<void(run_step_output)>;

    microphone(const mesh& mesh,
               size_t output_node,
               double sample_rate,
               const output_callback& callback);

    void operator()(cl::CommandQueue& queue, const cl::Buffer& buffer);

private:
    size_t output_node;
    std::array<cl_uint, 6> surrounding_nodes;
    float mesh_spacing;
    double sample_rate;
    glm::dvec3 velocity{0};
    output_callback callback;
};

}  // namespace postprocessor
}  // namespace waveguide
