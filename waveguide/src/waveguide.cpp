#include "waveguide/waveguide.h"
#include "waveguide/log_nan.h"
#include "waveguide/postprocessor/microphone.h"
#include "waveguide/postprocessor/visualiser.h"
#include "waveguide/preprocessor/single_soft_source.h"
#include "waveguide/surface_filters.h"

#include "glog/logging.h"

#include <cassert>

namespace waveguide {

waveguide::~waveguide() noexcept = default;

waveguide::waveguide(const cl::Context& context,
                     const cl::Device& device,
                     const mesh_boundary& boundary,
                     const glm::vec3& anchor,
                     double sample_rate)
        : waveguide(context,
                    device,
                    mesh(boundary,
                         config::grid_spacing(speed_of_sound, 1 / sample_rate),
                         anchor),
                    sample_rate,
                    filters::to_filter_coefficients(
                            boundary.get_scene_data().get_surfaces(),
                            sample_rate)) {}

waveguide::waveguide(
        const cl::Context& context,
        const cl::Device& device,
        const mesh& mesh,
        double sample_rate,
        aligned::vector<filters::canonical_coefficients> coefficients)
        : waveguide(context,
                    device,
                    mesh,
                    sample_rate,
                    mesh.get_condensed_nodes(),
                    coefficients) {}

waveguide::waveguide(
        const cl::Context& context,
        const cl::Device& device,
        const mesh& mesh,
        double sample_rate,
        aligned::vector<program::condensed_node> nodes,
        aligned::vector<filters::canonical_coefficients> coefficients)
        : queue(context, device)
        , program(context, device)
        , kernel(program.get_kernel())
        , lattice(mesh)
        , sample_rate(sample_rate)
        , previous(program.template get_info<CL_PROGRAM_CONTEXT>(),
                   CL_MEM_READ_WRITE,
                   nodes.size() * sizeof(cl_float))
        , current(program.template get_info<CL_PROGRAM_CONTEXT>(),
                  CL_MEM_READ_WRITE,
                  nodes.size() * sizeof(cl_float))
        , node_buffer(context, nodes.begin(), nodes.end(), true)
        , boundary_coefficients_buffer(
                  context, coefficients.begin(), coefficients.end(), true)
        , error_flag_buffer(context, CL_MEM_READ_WRITE, sizeof(cl_int)) {
    LOG(INFO) << "main memory node storage: "
              << (sizeof(program::condensed_node) * mesh.get_nodes().size() >>
                  20)
              << " MB";
}

size_t waveguide::init_and_run(
        size_t ideal_steps,
        const step_preprocessor& preprocessor,
        const aligned::vector<step_postprocessor>& postprocessors,
        const per_step_callback& callback,
        std::atomic_bool& keep_going) {
    //  init
    const auto context = program.get_info<CL_PROGRAM_CONTEXT>();

    auto boundary_buffer_1 =
            load_to_buffer(context, lattice.get_boundary_data<1>(), false);
    auto boundary_buffer_2 =
            load_to_buffer(context, lattice.get_boundary_data<2>(), false);
    auto boundary_buffer_3 =
            load_to_buffer(context, lattice.get_boundary_data<3>(), false);

    auto zero_mesh = [this](auto& buffer) {
        aligned::vector<cl_uchar> n(buffer.template getInfo<CL_MEM_SIZE>(), 0);
        cl::copy(queue, n.begin(), n.end(), buffer);
    };
    zero_mesh(previous);
    zero_mesh(current);

    //  run
    size_t step{0};
    for (; step != ideal_steps && keep_going; ++step) {

        //  set flag state to successful
        write_single_value(queue, error_flag_buffer, 0, program::id_success);

        //  update the mesh with new inputs
        preprocessor(queue, current, step);

        //  run kernel
        kernel(cl::EnqueueArgs(queue, cl::NDRange(lattice.get_nodes().size())),
               previous,
               current,
               node_buffer,
               to_cl_int3(lattice.get_dim()),
               boundary_buffer_1,
               boundary_buffer_2,
               boundary_buffer_3,
               boundary_coefficients_buffer,
               error_flag_buffer);

        //  read out flag value
        auto flag = read_single_value<program::ErrorCode>(
                queue, error_flag_buffer, 0);
        if (flag & program::id_inf_error) {
            throw std::runtime_error(
                    "pressure value is inf, check filter coefficients");
        }

        if (flag & program::id_nan_error) {
            throw std::runtime_error(
                    "pressure value is nan, check filter coefficients");
        }

        if (flag & program::id_outside_mesh_error) {
            throw std::runtime_error("tried to read non-existant node");
        }

        if (flag & program::id_suspicious_boundary_error) {
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

size_t waveguide::get_index_for_coordinate(const glm::vec3& v) const {
    return lattice.compute_index(lattice.compute_locator(v));
}

glm::vec3 waveguide::get_coordinate_for_index(size_t index) const {
    return to_vec3(lattice.get_nodes()[index].position);
}

const mesh& waveguide::get_mesh() const { return lattice; }
double waveguide::get_sample_rate() const { return sample_rate; }

bool waveguide::inside(size_t index) const {
    return lattice.get_nodes()[index].inside;
}

//----------------------------------------------------------------------------//

std::experimental::optional<aligned::vector<run_step_output>> init_and_run(
        waveguide& waveguide,
        const glm::vec3& e,
        const aligned::vector<float>& input,
        size_t output_node,
        size_t steps,
        std::atomic_bool& keep_going,
        const waveguide::per_step_callback& callback) {
    preprocessor::single_soft_source source(
            waveguide.get_index_for_coordinate(e), input);

    aligned::vector<run_step_output> ret;
    ret.reserve(steps);
    aligned::vector<waveguide::step_postprocessor> postprocessors{
            postprocessor::microphone(
                    waveguide.get_mesh(),
                    output_node,
                    waveguide.get_sample_rate(),
                    [&ret](const auto& i) { ret.push_back(i); })};

    if (waveguide.init_and_run(
                steps, source, postprocessors, callback, keep_going) == steps) {
        return ret;
    }

    return std::experimental::nullopt;
}

}  // namespace waveguide
