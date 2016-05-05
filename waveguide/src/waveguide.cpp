#include "waveguide/waveguide.h"

#include "common/conversions.h"
#include "common/sinc.h"
#include "common/stl_wrappers.h"

#include "glog/logging.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>

//----------------------------------------------------------------------------//

void TetrahedralWaveguide::setup(cl::CommandQueue& queue,
                                 size_type o,
                                 float sr) {
    auto transform_matrix = get_transform_matrix(PORTS, o, mesh.get_nodes());
    cl::copy(queue,
             transform_matrix.data(),
             transform_matrix.data() + TRANSFORM_MATRIX_ELEMENTS,
             transform_buffer);

    std::vector<cl_float3> starting_velocity(1, {{0, 0, 0, 0}});
    cl::copy(queue,
             starting_velocity.begin(),
             starting_velocity.end(),
             velocity_buffer);

    period = 1 / sr;
}

TetrahedralWaveguide::TetrahedralWaveguide(const TetrahedralProgram& program,
                                           cl::CommandQueue& queue,
                                           const TetrahedralMesh& mesh)
        : TetrahedralWaveguide(program, queue, mesh, mesh.get_nodes()) {
}

TetrahedralWaveguide::TetrahedralWaveguide(
    const TetrahedralProgram& program,
    cl::CommandQueue& queue,
    const TetrahedralMesh& mesh,
    std::vector<TetrahedralMesh::Node> nodes)
        : Waveguide<TetrahedralProgram>(program, queue, mesh.get_nodes().size())
        , mesh(mesh)
        , node_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                      nodes.begin(),
                      nodes.end(),
                      false)
        , transform_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                           CL_MEM_READ_WRITE,
                           sizeof(cl_float) * TRANSFORM_MATRIX_ELEMENTS)
        , velocity_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                          CL_MEM_READ_WRITE,
                          sizeof(cl_float3) * 1) {
}

TetrahedralWaveguide::TetrahedralWaveguide(const TetrahedralProgram& program,
                                           cl::CommandQueue& queue,
                                           const Boundary& boundary,
                                           float spacing,
                                           const Vec3f& anchor)
        : TetrahedralWaveguide(
              program, queue, TetrahedralMesh(boundary, spacing, anchor)) {
}

RunStepResult TetrahedralWaveguide::run_step(const WriteInfo& write_info,
                                             size_type o,
                                             cl::CommandQueue& queue,
                                             kernel_type& kernel,
                                             size_type nodes,
                                             cl::Buffer& previous,
                                             cl::Buffer& current,
                                             cl::Buffer& output) {
    std::vector<cl_float> out(1);
    std::vector<cl_float3> current_velocity(1);

    kernel(cl::EnqueueArgs(queue, cl::NDRange(nodes)),
           current,
           previous,
           node_buffer,
           transform_buffer,
           velocity_buffer,
           mesh.get_spacing(),
           period,
           o,
           output);

    cl::copy(queue, output, out.begin(), out.end());
    cl::copy(queue,
             velocity_buffer,
             current_velocity.begin(),
             current_velocity.end());

    auto velocity = to_vec3f(current_velocity.front());
    auto intensity = velocity * out.front();

    return RunStepResult(out.front(), intensity);
}

TetrahedralWaveguide::size_type TetrahedralWaveguide::get_index_for_coordinate(
    const Vec3f& v) const {
    return mesh.compute_index(mesh.compute_locator(v));
}

Vec3f TetrahedralWaveguide::get_coordinate_for_index(size_type index) const {
    return to_vec3f(mesh.get_nodes()[index].position);
}

const TetrahedralMesh& TetrahedralWaveguide::get_mesh() const {
    return mesh;
}

bool TetrahedralWaveguide::inside(size_type index) const {
    return mesh.get_nodes()[index].inside;
}

//----------------------------------------------------------------------------//

RectangularWaveguide::RectangularWaveguide(
    const RectangularProgram& program,
    cl::CommandQueue& queue,
    const RectangularMesh& mesh,
    std::vector<RectangularProgram::CanonicalCoefficients> coefficients)
        : RectangularWaveguide(program,
                               queue,
                               mesh,
                               mesh.get_condensed_nodes(),
                               mesh.get_boundary_data<1>(),
                               mesh.get_boundary_data<2>(),
                               mesh.get_boundary_data<3>(),
                               coefficients) {
}

RectangularWaveguide::RectangularWaveguide(
    const RectangularProgram& program,
    cl::CommandQueue& queue,
    const RectangularMesh& mesh,
    std::vector<RectangularMesh::CondensedNode> nodes,
    std::vector<RectangularProgram::BoundaryDataArray1> boundary_data_1,
    std::vector<RectangularProgram::BoundaryDataArray2> boundary_data_2,
    std::vector<RectangularProgram::BoundaryDataArray3> boundary_data_3,
    std::vector<RectangularProgram::CanonicalCoefficients> coefficients)
        : Waveguide<RectangularProgram>(program, queue, mesh.get_nodes().size())
        , mesh(mesh)
        , node_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                      nodes.begin(),
                      nodes.end(),
                      false)
        , transform_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                           CL_MEM_READ_WRITE,
                           sizeof(cl_float) * TRANSFORM_MATRIX_ELEMENTS)
        , velocity_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                          CL_MEM_READ_WRITE,
                          sizeof(cl_float3) * 1)
        , num_boundary_1(boundary_data_1.size())
        , boundary_data_1_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                                 boundary_data_1.begin(),
                                 boundary_data_1.end(),
                                 false)
        , num_boundary_2(boundary_data_2.size())
        , boundary_data_2_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                                 boundary_data_2.begin(),
                                 boundary_data_2.end(),
                                 false)
        , num_boundary_3(boundary_data_3.size())
        , boundary_data_3_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                                 boundary_data_3.begin(),
                                 boundary_data_3.end(),
                                 false)
        , boundary_coefficients_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                                       coefficients.begin(),
                                       coefficients.end(),
                                       false)
        , error_flag_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                            CL_MEM_READ_WRITE,
                            sizeof(cl_int)) {
    LOG(INFO) << "main memory node storage: "
              << (sizeof(RectangularProgram::NodeStruct) *
                      mesh.get_nodes().size() >>
                  20)
              << " MB";
}

void RectangularWaveguide::setup(cl::CommandQueue& queue,
                                 size_type o,
                                 float sr) {
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

    period = 1 / sr;
}

RunStepResult RectangularWaveguide::run_step(const WriteInfo& write_info,
                                             size_type o,
                                             cl::CommandQueue& queue,
                                             kernel_type& kernel,
                                             size_type nodes,
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
           period,
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

RectangularWaveguide::size_type RectangularWaveguide::get_index_for_coordinate(
    const Vec3f& v) const {
    return mesh.compute_index(mesh.compute_locator(v));
}

Vec3f RectangularWaveguide::get_coordinate_for_index(size_type index) const {
    return to_vec3f(mesh.get_nodes()[index].position);
}

const RectangularMesh& RectangularWaveguide::get_mesh() const {
    return mesh;
}

bool RectangularWaveguide::inside(size_type index) const {
    return mesh.get_nodes()[index].inside;
}

std::vector<RectangularProgram::CanonicalCoefficients>
RectangularWaveguide::to_filter_coefficients(std::vector<Surface> surfaces,
                                             float sr) {
    std::vector<RectangularProgram::CanonicalCoefficients> ret(surfaces.size());
    proc::transform(surfaces, ret.begin(), [sr](auto i) {
        return to_filter_coefficients(i, sr);
    });
    return ret;
}
