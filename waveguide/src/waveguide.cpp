#include "common/conversions.h"
#include "common/exceptions.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/microphone.h"
#include "waveguide/postprocessor/visualiser.h"
#include "waveguide/preprocessor/single_soft_source.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include <cassert>

namespace waveguide {

size_t run(const compute_context& cc,
           const mesh& mesh,
           size_t ideal_steps,
           const step_preprocessor& preprocessor,
           const aligned::vector<step_postprocessor>& postprocessors,
           const per_step_callback& callback,
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
    for (; step != ideal_steps && keep_going; ++step) {
        //  set flag state to successful
        write_single_value(queue, error_flag_buffer, 0, id_success);

        //  update the mesh with new inputs
        preprocessor(queue, current, step);

        //  run kernel
        kernel(cl::EnqueueArgs(queue,
                               cl::NDRange(mesh.get_structure()
                                                   .get_condensed_nodes()
                                                   .size())),
               previous,
               current,
               node_buffer,
               to_cl_int3(mesh.get_descriptor().dimensions),
               boundary_buffer_1,
               boundary_buffer_2,
               boundary_buffer_3,
               boundary_coefficients_buffer,
               error_flag_buffer);

        //  read out flag value
        const auto flag{
                read_single_value<error_code>(queue, error_flag_buffer, 0)};
        if (flag & id_inf_error) {
            throw exceptions::value_is_inf(
                    "pressure value is inf, check filter coefficients");
        }

        if (flag & id_nan_error) {
            throw exceptions::value_is_nan(
                    "pressure value is nan, check filter coefficients");
        }

        if (flag & id_outside_mesh_error) {
            throw std::runtime_error("tried to read non-existant node");
        }

        if (flag & id_suspicious_boundary_error) {
            throw std::runtime_error("suspicious boundary read");
        }

        for (auto& i : postprocessors) {
            i(queue, current, step);
        }

        std::swap(previous, current);

        callback(step);
    }
    return step;
}

//----------------------------------------------------------------------------//

aligned::vector<run_step_output> run(const compute_context& cc,
                                     const mesh& mesh,
                                     size_t source_index,
                                     const aligned::vector<float>& input,
                                     size_t receiver_index,
                                     double speed_of_sound,
                                     double acoustic_impedance,
                                     const per_step_callback& callback) {
    preprocessor::single_soft_source preprocessor(source_index, input);

    aligned::vector<run_step_output> ret;
    ret.reserve(input.size());
    aligned::vector<step_postprocessor> postprocessors{
            postprocessor::make_microphone(
                    mesh.get_descriptor(),
                    compute_sample_rate(mesh.get_descriptor(), speed_of_sound),
                    acoustic_impedance / speed_of_sound,
                    receiver_index,
                    std::back_inserter(ret))};

    const auto results{run(cc,
                           mesh,
                           input.size(),
                           preprocessor,
                           postprocessors,
                           callback,
                           true)};

    assert(results == input.size());

    return ret;
}

}  // namespace waveguide
