#include "waveguide/rectangular_waveguide.h"
#include "waveguide/log_nan.h"

#include "glog/logging.h"

#include <cassert>

namespace detail {

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
template <typename>
struct Type;
#define PRINT_TYPE(T) Type<T> CONCAT(t, __COUNTER__)
#define PRINT_TYPE_OF(T) PRINT_TYPE(decltype(T))

}  // namespace detail

struct rectangular_waveguide::rectangular_waveguide_run_info {
    rectangular_waveguide_run_info(
            const cl::Context& context,
            aligned::vector<rectangular_program::BoundaryDataArray1>&& bd1,
            aligned::vector<rectangular_program::BoundaryDataArray2>&& bd2,
            aligned::vector<rectangular_program::BoundaryDataArray3>&& bd3,
            std::array<cl_uint, num_ports>&& surrounding_port_indices)
            : velocity(0, 0, 0)
            , boundary_1(context, bd1.begin(), bd1.end(), false)
            , boundary_2(context, bd2.begin(), bd2.end(), false)
            , boundary_3(context, bd3.begin(), bd3.end(), false)
            , surrounding_port_indices(surrounding_port_indices) {
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

    glm::dvec3 velocity;
    cl::Buffer boundary_1;
    cl::Buffer boundary_2;
    cl::Buffer boundary_3;
    std::array<cl_uint, num_ports> surrounding_port_indices;
};

rectangular_waveguide::~rectangular_waveguide() noexcept = default;

rectangular_waveguide::rectangular_waveguide(const cl::Context& context,
                                             const cl::Device& device,
                                             const MeshBoundary& boundary,
                                             const glm::vec3& anchor,
                                             double sr)
        : rectangular_waveguide(
                  context,
                  device,
                  rectangular_mesh(boundary,
                                   config::grid_spacing(SPEED_OF_SOUND, 1 / sr),
                                   anchor),
                  sr,
                  rectangular_program::to_filter_coefficients(
                          boundary.get_surfaces(), sr)) {}

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
        , nodes(nodes.size())
        , previous(program.template get_info<CL_PROGRAM_CONTEXT>(),
                   CL_MEM_READ_WRITE,
                   nodes.size() * sizeof(cl_float))
        , current(program.template get_info<CL_PROGRAM_CONTEXT>(),
                  CL_MEM_READ_WRITE,
                  nodes.size() * sizeof(cl_float))
        , output(program.template get_info<CL_PROGRAM_CONTEXT>(),
                 CL_MEM_READ_WRITE,
                 sizeof(cl_float))
        , sample_rate(sample_rate)
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

rectangular_waveguide::run_step_output rectangular_waveguide::run_step(
        const run_step_input& write_info,
        size_t o,
        cl::CommandQueue& queue,
        kernel_type& kernel,
        size_t nodes,
        cl::Buffer& previous,
        cl::Buffer& current,
        cl::Buffer& output) {
    //  set flag state to 'successful'
    write_single_value(error_flag_buffer, 0, rectangular_program::id_success);

    //  add input pressure to current pressure at input node
    const auto current_pressure = read_single_value<cl_float>(current, 0);
    const auto new_pressure     = current_pressure + write_info.get_pressure();
    write_single_value(current, write_info.get_index(), new_pressure);

    //  run kernel
    kernel(cl::EnqueueArgs(queue, cl::NDRange(nodes)),
           previous,
           current,
           node_buffer,
           to_cl_int3(get_mesh().get_dim()),
           invocation->boundary_1,
           invocation->boundary_2,
           invocation->boundary_3,
           boundary_coefficients_buffer,
           error_flag_buffer);

    //  read out flag value
    auto flag = read_single_value<rectangular_program::ErrorCode>(
            error_flag_buffer, 0);
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

    //  copy out node pressure
    const auto out = read_single_value<cl_float>(current, o);

    //  copy out surrounding pressures
    aligned::vector<cl_float> surrounding;
    surrounding.reserve(num_ports);
    for (auto i : invocation->surrounding_port_indices) {
        surrounding.push_back(read_single_value<cl_float>(current, i));
    }

    //  pressure difference vector is obtained by subtracting
    //  the central junction pressure from
    //  the pressure values of neighboring junctions
    for (auto& i : surrounding) {
        i -= out;
        //  and dividing these terms by the spatial sampling period
        i /= mesh.get_spacing();
    }

    //  the approximation of the pressure gradient is obtained by multiplying
    //  the difference vector by the inverse projection matrix
    //  matrix looks like
    //      -0.5  0.5    0    0    0    0
    //         0    0 -0.5  0.5    0    0
    //         0    0    0    0 -0.5  0.5
    //  so I think the product is like this:
    glm::dvec3 m{surrounding[0] * -0.5 + surrounding[1] * 0.5,
                 surrounding[2] * -0.5 + surrounding[3] * 0.5,
                 surrounding[4] * -0.5 + surrounding[5] * 0.5};

    //  the result is scaled by the negative inverse of the ambient density
    static constexpr auto ambient_density = 1.225;
    auto dv                               = m / -ambient_density;
    //  and integrated using a discrete-time integrator
    invocation->velocity += (1.0 / sample_rate) * dv;

    //  the instantaneous intensity is obtained by multiplying the velocity and
    //  the pressure
    auto intensity = invocation->velocity * static_cast<double>(out);

    return run_step_output(out, intensity);
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

std::experimental::optional<
        aligned::vector<rectangular_waveguide::run_step_output>>
rectangular_waveguide::init_and_run(const glm::vec3& e,
                                    const aligned::vector<float>& input,
                                    size_t o,
                                    size_t steps,
                                    std::atomic_bool& keep_going,
                                    const per_step_callback& callback) {
    auto run_info = init(e, input, o, steps);
    return run_basic(run_info,
                     keep_going,
                     [this, &callback](const auto& run_info, auto i) {
                         auto ret = this->run_step(run_info, i);
                         callback();
                         return ret;
                     });
}

std::experimental::optional<
        aligned::vector<rectangular_waveguide::run_step_output>>
rectangular_waveguide::init_and_run_visualised(
        const glm::vec3& e,
        const aligned::vector<float>& input,
        size_t o,
        size_t steps,
        std::atomic_bool& keep_going,
        const per_step_callback& callback,
        const visualiser_callback& visual_callback) {
    auto run_info = init(e, input, o, steps);
    return run_basic(
            run_info,
            keep_going,
            [this, &callback, &visual_callback](const auto& run_info, auto i) {
                auto ret = this->run_step_visualised(run_info, i);
                callback();
                visual_callback(ret.second);
                return ret.first;
            });
}

std::experimental::optional<
        aligned::vector<rectangular_waveguide::run_step_output>>
rectangular_waveguide::run_basic(const run_info& run_info,
                                 std::atomic_bool& keep_going,
                                 const input_callback& callback) {
    aligned::vector<run_step_output> ret;
    ret.reserve(run_info.get_signal().size());

    for (const auto & i : run_info.get_signal()) {
        if (! keep_going) {
            return std::experimental::nullopt;
        }
        ret.push_back(callback(run_info, i));
    }

    return ret;
}

rectangular_waveguide::run_info rectangular_waveguide::init(
        const glm::vec3& e,
        const aligned::vector<float>& input_sig,
        size_t o,
        size_t steps) {

    auto context = program.get_info<CL_PROGRAM_CONTEXT>();
    invocation   = std::make_unique<rectangular_waveguide_run_info>(
            context,
            mesh.get_boundary_data<1>(),
            mesh.get_boundary_data<2>(),
            mesh.get_boundary_data<3>(),
            mesh.compute_neighbors(o));

    auto zero_mesh = [this](auto& buffer) {
        aligned::vector<cl_uchar> n(buffer.template getInfo<CL_MEM_SIZE>(), 0);
        cl::copy(queue, n.begin(), n.end(), buffer);
    };
    zero_mesh(previous);
    zero_mesh(current);

    auto t = input_sig;
    t.resize(steps, 0);
    return run_info(get_index_for_coordinate(e), t, o);
}

rectangular_waveguide::run_step_output rectangular_waveguide::run_step(
        const run_info& run_info, float input) {
    auto ret = run_step(run_step_input(run_info.get_input_index(), input),
                        run_info.get_output_index(),
                        queue,
                        kernel,
                        nodes,
                        previous,
                        current,
                        output);
    std::swap(current, previous);
    return ret;
}

std::pair<rectangular_waveguide::run_step_output, aligned::vector<cl_float>>
rectangular_waveguide::run_step_visualised(const run_info& run_info,
                                           float input) {
    auto ret = run_step(run_info, input);
    aligned::vector<cl_float> pressures(nodes, 0);
    cl::copy(queue, previous, pressures.begin(), pressures.end());
    return std::make_pair(ret, pressures);
}

constexpr size_t rectangular_waveguide::num_ports;

bool operator==(const rectangular_waveguide& a,
                const rectangular_waveguide& b) {
    return a.mesh == b.mesh;
}
