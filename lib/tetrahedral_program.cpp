#include "tetrahedral_program.h"

using namespace std;

TetrahedralProgram::TetrahedralProgram(const cl::Context& context,
                                       bool build_immediate)
        : Program(context, source, build_immediate) {
}

const string TetrahedralProgram::source{
#ifdef DIAGNOSTIC
    "#define DIAGNOSTIC\n"
#endif
    R"(
#define PORTS (4)

typedef struct {
    int ports[PORTS];
    float3 position;
    bool inside;
} Node;

kernel void waveguide
(   global float * current
,   global float * previous
,   global Node * nodes
,   global float * transform_matrix
,   global float3 * velocity_buffer
,   unsigned long read
,   global float * output
) {
    size_t index = get_global_id(0);
    global Node * node = nodes + index;

    if (! node->inside) {
        return;
    }

    float temp = 0;

    //  waveguide logic goes here
    for (int i = 0; i != PORTS; ++i) {
        int port_index = node->ports[i];
        if (port_index >= 0 && nodes[port_index].inside)
            temp += current[port_index];
    }

    temp /= 2;
    temp -= previous[index];

    previous[index] = temp;

    barrier(CLK_GLOBAL_MEM_FENCE);

    if (index == read) {
        *output = previous[index];

        //
        //  instantaneous intensity for mic modelling
        //

        float differences[PORTS] = {0, 0, 0, 0};
        for (int i = 0; i != PORTS; ++i) {
            int port_index = node->ports[i];
            if (port_index >= 0 && nodes[port_index].inside)
                differences[i] = (previous[port_index] - previous[index]) /
                    distance(nodes[port_index].position, nodes[index].position);
        }

        //  the default for Eigen is column-major matrices
        //  so we'll assume that transform_matrix is column-major

        //  multiply differences by transformation matrix
        float3 multiplied = (float3)(
            transform_matrix[0]  * differences[0] +
            transform_matrix[3]  * differences[1] +
            transform_matrix[6]  * differences[2] +
            transform_matrix[9]  * differences[3],
            transform_matrix[1]  * differences[0] +
            transform_matrix[4]  * differences[1] +
            transform_matrix[7]  * differences[2] +
            transform_matrix[10] * differences[3],
            transform_matrix[2]  * differences[0] +
            transform_matrix[5]  * differences[1] +
            transform_matrix[8]  * differences[2] +
            transform_matrix[11] * differences[3]);

        //  muliply by -1/ambient_density
        float ambient_density = 1.225;
        multiplied *= -1 / ambient_density;

        //  add result to previous velocity at this junction
        *velocity_buffer += multiplied;
    }
}
)"};
