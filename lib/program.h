#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

class WaveguideProgram : public cl::Program {
public:
    WaveguideProgram(const cl::Context & context, bool build_immediate);

private:
    static const std::string source;
};
