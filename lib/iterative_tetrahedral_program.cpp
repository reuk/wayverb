#include "iterative_tetrahedral_program.h"

using namespace std;

IterativeTetrahedralProgram::IterativeTetrahedralProgram(
    const cl::Context & context, bool build_immediate)
        : Program(context, source, build_immediate) {
}

const string IterativeTetrahedralProgram::source{
#ifdef DIAGNOSTIC
    "#define DIAGNOSTIC\n"
#endif
    R"(
    #define PORTS (4)
    #define CUBE_NODES (8)

    typedef struct {
        int ports[PORTS];
        float3 position;
        bool inside;
    } Node;

    typedef struct {
        int indices[PORTS];
    } Neighbors;

    typedef struct {
        int3 pos;
        int mod_ind;
    } Locator;

    int get_index(Locator l, int3 dim);
    int get_index(Locator l, int3 dim) {
        return (l.mod_ind) +
               (l.pos.x * CUBE_NODES) +
               (l.pos.y * dim.x * CUBE_NODES) +
               (l.pos.z * dim.y * dim.x * CUBE_NODES);
    }

    Locator get_locator(int index, int3 dim);
    Locator get_locator(int index, int3 dim) {
        int m = index % CUBE_NODES;
        index /= CUBE_NODES;
        int x = index % dim.x;
        index /= dim.x;
        int y = index % dim.y;
        index /= dim.y;
        int z = index % dim.z;
        return (Locator){(int3)(x, y, z), m};
    }

    //  TODO needs single-source-of-truthing
    constant Locator offset_table[CUBE_NODES][PORTS] = {
        {(Locator){(int3)(0, 0, 0), 2},
         (Locator){(int3)(-1, 0, -1), 3},
         (Locator){(int3)(-1, -1, 0), 6},
         (Locator){(int3)(0, -1, -1), 7}},
        {(Locator){(int3)(0, 0, 0), 2},
         (Locator){(int3)(0, 0, 0), 3},
         (Locator){(int3)(0, -1, 0), 6},
         (Locator){(int3)(0, -1, 0), 7}},
        {(Locator){(int3)(0, 0, 0), 0},
         (Locator){(int3)(0, 0, 0), 1},
         (Locator){(int3)(0, 0, 0), 4},
         (Locator){(int3)(0, 0, 0), 5}},
        {(Locator){(int3)(1, 0, 1), 0},
         (Locator){(int3)(0, 0, 0), 1},
         (Locator){(int3)(0, 0, 1), 4},
         (Locator){(int3)(1, 0, 0), 5}},
        {(Locator){(int3)(0, 0, 0), 2},
         (Locator){(int3)(0, 0, -1), 3},
         (Locator){(int3)(0, 0, 0), 6},
         (Locator){(int3)(0, 0, -1), 7}},
        {(Locator){(int3)(0, 0, 0), 2},
         (Locator){(int3)(-1, 0, 0), 3},
         (Locator){(int3)(-1, 0, 0), 6},
         (Locator){(int3)(0, 0, 0), 7}},
        {(Locator){(int3)(1, 1, 0), 0},
         (Locator){(int3)(0, 1, 0), 1},
         (Locator){(int3)(0, 0, 0), 4},
         (Locator){(int3)(1, 0, 0), 5}},
        {(Locator){(int3)(0, 1, 1), 0},
         (Locator){(int3)(0, 1, 0), 1},
         (Locator){(int3)(0, 0, 1), 4},
         (Locator){(int3)(0, 0, 0), 5}},
    };

    Neighbors get_neighbors(int index, int3 dim);
    Neighbors get_neighbors(int index, int3 dim) {
        Locator locator = get_locator(index, dim);

        Neighbors ret;
        for (int i = 0; i != PORTS; ++i) {
            Locator relative = offset_table[locator.mod_ind][i];
            int3 summed = locator.pos + relative.pos;

            bool is_neighbor = all((int3)(0) <= summed && summed < dim);
            ret.indices[i] = is_neighbor ? get_index((Locator){summed, relative.mod_ind}, dim) : -1;
        }
        return ret;
    }

    kernel void waveguide
    (   unsigned long write
    ,   float value
    ,   float attenuation
    ,   global float * next
    ,   global float * current
    ,   global float * previous
    ,   global Node * nodes
    ,   int3 dim
    ,   unsigned long read
    ,   global float * output
    ) {
        size_t index = get_global_id(0);
        global Node * node = nodes + index;

        if (! node->inside) {
            barrier(CLK_GLOBAL_MEM_FENCE);
            return;
        }

        if (index == write) {
            current[index] += value;
        }

        barrier(CLK_GLOBAL_MEM_FENCE);

        //  TODO should precompute this
        Neighbors neighbors = get_neighbors(index, dim);

        float temp = 0;

        //  waveguide logic goes here
        for (int i = 0; i != PORTS; ++i) {
            int port_index = neighbors.indices[i];
            if (port_index >= 0 && nodes[port_index].inside) {
                temp += current[port_index];
            }
        }

        temp /= 2;
        temp -= previous[index];

        temp *= attenuation;

        next[index] = temp;

        if (index == read) {
            *output = next[index];
        }
    }
    )"};
