#pragma once

#include "waveguide/cl/filters.h"

namespace waveguide {
namespace cl_sources {
constexpr const char* structs{R"(
typedef enum {
    id_success = 0,
    id_inf_error = 1 << 0,
    id_nan_error = 1 << 1,
    id_outside_range_error = 1 << 2,
    id_outside_mesh_error = 1 << 3,
    id_suspicious_boundary_error = 1 << 4,
} ErrorCode;

typedef struct {
    int boundary_type;
    uint boundary_index;
} CondensedNode;

typedef struct {
    FilterMemoryCanonical filter_memory;
    uint coefficient_index;
} BoundaryData;

typedef struct { BoundaryData array[1]; } BoundaryDataArray1;
typedef struct { BoundaryData array[2]; } BoundaryDataArray2;
typedef struct { BoundaryData array[3]; } BoundaryDataArray3;

#define NUM_SURROUNDING_PORTS_1 4
typedef struct {
    PortDirection array[NUM_SURROUNDING_PORTS_1];
} SurroundingPorts1;

#define NUM_SURROUNDING_PORTS_2 2
typedef struct {
    PortDirection array[NUM_SURROUNDING_PORTS_2];
} SurroundingPorts2;

)"};
}  // namespace cl_sources

//----------------------------------------------------------------------------//

typedef enum : cl_int {
    id_success = 0,
    id_inf_error = 1 << 0,
    id_nan_error = 1 << 1,
    id_outside_range_error = 1 << 2,
    id_outside_mesh_error = 1 << 3,
    id_suspicious_boundary_error = 1 << 4,
} ErrorCode;

//----------------------------------------------------------------------------//

struct alignas(1 << 3) condensed_node final {
    static constexpr size_t num_ports{6};
    cl_int boundary_type{};
    cl_uint boundary_index{};
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
    canonical_memory filter_memory{};
    cl_uint coefficient_index{};
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

using boundary_data_array1 = boundary_data_array<1>;
using boundary_data_array2 = boundary_data_array<2>;
using boundary_data_array3 = boundary_data_array<3>;

}  // namespace waveguide
