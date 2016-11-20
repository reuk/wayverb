#pragma once

#include "waveguide/mesh.h"

#include "core/cl/include.h"
#include "core/conversions.h"
#include "core/exceptions.h"

#include <atomic>
#include <cassert>
#include <functional>
#include <iostream>

namespace wayverb {
namespace waveguide {

/// Will set up and run a waveguide using an existing 'template' (the mesh).
///
/// cc:             OpenCL context and device to use
/// mesh:           contains node placements and surface filter information
/// pre:            will be run before each step, should inject inputs
/// post:           will be run after each step, should collect outputs
/// keep_going:     toggle this from another thread to quit early
///
/// returns:        the number of steps completed successfully

/// step_preprocessor
/// Run before each waveguide iteration.
///
/// returns:        true if the simulation should continue

/// step_postprocessor
/// Run after each waveguide iteration.
/// Could be a stateful object which accumulates mesh state in some way.

template <typename step_preprocessor, typename step_postprocessor>
size_t run(const core::compute_context& cc,
           const mesh& mesh,
           step_preprocessor&& pre,
           step_postprocessor&& post,
           const std::atomic_bool& keep_going) {
    const auto num_nodes = mesh.get_structure().get_condensed_nodes().size();

    const program program{cc};
    cl::CommandQueue queue{cc.context, cc.device};
    const auto make_zeroed_buffer = [&] {
        auto ret = cl::Buffer{
                cc.context, CL_MEM_READ_WRITE, sizeof(cl_float) * num_nodes};
        auto kernel = program.get_zero_buffer_kernel();
        kernel(cl::EnqueueArgs{queue, cl::NDRange{num_nodes}}, ret);
        return ret;
    };

    auto previous = make_zeroed_buffer();
    auto current = make_zeroed_buffer();

    const auto node_buffer = core::load_to_buffer(
            cc.context, mesh.get_structure().get_condensed_nodes(), true);

    const auto boundary_coefficients_buffer = core::load_to_buffer(
            cc.context, mesh.get_structure().get_coefficients(), true);

    cl::Buffer error_flag_buffer{cc.context, CL_MEM_READ_WRITE, sizeof(cl_int)};

    auto boundary_buffer_1 = core::load_to_buffer(
            cc.context, get_boundary_data<1>(mesh.get_structure()), false);
    auto boundary_buffer_2 = core::load_to_buffer(
            cc.context, get_boundary_data<2>(mesh.get_structure()), false);
    auto boundary_buffer_3 = core::load_to_buffer(
            cc.context, get_boundary_data<3>(mesh.get_structure()), false);

    auto kernel = program.get_kernel();

    //  run
    auto step = 0u;

    //  The preprocessor returns 'true' while it should be run.
    //  It also updates the mesh with new pressure values.
    for (; pre(queue, current, step) && keep_going; ++step) {
        //  set flag state to successful
        core::write_value(queue, error_flag_buffer, 0, id_success);

        //  run kernel
        kernel(cl::EnqueueArgs(queue,
                               cl::NDRange(mesh.get_structure()
                                                   .get_condensed_nodes()
                                                   .size())),
               previous,
               current,
               node_buffer,
               mesh.get_descriptor().dimensions,
               boundary_buffer_1,
               boundary_buffer_2,
               boundary_buffer_3,
               boundary_coefficients_buffer,
               error_flag_buffer);

        //  read out flag value
        if (const auto error_flag =
                    core::read_value<error_code>(queue, error_flag_buffer, 0)) {
            if (error_flag & id_inf_error) {
                throw core::exceptions::value_is_inf(
                        "Pressure value is inf, check filter coefficients.");
            }

            if (error_flag & id_nan_error) {
                throw core::exceptions::value_is_nan(
                        "Pressure value is nan, check filter coefficients.");
            }

            if (error_flag & id_outside_mesh_error) {
                throw std::runtime_error("Tried to read non-existant node.");
            }

            if (error_flag & id_suspicious_boundary_error) {
                throw std::runtime_error("Suspicious boundary read.");
            }
        }

        post(queue, current, step);

        std::swap(previous, current);
    }
    return step;
}

}  // namespace waveguide
}  // namespace wayverb
