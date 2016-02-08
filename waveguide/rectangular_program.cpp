#include "rectangular_program.h"

#include <iostream>

RectangularProgram::RectangularProgram(const cl::Context & context,
                                       bool build_immediate)
        : Program(context, source, build_immediate) {
    //            std::cout << source << std::endl;
}

const std::string RectangularProgram::source{
    "#define PORTS (" + std::to_string(PORTS) + ")\n" +
    "#define BIQUAD_SECTIONS (" + std::to_string(BIQUAD_SECTIONS) +
    ")\n"
#ifdef DIAGNOSTIC
    "#define DIAGNOSTIC\n"
#endif
    R"(
#define COURANT (1.0 / sqrt(3.0))
#define BIQUAD_ORDER (2)
#define CANONICAL_FILTER_ORDER (BIQUAD_SECTIONS * BIQUAD_ORDER)

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
} Node;

typedef struct {
    float array[BIQUAD_ORDER];
} BiquadMemory;

typedef struct {
    float b[BIQUAD_ORDER + 1];
    float a[BIQUAD_ORDER + 1];
} BiquadCoefficients;

typedef struct {
    BiquadMemory array[BIQUAD_SECTIONS];
} BiquadMemoryArray;

typedef struct {
    BiquadCoefficients array[BIQUAD_SECTIONS];
} BiquadCoefficientsArray;

typedef struct {
    float array[CANONICAL_FILTER_ORDER];
} CanonicalMemoryArray;

typedef struct {
    float b[CANONICAL_FILTER_ORDER + 1];
    float a[CANONICAL_FILTER_ORDER + 1];
} CanonicalCoefficientsArray;

float biquad_step(float input,
                  global BiquadMemory * m,
                  const global BiquadCoefficients * c);
float biquad_step(float input,
                  global BiquadMemory * m,
                  const global BiquadCoefficients * c) {
    int i = 0;
    float out = (input * c->b[i] + m->array[i]) / c->a[i];
    for (; i != BIQUAD_ORDER - 1; ++i) {
        m->array[i] = i * c->b[i + 1] - c->a[i + 1] * out + m->array[i + 1];
    }
    m->array[i] = i * c->b[i + 1] - c->a[i + 1] * out;
    return out;
}

float canonical_filter_step(float input,
                            global CanonicalMemoryArray * m,
                            const global CanonicalCoefficientsArray * c);
float canonical_filter_step(float input,
                            global CanonicalMemoryArray * m,
                            const global CanonicalCoefficientsArray * c) {
    int i = 0;
    float out = (input * c->b[i] + m->array[i]) / c->a[i];
    for (; i != CANONICAL_FILTER_ORDER - 1; ++i) {
        m->array[i] = i * c->b[i + 1] - c->a[i + 1] * out + m->array[i + 1];
    }
    m->array[i] = i * c->b[i + 1] - c->a[i + 1] * out;
    return out;
}

float biquad_cascade(float input,
                     global BiquadMemoryArray * bm,
                     const global BiquadCoefficientsArray * bc);
float biquad_cascade(float input,
                     global BiquadMemoryArray * bm,
                     const global BiquadCoefficientsArray * bc) {
    for (int i = 0; i != BIQUAD_SECTIONS; ++i) {
        input = biquad_step(input, bm->array + i, bc->array + i);
    }
    return input;
}

kernel void filter_test(const global float * input,
                        global float * output,
                        global BiquadMemoryArray * biquad_memory,
                        const global BiquadCoefficientsArray * biquad_coefficients) {
    size_t index = get_global_id(0);
    output[index] = biquad_cascade
    (   input[index]
    ,   biquad_memory + index
    ,   biquad_coefficients + index
    );
}

kernel void filter_test_2(const global float * input,
                          global float * output,
                          global CanonicalMemoryArray * canonical_memory,
                          const global CanonicalCoefficientsArray * canonical_coefficients) {
    size_t index = get_global_id(0);
    output[index] = canonical_filter_step
    (   input[index]
    ,   canonical_memory + index
    ,   canonical_coefficients + index
    );
}

typedef struct {
    float sk_current    [1];
    float sk_previous   [1];
    float sm_current    [1];
    float sm_previous   [1];
    float ghost_current [1];
    float ghost_previous[1];
    BiquadMemoryArray biquad_memory[1];
} BoundaryData1;

typedef struct {
    float sk_current    [2];
    float sk_previous   [2];
    float sm_current    [2];
    float sm_previous   [2];
    float ghost_current [2];
    float ghost_previous[2];
    BiquadMemoryArray biquad_memory[2];
} BoundaryData2;

typedef struct {
    float sk_current    [3];
    float sk_previous   [3];
    float sm_current    [3];
    float sm_previous   [3];
    float ghost_current [3];
    float ghost_previous[3];
    BiquadMemoryArray biquad_memory[3];
} BoundaryData3;

typedef enum {
    id_port_nx = 0,
    id_port_px = 1,
    id_port_ny = 2,
    id_port_py = 3,
    id_port_nz = 4,
    id_port_pz = 5,
} PortDirection;

PortDirection get_inner_node_direction(BoundaryType boundary_type);
PortDirection get_inner_node_direction(BoundaryType boundary_type) {
    switch(boundary_type) {
    case id_nx: return id_port_px;
    case id_px: return id_port_nx;
    case id_ny: return id_port_py;
    case id_py: return id_port_ny;
    case id_nz: return id_port_pz;
    case id_pz: return id_port_nz;

    default: return -1;
    }
}

int get_inner_node_index(const global Node * node, BoundaryType bt);
int get_inner_node_index(const global Node * node, BoundaryType bt) {
    return node->ports[get_inner_node_direction(bt)];
}

PortDirection opposite(PortDirection pd);
PortDirection opposite(PortDirection pd) {
    switch(pd) {
    case id_port_nx: return id_port_px;
    case id_port_px: return id_port_nx;
    case id_port_ny: return id_port_py;
    case id_port_py: return id_port_ny;
    case id_port_nz: return id_port_pz;
    case id_port_pz: return id_port_nz;

    default: return -1;
    }
}

//  call with the index of the BOUNDARY node, and the relative direction of the
//  ghost point
float ghost_point_pressure(const global float * current,
                           global float * previous,
                           const global Node * nodes,
                           BoundaryType bt);
float ghost_point_pressure(const global float * current,
                           global float * previous,
                           const global Node * nodes,
                           BoundaryType bt) {
    /*
    size_t index = get_global_id(0);
    const global Node * boundary_node = nodes + index;
    int inner_node_index = get_inner_node_index(boundary_node, bt);

    float current_inner_pressure = current[inner_node_index];
    */
    return 0;
}

void boundary_1d(const global Node * node);
void boundary_1d(const global Node * node) {

}


/*
 *

//  1d boundary

//  node ports are stored: -x, x, -y, y, -z, z
float courant = 1 / sqrt(3);

float next_boundary_pressure = ...;

BoundaryType bt;
Node node;
BoundaryData1 bd;
float current_ghost_pressure =
(   current[node.ports[get_inner_node(bt)]]
+   (a_coeffs[0] * (previous[index] - next_boundary_pressure))
/   (courant * b_coeffs[0])
+   (gn / b_coeffs[0])
);


 *
 */

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

kernel void waveguide
(   const global float * current
,   global float * previous
,   const global Node * nodes
,   global BoundaryData1 * boundary_data_1
,   global BoundaryData2 * boundary_data_2
,   global BoundaryData3 * boundary_data_3
,   const global float * transform_matrix
,   global float3 * velocity_buffer
,   float spatial_sampling_period
,   float T
,   ulong read
,   global float * output
) {
    size_t index = get_global_id(0);
    const global Node * node = nodes + index;

    float next_pressure = 0;

    //  find the next pressure at this node, assign it to next_pressure
    switch(popcount(node->bt)) {
        //  this is inside or outside, not a boundary
        case 0:
            if (node->inside) {
                for (int i = 0; i != PORTS; ++i) {
                    int port_index = node->ports[i];
                    if (port_index >= 0 && nodes[port_index].inside)
                        next_pressure += current[port_index];
                }

                next_pressure /= (PORTS / 2);
                next_pressure -= previous[index];
            }
            break;
        //  this is a 1d-boundary node
        case 1:
            break;
        //  this is an edge where two boundaries meet
        case 2:
            break;
        //  this is a corner where three boundaries meet
        case 3:
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
            if (port_index >= 0 && nodes[port_index].inside)
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
