#pragma once

#include "common/cl/cl_representation.h"
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
    static constexpr const char* value{R"(
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

struct alignas(1 << 4) node final {
    cl_float3 position{};        /// spatial position
    cl_uint ports[num_ports]{};  /// the indices of adjacent ports
    cl_int boundary_type{};      /// describes the boundary type
    cl_uint boundary_index{};    /// an index into a boundary descriptor array
    cl_char inside{};            /// is the node an air node?
};

template <>
struct cl_representation<node> final {
    static constexpr const char* value{R"(
typedef struct {
    float3 position;        //  spatial position
    uint ports[6];          //  the indices of adjacent ports
    int boundary_type;      //  describes the boundary type
    uint boundary_index;    //  an index into a boundary descriptor array
    char inside;            //  is the node an air node?
} node;
)"};
};

inline bool operator==(const node& a, const node& b) {
    return proc::equal(a.ports, std::begin(b.ports)) &&
           std::tie(a.position, a.inside, a.boundary_type, a.boundary_index) ==
                   std::tie(b.position,
                            b.inside,
                            b.boundary_type,
                            b.boundary_index);
}

inline bool operator!=(const node& a, const node& b) { return !(a == b); }

//----------------------------------------------------------------------------//

namespace cl_sources {

constexpr const char* utils{R"(
#ifndef UTILS_HEADER__
#define UTILS_HEADER__

#define no_neighbor (~(uint)(0))
#define PORTS (6)

typedef enum {
    id_port_nx = 0,
    id_port_px = 1,
    id_port_ny = 2,
    id_port_py = 3,
    id_port_nz = 4,
    id_port_pz = 5,
} PortDirection;

bool locator_outside(int3 locator, int3 dim);
bool locator_outside(int3 locator, int3 dim) {
    return any(locator < (int3)(0)) || any(dim <= locator);
}

int3 to_locator(size_t index, int3 dim);
int3 to_locator(size_t index, int3 dim) {
    const int xrem = index % dim.x, xquot = index / dim.x;
    const int yrem = xquot % dim.y, yquot = xquot / dim.y;
    const int zrem = yquot % dim.z;
    return (int3)(xrem, yrem, zrem);
}

size_t to_index(int3 locator, int3 dim);
size_t to_index(int3 locator, int3 dim) {
    return locator.x + locator.y * dim.x + locator.z * dim.x * dim.y;
}

uint neighbor_index(int3 locator, int3 dim, PortDirection pd);
uint neighbor_index(int3 locator, int3 dim, PortDirection pd) {
    switch (pd) {
        case id_port_nx: {
            locator += (int3)(-1, 0, 0);
            break;
        }
        case id_port_px: {
            locator += (int3)(1, 0, 0);
            break;
        }
        case id_port_ny: {
            locator += (int3)(0, -1, 0);
            break;
        }
        case id_port_py: {
            locator += (int3)(0, 1, 0);
            break;
        }
        case id_port_nz: {
            locator += (int3)(0, 0, -1);
            break;
        }
        case id_port_pz: {
            locator += (int3)(0, 0, 1);
            break;
        }
    }
    if (locator_outside(locator, dim))
        return no_neighbor;
    return to_index(locator, dim);
}

#endif
)"};

}//namespace cl_sources
