#pragma once

#include "rectangular_program.h"
#include "tetrahedral_program.h"
#include "iterative_tetrahedral_mesh.h"
#include "recursive_tetrahedral.h"
#include "cl_structs.h"
#include "logger.h"
#include "test_flag.h"

#include <array>
#include <type_traits>
#include <algorithm>

template <typename T>
class Waveguide {
public:
    using size_type = std::vector<cl_float>::size_type;
    using kernel_type = decltype(std::declval<T>().get_kernel());

    Waveguide(const T & program, cl::CommandQueue & queue, size_type nodes)
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

    virtual ~Waveguide() noexcept = default;

    virtual cl_float run_step(cl_float i,
                              size_type e,
                              size_type o,
                              cl_float attenuation,
                              cl::CommandQueue & queue,
                              kernel_type & kernel,
                              size_type nodes,
                              cl::Buffer & previous,
                              cl::Buffer & current,
                              cl::Buffer & next,
                              cl::Buffer & output) = 0;

    virtual std::vector<cl_float> run(std::vector<float> input,
                                      size_type e,
                                      size_type o,
                                      cl_float attenuation,
                                      size_type steps) {
        std::vector<cl_float> n(nodes, 0);
        cl::copy(queue, n.begin(), n.end(), next);
        cl::copy(queue, n.begin(), n.end(), current);
        cl::copy(queue, n.begin(), n.end(), previous);

        input.resize(steps, 0);

        std::vector<cl_float> ret(input.size());

        auto counter = 0u;
        std::transform(input.begin(),
                       input.end(),
                       ret.begin(),
                       [this, &counter, &steps, &attenuation, &e, &o](auto i) {
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

                           auto percent = counter * 100 / (steps - 1);
                           std::cout << "\r" << percent << "% done"
                                     << std::flush;

                           counter += 1;

                           return ret;
                       });

        std::cout << std::endl;

        return ret;
    }

    size_type get_nodes() const {
        return nodes;
    }

private:
    cl::CommandQueue & queue;
    kernel_type kernel;
    const size_type nodes;

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
    virtual ~RectangularWaveguide() noexcept = default;

    cl_float run_step(cl_float i,
                      size_type e,
                      size_type o,
                      cl_float attenuation,
                      cl::CommandQueue & queue,
                      kernel_type & kernel,
                      size_type nodes,
                      cl::Buffer & previous,
                      cl::Buffer & current,
                      cl::Buffer & next,
                      cl::Buffer & output) override;

    size_type get_index(cl_int3 pos) const;

private:
    const cl_int3 p;
};

class TetrahedralWaveguide : public Waveguide<TetrahedralProgram> {
public:
    TetrahedralWaveguide(const TetrahedralProgram & program,
                         cl::CommandQueue & queue,
                         std::vector<Node> nodes);
    virtual ~TetrahedralWaveguide() noexcept = default;

    cl_float run_step(cl_float i,
                      size_type e,
                      size_type o,
                      cl_float attenuation,
                      cl::CommandQueue & queue,
                      kernel_type & kernel,
                      size_type nodes,
                      cl::Buffer & previous,
                      cl::Buffer & current,
                      cl::Buffer & next,
                      cl::Buffer & output) override;

private:
    cl::Buffer node_buffer;
};

class RecursiveTetrahedralWaveguide : public TetrahedralWaveguide {
public:
    RecursiveTetrahedralWaveguide(const TetrahedralProgram & program,
                                  cl::CommandQueue & queue,
                                  const Boundary & boundary,
                                  Vec3f start,
                                  float spacing);
    virtual ~RecursiveTetrahedralWaveguide() noexcept = default;
};

class IterativeTetrahedralWaveguide : public TetrahedralWaveguide {
public:
    IterativeTetrahedralWaveguide(const TetrahedralProgram & program,
                                  cl::CommandQueue & queue,
                                  const Boundary & boundary,
                                  float cube_side);
    virtual ~IterativeTetrahedralWaveguide() noexcept = default;
};
