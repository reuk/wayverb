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

std::ostream& operator<<(std::ostream& os,
                         const RectangularProgram::BoundaryDataArray1& bda) {
    Bracketer bracketer(os);
    return to_stream(os, bda.array[0]);
}

void TetrahedralWaveguide::setup(cl::CommandQueue& queue,
                                 size_type o,
                                 float sr) {
    auto PORTS = decltype(mesh)::PORTS;
    transform_matrix = get_transform_matrix(PORTS, o, mesh.get_nodes());
    cl::copy(queue,
             transform_matrix.data(),
             transform_matrix.data() + PORTS * 3,
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
                           sizeof(cl_float) * 12)
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

RectangularWaveguide::RectangularWaveguide(const RectangularProgram& program,
                                           cl::CommandQueue& queue,
                                           const RectangularMesh& mesh)
        : RectangularWaveguide(
              program, queue, mesh, mesh.get_condensed_nodes()) {
}

RectangularWaveguide::RectangularWaveguide(
    const RectangularProgram& program,
    cl::CommandQueue& queue,
    const RectangularMesh& mesh,
    std::vector<RectangularMesh::CondensedNode> nodes) try : Waveguide
    <RectangularProgram>(program, queue, mesh.get_nodes().size()), mesh(mesh),
        node_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                    nodes.begin(),
                    nodes.end(),
                    false),
        transform_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                         CL_MEM_READ_WRITE,
                         sizeof(cl_float) * 18),
        velocity_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                        CL_MEM_READ_WRITE,
                        sizeof(cl_float3) * 1),
        boundary_data_1_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(RectangularProgram::BoundaryDataArray1) *
                                   mesh.compute_num_boundary<1>()),
        boundary_data_2_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(RectangularProgram::BoundaryDataArray2) *
                                   mesh.compute_num_boundary<2>()),
        boundary_data_3_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(RectangularProgram::BoundaryDataArray3) *
                                   mesh.compute_num_boundary<3>()),
        boundary_coefficients_buffer(
            program.getInfo<CL_PROGRAM_CONTEXT>(),
            CL_MEM_READ_WRITE,
            sizeof(RectangularProgram::CanonicalCoefficients) *
                mesh.compute_num_surface()),
        debug_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                     CL_MEM_READ_WRITE,
                     sizeof(cl_float) * nodes.size()) {
    }
catch (const cl::Error& e) {
    ::Logger::log_err(e.what());
    throw;
}

RectangularWaveguide::RectangularWaveguide(const RectangularProgram& program,
                                           cl::CommandQueue& queue,
                                           const Boundary& boundary,
                                           float spacing,
                                           const Vec3f& anchor)
        : RectangularWaveguide(
              program, queue, RectangularMesh(boundary, spacing, anchor)) {
}

void RectangularWaveguide::setup(cl::CommandQueue& queue,
                                 size_type o,
                                 float sr) {
    auto PORTS = decltype(mesh)::PORTS;
    transform_matrix = get_transform_matrix(PORTS, o, mesh.get_nodes());
    cl::copy(queue,
             transform_matrix.data(),
             transform_matrix.data() + PORTS * 3,
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

    //  TODO set coefficients properly
    cl::copy(queue,
             boundary_coefficients.begin(),
             boundary_coefficients.end(),
             boundary_coefficients_buffer);

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
        std::vector<RectangularProgram::BoundaryDataArray1> bda(
            mesh.compute_num_boundary<1>());
        cl::copy(queue, boundary_data_1_buffer, bda.begin(), bda.end());
        //        Logger::log_err("memory index 0: ", bda[0]);
        {
            auto it = std::find_if(
                bda.begin(),
                bda.end(),
                [](const auto& i) {
                    const auto& array = i.array[0].filter_memory.array;
                    return std::any_of(std::begin(array),
                                       std::end(array),
                                       [](auto i) { return std::isnan(i); });
                });

            if (it != bda.end()) {
                Logger::log_err("nan filter memory value at index: ",
                                it - bda.begin());
                Logger::log_err("memory: ", *it);
            }
        }
        {
            auto it = std::find_if(
                bda.begin(),
                bda.end(),
                [](const auto& i) {
                    const auto& array = i.array[0].filter_memory.array;
                    return std::any_of(std::begin(array),
                                       std::end(array),
                                       [](auto i) { return i; });
                });
            if (it != bda.end()) {
                Logger::log_err("nonzero filter memory value at index: ",
                                it - bda.begin());
                Logger::log_err("memory: ", *it);
            }
        }
    }

    {
        //  TODO remove this bit too
        cl::copy(queue, debug_buffer, debug.begin(), debug.end());
        auto it = std::find_if(
            std::begin(debug), std::end(debug), [](auto x) { return x; });
        if (it != debug.end()) {
            Logger::log_err("nonzero debug value: ",
                            *it,
                            " at index: ",
                            it - debug.begin());
        }
    }

    {
        //  TODO remove this bit too
        std::vector<cl_float> ret(nodes, 0);
        cl::copy(queue, previous, ret.begin(), ret.end());
        auto is_nan = std::find_if(
            ret.begin(), ret.end(), [](auto i) { return std::isnan(i); });
        if (is_nan != ret.end()) {
            Logger::log_err("nan pressure value: ",
                            *is_nan,
                            " at index: ",
                            is_nan - ret.begin());
        }
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

void RectangularWaveguide::set_boundary_coefficient(
    const RectangularProgram::CanonicalCoefficients& coefficients) {
    boundary_coefficients =
        std::vector<RectangularProgram::CanonicalCoefficients>(1, coefficients);
}
