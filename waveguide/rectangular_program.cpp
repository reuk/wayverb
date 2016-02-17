#include "rectangular_program.h"

#include <iostream>

RectangularProgram::RectangularProgram(const cl::Context & context,
                                       bool build_immediate)
        : Program(context, source, build_immediate) {
}

const std::string RectangularProgram::source{
    "#define PORTS " + std::to_string(PORTS) + "\n" +
    "#define BIQUAD_SECTIONS " + std::to_string(BIQUAD_SECTIONS) + "\n" +
    "#define BIQUAD_ORDER " + std::to_string(2) + "\n" +
    "#define CANONICAL_FILTER_ORDER " + std::to_string(BIQUAD_SECTIONS * 2) +
    "\n"
#ifdef DIAGNOSTIC
    "#define DIAGNOSTIC\n"
#endif
    R"(
#define COURANT (1.0 / sqrt(3.0))
#define COURANT_SQ (1.0 / 3.0)

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
    int boundary_index;
} Node;

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a##__VA_ARGS__

#define TEMPLATE_FILTER_MEMORY(order) \
    typedef struct { float array[order]; } CAT(FilterMemory, order);

TEMPLATE_FILTER_MEMORY(BIQUAD_ORDER);
TEMPLATE_FILTER_MEMORY(CANONICAL_FILTER_ORDER);

#define TEMPLATE_FILTER_COEFFICIENTS(order) \
    typedef struct {                        \
        float b[order + 1];                 \
        float a[order + 1];                 \
    } CAT(FilterCoefficients, order);

TEMPLATE_FILTER_COEFFICIENTS(BIQUAD_ORDER);
TEMPLATE_FILTER_COEFFICIENTS(CANONICAL_FILTER_ORDER);

typedef struct { FilterMemory2 array[BIQUAD_SECTIONS]; } BiquadMemoryArray;
typedef struct {
    FilterCoefficients2 array[BIQUAD_SECTIONS];
} BiquadCoefficientsArray;

//  we assume that a0 == 1.0f
//  that is, the FILTER COEFFICIENTS ARE NORMALISED
#define FILTER_STEP(order)                                                 \
    float CAT(filter_step_, order)(                                        \
        float input,                                                       \
        global CAT(FilterMemory, order) * m,                               \
        const global CAT(FilterCoefficients, order) * c);                  \
    float CAT(filter_step_, order)(                                        \
        float input,                                                       \
        global CAT(FilterMemory, order) * m,                               \
        const global CAT(FilterCoefficients, order) * c) {                 \
        int i = 0;                                                         \
        float out = input * c->b[i] + m->array[i];                         \
        for (; i != order - 1; ++i) {                                      \
            m->array[i] =                                                  \
                input * c->b[i + 1] - c->a[i + 1] * out + m->array[i + 1]; \
        }                                                                  \
        m->array[i] = input * c->b[i + 1] - c->a[i + 1] * out;             \
        return out;                                                        \
    }

FILTER_STEP(BIQUAD_ORDER);
FILTER_STEP(CANONICAL_FILTER_ORDER);

float biquad_cascade(float input,
                     global BiquadMemoryArray* bm,
                     const global BiquadCoefficientsArray* bc);
float biquad_cascade(float input,
                     global BiquadMemoryArray* bm,
                     const global BiquadCoefficientsArray* bc) {
    for (int i = 0; i != BIQUAD_SECTIONS; ++i) {
        input = CAT(filter_step_, BIQUAD_ORDER)(
            input, bm->array + i, bc->array + i);
    }
    return input;
}

kernel void filter_test(
    const global float* input,
    global float* output,
    global BiquadMemoryArray* biquad_memory,
    const global BiquadCoefficientsArray* biquad_coefficients) {
    size_t index = get_global_id(0);
    output[index] = biquad_cascade(
        input[index], biquad_memory + index, biquad_coefficients + index);
}

kernel void filter_test_2(const global float* input,
                          global float* output,
                          global CAT(FilterMemory, CANONICAL_FILTER_ORDER) *
                              canonical_memory,
                          const global CAT(FilterCoefficients,
                                           CANONICAL_FILTER_ORDER) *
                              canonical_coefficients) {
    size_t index = get_global_id(0);
    output[index] = CAT(filter_step_, CANONICAL_FILTER_ORDER)(
        input[index], canonical_memory + index, canonical_coefficients + index);
}

#define PRINT_SIZEOF(x) printf("gpu: sizeof(" #x "): %i\n", sizeof(x));

kernel void print_sizes() {
    if (index == 0) {
        PRINT_SIZEOF(BiquadMemoryArray);
        PRINT_SIZEOF(BiquadCoefficientsArray);
        PRINT_SIZEOF(CAT(FilterMemory, CANONICAL_FILTER_ORDER));
        PRINT_SIZEOF(CAT(FilterCoefficients, CANONICAL_FILTER_ORDER));
    }
}

typedef struct {
    CAT(FilterMemory, CANONICAL_FILTER_ORDER) filter_memory;
    int coefficient_index;
} BoundaryData;

typedef struct { BoundaryData array[1]; } BoundaryDataArray1;
typedef struct { BoundaryData array[2]; } BoundaryDataArray2;
typedef struct { BoundaryData array[3]; } BoundaryDataArray3;

typedef enum {
    id_port_nx = 0,
    id_port_px = 1,
    id_port_ny = 2,
    id_port_py = 3,
    id_port_nz = 4,
    id_port_pz = 5,
} PortDirection;

typedef struct { PortDirection array[1]; } InnerNodeDirections1;
typedef struct { PortDirection array[2]; } InnerNodeDirections2;
typedef struct { PortDirection array[3]; } InnerNodeDirections3;

InnerNodeDirections1 get_inner_node_directions_1(BoundaryType boundary_type);
InnerNodeDirections1 get_inner_node_directions_1(BoundaryType boundary_type) {
    switch (boundary_type) {
        case id_nx:
            return (InnerNodeDirections1){{id_port_px}};
        case id_px:
            return (InnerNodeDirections1){{id_port_nx}};
        case id_ny:
            return (InnerNodeDirections1){{id_port_py}};
        case id_py:
            return (InnerNodeDirections1){{id_port_ny}};
        case id_nz:
            return (InnerNodeDirections1){{id_port_pz}};
        case id_pz:
            return (InnerNodeDirections1){{id_port_nz}};

        default:
            return (InnerNodeDirections1){{-1}};
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

        default:
            return (InnerNodeDirections2){{-1, -1}};
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

        default:
            return (InnerNodeDirections3){{-1, -1, -1}};
    }
}

PortDirection opposite(PortDirection pd);
PortDirection opposite(PortDirection pd) {
    switch (pd) {
        case id_port_nx:
            return id_port_px;
        case id_port_px:
            return id_port_nx;
        case id_port_ny:
            return id_port_py;
        case id_port_py:
            return id_port_ny;
        case id_port_nz:
            return id_port_pz;
        case id_port_pz:
            return id_port_nz;

        default:
            return -1;
    }
}

typedef struct { PortDirection array[4]; } PlanePorts;

PlanePorts on_boundary_1(InnerNodeDirections1 pd);
PlanePorts on_boundary_1(InnerNodeDirections1 pd) {
    switch (pd.array[0]) {
        case id_port_nx:
        case id_port_px:
            return (PlanePorts){
                {id_port_ny, id_port_py, id_port_nz, id_port_pz}};
        case id_port_ny:
        case id_port_py:
            return (PlanePorts){
                {id_port_nx, id_port_px, id_port_nz, id_port_pz}};
        case id_port_nz:
        case id_port_pz:
            return (PlanePorts){
                {id_port_nx, id_port_px, id_port_ny, id_port_py}};

        default:
            return (PlanePorts){{-1, -1, -1, -1}};
    }
}

typedef struct { PortDirection array[2]; } AxisPorts;

AxisPorts on_boundary_2(InnerNodeDirections2 ind);
AxisPorts on_boundary_2(InnerNodeDirections2 ind) {
    if (ind.array[0] == id_port_nx || ind.array[0] == id_port_px ||
        ind.array[1] == id_port_nx || ind.array[1] == id_port_px) {
        if (ind.array[0] == id_port_ny || ind.array[0] == id_port_py ||
            ind.array[1] == id_port_ny || ind.array[1] == id_port_py) {
            return (AxisPorts){{id_port_nz, id_port_pz}};
        }
        return (AxisPorts){{id_port_ny, id_port_py}};
    }
    return (AxisPorts){{id_port_nx, id_port_px}};
}

float sum_on_plane(const global float* current,
                   const global Node* node,
                   InnerNodeDirections1 pd);
float sum_on_plane(const global float* current,
                   const global Node* node,
                   InnerNodeDirections1 pd) {
    float ret = 0;
    PlanePorts on_boundary = on_boundary_1(pd);
    for (int i = 0; i != 4; ++i) {
        int index = node->ports[on_boundary.array[i]];
        if (index == -1) {
            //  TODO error!
        }
        ret += current[index];
    }
    return ret;
}

float sum_on_axis(const global float* current,
                  const global Node* node,
                  InnerNodeDirections2 pd);
float sum_on_axis(const global float* current,
                  const global Node* node,
                  InnerNodeDirections2 pd) {
    float ret = 0;
    AxisPorts on_boundary = on_boundary_2(pd);
    for (int i = 0; i != 2; ++i) {
        int index = node->ports[on_boundary.array[i]];
        if (index == -1) {
            //  TODO error!
        }
        ret += current[index];
    }
    return ret;
}

//  call with the index of the BOUNDARY node, and the relative direction of the
//  ghost point
//
//  we don't actually care about the pressure at the ghost point other than to
//  calculate the boundary filter input
void ghost_point_pressure_update(const global float* current,
                                 const global Node* boundary_node,
                                 float next_pressure,
                                 float prev_pressure,
                                 global BoundaryData* boundary_data,
                                 const global CAT(FilterCoefficients,
                                                  CANONICAL_FILTER_ORDER) *
                                     boundary,
                                 PortDirection inner_direction);
void ghost_point_pressure_update(const global float* current,
                                 const global Node* boundary_node,
                                 float next_pressure,
                                 float prev_pressure,
                                 global BoundaryData* boundary_data,
                                 const global CAT(FilterCoefficients,
                                                  CANONICAL_FILTER_ORDER) *
                                     boundary,
                                 PortDirection inner_direction) {
    int inner_index = boundary_node->ports[inner_direction];
    float inner_pressure = current[inner_index];

    float filt_state = boundary_data->filter_memory.array[0];

    float b0 = boundary->b[0];
    float a0 = boundary->a[0];

    float ret = inner_pressure +
                (a0 * (prev_pressure - next_pressure)) / (b0 * COURANT) +
                filt_state / b0;

    //  now we can update the filter at this boundary node
    float filter_input = inner_pressure - ret;
    CAT(filter_step_, CANONICAL_FILTER_ORDER)(filter_input, &boundary_data->filter_memory, boundary);
}

//----------------------------------------------------------------------------//

float get_summed_surrounding_1(InnerNodeDirections1 i,
                               const global float* current,
                               const global Node* boundary_node);
float get_summed_surrounding_1(InnerNodeDirections1 i,
                               const global float* current,
                               const global Node* boundary_node) {
    return sum_on_plane(current, boundary_node, i);
}

float get_summed_surrounding_2(InnerNodeDirections2 i,
                               const global float* current,
                               const global Node* boundary_node);
float get_summed_surrounding_2(InnerNodeDirections2 i,
                               const global float* current,
                               const global Node* boundary_node) {
    return sum_on_axis(current, boundary_node, i);
}

float get_summed_surrounding_3(InnerNodeDirections3 i,
                               const global float* current,
                               const global Node* boundary_node);
float get_summed_surrounding_3(InnerNodeDirections3 i,
                               const global float* current,
                               const global Node* boundary_node) {
    return 0;
}

//----------------------------------------------------------------------------//

#define GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(dimensions)               \
    float CAT(get_current_surrounding_weighting_, dimensions)(               \
        const global float* current,                                         \
        const global Node* boundary_node,                                    \
        int bt);                                                             \
    float CAT(get_current_surrounding_weighting_, dimensions)(               \
        const global float* current,                                         \
        const global Node* boundary_node,                                    \
        int bt) {                                                            \
        CAT(InnerNodeDirections, dimensions)                                 \
        ind = CAT(get_inner_node_directions_, dimensions)(bt);               \
        float sum = 0;                                                       \
        for (int i = 0; i != dimensions; ++i) {                              \
            sum += 2 * current[boundary_node->ports[ind.array[i]]];          \
        }                                                                    \
        return COURANT_SQ * (sum + CAT(get_summed_surrounding_, dimensions)( \
                                       ind, current, boundary_node));        \
    }

GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(1);
GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(2);
GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(3);

//----------------------------------------------------------------------------//

float get_current_boundary_weighting(const global float* current);
float get_current_boundary_weighting(const global float* current) {
    return 2 * (1 - 3 * COURANT_SQ) * current[get_global_id(0)];
}

//----------------------------------------------------------------------------//

#define GET_FILTER_WEIGHTING_TEMPLATE(dimensions)                        \
    float CAT(get_filter_weighting_, dimensions)(                        \
        global CAT(BoundaryDataArray, dimensions) * boundary_data,       \
        const global Node* boundary_node,                                \
        const global CAT(FilterCoefficients, CANONICAL_FILTER_ORDER) *   \
            boundary_coefficients);                                      \
    float CAT(get_filter_weighting_, dimensions)(                        \
        global CAT(BoundaryDataArray, dimensions) * boundary_data,       \
        const global Node* boundary_node,                                \
        const global CAT(FilterCoefficients, CANONICAL_FILTER_ORDER) *   \
            boundary_coefficients) {                                     \
        global CAT(BoundaryDataArray, dimensions)* bda =                 \
            boundary_data + boundary_node->boundary_index;               \
        float sum = 0;                                                   \
        for (int i = 0; i != dimensions; ++i) {                          \
            const global CAT(FilterCoefficients,                         \
                             CANONICAL_FILTER_ORDER)* boundary =         \
                boundary_coefficients + bda->array[i].coefficient_index; \
            float filt_state = bda->array[i].filter_memory.array[0];     \
            float b0 = boundary->b[0];                                   \
            sum += filt_state / b0;                                      \
        }                                                                \
        return COURANT_SQ * sum;                                         \
    }

GET_FILTER_WEIGHTING_TEMPLATE(1);
GET_FILTER_WEIGHTING_TEMPLATE(2);
GET_FILTER_WEIGHTING_TEMPLATE(3);

//----------------------------------------------------------------------------//

#define GET_COEFF_WEIGHTING_TEMPLATE(dimensions)                         \
    float CAT(get_coeff_weighting_, dimensions)(                         \
        global CAT(BoundaryDataArray, dimensions) * boundary_data,       \
        const global Node* boundary_node,                                \
        const global CAT(FilterCoefficients, CANONICAL_FILTER_ORDER) *   \
            boundary_coefficients);                                      \
    float CAT(get_coeff_weighting_, dimensions)(                         \
        global CAT(BoundaryDataArray, dimensions) * boundary_data,       \
        const global Node* boundary_node,                                \
        const global CAT(FilterCoefficients, CANONICAL_FILTER_ORDER) *   \
            boundary_coefficients) {                                     \
        global CAT(BoundaryDataArray, dimensions)* bda =                 \
            boundary_data + boundary_node->boundary_index;               \
        float sum = 0;                                                   \
        for (int i = 0; i != dimensions; ++i) {                          \
            const global CAT(FilterCoefficients,                         \
                             CANONICAL_FILTER_ORDER)* boundary =         \
                boundary_coefficients + bda->array[i].coefficient_index; \
            sum += COURANT * boundary->a[0] / boundary->b[0];            \
        }                                                                \
        return sum;                                                      \
    }

GET_COEFF_WEIGHTING_TEMPLATE(1);
GET_COEFF_WEIGHTING_TEMPLATE(2);
GET_COEFF_WEIGHTING_TEMPLATE(3);

//----------------------------------------------------------------------------//

#define BOUNDARY_TEMPLATE(dimensions)                                     \
    float CAT(boundary_, dimensions)(                                     \
        const global float* current,                                      \
        const global float* previous,                                     \
        const global Node* boundary_node,                                 \
        global CAT(BoundaryDataArray, dimensions) * boundary_data,        \
        const global CAT(FilterCoefficients, CANONICAL_FILTER_ORDER) *    \
            boundary_coefficients,                                        \
        int bt);                                                          \
    float CAT(boundary_, dimensions)(                                     \
        const global float* current,                                      \
        const global float* previous,                                     \
        const global Node* boundary_node,                                 \
        global CAT(BoundaryDataArray, dimensions) * boundary_data,        \
        const global CAT(FilterCoefficients, CANONICAL_FILTER_ORDER) *    \
            boundary_coefficients,                                        \
        int bt) {                                                         \
        float current_surrounding_weighting =                             \
            CAT(get_current_surrounding_weighting_, dimensions)(          \
                current, boundary_node, bt);                              \
        float current_boundary_weighting =                                \
            get_current_boundary_weighting(current);                      \
        float filter_weighting = CAT(get_filter_weighting_, dimensions)(  \
            boundary_data, boundary_node, boundary_coefficients);         \
        float coeff_weighting = CAT(get_coeff_weighting_, dimensions)(    \
            boundary_data, boundary_node, boundary_coefficients);         \
        float prev_pressure = previous[get_global_id(0)];                 \
        float prev_weighting = (coeff_weighting - 1) * prev_pressure;     \
        float ret =                                                       \
            (current_surrounding_weighting + current_boundary_weighting + \
             filter_weighting + prev_weighting) /                         \
            (1 + coeff_weighting);                                        \
        CAT(InnerNodeDirections, dimensions)                              \
        inner_node_directions =                                           \
            CAT(get_inner_node_directions_, dimensions)(bt);              \
        global CAT(BoundaryDataArray, dimensions)* bda =                  \
            boundary_data + boundary_node->boundary_index;                \
        for (int i = 0; i != dimensions; ++i) {                           \
            const global CAT(FilterCoefficients,                          \
                             CANONICAL_FILTER_ORDER)* boundary =          \
                boundary_coefficients + bda->array[i].coefficient_index;  \
            ghost_point_pressure_update(current,                          \
                                        boundary_node,                    \
                                        ret,                              \
                                        prev_pressure,                    \
                                        &bda->array[i],                   \
                                        boundary,                         \
                                        inner_node_directions.array[i]);  \
        }                                                                 \
        return ret;                                                       \
    }

BOUNDARY_TEMPLATE(1);
BOUNDARY_TEMPLATE(2);
BOUNDARY_TEMPLATE(3);

//----------------------------------------------------------------------------//

kernel void waveguide(const global float* current,
                      global float* previous,
                      const global Node* nodes,
                      global BoundaryDataArray1* boundary_data_1,
                      global BoundaryDataArray2* boundary_data_2,
                      global BoundaryDataArray3* boundary_data_3,
                      const global CAT(FilterCoefficients,
                                       CANONICAL_FILTER_ORDER) *
                          boundary_coefficients,
                      const global float* transform_matrix,
                      global float3* velocity_buffer,
                      float spatial_sampling_period,
                      float T,
                      ulong read,
                      global float* output) {
    size_t index = get_global_id(0);
    const global Node* node = nodes + index;

    float next_pressure = 0;

    //  find the next pressure at this node, assign it to next_pressure
    switch (popcount(node->bt)) {
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
            next_pressure = boundary_1(current,
                                       previous,
                                       node,
                                       boundary_data_1,
                                       boundary_coefficients,
                                       node->bt);
            break;
        //  this is an edge where two boundaries meet
        case 2:
            next_pressure = boundary_2(current,
                                       previous,
                                       node,
                                       boundary_data_2,
                                       boundary_coefficients,
                                       node->bt);
            break;
        //  this is a corner where three boundaries meet
        case 3:
            next_pressure = boundary_3(current,
                                       previous,
                                       node,
                                       boundary_data_3,
                                       boundary_coefficients,
                                       node->bt);
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
            multiplied += (float3)(transform_matrix[0 + i * 3],
                                   transform_matrix[1 + i * 3],
                                   transform_matrix[2 + i * 3]) *
                          differences[i];
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
