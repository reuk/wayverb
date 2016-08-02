#include "waveguide/rectangular_waveguide.h"
#include "waveguide/log_nan.h"
#include "waveguide/postprocessors/microphone.h"
#include "waveguide/postprocessors/visualiser.h"

#include "glog/logging.h"

#include <cassert>

namespace {
struct boundary_buffers {
    boundary_buffers(
            const cl::Context& context,
            aligned::vector<rectangular_program::BoundaryDataArray1>&& bd1,
            aligned::vector<rectangular_program::BoundaryDataArray2>&& bd2,
            aligned::vector<rectangular_program::BoundaryDataArray3>&& bd3)
            : boundary_1(context, bd1.begin(), bd1.end(), false)
            , boundary_2(context, bd2.begin(), bd2.end(), false)
            , boundary_3(context, bd3.begin(), bd3.end(), false) {
#ifndef NDEBUG
        auto zeroed = [](const auto& arr) {
            return proc::all_of(arr, [](const auto& i) {
                return proc::all_of(i.array, [](const auto& i) {
                    return proc::all_of(i.filter_memory.array,
                                        [](const auto& i) { return !i; });
                });
            });
        };
        assert(zeroed(bd1));
        assert(zeroed(bd2));
        assert(zeroed(bd3));
#endif
    }

    cl::Buffer boundary_1;
    cl::Buffer boundary_2;
    cl::Buffer boundary_3;
};
}  // namespace

rectangular_waveguide::~rectangular_waveguide() noexcept = default;

rectangular_waveguide::rectangular_waveguide(const cl::Context& context,
                                             const cl::Device& device,
                                             const MeshBoundary& boundary,
                                             const glm::vec3& anchor,
                                             double sample_rate)
        : rectangular_waveguide(
                  context,
                  device,
                  rectangular_mesh(
                          boundary,
                          config::grid_spacing(SPEED_OF_SOUND, 1 / sample_rate),
                          anchor),
                  sample_rate,
                  rectangular_program::to_filter_coefficients(
                          boundary.get_surfaces(), sample_rate)) {}

rectangular_waveguide::rectangular_waveguide(
        const cl::Context& context,
        const cl::Device& device,
        const rectangular_mesh& mesh,
        double sample_rate,
        aligned::vector<rectangular_program::CanonicalCoefficients>
                coefficients)
        : rectangular_waveguide(context,
                                device,
                                mesh,
                                sample_rate,
                                mesh.get_condensed_nodes(),
                                coefficients) {}

rectangular_waveguide::rectangular_waveguide(
        const cl::Context& context,
        const cl::Device& device,
        const rectangular_mesh& mesh,
        double sample_rate,
        aligned::vector<rectangular_program::CondensedNodeStruct> nodes,
        aligned::vector<rectangular_program::CanonicalCoefficients>
                coefficients)
        : queue(context, device)
        , program(context, device)
        , kernel(program.get_kernel())
        , mesh(mesh)
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
              << (sizeof(rectangular_program::NodeStruct) *
                          mesh.get_nodes().size() >>
                  20)
              << " MB";
}

bool rectangular_waveguide::init_and_run(
        const glm::vec3& excitation_location,
        const aligned::vector<float>& input,
        const aligned::vector<std::unique_ptr<step_postprocessor>>&
                postprocessors,
        const per_step_callback& callback,
        std::atomic_bool& keep_going) {
    //  init
    const auto context = program.get_info<CL_PROGRAM_CONTEXT>();
    const boundary_buffers boundary_buffers(context,
                                            mesh.get_boundary_data<1>(),
                                            mesh.get_boundary_data<2>(),
                                            mesh.get_boundary_data<3>());

    const auto input_node = get_index_for_coordinate(excitation_location);

    auto zero_mesh = [this](auto& buffer) {
        aligned::vector<cl_uchar> n(buffer.template getInfo<CL_MEM_SIZE>(), 0);
        cl::copy(queue, n.begin(), n.end(), buffer);
    };
    zero_mesh(previous);
    zero_mesh(current);

    //  run
    for (auto pressure : input) {
        if (! keep_going) {
            return false;
        }

        write_single_value(
                queue, error_flag_buffer, 0, rectangular_program::id_success);

        //  add input pressure to current pressure at input node
        const auto current_pressure =
                read_single_value<cl_float>(queue, current, 0);
        const auto new_pressure = current_pressure + pressure;
        write_single_value(queue, current, input_node, new_pressure);

        //  run kernel
        kernel(cl::EnqueueArgs(queue, cl::NDRange(mesh.get_nodes().size())),
               previous,
               current,
               node_buffer,
               to_cl_int3(get_mesh().get_dim()),
               boundary_buffers.boundary_1,
               boundary_buffers.boundary_2,
               boundary_buffers.boundary_3,
               boundary_coefficients_buffer,
               error_flag_buffer);

        //  read out flag value
        auto flag = read_single_value<rectangular_program::ErrorCode>(
                queue, error_flag_buffer, 0);
        if (flag & rectangular_program::id_inf_error) {
            throw std::runtime_error(
                    "pressure value is inf, check filter coefficients");
        }

        if (flag & rectangular_program::id_nan_error) {
            throw std::runtime_error(
                    "pressure value is nan, check filter coefficients");
        }

        if (flag & rectangular_program::id_outside_mesh_error) {
            throw std::runtime_error("tried to read non-existant node");
        }

        if (flag & rectangular_program::id_suspicious_boundary_error) {
            throw std::runtime_error("suspicious boundary read");
        }

        for (const auto& i : postprocessors) {
            i->process(queue, current);
        }

        std::swap(previous, current);
    }

    return true;
}

//  TODO refactor this

std::experimental::optional<
        aligned::vector<rectangular_waveguide::run_step_output>>
rectangular_waveguide::init_and_run(const glm::vec3& e,
                                    const aligned::vector<float>& input,
                                    size_t output_node,
                                    size_t steps,
                                    std::atomic_bool& keep_going,
                                    const per_step_callback& callback) {
    auto t = input;
    t.resize(steps, 0);

    aligned::vector<run_step_output> ret;
    ret.reserve(steps);
    aligned::vector<std::unique_ptr<step_postprocessor>> postprocessors;
    postprocessors.push_back(
            std::make_unique<postprocessors::microphone_step_postprocessor>(
                    mesh,
                    output_node,
                    sample_rate,
                    [&ret](float pressure, const glm::vec3& intensity) {
                        ret.push_back(run_step_output{intensity, pressure});
                    }));

    if (init_and_run(e, t, postprocessors, callback, keep_going)) {
        return ret;
    }

    return std::experimental::nullopt;
}

std::experimental::optional<
        aligned::vector<rectangular_waveguide::run_step_output>>
rectangular_waveguide::init_and_run_visualised(
        const glm::vec3& e,
        const aligned::vector<float>& input,
        size_t output_node,
        size_t steps,
        std::atomic_bool& keep_going,
        const per_step_callback& callback,
        const visualiser_callback& visual_callback) {
    auto t = input;
    t.resize(steps, 0);

    aligned::vector<run_step_output> ret;
    ret.reserve(steps);
    aligned::vector<std::unique_ptr<step_postprocessor>> postprocessors;
    postprocessors.push_back(
            std::make_unique<postprocessors::microphone_step_postprocessor>(
                    mesh,
                    output_node,
                    sample_rate,
                    [&ret](float pressure, const glm::vec3& intensity) {
                        ret.push_back(run_step_output{intensity, pressure});
                    }));
    postprocessors.push_back(
            std::make_unique<postprocessors::visualiser_step_postprocessor>(
                    mesh.get_nodes().size(),
                    [&visual_callback](const auto& i) { visual_callback(i); }));

    if (init_and_run(e, t, postprocessors, callback, keep_going)) {
        return ret;
    }

    return std::experimental::nullopt;
}

size_t rectangular_waveguide::get_index_for_coordinate(
        const glm::vec3& v) const {
    return mesh.compute_index(mesh.compute_locator(v));
}

glm::vec3 rectangular_waveguide::get_coordinate_for_index(size_t index) const {
    return to_vec3f(mesh.get_nodes()[index].position);
}

const rectangular_mesh& rectangular_waveguide::get_mesh() const { return mesh; }

bool rectangular_waveguide::inside(size_t index) const {
    return mesh.get_nodes()[index].inside;
}
