#pragma once

#include "rectangular_program.h"
#include "recursive_tetrahedral_program.h"
#include "recursive_tetrahedral.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include <array>
#include <type_traits>

#define TESTING

class RectangularWaveguide {
public:
    using size_type = std::vector<cl_float>::size_type;
    using kernel_type =
        decltype(std::declval<RectangularProgram>().get_kernel());

    RectangularWaveguide(const RectangularProgram & program,
                         cl::CommandQueue & queue,
                         cl_int3 p);

    std::vector<cl_float> run(std::vector<float> input,
                              cl_int3 excitation,
                              cl_int3 read_head,
                              cl_float attenuation,
                              int steps);

private:
    cl::CommandQueue & queue;

    kernel_type kernel;

    size_type get_index(cl_int3 pos) const;

    const cl_int3 p;

    std::array<cl::Buffer, 3> storage;

    cl::Buffer & previous;
    cl::Buffer & current;
    cl::Buffer & next;

    cl::Buffer output;
};

class RecursiveTetrahedralWaveguide {
public:
    using size_type = std::vector<cl_float>::size_type;
    using kernel_type =
        decltype(std::declval<RecursiveTetrahedralProgram>().get_kernel());

    RecursiveTetrahedralWaveguide(const RecursiveTetrahedralProgram & program,
                         cl::CommandQueue & queue,
                         std::vector<LinkedTetrahedralNode> & nodes);

    std::vector<cl_float> run(std::vector<float> input,
                              size_type excitation,
                              size_type read_head,
                              cl_float attenuation,
                              int steps);

private:
    cl::CommandQueue & queue;

    kernel_type kernel;

#ifdef TESTING
    std::vector<LinkedTetrahedralNode> & nodes;
#endif

    const size_type node_size;
    cl::Buffer node_buffer;

    std::array<cl::Buffer, 3> storage;

    cl::Buffer & previous;
    cl::Buffer & current;
    cl::Buffer & next;

    cl::Buffer output;
};
