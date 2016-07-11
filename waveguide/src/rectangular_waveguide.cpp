#include "waveguide/log_nan.h"
#include "waveguide/rectangular_waveguide.h"

#include "glog/logging.h"

namespace detail {

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
template <typename>
struct Type;
#define PRINT_TYPE(T) Type<T> CONCAT(t, __COUNTER__)
#define PRINT_TYPE_OF(T) PRINT_TYPE(decltype(T))

}  // namespace detail

template <int PORTS>
struct rectangular_waveguide_run_info {
    struct buffer_size_pair {
        template <typename T>
        //  yes, I do mean to pass by value here
        //  cl buffer constructors take modifying iterators for mystifying
        //  reasons
        buffer_size_pair(const cl::Context& context, std::vector<T> u)
                : size(u.size())
                , buffer(context, u.begin(), u.end(), false) {
        }

        const size_t size;
        cl::Buffer buffer;
    };

private:
    template <typename T>
    static Eigen::Matrix<double, 3, PORTS> get_transform_matrix(
            int o, const T& nodes) {
        Eigen::Matrix<double, PORTS, 3> ret;
        auto count = 0u;
        const auto& this_node = nodes[o];
        auto basis = to_vec3f(this_node.position);
        for (auto i = 0u; i != PORTS; ++i) {
            auto port = this_node.ports[i];
            if (port != RectangularProgram::NO_NEIGHBOR) {
                auto pos =
                        glm::normalize(to_vec3f(nodes[port].position) - basis);
                ret.row(count++) << pos.x, pos.y, pos.z;
            }
        }
        return pinv(ret);
    }

public:
    template <typename T>
    rectangular_waveguide_run_info(
            int o,
            const T& nodes,
            const cl::Context& context,
            const std::vector<RectangularProgram::BoundaryDataArray1>& bd1,
            const std::vector<RectangularProgram::BoundaryDataArray2>& bd2,
            const std::vector<RectangularProgram::BoundaryDataArray3>& bd3)
            : transform_matrix(get_transform_matrix(o, nodes))
            , velocity(0, 0, 0)
            , boundary_1(context, bd1)
            , boundary_2(context, bd2)
            , boundary_3(context, bd3) {
    }

    Eigen::Matrix<double, 3, PORTS> transform_matrix;
    glm::dvec3 velocity;
    buffer_size_pair boundary_1;
    buffer_size_pair boundary_2;
    buffer_size_pair boundary_3;
};

template <BufferType buffer_type>
RectangularWaveguide<buffer_type>::~RectangularWaveguide() noexcept = default;

template <BufferType buffer_type>
RectangularWaveguide<buffer_type>::RectangularWaveguide(
        const RectangularProgram& program,
        const MeshBoundary& boundary,
        const glm::vec3& anchor,
        float sr)
        : RectangularWaveguide(
                  program,
                  RectangularMesh(boundary,
                                  config::grid_spacing(SPEED_OF_SOUND, 1 / sr),
                                  anchor),
                  sr,
                  RectangularProgram::to_filter_coefficients(
                          boundary.get_surfaces(), sr)) {
}

template <BufferType buffer_type>
RectangularWaveguide<buffer_type>::RectangularWaveguide(
        const typename Base::ProgramType& program,
        const RectangularMesh& mesh,
        float sample_rate,
        std::vector<RectangularProgram::CanonicalCoefficients>
                coefficients)
        : RectangularWaveguide(program,
                               mesh,
                               sample_rate,
                               mesh.get_condensed_nodes(),
                               coefficients) {
}

template <BufferType buffer_type>
RectangularWaveguide<buffer_type>::RectangularWaveguide(
        const typename Base::ProgramType& program,
        const RectangularMesh& mesh,
        float sample_rate,
        std::vector<RectangularMesh::CondensedNode>
                nodes,
        std::vector<RectangularProgram::CanonicalCoefficients>
                coefficients)
        : Waveguide<RectangularProgram, buffer_type>(
                  program, mesh.get_nodes().size(), sample_rate)
        , mesh(mesh)
        , node_buffer(program.template get_info<CL_PROGRAM_CONTEXT>(),
                      nodes.begin(),
                      nodes.end(),
                      false)
        , boundary_coefficients_buffer(
                  program.template get_info<CL_PROGRAM_CONTEXT>(),
                  coefficients.begin(),
                  coefficients.end(),
                  false)
        , surrounding_buffer(program.template get_info<CL_PROGRAM_CONTEXT>(),
                             CL_MEM_READ_WRITE,
                             sizeof(cl_float) * PORTS)
        , surrounding(PORTS, 0)
        , error_flag_buffer(program.template get_info<CL_PROGRAM_CONTEXT>(),
                            CL_MEM_READ_WRITE,
                            sizeof(cl_int)) {
    LOG(INFO) << "main memory node storage: "
              << (sizeof(RectangularProgram::NodeStruct) *
                          mesh.get_nodes().size() >>
                  20)
              << " MB";
}

template <BufferType buffer_type>
void RectangularWaveguide<buffer_type>::setup(cl::CommandQueue& queue,
                                              size_t o) {
    try {
        auto context =
                this->get_program().template get_info<CL_PROGRAM_CONTEXT>();
        invocation = std::make_unique<rectangular_waveguide_run_info<PORTS>>(
                o,
                mesh.get_nodes(),
                context,
                mesh.get_boundary_data<1>(),
                mesh.get_boundary_data<2>(),
                mesh.get_boundary_data<3>());
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw;
    }
}

template <BufferType buffer_type>
RunStepResult RectangularWaveguide<buffer_type>::run_step(
        const typename Base::WriteInfo& write_info,
        size_t o,
        cl::CommandQueue& queue,
        typename Base::kernel_type& kernel,
        size_t nodes,
        cl::Buffer& previous,
        cl::Buffer& current,
        cl::Buffer& output) {
    auto flag = RectangularProgram::id_success;
    cl::copy(queue, (&flag) + 0, (&flag) + 1, error_flag_buffer);

    RectangularProgram::InputInfo input_info{write_info.index,
                                             write_info.pressure};

    kernel(cl::EnqueueArgs(queue, cl::NDRange(nodes)),
           input_info,
           current,
           previous,
           node_buffer,
           to_cl_int3(get_mesh().get_dim()),
           invocation->boundary_1.buffer,
           invocation->boundary_2.buffer,
           invocation->boundary_3.buffer,
           boundary_coefficients_buffer,
           o,
           output,
           surrounding_buffer,
           error_flag_buffer);

    cl::copy(queue, error_flag_buffer, (&flag) + 0, (&flag) + 1);

    //        if (flag & RectangularProgram::id_outside_range_error) {
    //            throw std::runtime_error("pressure value is outside valid
    //            range");
    //        }

    if (flag & RectangularProgram::id_inf_error) {
        throw std::runtime_error(
                "pressure value is inf, check filter coefficients");
    }

    if (flag & RectangularProgram::id_nan_error) {
        throw std::runtime_error(
                "pressure value is nan, check filter coefficients");
    }

    if (flag & RectangularProgram::id_outside_mesh_error) {
        throw std::runtime_error("tried to read non-existant node");
    }

    if (flag & RectangularProgram::id_suspicious_boundary_error) {
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
    glm::dvec3 m{surrounding[0] * -0.5 + surrounding[1] * 0.5,
                 surrounding[2] * -0.5 + surrounding[3] * 0.5,
                 surrounding[4] * -0.5 + surrounding[5] * 0.5};

    //  the result is scaled by the negative inverse of the ambient density
    static constexpr auto ambient_density = 1.225;
    auto dv = m / -ambient_density;
    //  and integrated using a discrete-time integrator
    invocation->velocity += this->get_period() * dv;

    //  the instantaneous intensity is obtained by multiplying the velocity and
    //  the pressure
    auto intensity = invocation->velocity * static_cast<double>(out);

    return RunStepResult{out, intensity};
}

template <BufferType buffer_type>
size_t RectangularWaveguide<buffer_type>::get_index_for_coordinate(
        const glm::vec3& v) const {
    return mesh.compute_index(mesh.compute_locator(v));
}

template <BufferType buffer_type>
glm::vec3 RectangularWaveguide<buffer_type>::get_coordinate_for_index(
        size_t index) const {
    return to_vec3f(mesh.get_nodes()[index].position);
}

template <BufferType buffer_type>
const RectangularMesh& RectangularWaveguide<buffer_type>::get_mesh() const {
    return mesh;
}

template <BufferType buffer_type>
bool RectangularWaveguide<buffer_type>::inside(size_t index) const {
    return mesh.get_nodes()[index].inside;
}

//  instantiate - maybe special-case out the GL version if gl is not present?

template class RectangularWaveguide<BufferType::cl>;

template <BufferType bt>
constexpr int RectangularWaveguide<bt>::PORTS;
