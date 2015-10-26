#include "waveguide.h"
#include "test_flag.h"
#include "conversions.h"

#include <iostream>
#include <algorithm>
#include <cstdlib>

using namespace std;

TetrahedralWaveguide::TetrahedralWaveguide(const TetrahedralProgram & program,
                                           cl::CommandQueue & queue,
                                           const std::vector<Node> & nodes)
        : Waveguide<TetrahedralProgram>(program, queue, nodes.size())
        , nodes(nodes)
        , node_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                      this->nodes.begin(),
                      this->nodes.end(),
                      true) {
#ifdef TESTING
    auto fname = build_string("./file-positions.txt");
    ofstream file(fname);
    for (const auto & i : nodes) {
        file << build_string(i.position.x,
                             " ",
                             i.position.y,
                             " ",
                             i.position.z,
                             " ",
                             i.inside) << endl;
    }
#endif
}

cl_float TetrahedralWaveguide::run_step(size_type o,
                                        cl::CommandQueue & queue,
                                        kernel_type & kernel,
                                        size_type nodes,
                                        cl::Buffer & previous,
                                        cl::Buffer & current,
                                        cl::Buffer & output) {
    if (o > this->nodes.size()) {
        throw runtime_error("requested output node does not exist");
    }

    if (!this->nodes[o].inside) {
        throw runtime_error("requested output node is outside boundary");
    }

    vector<cl_float> out(1);

    kernel(cl::EnqueueArgs(queue, cl::NDRange(nodes)),
           current,
           previous,
           node_buffer,
           o,
           output);

    cl::copy(queue, output, out.begin(), out.end());

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

    return out.front();
}

IterativeTetrahedralWaveguide::IterativeTetrahedralWaveguide(
    const TetrahedralProgram & program,
    cl::CommandQueue & queue,
    const Boundary & boundary,
    float cube_side)
        : IterativeTetrahedralWaveguide(
              program, queue, IterativeTetrahedralMesh(boundary, cube_side)) {
}

IterativeTetrahedralWaveguide::IterativeTetrahedralWaveguide(
    const TetrahedralProgram & program,
    cl::CommandQueue & queue,
    const IterativeTetrahedralMesh & mesh)
        : TetrahedralWaveguide(program, queue, mesh.nodes)
        , mesh(mesh) {
}

IterativeTetrahedralWaveguide::size_type
IterativeTetrahedralWaveguide::get_index_for_coordinate(const Vec3f & v) const {
    return mesh.get_index(mesh.get_locator(v));
}

Vec3f IterativeTetrahedralWaveguide::get_coordinate_for_index(
    size_type index) const {
    return convert(mesh.nodes[index].position);
}
