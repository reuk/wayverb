#pragma once

#include "waveguide/cl/utils.h"

template <size_t n>
struct alignas(1 << 2) boundary_index_array final {
    cl_uint array[n];
};

using boundary_index_array_1 = boundary_index_array<1>;
using boundary_index_array_2 = boundary_index_array<2>;
using boundary_index_array_3 = boundary_index_array<3>;

template <>
struct cl_representation<boundary_index_array_1> final {
    static constexpr auto value{R"(
typedef struct { uint array[1]; } boundary_index_array_1;
)"};
};

template <>
struct cl_representation<boundary_index_array_2> final {
    static constexpr auto value{R"(
typedef struct { uint array[2]; } boundary_index_array_2;
)"};
};

template <>
struct cl_representation<boundary_index_array_3> final {
    static constexpr auto value{R"(
typedef struct { uint array[3]; } boundary_index_array_3;
)"};
};

