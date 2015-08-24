#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

class Kernel {
public:
    Kernel(const cl::Context & context,
           const cl::Device & device,
           const std::string & kernel,
           bool verbose = false);

private:
    cl::Program program;
};

class WaveguideKernel : public Kernel {
public:
    WaveguideKernel(const cl::Context & context,
                    const cl::Device & device,
                    bool verbose = false);

private:
    static const std::string kernel;
};
