#pragma once

#include "waveguide/waveguide.h"

namespace waveguide {

struct descriptor;

namespace postprocessor {

namespace detail {

class microphone_state final {
public:
    microphone_state(const descriptor& mesh_descriptor,
                     size_t output_node,
                     double sample_rate,
                     double ambient_density);

    run_step_output operator()(cl::CommandQueue& queue,
                               const cl::Buffer& buffer,
                               size_t step);

    size_t get_output_node() const;

private:
    size_t output_node;
    std::array<cl_uint, 6> surrounding_nodes;
    float mesh_spacing;
    double sample_rate;
    double ambient_density;
    glm::dvec3 velocity{0};
};

}  // namespace detail

class microphone final {
public:
    //  node pressure and intensity are passed to the callback
    using output_callback = std::function<void(run_step_output)>;

    microphone(const descriptor& mesh_descriptor,
               size_t output_node,
               double sample_rate,
               double ambient_density,
               const output_callback& callback);

    void operator()(cl::CommandQueue& queue,
                    const cl::Buffer& buffer,
                    size_t step);

private:
    detail::microphone_state microphone_state;
    output_callback callback;
};

class multi_microphone final {
    //  node pressure and intensity are passed to the callback
    using output_callback = std::function<void(
            const aligned::vector<std::tuple<run_step_output, size_t>>&)>;

    multi_microphone(const descriptor& mesh,
                     const aligned::vector<size_t>& output_node,
                     double sample_rate,
                     double ambient_density,
                     const output_callback& callback);

    void operator()(cl::CommandQueue& queue,
                    const cl::Buffer& buffer,
                    size_t step);

private:
    aligned::vector<detail::microphone_state> state;
    output_callback callback;
};

}  // namespace postprocessor
}  // namespace waveguide
