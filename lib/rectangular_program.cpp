#include "rectangular_program.h"

using namespace std;

RectangularProgram::RectangularProgram(const cl::Context & context,
                                       bool build_immediate)
        : Program(context, source, build_immediate) {
}

const string RectangularProgram::source{
#ifdef DIAGNOSTIC
    "#define DIAGNOSTIC\n"
#endif
    R"(
#define PORTS (6)

typedef enum {
    id_inside = 1,
    id_boundary,
    id_outside,
} NodeType;

typedef struct {
    int ports[PORTS];
    float3 position;
    NodeType inside;
} RectNode;

kernel void waveguide
(   global float * current
,   global float * previous
,   const global RectNode * nodes
,   const global float * transform_matrix
,   global float3 * velocity_buffer
,   float spatial_sampling_period
,   float T
,   unsigned long read
,   global float * output
) {
    size_t index = get_global_id(0);
    const global RectNode * node = nodes + index;

    if (node->inside != id_inside) {
        return;
    }

    float temp = 0;

    //  waveguide logic goes here
    for (int i = 0; i != PORTS; ++i) {
        int port_index = node->ports[i];
        if (port_index >= 0 && nodes[port_index].inside == id_inside)
            temp += current[port_index];
    }

    temp /= 3;
    temp -= previous[index];

    barrier(CLK_GLOBAL_MEM_FENCE);
    previous[index] = temp;
    barrier(CLK_GLOBAL_MEM_FENCE);

    if (index == read) {
        *output = previous[index];

        //
        //  instantaneous intensity for mic modelling
        //

        float differences[PORTS] = {0, 0, 0, 0, 0, 0};
        for (int i = 0; i != PORTS; ++i) {
            int port_index = node->ports[i];
            if (port_index >= 0 && nodes[port_index].inside == id_inside)
                differences[i] = (previous[port_index] - previous[index]) /
                    spatial_sampling_period;
        }

        //  the default for Eigen is column-major matrices
        //  so we'll assume that transform_matrix is column-major

        //  multiply differences by transformation matrix
        float3 multiplied = (float3)(
            transform_matrix[0]  * differences[0] +
            transform_matrix[3]  * differences[1] +
            transform_matrix[6]  * differences[2] +
            transform_matrix[9]  * differences[3] +
            transform_matrix[12] * differences[4] +
            transform_matrix[15] * differences[5],
            transform_matrix[1]  * differences[0] +
            transform_matrix[4]  * differences[1] +
            transform_matrix[7]  * differences[2] +
            transform_matrix[10] * differences[3] +
            transform_matrix[13] * differences[4] +
            transform_matrix[16] * differences[5],
            transform_matrix[2]  * differences[0] +
            transform_matrix[5]  * differences[1] +
            transform_matrix[8]  * differences[2] +
            transform_matrix[11] * differences[3] +
            transform_matrix[14] * differences[4] +
            transform_matrix[17] * differences[5]);

        //  muliply by -1/ambient_density
        float ambient_density = 1.225;
        multiplied /= -ambient_density;

        //  numerical integration
        //
        //  I thought integration meant just adding to the previous value like
        //  *velocity_buffer += multiplied;
        //  but apparently the integrator has the transfer function
        //  Hint(z) = Tz^-1 / (1 - z^-1)
        //  so hopefully this is right
        //
        //  Hint(z) = Y(z)/X(z) = Tz^-1/(1 - z^-1)
        //  y(n) = Tx(n - 1) + y(n - 1)

        *velocity_buffer += T * multiplied;
    }
}
)"};
