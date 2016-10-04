#pragma once

#include "common/cl/representation.h"
#include "common/cl/traits.h"
#include "common/stl_wrappers.h"

#include <string>

typedef enum : cl_int {
    id_none = 0,
    id_inside = 1 << 0,
    id_nx = 1 << 1,
    id_px = 1 << 2,
    id_ny = 1 << 3,
    id_py = 1 << 4,
    id_nz = 1 << 5,
    id_pz = 1 << 6,
    id_reentrant = 1 << 7,
} boundary_type;

template <>
struct cl_representation<boundary_type> final {
    static constexpr auto value{R"(
typedef enum {
    id_none = 0,
    id_inside = 1 << 0,
    id_nx = 1 << 1,
    id_px = 1 << 2,
    id_ny = 1 << 3,
    id_py = 1 << 4,
    id_nz = 1 << 5,
    id_pz = 1 << 6,
    id_reentrant = 1 << 7,
} boundary_type;
)"};
};

constexpr boundary_type port_index_to_boundary_type(unsigned int i) {
    return static_cast<boundary_type>(1 << (i + 1));
}

//----------------------------------------------------------------------------//

constexpr auto no_neighbor{~cl_uint{0}};
constexpr auto num_ports{size_t{6}};

namespace cl_sources {
extern const char* utils;
}//namespace cl_sources
