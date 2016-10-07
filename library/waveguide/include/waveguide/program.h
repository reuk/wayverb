#pragma once

#include "common/cl/common.h"
#include "common/hrtf.h"
#include "common/program_wrapper.h"
#include "common/scene_data.h"
#include "waveguide/cl/structs.h"

#include "utilities/decibels.h"
#include "utilities/string_builder.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace waveguide {

class program final {
public:
    program(const compute_context& cc);

    auto get_kernel() const {
        return program_wrapper.get_kernel<cl::Buffer,  /// previous
                                          cl::Buffer,  /// current
                                          cl::Buffer,  /// nodes
                                          cl_int3,     /// dimensions
                                          cl::Buffer,  /// boundary_data_1
                                          cl::Buffer,  /// boundary_data_2
                                          cl::Buffer,  /// boundary_data_3
                                          cl::Buffer,  /// boundary_coefficients
                                          cl::Buffer   /// error_flag
                                          >("condensed_waveguide");
    }

    auto get_filter_test_kernel() const {
        return program_wrapper
                .get_kernel<cl::Buffer, cl::Buffer, cl::Buffer, cl::Buffer>(
                        "filter_test");
    }

    auto get_filter_test_2_kernel() const {
        return program_wrapper
                .get_kernel<cl::Buffer, cl::Buffer, cl::Buffer, cl::Buffer>(
                        "filter_test_2");
    }

    template <cl_program_info T>
    auto get_info() const {
        return program_wrapper.template get_info<T>();
    }

    cl::Device get_device() const { return program_wrapper.get_device(); }

private:
    program_wrapper program_wrapper;
};

}  // namespace waveguide
