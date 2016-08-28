#pragma once

#include "waveguide/waveguide.h"

namespace waveguide {

struct descriptor;

namespace postprocessor {

namespace detail {

class microphone_state final {
public:
    microphone_state(const descriptor& mesh_descriptor,
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

template <typename It>
class microphone final {
public:
    microphone(const descriptor& mesh_descriptor,
               double sample_rate,
               double ambient_density,
               size_t output_node,
               It output_iterator)
            : microphone_state_(mesh_descriptor,
                                sample_rate,
                                ambient_density,
                                output_node)
            , output_iterator_(std::move(output_iterator)) {
        if (ambient_density < 0.5 || 10 < ambient_density) {
            throw std::runtime_error{
                    "ambient density value looks a bit suspicious"};
        }
    }

    void operator()(cl::CommandQueue& queue,
                    const cl::Buffer& buffer,
                    size_t step) {
        *output_iterator_++ = microphone_state_(queue, buffer, step);
    }

private:
    detail::microphone_state microphone_state_;
    It output_iterator_;
};

template <typename It>
auto make_microphone(const descriptor& mesh_descriptor,
                     double sample_rate,
                     double ambient_density,
                     size_t output_node,
                     It output_iterator) {
    return microphone<It>{mesh_descriptor,
                          sample_rate,
                          ambient_density,
                          output_node,
                          output_iterator};
}

}  // namespace postprocessor
}  // namespace waveguide
