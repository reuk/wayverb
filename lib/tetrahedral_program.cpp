#include "tetrahedral_program.h"

using namespace std;

TetrahedralProgram::TetrahedralProgram(const cl::Context & context,
                                   bool build_immediate)
        : Program(context, source, build_immediate) {
}

const string TetrahedralProgram::source{
#ifdef DIAGNOSTIC
    "#define DIAGNOSTIC\n"
#endif
    R"(

    typedef struct {
        int ports[4];
        float3 position;
    } Node;

    kernel void waveguide
    (   unsigned long write
    ,   float value
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
            next[index] += value;
        }

        float temp = 0;

        const size_t max_ports = sizeof(Node::ports) / sizeof(int);
        for (int i = 0; i != max_ports; ++i) {
            int port_index = node->ports[i];

            if (port_index >= 0) {
                global Node * port = nodes + port_index;

                temp += current[port_index] / (1 + previous[port_index]);
            }
        }

        temp *= 0.5;

        /*
        const float DAMPING_FACTOR = 0.995;
        temp *= DAMPING_FACTOR;
        */

        next[index] = temp;

        if (index == read) {
            *output = temp;
        }
    }
    )"};
