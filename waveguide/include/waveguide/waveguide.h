#pragma once

#include "config.h"
#include "program.h"

#include "glm/glm.hpp"

namespace waveguide {

namespace mesh {
class model;
}  // namespace mesh

/// arguments
///     the queue to use
///     the current mesh state buffer
///     the step number
using step_preprocessor =
        std::function<void(cl::CommandQueue&, cl::Buffer&, size_t)>;

/// arguments
///     the queue to use
///     the current mesh state buffer
//      the step number
using step_postprocessor =
        std::function<void(cl::CommandQueue&, const cl::Buffer&, size_t)>;

using per_step_callback = std::function<void(size_t)>;

/// Will set up and run a waveguide using an existing 'template' (the
/// mesh::model).
/// Could be part of a class which stores the template and cl buffers to make
/// multiple runs faster.
size_t run(const cl::Context& context,
           const cl::Device& device,
           const mesh::model& model,
           size_t ideal_steps,
           const step_preprocessor& preprocessor,
           const aligned::vector<step_postprocessor>& postprocessors,
           const per_step_callback& callback,
           std::atomic_bool& keep_going);

struct run_step_output final {
    glm::vec3 intensity;
    float pressure;
};

/// Simplified run function to just hammer through an entire simulation and
/// return the output. Nothing fancy.
aligned::vector<run_step_output> run(const cl::Context&,
                                     const cl::Device&,
                                     const mesh::model& model,
                                     size_t input_node,
                                     const aligned::vector<float>& input,
                                     size_t output_node,
                                     double speed_of_sound,
                                     double ambient_density,
                                     const per_step_callback& callback);

}  // namespace waveguide
