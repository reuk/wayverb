#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

class TetrahedralProgram : public cl::Program {
public:
    TetrahedralProgram(const cl::Context & context,
                       bool build_immediate = false);

private:
    static const std::string source;
};
