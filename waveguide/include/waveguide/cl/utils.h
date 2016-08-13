#pragma once

#include <string>

namespace waveguide {
namespace cl_sources {

constexpr const char* utils{R"(
#define PORTS        (6)
#define courant      (1.0f / sqrt(3.0f))
#define courant_sq   (1.0f / 3.0f)
constant uint no_neighbor = ~(uint)0;

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
} BoundaryType;

typedef enum {
    id_port_nx = 0,
    id_port_px = 1,
    id_port_ny = 2,
    id_port_py = 3,
    id_port_nz = 4,
    id_port_pz = 5,
} PortDirection;

typedef struct {
    float3 position;        //  spatial position
    uint ports[PORTS];      //  the indices of adjacent ports
    int boundary_type;      //  describes the boundary type
    uint boundary_index;    //  an index into a boundary descriptor array
    char inside;            //  is the node an air node?
} Node;

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

typedef struct { PortDirection array[1]; } InnerNodeDirections1;
typedef struct { PortDirection array[2]; } InnerNodeDirections2;
typedef struct { PortDirection array[3]; } InnerNodeDirections3;

InnerNodeDirections1 get_inner_node_directions_1(BoundaryType boundary_type);
InnerNodeDirections1 get_inner_node_directions_1(BoundaryType boundary_type) {
    switch (boundary_type) {
        case id_nx: return (InnerNodeDirections1){{id_port_nx}};
        case id_px: return (InnerNodeDirections1){{id_port_px}};
        case id_ny: return (InnerNodeDirections1){{id_port_ny}};
        case id_py: return (InnerNodeDirections1){{id_port_py}};
        case id_nz: return (InnerNodeDirections1){{id_port_nz}};
        case id_pz: return (InnerNodeDirections1){{id_port_pz}};

        default: return (InnerNodeDirections1){{-1}};
    }
}

InnerNodeDirections2 get_inner_node_directions_2(int boundary_type);
InnerNodeDirections2 get_inner_node_directions_2(int boundary_type) {
    switch (boundary_type) {
        case id_nx | id_ny:
            return (InnerNodeDirections2){{id_port_nx, id_port_ny}};
        case id_nx | id_py:
            return (InnerNodeDirections2){{id_port_nx, id_port_py}};
        case id_px | id_ny:
            return (InnerNodeDirections2){{id_port_px, id_port_ny}};
        case id_px | id_py:
            return (InnerNodeDirections2){{id_port_px, id_port_py}};
        case id_nx | id_nz:
            return (InnerNodeDirections2){{id_port_nx, id_port_nz}};
        case id_nx | id_pz:
            return (InnerNodeDirections2){{id_port_nx, id_port_pz}};
        case id_px | id_nz:
            return (InnerNodeDirections2){{id_port_px, id_port_nz}};
        case id_px | id_pz:
            return (InnerNodeDirections2){{id_port_px, id_port_pz}};
        case id_ny | id_nz:
            return (InnerNodeDirections2){{id_port_ny, id_port_nz}};
        case id_ny | id_pz:
            return (InnerNodeDirections2){{id_port_ny, id_port_pz}};
        case id_py | id_nz:
            return (InnerNodeDirections2){{id_port_py, id_port_nz}};
        case id_py | id_pz:
            return (InnerNodeDirections2){{id_port_py, id_port_pz}};

        default: return (InnerNodeDirections2){{-1, -1}};
    }
}

InnerNodeDirections3 get_inner_node_directions_3(int boundary_type);
InnerNodeDirections3 get_inner_node_directions_3(int boundary_type) {
    switch (boundary_type) {
        case id_nx | id_ny | id_nz:
            return (InnerNodeDirections3){{id_port_nx, id_port_ny, id_port_nz}};
        case id_nx | id_ny | id_pz:
            return (InnerNodeDirections3){{id_port_nx, id_port_ny, id_port_pz}};
        case id_nx | id_py | id_nz:
            return (InnerNodeDirections3){{id_port_nx, id_port_py, id_port_nz}};
        case id_nx | id_py | id_pz:
            return (InnerNodeDirections3){{id_port_nx, id_port_py, id_port_pz}};
        case id_px | id_ny | id_nz:
            return (InnerNodeDirections3){{id_port_px, id_port_ny, id_port_nz}};
        case id_px | id_ny | id_pz:
            return (InnerNodeDirections3){{id_port_px, id_port_ny, id_port_pz}};
        case id_px | id_py | id_nz:
            return (InnerNodeDirections3){{id_port_px, id_port_py, id_port_nz}};
        case id_px | id_py | id_pz:
            return (InnerNodeDirections3){{id_port_px, id_port_py, id_port_pz}};

        default: return (InnerNodeDirections3){{-1, -1, -1}};
    }
}

PortDirection opposite(PortDirection pd);
PortDirection opposite(PortDirection pd) {
    switch (pd) {
        case id_port_nx: return id_port_px;
        case id_port_px: return id_port_nx;
        case id_port_ny: return id_port_py;
        case id_port_py: return id_port_ny;
        case id_port_nz: return id_port_pz;
        case id_port_pz: return id_port_nz;

        default: return -1;
    }
}

)"};
}  // namespace cl_sources

namespace mesh {

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

constexpr boundary_type port_index_to_boundary_type(unsigned int i) {
    return static_cast<boundary_type>(1 << (i + 1));
}

constexpr cl_uint no_neighbor{~cl_uint{0}};

struct alignas(1 << 4) node final {
    static constexpr size_t num_ports{6};

    cl_float3 position{};        /// spatial position
    cl_uint ports[num_ports]{};  /// the indices of adjacent ports
    cl_int boundary_type{};      /// describes the boundary type
    cl_uint boundary_index{};    /// an index into a boundary descriptor array
    cl_char inside{};            /// is the node an air node?
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

}  // namespace mesh
}  // namespace waveguide
