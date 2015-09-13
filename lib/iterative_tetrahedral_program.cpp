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

    typedef struct {
        bool inside;
    } Node;

    kernel void waveguide
    (   unsigned long write
    ,   float value
    ,   float attenuation
    ,   global float * next
    ,   global float * current
    ,   global float * previous
    ,   global Node * nodes
    ,   unsigned long read
    ,   global float * output
    ) {
        size_t index = get_global_id(0);
        global Node * node = nodes + index;

        if (index == write) {
            current[index] += value;
        }

        barrier(CLK_GLOBAL_MEM_FENCE);

        float temp = 0;

        /*
        //  TODO
        //  waveguide logic goes here
        for (int i = 0; i != PORTS; ++i) {
            int port_index = neighbor[i];
            //  TODO set port_index to -1 if not a neighbor I guess
            if (port_index >= 0 && nodes[port_index].inside)
                temp += current[port_index];
            }
        }
        */

        temp /= 2;
        temp -= previous[index];

        //  TODO probably not right way to damp?
        temp *= attenuation;

        next[index] = temp;

        if (index == read) {
            *output = next[index];
        }
    }
    )"};
