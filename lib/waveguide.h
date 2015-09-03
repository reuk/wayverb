#pragma once

#include "rectangular_program.h"
#include "tetrahedral_program.h"
#include "tetrahedral.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include <array>

class RectangularWaveguide {
public:
    using size_type = std::vector<cl_float>::size_type;

    RectangularWaveguide(const RectangularProgram & program,
                         cl::CommandQueue & queue,
                         cl_int3 p);

    std::vector<cl_float> run(std::vector<float> input,
                              cl_int3 excitation,
                              cl_int3 read_head,
                              int steps);

private:
    const RectangularProgram & program;
    cl::CommandQueue & queue;

    size_type get_index(cl_int3 pos) const;

    const cl_int3 p;

    std::array<cl::Buffer, 3> storage;

    cl::Buffer & previous;
    cl::Buffer & current;
    cl::Buffer & next;

    cl::Buffer output;
};

class TetrahedralWaveguide {
public:
    using size_type = std::vector<cl_float>::size_type;

    TetrahedralWaveguide(const TetrahedralProgram & program,
                         cl::CommandQueue & queue,
                         std::vector<Node> & nodes);

    std::vector<cl_float> run(std::vector<float> input,
                              size_type excitation,
                              size_type read_head,
                              int steps);

private:
    const TetrahedralProgram & program;
    cl::CommandQueue & queue;

    const size_type node_size;
    cl::Buffer node_buffer;

    std::array<cl::Buffer, 3> storage;

    cl::Buffer & previous;
    cl::Buffer & current;
    cl::Buffer & next;

    cl::Buffer output;
};
