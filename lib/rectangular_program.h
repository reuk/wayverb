#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

class RectangularProgram : public cl::Program {
public:
    RectangularProgram(const cl::Context & context,
                       bool build_immediate = false);

private:
    static const std::string source;
};
