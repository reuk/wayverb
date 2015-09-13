#include "waveguide.h"

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include "logger.h"

using namespace std;

RectangularWaveguide::RectangularWaveguide(const RectangularProgram & program,
                                           cl::CommandQueue & queue,
                                           cl_int3 p)
        : queue(queue)
        , kernel(program.get_kernel())
        , p(p)
        , storage({{cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * p.x * p.y * p.z),
                    cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * p.x * p.y * p.z),
                    cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * p.x * p.y * p.z)}})
        , previous(storage[0])
        , current(storage[1])
        , next(storage[2])
        , output(program.getInfo<CL_PROGRAM_CONTEXT>(),
                 CL_MEM_READ_WRITE,
                 sizeof(cl_float)) {
}

RectangularWaveguide::size_type RectangularWaveguide::get_index(
    cl_int3 pos) const {
    return pos.x + pos.y * p.x + pos.z * p.x * p.y;
}

vector<cl_float> RectangularWaveguide::run(vector<float> input,
                                           cl_int3 e,
                                           cl_int3 o,
                                           cl_float attenuation,
                                           int steps) {
    vector<cl_float> nodes(p.x * p.y * p.z, 0);
    cl::copy(queue, nodes.begin(), nodes.end(), next);
    cl::copy(queue, nodes.begin(), nodes.end(), current);
    cl::copy(queue, nodes.begin(), nodes.end(), previous);

    input.resize(steps, 0);

    vector<cl_float> ret(input.size());
    vector<cl_float> out(1);

    transform(input.begin(),
              input.end(),
              ret.begin(),
              [this, &attenuation, &e, &o, &out](auto i) {
                  kernel(cl::EnqueueArgs(queue, cl::NDRange(p.x, p.y, p.z)),
                         this->get_index(e),
                         i,
                         attenuation,
                         next,
                         current,
                         previous,
                         -1,
                         this->get_index(o),
                         output);

                  cl::copy(queue, output, out.begin(), out.end());

                  auto & temp = previous;
                  previous = current;
                  current = next;
                  next = temp;

                  return out.front();
              });

    return ret;
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
    std::vector<LinkedTetrahedralNode> nodes)
        : queue(queue)
        , kernel(program.get_kernel())
#ifdef TESTING
        , nodes(nodes)
#endif
        , node_size(nodes.size())
        , node_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                      nodes.begin(),
                      nodes.end(),
                      true,
                      false)
        , storage({{cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * node_size),
                    cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * node_size),
                    cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * node_size)}})
        , previous(storage[0])
        , current(storage[1])
        , next(storage[2])
        , output(program.getInfo<CL_PROGRAM_CONTEXT>(),
                 CL_MEM_READ_WRITE,
                 sizeof(cl_float)) {
}

vector<cl_float> RecursiveTetrahedralWaveguide::run(std::vector<float> input,
                                                    size_type e,
                                                    size_type o,
                                                    cl_float attenuation,
                                                    int steps) {
    vector<cl_float> node_values(node_size, 0);
    cl::copy(queue, node_values.begin(), node_values.end(), next);
    cl::copy(queue, node_values.begin(), node_values.end(), current);
    cl::copy(queue, node_values.begin(), node_values.end(), previous);

    input.resize(steps, 0);

    vector<cl_float> ret(input.size());
    vector<cl_float> out(1);

    auto ind = 0;
    transform(input.begin(),
              input.end(),
              ret.begin(),
              [this, &attenuation, &ind, &node_values, &e, &o, &out](auto i) {
                  kernel(cl::EnqueueArgs(queue, cl::NDRange(node_size)),
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
                  cl::copy(queue, next, node_values.begin(), node_values.end());
                  auto fname = build_string("./file-", ind++, ".txt");
                  cout << "writing file " << fname << endl;
                  ofstream file(fname);
                  for (auto j = 0u; j != nodes.size(); ++j) {
                      const auto & n = nodes[j];
                      file << n.position.x << " " << n.position.y << " "
                           << n.position.z << " " << node_values[j] << endl;
                  }
#endif

                  auto & temp = previous;
                  previous = current;
                  current = next;
                  next = temp;

                  return out.front();
              });

    return ret;
}

IterativeTetrahedralWaveguide::IterativeTetrahedralWaveguide(
    const IterativeTetrahedralProgram & program,
    cl::CommandQueue & queue,
    const Boundary & boundary,
    float cube_side)
        : queue(queue)
        , kernel(program.get_kernel())
        , mesh(boundary, cube_side)
        , node_size(mesh.nodes.size())
        , node_buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                      mesh.nodes.begin(),
                      mesh.nodes.end(),
                      true,
                      false)
        , storage({{cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * node_size),
                    cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * node_size),
                    cl::Buffer(program.getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * node_size)}})
        , previous(storage[0])
        , current(storage[1])
        , next(storage[2])
        , output(program.getInfo<CL_PROGRAM_CONTEXT>(),
                 CL_MEM_READ_WRITE,
                 sizeof(cl_float)) {
}

vector<cl_float> IterativeTetrahedralWaveguide::run(std::vector<float> input,
                                                    size_type e,
                                                    size_type o,
                                                    cl_float attenuation,
                                                    int steps) {
    vector<cl_float> node_values(node_size, 0);
    cl::copy(queue, node_values.begin(), node_values.end(), next);
    cl::copy(queue, node_values.begin(), node_values.end(), current);
    cl::copy(queue, node_values.begin(), node_values.end(), previous);

    input.resize(steps, 0);

    vector<cl_float> ret(input.size());
    vector<cl_float> out(1);

    auto ind = 0;
    transform(input.begin(),
              input.end(),
              ret.begin(),
              [this, &attenuation, &ind, &node_values, &e, &o, &out](auto i) {
                  kernel(cl::EnqueueArgs(queue, cl::NDRange(node_size)),
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
                  cl::copy(queue, next, node_values.begin(), node_values.end());
                  auto fname = build_string("./file-", ind++, ".txt");
                  cout << "writing file " << fname << endl;
                  ofstream file(fname);
                  for (auto j = 0u; j != nodes.size(); ++j) {
                      const auto & n = nodes[j];
                      file << n.position.x << " " << n.position.y << " "
                           << n.position.z << " " << node_values[j] << endl;
                  }
#endif

                  auto & temp = previous;
                  previous = current;
                  current = next;
                  next = temp;

                  return out.front();
              });

    return ret;
}
