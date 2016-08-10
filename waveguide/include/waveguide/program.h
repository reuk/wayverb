#pragma once

#include "common/cl_common.h"
#include "common/decibels.h"
#include "common/hrtf.h"
#include "common/program_wrapper.h"
#include "common/scene_data.h"
#include "common/stl_wrappers.h"
#include "common/string_builder.h"

#include "waveguide/filters.h"
#include "waveguide/mesh_setup.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace waveguide {

class program final {
public:
    typedef enum : cl_int {
        id_success = 0,
        id_inf_error = 1 << 0,
        id_nan_error = 1 << 1,
        id_outside_range_error = 1 << 2,
        id_outside_mesh_error = 1 << 3,
        id_suspicious_boundary_error = 1 << 4,
    } ErrorCode;

    struct alignas(1 << 3) condensed_node final {
        static constexpr size_t num_ports{6};
        cl_int boundary_type{};
        cl_uint boundary_index{};
    };

    static condensed_node get_condensed(const mesh_setup_program::node& n);

    /// Stores filter coefficients for a single high-order filter, and an index
    /// into an array of filter parameters which describe the filter being
    /// modelled.
    struct alignas(1 << 3) boundary_data final {
        filters::canonical_memory filter_memory{};
        cl_uint coefficient_index{};
    };

    template <size_t D>
    struct alignas(1 << 3) boundary_data_array final {
        static constexpr size_t DIMENSIONS{D};
        boundary_data array[DIMENSIONS]{};
    };

    template <size_t N>
    static boundary_data_array<N> construct_boundary_data_array(
            const std::array<cl_uint, N>& arr) {
        boundary_data_array<N> ret{};
        for (auto i = 0u; i != N; ++i) {
            ret.array[i].coefficient_index = arr[i];
        }
        return ret;
    }

    using boundary_data_array1 = boundary_data_array<1>;
    using boundary_data_array2 = boundary_data_array<2>;
    using boundary_data_array3 = boundary_data_array<3>;

    static constexpr auto num_ports{condensed_node::num_ports};

    explicit program(const cl::Context& context, const cl::Device& device);

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

inline bool operator==(const program::condensed_node& a,
                       const program::condensed_node& b) {
    return std::tie(a.boundary_type, a.boundary_index) ==
           std::tie(b.boundary_type, b.boundary_index);
}

inline bool operator!=(const program::condensed_node& a,
                       const program::condensed_node& b) {
    return !(a == b);
}

inline bool operator==(const program::boundary_data& a,
                       const program::boundary_data& b) {
    return std::tie(a.filter_memory, a.coefficient_index) ==
           std::tie(b.filter_memory, b.coefficient_index);
}

inline bool operator!=(const program::boundary_data& a,
                       const program::boundary_data& b) {
    return !(a == b);
}

template <size_t D>
bool operator==(const program::boundary_data_array<D>& a,
                const program::boundary_data_array<D>& b) {
    return proc::equal(a.array, std::begin(b.array));
}

template <size_t D>
bool operator!=(const program::boundary_data_array<D>& a,
                const program::boundary_data_array<D>& b) {
    return !(a == b);
}

}  // namespace waveguide
