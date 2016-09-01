#pragma once

#include "waveguide/waveguide.h"

#include "common/output_iterator_callback.h"

struct mesh_descriptor;

namespace waveguide {
namespace postprocessor {

namespace detail {

class microphone_state final {
public:
    microphone_state(const mesh_descriptor& mesh_descriptor,
                     double sample_rate,
                     double ambient_density,
                     size_t output_node);

    run_step_output operator()(cl::CommandQueue& queue,
                               const cl::Buffer& buffer,
                               size_t step);

    size_t get_output_node() const;

private:
    float mesh_spacing_;
    double sample_rate_;
    double ambient_density_;
    size_t output_node_;
    std::array<cl_uint, 6> surrounding_nodes_;
    glm::dvec3 velocity_{0};
};

}  // namespace detail

//----------------------------------------------------------------------------//

class microphone final {
public:
    using output_callback = std::function<void(run_step_output)>;

    microphone(const mesh_descriptor& mesh_descriptor,
               double sample_rate,
               double ambient_density,
               size_t output_node,
               output_callback callback);

    void operator()(cl::CommandQueue& queue,
                    const cl::Buffer& buffer,
                    size_t step);

private:
    detail::microphone_state microphone_state_;
    output_callback callback_;
};

}  // namespace postprocessor
}  // namespace waveguide
