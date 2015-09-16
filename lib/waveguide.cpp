#include "waveguide.h"

#include <iostream>
#include <algorithm>
#include <cstdlib>

using namespace std;

RectangularWaveguide::RectangularWaveguide(const RectangularProgram & program,
                                           cl::CommandQueue & queue,
                                           cl_int3 p)
        : Waveguide(program, queue, p.x * p.y * p.z)
        , p(p) {
}

RectangularWaveguide::size_type RectangularWaveguide::get_index(
    cl_int3 pos) const {
    return pos.x + pos.y * p.x + pos.z * p.x * p.y;
}

Vec3f RectangularWaveguide::get_node_position(size_type index) const {
    //  TODO yeah this isn't right at all
    return index;
}
bool RectangularWaveguide::get_node_inside(size_type index) const {
    return index < get_nodes();
}

cl_float RectangularWaveguide::run_step(cl_float i,
                                        int e,
                                        int o,
                                        cl_float attenuation,
                                        cl::CommandQueue & queue,
                                        kernel_type & kernel,
                                        int nodes,
                                        cl::Buffer & previous,
                                        cl::Buffer & current,
                                        cl::Buffer & next,
                                        cl::Buffer & output) {
    vector<cl_float> out(1);

    kernel(cl::EnqueueArgs(queue, cl::NDRange(p.x, p.y, p.z)),
           e,
           i,
           attenuation,
           next,
           current,
           previous,
           -1,
           o,
           output);

    cl::copy(queue, output, out.begin(), out.end());

    return out.front();
}

RecursiveTetrahedralWaveguide::RecursiveTetrahedralWaveguide(
    const RecursiveTetrahedralProgram & program,
    cl::CommandQueue & queue,
    const Boundary & boundary,
    Vec3f start,
    float spacing)
        : RecursiveTetrahedralWaveguide(
              program, queue, tetrahedral_mesh(boundary, start, spacing)) {
}

RecursiveTetrahedralWaveguide::RecursiveTetrahedralWaveguide(
    const RecursiveTetrahedralProgram & program,
    cl::CommandQueue & queue,
    vector<TetrahedralNode> nodes)
        : Waveguide(program, queue, nodes.size())
        , nodes(nodes)
        , node_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                      nodes.begin(),
                      nodes.end(),
                      false) {
}

Vec3f RecursiveTetrahedralWaveguide::get_node_position(size_type index) const {
    auto p = nodes[index].position;
    return Vec3f(p.x, p.y, p.z);
}
bool RecursiveTetrahedralWaveguide::get_node_inside(size_type index) const {
    return index < get_nodes();
}

cl_float RecursiveTetrahedralWaveguide::run_step(cl_float i,
                                                 int e,
                                                 int o,
                                                 cl_float attenuation,
                                                 cl::CommandQueue & queue,
                                                 kernel_type & kernel,
                                                 int nodes,
                                                 cl::Buffer & previous,
                                                 cl::Buffer & current,
                                                 cl::Buffer & next,
                                                 cl::Buffer & output) {
    vector<cl_float> out(1);

    kernel(cl::EnqueueArgs(queue, cl::NDRange(nodes)),
           e,
           i,
           attenuation,
           next,
           current,
           previous,
           node_buffer,
           o,
           output);

    cl::copy(queue, output, out.begin(), out.end());

#ifdef TESTING
    static int ind = 0;

    vector<cl_float> node_values(nodes);
    cl::copy(queue, next, node_values.begin(), node_values.end());
    auto fname = build_string("./file-", ind++, ".txt");
    cout << "writing file " << fname << endl;
    ofstream file(fname);
    for (auto j = 0u; j != nodes; ++j) {
        file << node_values[j] << endl;
    }
#endif

    return out.front();
}

IterativeTetrahedralWaveguide::IterativeTetrahedralWaveguide(
    const IterativeTetrahedralProgram & program,
    cl::CommandQueue & queue,
    const Boundary & boundary,
    float cube_side)
        : IterativeTetrahedralWaveguide(
              program, queue, IterativeTetrahedralMesh(boundary, cube_side)) {
}

IterativeTetrahedralWaveguide::IterativeTetrahedralWaveguide(
    const IterativeTetrahedralProgram & program,
    cl::CommandQueue & queue,
    IterativeTetrahedralMesh mesh)
        : Waveguide(program, queue, mesh.nodes.size())
        , mesh(mesh)
        , node_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                      mesh.nodes.begin(),
                      mesh.nodes.end(),
                      false)
{
}

Vec3f IterativeTetrahedralWaveguide::get_node_position(size_type index) const {
    auto p = mesh.nodes[index].position;
    return Vec3f(p.x, p.y, p.z);
}
bool IterativeTetrahedralWaveguide::get_node_inside(size_type index) const {
    return mesh.nodes[index].inside;
}

cl_float IterativeTetrahedralWaveguide::run_step(cl_float i,
                                                 int e,
                                                 int o,
                                                 cl_float attenuation,
                                                 cl::CommandQueue & queue,
                                                 kernel_type & kernel,
                                                 int nodes,
                                                 cl::Buffer & previous,
                                                 cl::Buffer & current,
                                                 cl::Buffer & next,
                                                 cl::Buffer & output) {
    vector<cl_float> out(1);

    kernel(cl::EnqueueArgs(queue, cl::NDRange(nodes)),
           e,
           i,
           attenuation,
           next,
           current,
           previous,
           node_buffer,
           cl_int3{{mesh.dim.x, mesh.dim.y, mesh.dim.z}},
           o,
           output);

    cl::copy(queue, output, out.begin(), out.end());

#ifdef TESTING
    static int ind = 0;

    vector<cl_float> node_values(nodes);
    cl::copy(queue, next, node_values.begin(), node_values.end());
    auto fname = build_string("./file-", ind++, ".txt");
    cout << "writing file " << fname << endl;
    ofstream file(fname);
    for (auto j = 0u; j != nodes; ++j) {
        if (get_node_inside(j)) {
            file << node_values[j] << endl;
        }
    }
#endif

    return out.front();
}
