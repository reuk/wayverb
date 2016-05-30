#include "waveguide/waveguide.h"

#include "common/conversions.h"
#include "common/sinc.h"
#include "common/stl_wrappers.h"

#include "glog/logging.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>

//----------------------------------------------------------------------------//

namespace detail {

BufferTypeTrait<BufferType::cl>::storage_array_type
BufferTypeTrait<BufferType::cl>::create_waveguide_storage(
    const cl::Context& context, size_t nodes) {
    return {{cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * nodes),
             cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * nodes)}};
}

}  // namespace detail

//----------------------------------------------------------------------------//

template <BufferType buffer_type>
RectangularWaveguide<buffer_type>::RectangularWaveguide(
    const RectangularProgram& program,
    cl::CommandQueue& queue,
    const MeshBoundary& boundary,
    const glm::vec3& anchor,
    float sr)
        : RectangularWaveguide(
              program,
              queue,
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
    cl::CommandQueue& queue,
    const RectangularMesh& mesh,
    float sample_rate,
    std::vector<RectangularProgram::CanonicalCoefficients> coefficients)
        : RectangularWaveguide(program,
                               queue,
                               mesh,
                               sample_rate,
                               mesh.get_condensed_nodes(),
                               mesh.get_boundary_data<1>(),
                               mesh.get_boundary_data<2>(),
                               mesh.get_boundary_data<3>(),
                               coefficients) {
}

template <BufferType buffer_type>
RectangularWaveguide<buffer_type>::RectangularWaveguide(
    const typename Base::ProgramType& program,
    cl::CommandQueue& queue,
    const RectangularMesh& mesh,
    float sample_rate,
    std::vector<RectangularMesh::CondensedNode> nodes,
    std::vector<RectangularProgram::BoundaryDataArray1> boundary_data_1,
    std::vector<RectangularProgram::BoundaryDataArray2> boundary_data_2,
    std::vector<RectangularProgram::BoundaryDataArray3> boundary_data_3,
    std::vector<RectangularProgram::CanonicalCoefficients> coefficients)
        : Waveguide<RectangularProgram, buffer_type>(
              program, queue, mesh.get_nodes().size(), sample_rate)
        , mesh(mesh)
        , node_buffer(program.template getInfo<CL_PROGRAM_CONTEXT>(),
                      nodes.begin(),
                      nodes.end(),
                      false)
        , transform_buffer(program.template getInfo<CL_PROGRAM_CONTEXT>(),
                           CL_MEM_READ_WRITE,
                           sizeof(cl_float) * TRANSFORM_MATRIX_ELEMENTS)
        , velocity_buffer(program.template getInfo<CL_PROGRAM_CONTEXT>(),
                          CL_MEM_READ_WRITE,
                          sizeof(cl_float3) * 1)
        , num_boundary_1(boundary_data_1.size())
        , boundary_data_1_buffer(program.template getInfo<CL_PROGRAM_CONTEXT>(),
                                 boundary_data_1.begin(),
                                 boundary_data_1.end(),
                                 false)
        , num_boundary_2(boundary_data_2.size())
        , boundary_data_2_buffer(program.template getInfo<CL_PROGRAM_CONTEXT>(),
                                 boundary_data_2.begin(),
                                 boundary_data_2.end(),
                                 false)
        , num_boundary_3(boundary_data_3.size())
        , boundary_data_3_buffer(program.template getInfo<CL_PROGRAM_CONTEXT>(),
                                 boundary_data_3.begin(),
                                 boundary_data_3.end(),
                                 false)
        , boundary_coefficients_buffer(
              program.template getInfo<CL_PROGRAM_CONTEXT>(),
              coefficients.begin(),
              coefficients.end(),
              false)
        , error_flag_buffer(program.template getInfo<CL_PROGRAM_CONTEXT>(),
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
    auto transform_matrix = get_transform_matrix(PORTS, o, mesh.get_nodes());
    cl::copy(queue,
             transform_matrix.data(),
             transform_matrix.data() + TRANSFORM_MATRIX_ELEMENTS,
             transform_buffer);

    std::vector<cl_float3> starting_velocity{{{0, 0, 0, 0}}};
    cl::copy(queue,
             starting_velocity.begin(),
             starting_velocity.end(),
             velocity_buffer);
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
    std::vector<cl_float> out(1);
    std::vector<cl_float3> current_velocity(1);

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
           boundary_data_1_buffer,
           boundary_data_2_buffer,
           boundary_data_3_buffer,
           boundary_coefficients_buffer,
           transform_buffer,
           velocity_buffer,
           mesh.get_spacing(),
           this->get_period(),
           o,
           output,
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

    cl::copy(queue, output, out.begin(), out.end());
    cl::copy(queue,
             velocity_buffer,
             current_velocity.begin(),
             current_velocity.end());

    auto velocity = to_vec3f(current_velocity.front());
    auto intensity = velocity * out.front();

    return RunStepResult(out.front(), intensity);
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
