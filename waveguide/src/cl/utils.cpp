#include "waveguide/cl/utils.h"

namespace cl_sources {

const char* utils{R"(
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
