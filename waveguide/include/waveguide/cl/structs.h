#pragma once

#include "waveguide/cl/filter_structs.h"

typedef enum : cl_int {
    id_success = 0,
    id_inf_error = 1 << 0,
    id_nan_error = 1 << 1,
    id_outside_range_error = 1 << 2,
    id_outside_mesh_error = 1 << 3,
    id_suspicious_boundary_error = 1 << 4,
} error_code;

template <>
struct cl_representation<error_code> final {
    static constexpr const char* value{R"(
#ifndef ERROR_CODE_DEFINITION__
#define ERROR_CODE_DEFINITION__
typedef enum {
    id_success = 0,
    id_inf_error = 1 << 0,
    id_nan_error = 1 << 1,
    id_outside_range_error = 1 << 2,
    id_outside_mesh_error = 1 << 3,
    id_suspicious_boundary_error = 1 << 4,
} error_code;
#endif
)"};
};

//----------------------------------------------------------------------------//

struct alignas(1 << 3) condensed_node final {
    cl_int boundary_type{};
    cl_uint boundary_index{};
};

template <>
struct cl_representation<condensed_node> final {
    static constexpr const char* value{R"(
#ifndef CONDENSED_NODE_DEFINITION__
#define CONDENSED_NODE_DEFINITION__
typedef struct {
    int boundary_type;
    uint boundary_index;
} condensed_node;
#endif
)"};
};

inline bool operator==(const condensed_node& a, const condensed_node& b) {
    return std::tie(a.boundary_type, a.boundary_index) ==
           std::tie(b.boundary_type, b.boundary_index);
}

inline bool operator!=(const condensed_node& a, const condensed_node& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

/// Stores filter coefficients for a single high-order filter, and an index
/// into an array of filter parameters which describe the filter being
/// modelled.
struct alignas(1 << 3) boundary_data final {
    memory_canonical filter_memory{};
    cl_uint coefficient_index{};
};

template <>
struct cl_representation<boundary_data> final {
    static constexpr const char* value{R"(
#ifndef BOUNDARY_DATA_DEFINITION__
#define BOUNDARY_DATA_DEFINITION__
typedef struct {
    memory_canonical filter_memory;
    uint coefficient_index;
} boundary_data;
#endif
)"};
};

inline bool operator==(const boundary_data& a, const boundary_data& b) {
    return std::tie(a.filter_memory, a.coefficient_index) ==
           std::tie(b.filter_memory, b.coefficient_index);
}

inline bool operator!=(const boundary_data& a, const boundary_data& b) {
    return !(a == b);
}

//----------------------------------------------------------------------------//

template <size_t D>
struct alignas(1 << 3) boundary_data_array final {
    static constexpr size_t DIMENSIONS{D};
    boundary_data array[DIMENSIONS]{};
};

template <size_t D>
bool operator==(const boundary_data_array<D>& a,
                const boundary_data_array<D>& b) {
    return proc::equal(a.array, std::begin(b.array));
}

template <size_t D>
bool operator!=(const boundary_data_array<D>& a,
                const boundary_data_array<D>& b) {
    return !(a == b);
}

using boundary_data_array_1 = boundary_data_array<1>;
using boundary_data_array_2 = boundary_data_array<2>;
using boundary_data_array_3 = boundary_data_array<3>;

template<>
struct cl_representation<boundary_data_array_1> final {
    static constexpr const char* value{R"(
#ifndef BOUNDARY_DATA_ARRAY_1_DEFINITION__
#define BOUNDARY_DATA_ARRAY_1_DEFINITION__
typedef struct { boundary_data array[1]; } boundary_data_array_1;
#endif
)"};
};

template<>
struct cl_representation<boundary_data_array_2> final {
    static constexpr const char* value{R"(
#ifndef BOUNDARY_DATA_ARRAY_2_DEFINITION__
#define BOUNDARY_DATA_ARRAY_2_DEFINITION__
typedef struct { boundary_data array[2]; } boundary_data_array_2;
#endif
)"};
};

template<>
struct cl_representation<boundary_data_array_3> final {
    static constexpr const char* value{R"(
#ifndef BOUNDARY_DATA_ARRAY_3_DEFINITION__
#define BOUNDARY_DATA_ARRAY_3_DEFINITION__
typedef struct { boundary_data array[3]; } boundary_data_array_3;
#endif
)"};
};
