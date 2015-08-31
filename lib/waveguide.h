#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include <array>

class Waveguide {
public:
    using size_type = std::vector<cl_float>::size_type;

    Waveguide(const cl::Program & program, cl::CommandQueue & queue, cl_int3 p);

    std::vector<cl_float> run(const std::vector<float> & input,
                              cl_int3 excitation,
                              cl_int3 read_head,
                              int steps);

private:
    const cl::Program & program;
    cl::CommandQueue & queue;

    size_type get_index(cl_int3 pos) const;

    const cl_int3 p;

    std::array<cl::Buffer, 3> storage;

    cl::Buffer & previous;
    cl::Buffer & current;
    cl::Buffer & next;

    cl::Buffer output;
};
