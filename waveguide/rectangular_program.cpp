#include "rectangular_program.h"
#include "stl_wrappers.h"

#include <cmath>
#include <iostream>

RectangularProgram::RectangularProgram(const cl::Context& context,
                                       bool build_immediate)
        : Program(context, source, build_immediate) {
}

RectangularProgram::CondensedNodeStruct RectangularProgram::condense(
    const NodeStruct& n) {
    return CondensedNodeStruct{
        n.boundary_type | (n.inside ? id_inside : id_none), n.boundary_index};
}

RectangularProgram::CanonicalCoefficients RectangularProgram::convolve(
    const BiquadCoefficientsArray& a) {
    std::array<BiquadCoefficients, BiquadCoefficientsArray::BIQUAD_SECTIONS> t;
    proc::copy(a.array, t.begin());
    return reduce(t,
                  [](const auto& i, const auto& j) { return convolve(i, j); });
}

/// Given a set of canonical coefficients describing a reflectance filter,
/// produce an impedance filter which describes the reflective surface
RectangularProgram::CanonicalCoefficients
RectangularProgram::to_impedance_coefficients(const CanonicalCoefficients& c) {
    CanonicalCoefficients ret;
    proc::transform(
        c.a, std::begin(c.b), std::begin(ret.b), [](auto a, auto b) {
            return a + b;
        });
    proc::transform(
        c.a, std::begin(c.b), std::begin(ret.a), [](auto a, auto b) {
            return a - b;
        });

    if (ret.a[0] != 0) {
        auto norm = 1.0 / ret.a[0];
        auto do_normalize = [norm](auto& i) {
            proc::for_each(i, [norm](auto& i) { i *= norm; });
        };
        do_normalize(ret.b);
        do_normalize(ret.a);
    }

    return ret;
}

//  ostreams  ----------------------------------------------------------------//

std::ostream& operator<<(std::ostream& os,
                         const RectangularProgram::BiquadCoefficientsArray& n) {
    Bracketer bracketer(os);
    for (const auto& i : n.array)
        to_stream(os, i);
    return os;
}

std::ostream& operator<<(std::ostream& os,
                         const RectangularProgram::CanonicalMemory& m) {
    Bracketer bracketer(os);
    for (const auto& i : m.array)
        to_stream(os, i, "  ");
    return os;
}

std::ostream& operator<<(std::ostream& os,
                         const RectangularProgram::BoundaryData& m) {
    return to_stream(os, m.filter_memory);
}

std::ostream& operator<<(std::ostream& os,
                         const RectangularProgram::CondensedNodeStruct& cns) {
    Bracketer bracketer(os);
    return to_stream(os,
                     "bt: ",
                     cns.boundary_type,
                     "  boundary_index: ",
                     cns.boundary_index);
}

//----------------------------------------------------------------------------//

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
#define COURANT (1.0f / sqrt(3.0f))
#define COURANT_SQ (1.0f / 3.0f)

#define NO_NEIGHBOR (~(uint)0)

typedef double FilterReal;

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
    id_outside_range_error = 1 << 2,
    id_outside_mesh_error = 1 << 3,
    id_suspicious_boundary_error = 1 << 4,
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
    typedef struct { FilterReal array[order]; } CAT(FilterMemory, order);

TEMPLATE_FILTER_MEMORY(BIQUAD_ORDER);
TEMPLATE_FILTER_MEMORY(CANONICAL_FILTER_ORDER);

#define TEMPLATE_FILTER_COEFFICIENTS(order) \
    typedef struct {                        \
        FilterReal b[order + 1];            \
        FilterReal a[order + 1];            \
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

#define FILTER_STEP(order)                                              \
    FilterReal CAT(filter_step_, order)(                                \
        FilterReal input,                                               \
        global CAT(FilterMemory, order) * m,                            \
        const global CAT(FilterCoefficients, order) * c);               \
    FilterReal CAT(filter_step_, order)(                                \
        FilterReal input,                                               \
        global CAT(FilterMemory, order) * m,                            \
        const global CAT(FilterCoefficients, order) * c) {              \
        FilterReal output = (input * c->b[0] + m->array[0]) / c->a[0];  \
        for (int i = 0; i != order - 1; ++i) {                          \
            FilterReal b = c->b[i + 1] == 0 ? 0 : c->b[i + 1] * input;  \
            FilterReal a = c->a[i + 1] == 0 ? 0 : c->a[i + 1] * output; \
            m->array[i] = b - a + m->array[i + 1];                      \
        }                                                               \
        FilterReal b = c->b[order] == 0 ? 0 : c->b[order] * input;      \
        FilterReal a = c->a[order] == 0 ? 0 : c->a[order] * output;     \
        m->array[order - 1] = b - a;                                    \
        return output;                                                  \
    }

FILTER_STEP(BIQUAD_ORDER);
FILTER_STEP(CANONICAL_FILTER_ORDER);

#define filter_step_biquad CAT(filter_step_, BIQUAD_ORDER)
#define filter_step_canonical CAT(filter_step_, CANONICAL_FILTER_ORDER)

float biquad_cascade(FilterReal input,
                     global BiquadMemoryArray* bm,
                     const global BiquadCoefficientsArray* bc);
float biquad_cascade(FilterReal input,
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
    float inner_pressure,
    global BoundaryData* boundary_data,
    const global FilterCoefficientsCanonical* boundary,
    global float* debug_buffer);
void ghost_point_pressure_update(
    float next_pressure,
    float prev_pressure,
    float inner_pressure,
    global BoundaryData* boundary_data,
    const global FilterCoefficientsCanonical* boundary,
    global float* debug_buffer) {
    FilterReal filt_state = boundary_data->filter_memory.array[0];
    FilterReal b0 = boundary->b[0];
    FilterReal a0 = boundary->a[0];

    FilterReal diff = (a0 * (prev_pressure - next_pressure)) / (b0 * COURANT) +
                      (filt_state / b0);
#if 0
    FilterReal ghost_pressure = inner_pressure + diff;
    FilterReal filter_input = inner_pressure - ghost_pressure;
#else
    FilterReal filter_input = -diff;
#endif
    filter_step_canonical(
        filter_input, &(boundary_data->filter_memory), boundary);
}

kernel void ghost_point_test(
    const global float* input,
    global BoundaryData* boundary_data,
    const global FilterCoefficientsCanonical* canonical_coefficients,
    global float* debug_buffer) {
    size_t index = get_global_id(0);
    ghost_point_pressure_update(input[index],
                                0,
                                0,
                                boundary_data + index,
                                canonical_coefficients,
                                debug_buffer);
}

//----------------------------------------------------------------------------//

#define TEMPLATE_SUM_SURROUNDING_PORTS(dimensions)                           \
    float CAT(get_summed_surrounding_, dimensions)(                          \
        const global CondensedNode* nodes,                                   \
        CAT(InnerNodeDirections, dimensions) pd,                             \
        const global float* current,                                         \
        int3 locator,                                                        \
        int3 dim,                                                            \
        global int* error_flag);                                             \
    float CAT(get_summed_surrounding_, dimensions)(                          \
        const global CondensedNode* nodes,                                   \
        CAT(InnerNodeDirections, dimensions) pd,                             \
        const global float* current,                                         \
        int3 locator,                                                        \
        int3 dim,                                                            \
        global int* error_flag) {                                            \
        float ret = 0;                                                       \
        CAT(SurroundingPorts, dimensions)                                    \
        on_boundary = CAT(on_boundary_, dimensions)(pd);                     \
        for (int i = 0; i != CAT(NUM_SURROUNDING_PORTS_, dimensions); ++i) { \
            uint index = neighbor_index(locator, dim, on_boundary.array[i]); \
            if (index == NO_NEIGHBOR) {                                      \
                *error_flag |= id_outside_mesh_error;                        \
                return 0;                                                    \
            }                                                                \
            int boundary_type = nodes[index].boundary_type;                  \
            if (boundary_type == id_none || boundary_type == id_inside) {    \
                *error_flag |= id_suspicious_boundary_error;                 \
            }                                                                \
            ret += current[index];                                           \
        }                                                                    \
        return ret;                                                          \
    }

TEMPLATE_SUM_SURROUNDING_PORTS(1);
TEMPLATE_SUM_SURROUNDING_PORTS(2);

float get_summed_surrounding_3(const global CondensedNode* nodes,
                               InnerNodeDirections3 i,
                               const global float* current,
                               int3 locator,
                               int3 dimensions,
                               global int* error_flag);
float get_summed_surrounding_3(const global CondensedNode* nodes,
                               InnerNodeDirections3 i,
                               const global float* current,
                               int3 locator,
                               int3 dimensions,
                               global int* error_flag) {
    return 0;
}

//----------------------------------------------------------------------------//

float get_inner_pressure(const global CondensedNode* nodes,
                         const global float* current,
                         int3 locator,
                         int3 dim,
                         PortDirection bt,
                         global int* error_flag);
float get_inner_pressure(const global CondensedNode* nodes,
                         const global float* current,
                         int3 locator,
                         int3 dim,
                         PortDirection bt,
                         global int* error_flag) {
    uint neighbor = neighbor_index(locator, dim, bt);
    if (neighbor == NO_NEIGHBOR) {
        *error_flag |= id_outside_mesh_error;
        return 0;
    }
    return current[neighbor];
}

#define GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(dimensions)               \
    float CAT(get_current_surrounding_weighting_, dimensions)(               \
        const global CondensedNode* nodes,                                   \
        const global float* current,                                         \
        int3 locator,                                                        \
        int3 dim,                                                            \
        CAT(InnerNodeDirections, dimensions) ind,                            \
        global int* error_flag,                                              \
        global float* debug_buffer);                                         \
    float CAT(get_current_surrounding_weighting_, dimensions)(               \
        const global CondensedNode* nodes,                                   \
        const global float* current,                                         \
        int3 locator,                                                        \
        int3 dim,                                                            \
        CAT(InnerNodeDirections, dimensions) ind,                            \
        global int* error_flag,                                              \
        global float* debug_buffer) {                                        \
        float sum = 0;                                                       \
        for (int i = 0; i != dimensions; ++i) {                              \
            sum +=                                                           \
                2 *                                                          \
                get_inner_pressure(                                          \
                    nodes, current, locator, dim, ind.array[i], error_flag); \
        }                                                                    \
        return COURANT_SQ *                                                  \
               (sum + CAT(get_summed_surrounding_, dimensions)(              \
                          nodes, ind, current, locator, dim, error_flag));   \
    }

GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(1);
GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(2);
GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(3);

//----------------------------------------------------------------------------//

#define GET_FILTER_WEIGHTING_TEMPLATE(dimensions)                              \
    float CAT(get_filter_weighting_, dimensions)(                              \
        global CAT(BoundaryDataArray, dimensions) * bda,                       \
        const global FilterCoefficientsCanonical* boundary_coefficients);      \
    float CAT(get_filter_weighting_, dimensions)(                              \
        global CAT(BoundaryDataArray, dimensions) * bda,                       \
        const global FilterCoefficientsCanonical* boundary_coefficients) {     \
        float sum = 0;                                                         \
        for (int i = 0; i != dimensions; ++i) {                                \
            BoundaryData bd = bda->array[i];                                   \
            FilterReal filt_state = bd.filter_memory.array[0];                 \
            sum +=                                                             \
                filt_state / boundary_coefficients[bd.coefficient_index].b[0]; \
        }                                                                      \
        return COURANT_SQ * sum;                                               \
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
            sum += boundary->a[0] / boundary->b[0];                        \
        }                                                                  \
        return sum * COURANT;                                              \
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
        const global CondensedNode* nodes,                                     \
        int3 locator,                                                          \
        int3 dim,                                                              \
        global CAT(BoundaryDataArray, dimensions) * boundary_data,             \
        const global FilterCoefficientsCanonical* boundary_coefficients,       \
        global int* error_flag,                                                \
        global float* debug_buffer);                                           \
    float CAT(boundary_, dimensions)(                                          \
        const global float* current,                                           \
        float prev_pressure,                                                   \
        CondensedNode node,                                                    \
        const global CondensedNode* nodes,                                     \
        int3 locator,                                                          \
        int3 dim,                                                              \
        global CAT(BoundaryDataArray, dimensions) * boundary_data,             \
        const global FilterCoefficientsCanonical* boundary_coefficients,       \
        global int* error_flag,                                                \
        global float* debug_buffer) {                                          \
        CAT(InnerNodeDirections, dimensions)                                   \
        ind = CAT(get_inner_node_directions_, dimensions)(node.boundary_type); \
        float current_surrounding_weighting =                                  \
            CAT(get_current_surrounding_weighting_, dimensions)(               \
                nodes, current, locator, dim, ind, error_flag, debug_buffer);  \
        global CAT(BoundaryDataArray, dimensions)* bda =                       \
            boundary_data + node.boundary_index;                               \
        const float filter_weighting = CAT(get_filter_weighting_, dimensions)( \
            bda, boundary_coefficients);                                       \
        const float coeff_weighting =                                          \
            CAT(get_coeff_weighting_, dimensions)(bda, boundary_coefficients); \
        const float prev_weighting = (coeff_weighting - 1) * prev_pressure;    \
        const float ret = (current_surrounding_weighting + filter_weighting +  \
                     prev_weighting) /                                         \
                    (1 + coeff_weighting);                                     \
        for (int i = 0; i != dimensions; ++i) {                                \
            global BoundaryData* bd = bda->array + i;                          \
            const global FilterCoefficientsCanonical* boundary =               \
                boundary_coefficients + bd->coefficient_index;                 \
            ghost_point_pressure_update(                                       \
                ret,                                                           \
                prev_pressure,                                                 \
                get_inner_pressure(                                            \
                    nodes, current, locator, dim, ind.array[i], error_flag),   \
                bd,                                                            \
                boundary,                                                      \
                debug_buffer);                                                 \
        }                                                                      \
        return ret;                                                            \
    }

BOUNDARY_TEMPLATE(1);
BOUNDARY_TEMPLATE(2);
BOUNDARY_TEMPLATE(3);

//----------------------------------------------------------------------------//

#define RANGE (1)
#define ENABLE_BOUNDARIES (1)

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
                                global int* error_flag,
                                global float* debug_buffer) {
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
#if ENABLE_BOUNDARIES
                next_pressure = boundary_1(current,
                                           prev_pressure,
                                           node,
                                           nodes,
                                           locator,
                                           dimensions,
                                           boundary_data_1,
                                           boundary_coefficients,
                                           error_flag,
                                           debug_buffer);
#endif
            }
            break;
        //  this is an edge where two boundaries meet
        case 2:
#if ENABLE_BOUNDARIES
            next_pressure = boundary_2(current,
                                       prev_pressure,
                                       node,
                                       nodes,
                                       locator,
                                       dimensions,
                                       boundary_data_2,
                                       boundary_coefficients,
                                       error_flag,
                                       debug_buffer);
#endif
            break;
        //  this is a corner where three boundaries meet
        case 3:
#if ENABLE_BOUNDARIES
            next_pressure = boundary_3(current,
                                       prev_pressure,
                                       node,
                                       nodes,
                                       locator,
                                       dimensions,
                                       boundary_data_3,
                                       boundary_coefficients,
                                       error_flag,
                                       debug_buffer);
#endif
            break;
    }

    if (next_pressure < -RANGE || RANGE < next_pressure)
        *error_flag |= id_outside_range_error;
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

)"};
