#pragma once

#include "cl_structs.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

class RectangularProgram : public cl::Program {
public:
    RectangularProgram(const cl::Context & context,
                       bool build_immediate = false);

    auto get_kernel() const {
        return cl::make_kernel<cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl_float,
                               cl_float,
                               cl_ulong,
                               cl::Buffer>(*this, "waveguide");
    }

    auto get_filter_test_kernel() const {
        return cl::make_kernel<cl::Buffer, cl::Buffer, cl::Buffer, cl::Buffer>(
            *this, "filter_test");
    }

private:
    static constexpr int PORTS = 6;
    static constexpr int BIQUAD_SECTIONS = BiquadMemoryArray::BIQUAD_SECTIONS;
    static const std::string source;
};
