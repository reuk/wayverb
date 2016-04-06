#include "waveguide.h"
#include "test_flag.h"
#include "conversions.h"
#include "hrtf.h"
#include "db.h"
#include "stl_wrappers.h"
#include "sinc.h"

#include <iostream>
#include <algorithm>
#include <cstdlib>

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
        , boundary_data_1_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                                 boundary_data_1.begin(),
                                 boundary_data_1.end(),
                                 false)
        , boundary_data_2_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                                 boundary_data_2.begin(),
                                 boundary_data_2.end(),
                                 false)
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

    auto flag = RectangularProgram::id_success;
    cl::copy(queue, (&flag) + 0, (&flag) + 1, error_flag_buffer);

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
           error_flag_buffer);

    cl::copy(queue, error_flag_buffer, (&flag) + 0, (&flag) + 1);

    //    if (flag & RectangularProgram::id_outside_range_error)
    //        throw std::runtime_error("pressure value is outside valid range");

    if (flag & RectangularProgram::id_inf_error)
        throw std::runtime_error(
            "pressure value is inf, check filter coefficients");

    if (flag & RectangularProgram::id_nan_error)
        throw std::runtime_error(
            "pressure value is nan, check filter coefficients");

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

RectangularProgram::CanonicalCoefficients
RectangularWaveguide::to_filter_coefficients(const Surface& surface, float sr) {
    std::array<RectangularProgram::FilterDescriptor,
               RectangularProgram::BiquadCoefficientsArray::BIQUAD_SECTIONS>
        descriptors;

    //  for each descriptor
    for (auto i = 0u; i != descriptors.size(); ++i) {
        //  compute gain and centre frequency for notch
        float gain = a2db((surface.specular.s[i] + surface.diffuse.s[i]) / 2);
        auto centre = (HrtfData::EDGES[i + 0] + HrtfData::EDGES[i + 1]) / 2;
        //  produce a filter descriptor struct for this filter
        descriptors[i] =
            RectangularProgram::FilterDescriptor{gain, centre, 1.414};
    }
    //  transform filter parameters into a set of biquad coefficients
    auto individual_coeffs =
        RectangularProgram::get_peak_biquads_array(descriptors, sr);
    //  combine biquad coefficients into coefficients for a single high-order
    //  filter
    auto ret = RectangularProgram::convolve(individual_coeffs);

    //  transform from reflection filter to impedance filter
    ret = RectangularProgram::to_impedance_coefficients(ret);

    if (!RectangularProgram::is_stable(ret))
        LOG(INFO)
            << "warning: impedance filter coefficients may produce unstable "
               "filter";

    return ret;
}

std::vector<RectangularProgram::CanonicalCoefficients>
RectangularWaveguide::to_filter_coefficients(std::vector<Surface> surfaces,
                                             float sr) {
    surfaces.resize(
        RectangularProgram::BiquadCoefficientsArray::BIQUAD_SECTIONS);

    std::vector<RectangularProgram::CanonicalCoefficients> ret(surfaces.size());
    proc::transform(surfaces,
                    ret.begin(),
                    [sr](auto i) { return to_filter_coefficients(i, sr); });
    return ret;
}
