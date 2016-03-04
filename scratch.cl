#define COURANT (1.0f / sqrt(3.0f))
#define COURANT_SQ (1.0f / 3.0f)

#define NO_NEIGHBOR (~(uint)0)

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
    id_success = 0,
    id_inf_error = 1 << 0,
    id_nan_error = 1 << 1,
} ErrorCode;

typedef struct {
    uint ports[PORTS];
    float3 position;
    bool inside;
    int boundary_type;
    uint boundary_index;
} Node;

typedef struct {
    int boundary_type;
    uint boundary_index;
} CondensedNode;

#define CAT(a, b) PRIMITIVE_CAT(a, b)
#define PRIMITIVE_CAT(a, b) a##b

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

#define FilterMemoryBiquad CAT(FilterMemory, BIQUAD_ORDER)
#define FilterMemoryCanonical CAT(FilterMemory, CANONICAL_FILTER_ORDER)
#define FilterCoefficientsBiquad CAT(FilterCoefficients, BIQUAD_ORDER)
#define FilterCoefficientsCanonical \
    CAT(FilterCoefficients, CANONICAL_FILTER_ORDER)

typedef struct { FilterMemoryBiquad array[BIQUAD_SECTIONS]; } BiquadMemoryArray;
typedef struct {
    FilterCoefficientsBiquad array[BIQUAD_SECTIONS];
} BiquadCoefficientsArray;

//  we assume that a0 == 1.0f
//  that is, the FILTER COEFFICIENTS ARE NORMALISED
#define FILTER_STEP(order)                                                    \
    float CAT(filter_step_, order)(                                           \
        float input,                                                          \
        global CAT(FilterMemory, order) * m,                                  \
        const global CAT(FilterCoefficients, order) * c);                     \
    float CAT(filter_step_, order)(                                           \
        float input,                                                          \
        global CAT(FilterMemory, order) * m,                                  \
        const global CAT(FilterCoefficients, order) * c) {                    \
        float output = input * c->b[0] + m->array[0];                         \
        for (int i = 0; i != order - 1; ++i) {                                \
            m->array[i] =                                                     \
                input * c->b[i + 1] - c->a[i + 1] * output + m->array[i + 1]; \
        }                                                                     \
        m->array[order - 1] = input * c->b[order] - c->a[order] * output;     \
        return output;                                                        \
    }

FILTER_STEP(BIQUAD_ORDER);
FILTER_STEP(CANONICAL_FILTER_ORDER);

#define filter_step_biquad CAT(filter_step_, BIQUAD_ORDER)
#define filter_step_canonical CAT(filter_step_, CANONICAL_FILTER_ORDER)

float biquad_cascade(float input,
                     global BiquadMemoryArray* bm,
                     const global BiquadCoefficientsArray* bc);
float biquad_cascade(float input,
                     global BiquadMemoryArray* bm,
                     const global BiquadCoefficientsArray* bc) {
    for (int i = 0; i != BIQUAD_SECTIONS; ++i) {
        input = filter_step_biquad(input, bm->array + i, bc->array + i);
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

kernel void filter_test_2(
    const global float* input,
    global float* output,
    global FilterMemoryCanonical* canonical_memory,
    const global FilterCoefficientsCanonical* canonical_coefficients) {
    size_t index = get_global_id(0);
    output[index] = filter_step_canonical(
        input[index], canonical_memory + index, canonical_coefficients + index);
}

#define PRINT_SIZEOF(x) printf("gpu: sizeof(" #x "): %i\n", sizeof(x));

typedef struct {
    FilterMemoryCanonical filter_memory;
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

bool locator_outside(int3 locator, int3 dimensions);
bool locator_outside(int3 locator, int3 dimensions) {
    return any(locator < (int3)(0)) || any(dimensions <= locator);
}

int3 to_locator(size_t index, int3 dim);
int3 to_locator(size_t index, int3 dim) {
    int xrem = index % dim.x, xquot = index / dim.x;
    int yrem = xquot % dim.y, yquot = xquot / dim.y;
    int zrem = yquot % dim.z;
    return (int3)(xrem, yrem, zrem);
}

size_t to_index(int3 locator, int3 dim);
size_t to_index(int3 locator, int3 dim) {
    return locator.x + locator.y * dim.x + locator.z * dim.x * dim.y;
}

uint neighbor_index(int3 locator, int3 dimensions, PortDirection pd);
uint neighbor_index(int3 locator, int3 dimensions, PortDirection pd) {
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
    if (locator_outside(locator, dimensions))
        return NO_NEIGHBOR;
    return to_index(locator, dimensions);
}

typedef struct { PortDirection array[1]; } InnerNodeDirections1;
typedef struct { PortDirection array[2]; } InnerNodeDirections2;
typedef struct { PortDirection array[3]; } InnerNodeDirections3;

InnerNodeDirections1 get_inner_node_directions_1(BoundaryType boundary_type);
InnerNodeDirections1 get_inner_node_directions_1(BoundaryType boundary_type) {
    switch (boundary_type) {
        case id_nx:
            return (InnerNodeDirections1){{id_port_nx}};
        case id_px:
            return (InnerNodeDirections1){{id_port_px}};
        case id_ny:
            return (InnerNodeDirections1){{id_port_ny}};
        case id_py:
            return (InnerNodeDirections1){{id_port_py}};
        case id_nz:
            return (InnerNodeDirections1){{id_port_nz}};
        case id_pz:
            return (InnerNodeDirections1){{id_port_pz}};

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

#define NUM_SURROUNDING_PORTS_1 4
#define NUM_SURROUNDING_PORTS_2 2

typedef struct { PortDirection array[4]; } SurroundingPorts1;
typedef struct { PortDirection array[2]; } SurroundingPorts2;

SurroundingPorts1 on_boundary_1(InnerNodeDirections1 pd);
SurroundingPorts1 on_boundary_1(InnerNodeDirections1 pd) {
    switch (pd.array[0]) {
        case id_port_nx:
        case id_port_px:
            return (SurroundingPorts1){
                {id_port_ny, id_port_py, id_port_nz, id_port_pz}};
        case id_port_ny:
        case id_port_py:
            return (SurroundingPorts1){
                {id_port_nx, id_port_px, id_port_nz, id_port_pz}};
        case id_port_nz:
        case id_port_pz:
            return (SurroundingPorts1){
                {id_port_nx, id_port_px, id_port_ny, id_port_py}};

        default:
            return (SurroundingPorts1){{-1, -1, -1, -1}};
    }
}

SurroundingPorts2 on_boundary_2(InnerNodeDirections2 ind);
SurroundingPorts2 on_boundary_2(InnerNodeDirections2 ind) {
    if (ind.array[0] == id_port_nx || ind.array[0] == id_port_px ||
        ind.array[1] == id_port_nx || ind.array[1] == id_port_px) {
        if (ind.array[0] == id_port_ny || ind.array[0] == id_port_py ||
            ind.array[1] == id_port_ny || ind.array[1] == id_port_py) {
            return (SurroundingPorts2){{id_port_nz, id_port_pz}};
        }
        return (SurroundingPorts2){{id_port_ny, id_port_py}};
    }
    return (SurroundingPorts2){{id_port_nx, id_port_px}};
}

//  call with the index of the BOUNDARY node, and the relative direction of the
//  ghost point
//
//  we don't actually care about the pressure at the ghost point other than to
//  calculate the boundary filter input
void ghost_point_pressure_update(
    float next_pressure,
    float prev_pressure,
    global BoundaryData* boundary_data,
    const global FilterCoefficientsCanonical* boundary,
    global float* debug_buffer);
void ghost_point_pressure_update(
    float next_pressure,
    float prev_pressure,
    global BoundaryData* boundary_data,
    const global FilterCoefficientsCanonical* boundary,
    global float* debug_buffer) {
    //  TODO this is quite different from what's presented in the paper
    //  check that everything cancels in the way that I think it does

    float filt_state = boundary_data->filter_memory.array[0];
    float b0 = boundary->b[0];
    float a0 = boundary->a[0];

    float filter_input =
        -((a0 * (prev_pressure - next_pressure)) / (b0 * COURANT) +
          filt_state / b0);
    filter_step_canonical(
        filter_input, &(boundary_data->filter_memory), boundary);
}

//----------------------------------------------------------------------------//

#define TEMPLATE_SUM_SURROUNDING_PORTS(dimensions)                           \
    float CAT(get_summed_surrounding_, dimensions)(                          \
        CAT(InnerNodeDirections, dimensions) pd,                             \
        const global float* current,                                         \
        int3 locator,                                                        \
        int3 dim,                                                            \
        global float* debug_buffer);                                         \
    float CAT(get_summed_surrounding_, dimensions)(                          \
        CAT(InnerNodeDirections, dimensions) pd,                             \
        const global float* current,                                         \
        int3 locator,                                                        \
        int3 dim,                                                            \
        global float* debug_buffer) {                                        \
        float ret = 0;                                                       \
        CAT(SurroundingPorts, dimensions)                                    \
        on_boundary = CAT(on_boundary_, dimensions)(pd);                     \
        for (int i = 0; i != CAT(NUM_SURROUNDING_PORTS_, dimensions); ++i) { \
            uint index = neighbor_index(locator, dim, on_boundary.array[i]); \
            if (index == NO_NEIGHBOR) {                                      \
                /*TODO this is an error!*/                                   \
            }                                                                \
            ret += current[index];                                           \
        }                                                                    \
        return ret;                                                          \
    }

TEMPLATE_SUM_SURROUNDING_PORTS(1);
TEMPLATE_SUM_SURROUNDING_PORTS(2);

float get_summed_surrounding_3(InnerNodeDirections3 i,
                               const global float* current,
                               int3 locator,
                               int3 dimensions,
                               global float* debug_buffer);
float get_summed_surrounding_3(InnerNodeDirections3 i,
                               const global float* current,
                               int3 locator,
                               int3 dimensions,
                               global float* debug_buffer) {
    return 0;
}

//----------------------------------------------------------------------------//

#define GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(dimensions)          \
    float CAT(get_current_surrounding_weighting_, dimensions)(          \
        const global float* current,                                    \
        int3 locator,                                                   \
        int3 dim,                                                       \
        int bt,                                                         \
        global float* debug_buffer);                                    \
    float CAT(get_current_surrounding_weighting_, dimensions)(          \
        const global float* current,                                    \
        int3 locator,                                                   \
        int3 dim,                                                       \
        int bt,                                                         \
        global float* debug_buffer) {                                   \
        CAT(InnerNodeDirections, dimensions)                            \
        ind = CAT(get_inner_node_directions_, dimensions)(bt);          \
        float sum = 0;                                                  \
        for (int i = 0; i != dimensions; ++i) {                         \
            uint neighbor = neighbor_index(locator, dim, ind.array[i]); \
            if (neighbor != NO_NEIGHBOR)                                \
                sum += 2 * current[neighbor];                           \
        }                                                               \
        return COURANT_SQ *                                             \
               (sum + CAT(get_summed_surrounding_, dimensions)(         \
                          ind, current, locator, dim, debug_buffer));   \
    }

GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(1);
GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(2);
GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(3);

//----------------------------------------------------------------------------//

//  if COURANT_SQ is 1/3 this will always return 0
float get_current_boundary_weighting(const global float* current);
float get_current_boundary_weighting(const global float* current) {
    return 2 * (1 - 3 * COURANT_SQ) * current[get_global_id(0)];
}

//----------------------------------------------------------------------------//

#define GET_FILTER_WEIGHTING_TEMPLATE(dimensions)                            \
    float CAT(get_filter_weighting_, dimensions)(                            \
        global CAT(BoundaryDataArray, dimensions) * bda,                     \
        const global FilterCoefficientsCanonical* boundary_coefficients,     \
        global float* debug_buffer);                                         \
    float CAT(get_filter_weighting_, dimensions)(                            \
        global CAT(BoundaryDataArray, dimensions) * bda,                     \
        const global FilterCoefficientsCanonical* boundary_coefficients,     \
        global float* debug_buffer) {                                        \
        float sum = 0;                                                       \
        for (int i = 0; i != dimensions; ++i) {                              \
            float filt_state = bda->array[i].filter_memory.array[0];         \
            sum +=                                                           \
                filt_state /                                                 \
                boundary_coefficients[bda->array[i].coefficient_index].b[0]; \
        }                                                                    \
        return COURANT_SQ * sum;                                             \
    }

GET_FILTER_WEIGHTING_TEMPLATE(1);
GET_FILTER_WEIGHTING_TEMPLATE(2);
GET_FILTER_WEIGHTING_TEMPLATE(3);

//----------------------------------------------------------------------------//

#define GET_COEFF_WEIGHTING_TEMPLATE(dimensions)                           \
    float CAT(get_coeff_weighting_, dimensions)(                           \
        global CAT(BoundaryDataArray, dimensions) * bda,                   \
        const global FilterCoefficientsCanonical* boundary_coefficients);  \
    float CAT(get_coeff_weighting_, dimensions)(                           \
        global CAT(BoundaryDataArray, dimensions) * bda,                   \
        const global FilterCoefficientsCanonical* boundary_coefficients) { \
        float sum = 0;                                                     \
        for (int i = 0; i != dimensions; ++i) {                            \
            const global FilterCoefficientsCanonical* boundary =           \
                boundary_coefficients + bda->array[i].coefficient_index;   \
            sum += COURANT * boundary->a[0] / boundary->b[0];              \
        }                                                                  \
        return sum;                                                        \
    }

GET_COEFF_WEIGHTING_TEMPLATE(1);
GET_COEFF_WEIGHTING_TEMPLATE(2);
GET_COEFF_WEIGHTING_TEMPLATE(3);

//----------------------------------------------------------------------------//

#define BOUNDARY_TEMPLATE(dimensions)                                          \
    float CAT(boundary_, dimensions)(                                          \
        const global float* current,                                           \
        float prev_pressure,                                                   \
        CondensedNode node,                                                    \
        int3 locator,                                                          \
        int3 dim,                                                              \
        global CAT(BoundaryDataArray, dimensions) * boundary_data,             \
        const global FilterCoefficientsCanonical* boundary_coefficients,       \
        global float* debug_buffer);                                           \
    float CAT(boundary_, dimensions)(                                          \
        const global float* current,                                           \
        float prev_pressure,                                                   \
        CondensedNode node,                                                    \
        int3 locator,                                                          \
        int3 dim,                                                              \
        global CAT(BoundaryDataArray, dimensions) * boundary_data,             \
        const global FilterCoefficientsCanonical* boundary_coefficients,       \
        global float* debug_buffer) {                                          \
        float current_surrounding_weighting =                                  \
            CAT(get_current_surrounding_weighting_, dimensions)(               \
                current, locator, dim, node.boundary_type, debug_buffer);      \
        global CAT(BoundaryDataArray, dimensions)* bda =                       \
            boundary_data + node.boundary_index;                               \
        float filter_weighting = CAT(get_filter_weighting_, dimensions)(       \
            bda, boundary_coefficients, debug_buffer);                         \
        float coeff_weighting =                                                \
            CAT(get_coeff_weighting_, dimensions)(bda, boundary_coefficients); \
        float prev_weighting = (coeff_weighting - 1) * prev_pressure;          \
        float ret = (current_surrounding_weighting + filter_weighting +        \
                     prev_weighting) /                                         \
                    (1 + coeff_weighting);                                     \
        for (int i = 0; i != dimensions; ++i) {                                \
            global BoundaryData* bd = bda->array + i;                          \
            const global FilterCoefficientsCanonical* boundary =               \
                boundary_coefficients + bd->coefficient_index;                 \
            ghost_point_pressure_update(                                       \
                ret, prev_pressure, bd, boundary, debug_buffer);               \
        }                                                                      \
        debug_buffer[get_global_id(0)] = ret;                                  \
        return ret;                                                            \
    }

//        float current_boundary_weighting =                                     \
//            get_current_boundary_weighting(current);                           \
//        CAT(InnerNodeDirections, dimensions)                                   \
//        inner_node_directions =                                                \
//            CAT(get_inner_node_directions_, dimensions)(node.boundary_type);              \
//
//        for (...)
//
//            ghost_point_pressure_update(current,                               \
//                                        locator,                               \
//                                        dim,                                   \
//                                        ret,                                   \
//                                        prev_pressure,                         \
//                                        bd,                                    \
//                                        boundary,                              \
//                                        inner_node_directions.array[i],        \
//                                        debug_buffer);                         \

BOUNDARY_TEMPLATE(1);
BOUNDARY_TEMPLATE(2);
BOUNDARY_TEMPLATE(3);

//----------------------------------------------------------------------------//

kernel void condensed_waveguide(const global float* current,
                                global float* previous,
                                const global CondensedNode* nodes,
                                int3 dimensions,
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
                                global float* output,
                                global float* debug_buffer,
                                global int* error_flag) {
    size_t index = get_global_id(0);
    CondensedNode node = nodes[index];

    int3 locator = to_locator(index, dimensions);

    float prev_pressure = previous[index];
    float next_pressure = 0;

    //  find the next pressure at this node, assign it to next_pressure
    switch (popcount(node.boundary_type)) {
        //  this is inside or outside, not a boundary
        case 1:
            if (node.boundary_type & id_inside ||
                node.boundary_type & id_reentrant) {
                for (int i = 0; i != PORTS; ++i) {
                    uint port_index = neighbor_index(locator, dimensions, i);
                    //                    if (port_index != NO_NEIGHBOR &&
                    //                        nodes[port_index].bt & id_inside)
                    if (port_index != NO_NEIGHBOR)
                        next_pressure += current[port_index];
                }

                next_pressure /= (PORTS / 2);
                next_pressure -= prev_pressure;
            } else {
                next_pressure = boundary_1(current,
                                           prev_pressure,
                                           node,
                                           locator,
                                           dimensions,
                                           boundary_data_1,
                                           boundary_coefficients,
                                           debug_buffer);
            }
            break;
        //  this is an edge where two boundaries meet
        case 2:
            next_pressure = boundary_2(current,
                                       prev_pressure,
                                       node,
                                       locator,
                                       dimensions,
                                       boundary_data_2,
                                       boundary_coefficients,
                                       debug_buffer);
            break;
        //  this is a corner where three boundaries meet
        case 3:
            next_pressure = boundary_3(current,
                                       prev_pressure,
                                       node,
                                       locator,
                                       dimensions,
                                       boundary_data_3,
                                       boundary_coefficients,
                                       debug_buffer);
            break;
    }

    if (isinf(next_pressure))
        *error_flag |= id_inf_error;
    if (isnan(next_pressure))
        *error_flag |= id_nan_error;

    previous[index] = next_pressure;

    if (index == read) {
        *output = previous[index];

        //
        //  instantaneous intensity for mic modelling
        //

        //  TODO (maybe) move as much of this as possible outside kernel
        //  there's a small chance I'll read previous[port_index] before it
        //  is updated by the appropriate thread but whatever
        float differences[PORTS] = {0};
        for (int i = 0; i != PORTS; ++i) {
            uint port_index = neighbor_index(locator, dimensions, i);
            if (port_index != NO_NEIGHBOR &&
                nodes[port_index].boundary_type & id_inside)
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
        float ambient_density = 1.225f;
        multiplied /= -ambient_density;

        //  numerical integration
        *velocity_buffer += T * multiplied;
    }
}

kernel void classify_boundaries(global Node* nodes,
                                int3 dimensions,
                                int set_bits) {
    size_t index = get_global_id(0);
    global Node* node = nodes + index;

    if (!node->inside && node->boundary_type == id_none) {
        for (int i = 0; i != PORTS; ++i) {
            uint port_ind = node->ports[i];
            if (port_ind != NO_NEIGHBOR) {
                global Node* port_node = nodes + port_ind;
                if ((!set_bits && port_node->inside) ||
                    (set_bits && port_node->boundary_type != id_reentrant &&
                     popcount(port_node->boundary_type) == set_bits)) {
                    node->boundary_type |= 1 << (i + 1);
                }
            }
        }
        int final_bits = popcount(node->boundary_type);
        if (set_bits + 1 < final_bits) {
            node->boundary_type = id_reentrant;
        } else if (final_bits <= set_bits) {
            node->boundary_type = id_none;
        }
    }
}

