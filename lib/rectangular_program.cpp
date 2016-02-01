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


/*

float p = <air density>
float c = <speed of sound>
float T = <time step>
float R = <resistance>
float K = <spring constant>
float M = <mass per unit area>

float courant = 1 / sqrt(3);
float cs = courant * courant;

float ar = R / (p * c);
float ak = (T * K) / (2 * p * c);
float am = (2 * M) / (T * p * c);
float a = (ar + ak + am);

//  1 boundary update formula

//  ghost
g = ((ak - am) / a) * (previous(x - 1, y, z) - previous(x + 1, y, z)) +
    (ak * Sk + am * Sm) / a;

//current(x + 1, y, z) = current(x - 1, y, z) + (previous(x, y, z) - next(x, y, z)) / (courant * a) + g
//current(x + 1, y, z) - current(x - 1, y, z) = (previous(x, y, z) - next(x, y, z)) / (courant * a) + g
//courant * a * (current(x + 1, y, z) - current(x - 1, y, z) = previous(x, y, z) - next(x, y, z) + g
next(x, y, z) = previous(x, y, z) - courant * a * (current(x + 1, y, z) - current(x - 1, y, z)) + g

//  boundary
next(x, y, z) =
(   cs
*   (   2 * current(x - 1, y, z)
    +       current(x, y + 1, z) + current(x, y - 1, z)
    +       current(x, y, z + 1) + current(x, y, z - 1)
    )
+   2 * (1 - 3 * cs) * current(x, y, z)
+   (courant / a - 1) * previous(x, y, z)
+   (cs * (ak - am) / a) * (previous(x - 1, y, z) - previous(x + 1, y, z))
+   (cs * ak / a) * Sk + (cs * am / a) * Sm
) / (1 + courant / a);

Sk = current(x - 1, y, z) - current(x + 1, y, z) + previous(x - 1, y, z) - previous(x + 1, y, z) + Sk_prev;
Sm = current(x - 1, y, z) - current(x + 1, y, z) + previous(x - 1, y, z) - previous(x + 1, y, z) - Sm_prev;

//  each boundary node must store values of Sk, Sm, boundary node, and ghost point

//  current ghost point state relies on
//      previous and current mesh state
//      previous and next boundary state
//      previous ghost point state
//  current Sk and Sm relies on
//      previous and current mesh state
//      previous and current ghost point state
//  next boundary node state relies on
//      current mesh state
//      previous and current boundary node state
//      previous ghost point state

*/

typedef enum {
    id_none = 0,
    id_nx = 1 << 1,
    id_px = 1 << 2,
    id_ny = 1 << 3,
    id_py = 1 << 4,
    id_nz = 1 << 5,
    id_pz = 1 << 6,
    id_reentrant = 1 << 7,
} BoundaryType;

typedef struct {
    int ports[PORTS];
    float3 position;
    bool inside;
    int bt;
} RectNode;

kernel void waveguide
(   const global float * current
,   global float * previous
,   const global RectNode * nodes
,   const global float * transform_matrix
,   global float3 * velocity_buffer
,   float spatial_sampling_period
,   float T
,   ulong read
,   global float * output
) {
    size_t index = get_global_id(0);
    const global RectNode * node = nodes + index;

    float next_pressure = 0;

    //  find the next pressure at this node, assign it to next_pressure
    switch(popcount(node->bt)) {
        //  this is inside or outside, not a boundary
        case 0:
            if (node->inside) {
                for (int i = 0; i != PORTS; ++i) {
                    int port_index = node->ports[i];
                    if (port_index >= 0 && nodes[port_index].inside == id_inside)
                        next_pressure += current[port_index];
                }

                next_pressure /= (PORTS / 2);
                next_pressure -= previous[index];
            }
            break;
        //  this is a 1d-boundary node
        case 1:
            printf("%s\n", "1d boundary");
            break;
        //  this is an edge where two boundaries meet
        case 2:
            printf("%s\n", "2d boundary");
            break;
        //  this is a corner where three boundaries meet
        case 3:
            printf("%s\n", "3d boundary");
            break;
    }

    barrier(CLK_GLOBAL_MEM_FENCE);
    previous[index] = next_pressure;
    barrier(CLK_GLOBAL_MEM_FENCE);

    if (index == read) {
        *output = previous[index];

        //
        //  instantaneous intensity for mic modelling
        //

        float differences[PORTS] = {0};
        for (int i = 0; i != PORTS; ++i) {
            int port_index = node->ports[i];
            if (port_index >= 0 && nodes[port_index].inside == id_inside)
                differences[i] = (previous[port_index] - previous[index]) /
                    spatial_sampling_period;
        }

        //  the default for Eigen is column-major matrices
        //  so we'll assume that transform_matrix is column-major

        //  multiply differences by transformation matrix
        float3 multiplied = (float3)(0);
        for (int i = 0; i != PORTS; ++i) {
            multiplied += (float3)(
                transform_matrix[0 + i * 3],
                transform_matrix[1 + i * 3],
                transform_matrix[2 + i * 3]
            ) * differences[i];
        }

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
