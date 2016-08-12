#pragma once

#include "common/cl_common.h"
#include "common/decibels.h"
#include "common/hrtf.h"
#include "common/program_wrapper.h"
#include "common/scene_data.h"
#include "common/stl_wrappers.h"
#include "common/string_builder.h"
#include "waveguide/cl/structs.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace waveguide {

class program final {
public:
    static constexpr auto num_ports{condensed_node::num_ports};

    program(const cl::Context& context, const cl::Device& device);

    auto get_kernel() const {
        return program_wrapper.get_kernel<cl::Buffer,
                                          cl::Buffer,
                                          cl::Buffer,
                                          cl_int3,
                                          cl::Buffer,
                                          cl::Buffer,
                                          cl::Buffer,
                                          cl::Buffer,
                                          cl::Buffer>("condensed_waveguide");
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
    static const std::string source;

    program_wrapper program_wrapper;
};

}  // namespace waveguide
