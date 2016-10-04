#pragma once

#include "CL/cl.hpp"

#include <atomic>
#include <functional>

class compute_context;

namespace waveguide {

class mesh;

/// Run before each waveguide iteration.
///
/// returns:        true if the simulation should continue
using step_preprocessor = std::function<bool(
        cl::CommandQueue& queue, cl::Buffer& buffer, size_t step)>;

/// Run after each waveguide iteration.
/// Could be a stateful object which accumulates mesh state in some way.
using step_postprocessor = std::function<void(
        cl::CommandQueue& queue, const cl::Buffer& buffer, size_t step)>;

/// Will set up and run a waveguide using an existing 'template' (the mesh).
///
/// cc:             OpenCL context and device to use
/// mesh:           contains node placements and surface filter information
/// pre:            will be run before each step, should inject inputs
/// post:           will be run after each step, should collect outputs
/// keep_going:     toggle this from another thread to quit early
///
/// returns:        the number of steps completed successfully

size_t run(const compute_context& cc,
           const mesh& mesh,
           const step_preprocessor& pre,
           const step_postprocessor& post,
           const std::atomic_bool& keep_going);

}  // namespace waveguide
