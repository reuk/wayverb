#include "waveguide/waveguide.h"
#include "common/conversions.h"
#include "common/exceptions.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/visualiser.h"
#include "waveguide/surface_filters.h"

#include <cassert>
#include <iostream>

namespace waveguide {

size_t run(const compute_context& cc,
           const mesh& mesh,
           const step_preprocessor& preprocessor,
           const step_postprocessor& postprocessor,
           const std::atomic_bool& keep_going) {
    cl::Buffer previous{cc.context,
                        CL_MEM_READ_WRITE,
                        mesh.get_structure().get_condensed_nodes().size() *
                                sizeof(cl_float)};

    cl::Buffer current{cc.context,
                       CL_MEM_READ_WRITE,
                       mesh.get_structure().get_condensed_nodes().size() *
                               sizeof(cl_float)};

    const auto node_buffer{load_to_buffer(
            cc.context, mesh.get_structure().get_condensed_nodes(), true)};

    const auto boundary_coefficients_buffer{load_to_buffer(
            cc.context, mesh.get_structure().get_coefficients(), true)};

    cl::Buffer error_flag_buffer{cc.context, CL_MEM_READ_WRITE, sizeof(cl_int)};

    auto boundary_buffer_1{load_to_buffer(
            cc.context, get_boundary_data<1>(mesh.get_structure()), false)};
    auto boundary_buffer_2{load_to_buffer(
            cc.context, get_boundary_data<2>(mesh.get_structure()), false)};
    auto boundary_buffer_3{load_to_buffer(
            cc.context, get_boundary_data<3>(mesh.get_structure()), false)};

    cl::CommandQueue queue{cc.context, cc.device};

    const auto zero_mesh{[&](auto& buffer) {
        aligned::vector<cl_uchar> n(buffer.template getInfo<CL_MEM_SIZE>(), 0);
        cl::copy(queue, n.begin(), n.end(), buffer);
    }};
    zero_mesh(previous);
    zero_mesh(current);

    const program program{cc};
    auto kernel{program.get_kernel()};

    //  run
    auto step{0u};

    //  The preprocessor returns 'true' while it should be run.
    //  It also updates the mesh with new pressure values.
    for (; preprocessor(queue, current, step) && keep_going; ++step) {
        //  set flag state to successful
        write_value(queue, error_flag_buffer, 0, id_success);

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
        if (const auto error_flag{
                    read_value<error_code>(queue, error_flag_buffer, 0)}) {
            if (error_flag & id_inf_error) {
                throw exceptions::value_is_inf(
                        "pressure value is inf, check filter coefficients");
            }

            if (error_flag & id_nan_error) {
                throw exceptions::value_is_nan(
                        "pressure value is nan, check filter coefficients");
            }

            if (error_flag & id_outside_mesh_error) {
                throw std::runtime_error("tried to read non-existant node");
            }

            if (error_flag & id_suspicious_boundary_error) {
                throw std::runtime_error("suspicious boundary read");
            }
        }

        postprocessor(queue, current, step);

        std::swap(previous, current);
    }
    return step;
}

}  // namespace waveguide
