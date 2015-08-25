#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

class Waveguide {
public:
    Waveguide(const cl::Program & program, cl::CommandQueue & queue, cl_int3 p);

    std::vector<float> run(cl_int3 excitation, cl_int3 read_head, int steps);

private:
    const cl::Program & program;
    cl::CommandQueue & queue;

    size_t get_index(cl_int3 pos) const;

    const cl_int3 p;

    cl::Buffer next;
    cl::Buffer current;
    cl::Buffer previous;
};
