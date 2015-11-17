#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

class TetrahedralProgram : public cl::Program {
public:
    TetrahedralProgram(const cl::Context& context,
                       bool build_immediate = false);

    auto get_kernel() const {
        return cl::make_kernel<cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl_ulong,
                               cl::Buffer>(*this, "waveguide");
    }

private:
    static const std::string source;
};
