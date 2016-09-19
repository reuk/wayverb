#pragma once

#include "waveguide/config.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/microphone.h"
#include "waveguide/preprocessor/single_soft_source.h"
#include "waveguide/program.h"

#include "glm/glm.hpp"

namespace waveguide {

class mesh;

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

/// Will set up and run a waveguide using an existing 'template' (the mesh).
/// Could be part of a class which stores the template and cl buffers to make
/// multiple runs faster.
size_t run(const compute_context& cc,
           const mesh& mesh,
           size_t ideal_steps,
           const step_preprocessor& preprocessor,
           const aligned::vector<step_postprocessor>& postprocessors,
           const per_step_callback& callback,
           const std::atomic_bool& keep_going);

/// Simplified run function to just hammer through an entire simulation and
/// return the output. Nothing fancy.
template <typename It>
aligned::vector<postprocessor::microphone_state::output> run(
        const compute_context& cc,
        const mesh& mesh,
        size_t input_node,
        It begin,
        It end,
        size_t output_node,
        double speed_of_sound,
        double acoustic_impedance,
        const per_step_callback& callback) {
    auto prep{preprocessor::make_single_soft_source(input_node, begin, end)};

    aligned::vector<postprocessor::microphone_state::output> ret{};

    aligned::vector<step_postprocessor> postprocessors{
            postprocessor::microphone{
                    mesh.get_descriptor(),
                    compute_sample_rate(mesh.get_descriptor(), speed_of_sound),
                    acoustic_impedance / speed_of_sound,
                    output_node,
                    make_output_iterator_callback(std::back_inserter(ret))}};

    run(cc,
        mesh,
        std::distance(begin, end),
        prep,
        postprocessors,
        callback,
        true);

    return ret;
}

}  // namespace waveguide
