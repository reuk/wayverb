#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

class Waveguide {
public:
    Waveguide(const cl::Program & program, cl::CommandQueue & queue, int x, int y, int z);

    void iterate() const;
private:
    const cl::Program & program;
    cl::CommandQueue & queue;

    const int x;
    const int y;
    const int z;

    cl::Buffer mesh;
};
