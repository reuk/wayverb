#include "waveguide.h"
#include "test_flag.h"
#include "conversions.h"

#include <iostream>
#include <algorithm>
#include <cstdlib>

template <typename T>
T pinv(const T& a, float epsilon = std::numeric_limits<float>::epsilon()) {
    //  taken from http://eigen.tuxfamily.org/bz/show_bug.cgi?id=257
    Eigen::JacobiSVD<T> svd(a, Eigen::ComputeThinU | Eigen::ComputeThinV);
    auto tolerance = epsilon * std::max(a.cols(), a.rows()) *
                     svd.singularValues().array().abs()(0);
    return svd.matrixV() *
           (svd.singularValues().array().abs() > tolerance)
               .select(svd.singularValues().array().inverse(), 0)
               .matrix()
               .asDiagonal() *
           svd.matrixU().adjoint();
}

void TetrahedralWaveguide::setup(cl::CommandQueue& queue,
                                 size_type o,
                                 float sr) {
    Eigen::MatrixXf umat(4, 3);
    auto count = 0u;
    auto basis = convert(mesh.get_nodes()[o].position);
    for (const auto& i : mesh.get_nodes()[o].ports) {
        auto pos = (convert(mesh.get_nodes()[i].position) - basis).normalized();
        umat.row(count++) << pos.x, pos.y, pos.z;
    }

    transform_matrix = pinv(umat);
    Logger::log("transform_matrix: ", transform_matrix);
    cl::copy(queue,
             transform_matrix.data(),
             transform_matrix.data() + 12,
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
                                           const IterativeTetrahedralMesh& mesh)
        : Waveguide<TetrahedralProgram>(program, queue, mesh.get_nodes().size())
        , mesh(mesh)
        //    TODO this seems like it's asking for problems
        , node_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                      const_cast<Node*>(this->mesh.get_nodes().data()),
                      const_cast<Node*>(this->mesh.get_nodes().data()) +
                          this->mesh.get_nodes().size(),
                      true)
        , transform_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                           CL_MEM_READ_WRITE,
                           sizeof(cl_float) * 12)
        , velocity_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                          CL_MEM_READ_WRITE,
                          sizeof(cl_float3) * 1) {
#ifdef TESTING
    auto fname = build_string("./file-positions.txt");
    ofstream file(fname);
    for (const auto& i : nodes) {
        file << build_string(i.position.x,
                             " ",
                             i.position.y,
                             " ",
                             i.position.z,
                             " ",
                             i.inside)
             << endl;
    }
#endif
}

TetrahedralWaveguide::TetrahedralWaveguide(const TetrahedralProgram& program,
                                           cl::CommandQueue& queue,
                                           const Boundary& boundary,
                                           float spacing)
        : TetrahedralWaveguide(
              program, queue, IterativeTetrahedralMesh(boundary, spacing)) {
}

RunStepResult TetrahedralWaveguide::run_step(size_type o,
                                             cl::CommandQueue& queue,
                                             kernel_type& kernel,
                                             size_type nodes,
                                             cl::Buffer& previous,
                                             cl::Buffer& current,
                                             cl::Buffer& output) {
    if (o > this->mesh.get_nodes().size()) {
        throw std::runtime_error("requested output node does not exist");
    }

    if (!this->mesh.get_nodes()[o].inside) {
        throw std::runtime_error("requested output node is outside boundary");
    }

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

    auto velocity = convert(current_velocity.front());
    auto intensity = velocity * out.front();

    Logger::log("velocity: ", velocity);
    Logger::log("intensity: ", intensity);

#ifdef TESTING
    static size_type ind = 0;

    vector<cl_float> node_values(nodes);
    cl::copy(queue, previous, node_values.begin(), node_values.end());
    auto fname = build_string("./file-", ind++, ".txt");
    ofstream file(fname);
    for (auto j = 0u; j != nodes; ++j) {
        file << build_string(node_values[j]) << endl;
    }
#endif

    return RunStepResult(out.front(), intensity);
}

TetrahedralWaveguide::size_type TetrahedralWaveguide::get_index_for_coordinate(
    const Vec3f& v) const {
    return mesh.get_index(mesh.get_locator(v));
}

Vec3f TetrahedralWaveguide::get_coordinate_for_index(size_type index) const {
    return convert(mesh.get_nodes()[index].position);
}
