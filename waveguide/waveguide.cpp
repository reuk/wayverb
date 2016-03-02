#include "waveguide.h"
#include "test_flag.h"
#include "conversions.h"

#include <iostream>
#include <algorithm>
#include <cstdlib>

//----------------------------------------------------------------------------//

std::ostream& operator<<(std::ostream& os, const cl_float3& f) {
    Bracketer bracketer(os);
    return to_stream(os, f.s[0], "  ", f.s[1], "  ", f.s[2], "  ");
}

std::ostream& operator<<(std::ostream& os,
                         const RectangularProgram::CanonicalMemory& m) {
    Bracketer bracketer(os);
    for (const auto& i : m.array)
        to_stream(os, i, "  ");
    return os;
}

std::ostream& operator<<(std::ostream& os,
                         const RectangularProgram::BoundaryData& m) {
    return to_stream(os, m.filter_memory);
}

template <int I>
std::ostream& operator<<(std::ostream& os,
                         const RectangularProgram::BoundaryDataArray<I>& bda) {
    Bracketer bracketer(os);
    for (const auto& i : bda.array)
        to_stream(os, i);
    return os;
}

std::ostream& operator<<(std::ostream& os,
                         const RectangularProgram::CondensedNodeStruct& cns) {
    Bracketer bracketer(os);
    return to_stream(
        os, "bt: ", cns.bt, "  boundary_index: ", cns.boundary_index);
}

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

RunStepResult TetrahedralWaveguide::run_step(size_type o,
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
        : RectangularWaveguide(
              program, queue, mesh, mesh.get_condensed_nodes(), coefficients) {
}

RectangularWaveguide::RectangularWaveguide(
    const RectangularProgram& program,
    cl::CommandQueue& queue,
    const RectangularMesh& mesh,
    std::vector<RectangularMesh::CondensedNode> nodes,
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
        , boundary_data_1_buffer(
              program.getInfo<CL_PROGRAM_CONTEXT>(),
              CL_MEM_READ_WRITE,
              sizeof(RectangularProgram::BoundaryDataArray1) *
                  mesh.compute_num_boundary<1>())
        , boundary_data_2_buffer(
              program.getInfo<CL_PROGRAM_CONTEXT>(),
              CL_MEM_READ_WRITE,
              sizeof(RectangularProgram::BoundaryDataArray2) *
                  mesh.compute_num_boundary<2>())
        , boundary_data_3_buffer(
              program.getInfo<CL_PROGRAM_CONTEXT>(),
              CL_MEM_READ_WRITE,
              sizeof(RectangularProgram::BoundaryDataArray3) *
                  mesh.compute_num_boundary<3>())
        , boundary_coefficients_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                                       coefficients.begin(),
                                       coefficients.end(),
                                       false)
        , debug_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                       CL_MEM_READ_WRITE,
                       sizeof(cl_float) * nodes.size()) {
}

RectangularWaveguide::RectangularWaveguide(
    const RectangularProgram& program,
    cl::CommandQueue& queue,
    const Boundary& boundary,
    float spacing,
    const Vec3f& anchor,
    const RectangularProgram::CanonicalCoefficients& coefficients)
        : RectangularWaveguide(program,
                               queue,
                               RectangularMesh(boundary, spacing, anchor),
                               {coefficients}) {
}

void RectangularWaveguide::setup(cl::CommandQueue& queue,
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

    //  TODO set boundary data structures properly
    setup_boundary_data_buffer<1>(queue, boundary_data_1_buffer);
    setup_boundary_data_buffer<2>(queue, boundary_data_2_buffer);
    setup_boundary_data_buffer<3>(queue, boundary_data_3_buffer);

    period = 1 / sr;
}

RunStepResult RectangularWaveguide::run_step(size_type o,
                                             cl::CommandQueue& queue,
                                             kernel_type& kernel,
                                             size_type nodes,
                                             cl::Buffer& previous,
                                             cl::Buffer& current,
                                             cl::Buffer& output) {
    std::vector<cl_float> out(1);
    std::vector<cl_float3> current_velocity(1);

    //  TODO remove this
    std::vector<cl_float> debug(nodes, 0);
    cl::copy(queue, debug.begin(), debug.end(), debug_buffer);

    kernel(cl::EnqueueArgs(queue, cl::NDRange(nodes)),
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
           debug_buffer);

    cl::copy(queue, output, out.begin(), out.end());
    cl::copy(queue,
             velocity_buffer,
             current_velocity.begin(),
             current_velocity.end());

    {
        //  TODO remove this bit
        std::vector<RectangularProgram::BoundaryDataArray1> bda(
            mesh.compute_num_boundary<1>());
        cl::copy(queue, boundary_data_1_buffer, bda.begin(), bda.end());
        log_nan_or_nonzero(bda, "filter memory (1)");
    }

    {
        //  TODO remove this bit
        std::vector<RectangularProgram::BoundaryDataArray2> bda(
            mesh.compute_num_boundary<2>());
        cl::copy(queue, boundary_data_2_buffer, bda.begin(), bda.end());
        log_nan_or_nonzero(bda, "filter memory (2)");
    }

    {
        //  TODO remove this bit
        std::vector<RectangularProgram::BoundaryDataArray3> bda(
            mesh.compute_num_boundary<3>());
        cl::copy(queue, boundary_data_3_buffer, bda.begin(), bda.end());
        log_nan_or_nonzero(bda, "filter memory (3)");
    }

    {
        //  TODO remove this bit too
        cl::copy(queue, debug_buffer, debug.begin(), debug.end());
        log_nan_or_nonzero(debug, "debug");
    }

    {
        //  TODO remove this bit also
        std::vector<cl_float> ret(nodes, 0);
        cl::copy(queue, previous, ret.begin(), ret.end());
        log_nan_or_nonzero(ret, "pressure");
    }

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
