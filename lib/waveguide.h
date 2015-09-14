#pragma once

#include "rectangular_program.h"
#include "recursive_tetrahedral_program.h"
#include "iterative_tetrahedral_program.h"
#include "iterative_tetrahedral_mesh.h"
#include "recursive_tetrahedral.h"
#include "cl_structs.h"

#include <array>
#include <type_traits>
#include <algorithm>

//#define TESTING

template <typename T>
class Waveguide {
public:
    using size_type = std::vector<cl_float>::size_type;
    using kernel_type = decltype(std::declval<T>().get_kernel());

    Waveguide(const T & program, cl::CommandQueue & queue, int nodes)
            : queue(queue)
            , kernel(program.get_kernel())
            , nodes(nodes)
            , storage(
                  {{cl::Buffer(program.template getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * nodes),
                    cl::Buffer(program.template getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * nodes),
                    cl::Buffer(program.template getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * nodes)}})
            , previous(storage[0])
            , current(storage[1])
            , next(storage[2])
            , output(program.template getInfo<CL_PROGRAM_CONTEXT>(),
                     CL_MEM_READ_WRITE,
                     sizeof(cl_float)) {
    }

    virtual cl_float run_step(cl_float i,
                              int e,
                              int o,
                              cl_float attenuation,
                              cl::CommandQueue & queue,
                              kernel_type & kernel,
                              int nodes,
                              cl::Buffer & previous,
                              cl::Buffer & current,
                              cl::Buffer & next,
                              cl::Buffer & output) = 0;

    virtual std::vector<cl_float> run(std::vector<float> input,
                                      int e,
                                      int o,
                                      cl_float attenuation,
                                      int steps) {
        std::vector<cl_float> n(nodes, 0);
        cl::copy(queue, n.begin(), n.end(), next);
        cl::copy(queue, n.begin(), n.end(), current);
        cl::copy(queue, n.begin(), n.end(), previous);

        input.resize(steps, 0);

        std::vector<cl_float> ret(input.size());

        std::transform(input.begin(),
                       input.end(),
                       ret.begin(),
                       [this, &attenuation, &e, &o](auto i) {
                           auto ret = this->run_step(i,
                                                     e,
                                                     o,
                                                     attenuation,
                                                     queue,
                                                     kernel,
                                                     nodes,
                                                     previous,
                                                     current,
                                                     next,
                                                     output);
                           auto & temp = previous;
                           previous = current;
                           current = next;
                           next = temp;
                           return ret;
                       });

        return ret;
    }

private:
    cl::CommandQueue & queue;
    kernel_type kernel;
    const int nodes;

    std::array<cl::Buffer, 3> storage;

    cl::Buffer & previous;
    cl::Buffer & current;
    cl::Buffer & next;

    cl::Buffer output;
};

class RectangularWaveguide : public Waveguide<RectangularProgram> {
public:
    RectangularWaveguide(const RectangularProgram & program,
                         cl::CommandQueue & queue,
                         cl_int3 p);

    cl_float run_step(cl_float i,
                      int e,
                      int o,
                      cl_float attenuation,
                      cl::CommandQueue & queue,
                      kernel_type & kernel,
                      int nodes,
                      cl::Buffer & previous,
                      cl::Buffer & current,
                      cl::Buffer & next,
                      cl::Buffer & output) override;

    size_type get_index(cl_int3 pos) const;

private:
    const cl_int3 p;
};

class RecursiveTetrahedralWaveguide
    : public Waveguide<RecursiveTetrahedralProgram> {
public:
    RecursiveTetrahedralWaveguide(const RecursiveTetrahedralProgram & program,
                                  cl::CommandQueue & queue,
                                  const Boundary & boundary,
                                  Vec3f start,
                                  float spacing);

    cl_float run_step(cl_float i,
                      int e,
                      int o,
                      cl_float attenuation,
                      cl::CommandQueue & queue,
                      kernel_type & kernel,
                      int nodes,
                      cl::Buffer & previous,
                      cl::Buffer & current,
                      cl::Buffer & next,
                      cl::Buffer & output) override;

private:
    RecursiveTetrahedralWaveguide(const RecursiveTetrahedralProgram & program,
                                  cl::CommandQueue & queue,
                                  std::vector<LinkedTetrahedralNode> nodes);

#ifdef TESTING
    std::vector<LinkedTetrahedralNode> & nodes;
#endif
    cl::Buffer node_buffer;
};

class IterativeTetrahedralWaveguide
    : public Waveguide<IterativeTetrahedralProgram> {
public:
    IterativeTetrahedralWaveguide(const IterativeTetrahedralProgram & program,
                                  cl::CommandQueue & queue,
                                  const Boundary & boundary,
                                  float cube_side);

    cl_float run_step(cl_float i,
                      int e,
                      int o,
                      cl_float attenuation,
                      cl::CommandQueue & queue,
                      kernel_type & kernel,
                      int nodes,
                      cl::Buffer & previous,
                      cl::Buffer & current,
                      cl::Buffer & next,
                      cl::Buffer & output) override;

private:
    IterativeTetrahedralWaveguide(const IterativeTetrahedralProgram & program,
                                  cl::CommandQueue & queue,
                                  IterativeTetrahedralMesh mesh);

    IterativeTetrahedralMesh mesh;
    cl::Buffer node_buffer;
};
