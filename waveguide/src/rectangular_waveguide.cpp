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

struct rectangular_waveguide_run_info {
    rectangular_waveguide_run_info(
            const cl::Context& context,
            aligned::vector<rectangular_program::BoundaryDataArray1>&& bd1,
            aligned::vector<rectangular_program::BoundaryDataArray2>&& bd2,
            aligned::vector<rectangular_program::BoundaryDataArray3>&& bd3)
            : velocity(0, 0, 0)
            , boundary_1(context, bd1.begin(), bd1.end(), false)
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

    glm::dvec3 velocity;
    cl::Buffer boundary_1;
    cl::Buffer boundary_2;
    cl::Buffer boundary_3;
};

RectangularWaveguide::~RectangularWaveguide() noexcept = default;

RectangularWaveguide::RectangularWaveguide(const cl::Context& context,
                                           const cl::Device& device,
                                           const MeshBoundary& boundary,
                                           const glm::vec3& anchor,
                                           float sr)
        : RectangularWaveguide(
                  context,
                  device,
                  RectangularMesh(boundary,
                                  config::grid_spacing(SPEED_OF_SOUND, 1 / sr),
                                  anchor),
                  sr,
                  rectangular_program::to_filter_coefficients(
                          boundary.get_surfaces(), sr)) {}

RectangularWaveguide::RectangularWaveguide(
        const cl::Context& context,
        const cl::Device& device,
        const RectangularMesh& mesh,
        float sample_rate,
        aligned::vector<rectangular_program::CanonicalCoefficients>
                coefficients)
        : RectangularWaveguide(context,
                               device,
                               mesh,
                               sample_rate,
                               mesh.get_condensed_nodes(),
                               coefficients) {}

RectangularWaveguide::RectangularWaveguide(
        const cl::Context& context,
        const cl::Device& device,
        const RectangularMesh& mesh,
        float sample_rate,
        aligned::vector<RectangularMesh::CondensedNode> nodes,
        aligned::vector<rectangular_program::CanonicalCoefficients>
                coefficients)
        : Waveguide<rectangular_program>(
                  context, device, mesh.get_nodes().size(), sample_rate)
        , mesh(mesh)
        , node_buffer(context, nodes.begin(), nodes.end(), true)
        , boundary_coefficients_buffer(
                  context, coefficients.begin(), coefficients.end(), true)
        , surrounding(PORTS, 0)
        , surrounding_buffer(
                  context, surrounding.begin(), surrounding.end(), false)
        , error_flag_buffer(context, CL_MEM_READ_WRITE, sizeof(cl_int)) {
    LOG(INFO) << "main memory node storage: "
              << (sizeof(rectangular_program::NodeStruct) *
                          mesh.get_nodes().size() >>
                  20)
              << " MB";
}

void RectangularWaveguide::setup(cl::CommandQueue& queue, size_t o) {
    try {
        auto context =
                this->get_program().template get_info<CL_PROGRAM_CONTEXT>();
        invocation = std::make_unique<rectangular_waveguide_run_info>(
                context,
                mesh.get_boundary_data<1>(),
                mesh.get_boundary_data<2>(),
                mesh.get_boundary_data<3>());

        proc::fill(surrounding, 0);
        cl::copy(queue,
                 surrounding.begin(),
                 surrounding.end(),
                 surrounding_buffer);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw;
    }
}

RunStepResult RectangularWaveguide::run_step(
        const typename Base::WriteInfo& write_info,
        size_t o,
        cl::CommandQueue& queue,
        typename Base::kernel_type& kernel,
        size_t nodes,
        cl::Buffer& previous,
        cl::Buffer& current,
        cl::Buffer& output) {
    auto flag = rectangular_program::id_success;
    cl::copy(queue, (&flag) + 0, (&flag) + 1, error_flag_buffer);

    rectangular_program::InputInfo input_info{write_info.index,
                                              write_info.pressure};

    kernel(cl::EnqueueArgs(queue, cl::NDRange(nodes)),
           input_info,
           previous,
           current,
           node_buffer,
           to_cl_int3(get_mesh().get_dim()),
           invocation->boundary_1,
           invocation->boundary_2,
           invocation->boundary_3,
           boundary_coefficients_buffer,
           o,
           output,
           surrounding_buffer,
           error_flag_buffer);

    cl::copy(queue, error_flag_buffer, (&flag) + 0, (&flag) + 1);

    //        if (flag & rectangular_program::id_outside_range_error) {
    //            throw std::runtime_error("pressure value is outside valid
    //            range");
    //        }

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

    //  pressure difference vector is obtained by subtracting
    //  the central junction pressure from
    cl_float out;
    cl::copy(queue, output, (&out), (&out) + 1);

    //  the pressure values of neighboring junctions
    cl::copy(queue, surrounding_buffer, surrounding.begin(), surrounding.end());

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
    invocation->velocity += this->get_period() * dv;

    //  the instantaneous intensity is obtained by multiplying the velocity and
    //  the pressure
    auto intensity = invocation->velocity * static_cast<double>(out);

    return RunStepResult{out, intensity};
}

size_t RectangularWaveguide::get_index_for_coordinate(
        const glm::vec3& v) const {
    return mesh.compute_index(mesh.compute_locator(v));
}

glm::vec3 RectangularWaveguide::get_coordinate_for_index(size_t index) const {
    return to_vec3f(mesh.get_nodes()[index].position);
}

const RectangularMesh& RectangularWaveguide::get_mesh() const { return mesh; }

bool RectangularWaveguide::inside(size_t index) const {
    return mesh.get_nodes()[index].inside;
}

constexpr int RectangularWaveguide::PORTS;

bool operator==(const RectangularWaveguide& a, const RectangularWaveguide& b) {
    return a.mesh == b.mesh;
}
