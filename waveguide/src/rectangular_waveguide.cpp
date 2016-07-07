#include "waveguide/rectangular_waveguide.h"
#include "waveguide/log_nan.h"

#include "glog/logging.h"

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
        std::vector<RectangularProgram::CanonicalCoefficients> coefficients)
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
        std::vector<RectangularMesh::CondensedNode> nodes,
        std::vector<RectangularProgram::CanonicalCoefficients> coefficients)
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
        invocation =
                std::make_unique<invocation_info>(o,
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

    RectangularProgram::InputInfo input_info{
            write_info.index, write_info.pressure, write_info.is_on};

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

    if (flag & RectangularProgram::id_outside_range_error)
        throw std::runtime_error("pressure value is outside valid range");

    if (flag & RectangularProgram::id_inf_error)
        throw std::runtime_error(
                "pressure value is inf, check filter coefficients");

    if (flag & RectangularProgram::id_nan_error)
        throw std::runtime_error(
                "pressure value is nan, check filter coefficients");

    if (flag & RectangularProgram::id_outside_mesh_error)
        throw std::runtime_error("tried to read non-existant node");

    if (flag & RectangularProgram::id_suspicious_boundary_error)
        throw std::runtime_error("suspicious boundary read");

    cl_float out;
    cl::copy(queue, output, (&out), (&out) + 1);
    cl::copy(queue, surrounding_buffer, surrounding.begin(), surrounding.end());

    for (auto& i : surrounding) {
        i -= out / mesh.get_spacing();
    }

    Eigen::Matrix<float, PORTS, 1> surrounding_vec;
    surrounding_vec << surrounding[0], surrounding[1], surrounding[2],
            surrounding[3], surrounding[4], surrounding[5];

    auto multiplied = invocation->transform_matrix * surrounding_vec;
    float x = multiplied[0];
    float y = multiplied[1];
    float z = multiplied[2];
    glm::vec3 m(x, y, z);
    static constexpr auto ambient_density = 1.225f;
    invocation->velocity += this->get_period() * (m / -ambient_density);
    auto intensity = invocation->velocity * out;

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
