#include "waveguide/program.h"

#include "cl/structs.h"
#include "cl/utils.h"
#include "cl/filters.h"

#include "common/stl_wrappers.h"

namespace waveguide {

program::program(const cl::Context& context, const cl::Device& device)
        : program_wrapper(
                  context,
                  device,
                  std::vector<std::string>{cl_sources::get_struct_definitions(
                                                   filters::biquad_sections),
                                           cl_sources::utils,
                                           cl_sources::filters,
                                           source}) {}

program::condensed_node program::get_condensed(const mesh_setup::node& n) {
    return condensed_node{n.boundary_type | (n.inside ? mesh_setup::id_inside
                                                      : mesh_setup::id_none),
                          n.boundary_index};
}

//----------------------------------------------------------------------------//

const std::string program::source{R"(

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

        default: return (SurroundingPorts1){{-1, -1, -1, -1}};
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
        const global FilterCoefficientsCanonical* boundary);
void ghost_point_pressure_update(
        float next_pressure,
        float prev_pressure,
        float inner_pressure,
        global BoundaryData* boundary_data,
        const global FilterCoefficientsCanonical* boundary) {
    FilterReal filt_state = boundary_data->filter_memory.array[0];
    FilterReal b0         = boundary->b[0];
    FilterReal a0         = boundary->a[0];

    FilterReal diff = (a0 * (prev_pressure - next_pressure)) / (b0 * courant) +
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

//----------------------------------------------------------------------------//

#define TEMPLATE_SUM_SURROUNDING_PORTS(dimensions)                           \
    float CAT(get_summed_surrounding_, dimensions)(                          \
            const global CondensedNode* nodes,                               \
            CAT(InnerNodeDirections, dimensions) pd,                         \
            const global float* current,                                     \
            int3 locator,                                                    \
            int3 dim,                                                        \
            global int* error_flag);                                         \
    float CAT(get_summed_surrounding_, dimensions)(                          \
            const global CondensedNode* nodes,                               \
            CAT(InnerNodeDirections, dimensions) pd,                         \
            const global float* current,                                     \
            int3 locator,                                                    \
            int3 dim,                                                        \
            global int* error_flag) {                                        \
        float ret = 0;                                                       \
        CAT(SurroundingPorts, dimensions)                                    \
        on_boundary = CAT(on_boundary_, dimensions)(pd);                     \
        for (int i = 0; i != CAT(NUM_SURROUNDING_PORTS_, dimensions); ++i) { \
            uint index = neighbor_index(locator, dim, on_boundary.array[i]); \
            if (index == no_neighbor) {                                      \
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
    if (neighbor == no_neighbor) {
        *error_flag |= id_outside_mesh_error;
        return 0;
    }
    return current[neighbor];
}

#define GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(dimensions)                 \
    float CAT(get_current_surrounding_weighting_, dimensions)(                 \
            const global CondensedNode* nodes,                                 \
            const global float* current,                                       \
            int3 locator,                                                      \
            int3 dim,                                                          \
            CAT(InnerNodeDirections, dimensions) ind,                          \
            global int* error_flag);                                           \
    float CAT(get_current_surrounding_weighting_, dimensions)(                 \
            const global CondensedNode* nodes,                                 \
            const global float* current,                                       \
            int3 locator,                                                      \
            int3 dim,                                                          \
            CAT(InnerNodeDirections, dimensions) ind,                          \
            global int* error_flag) {                                          \
        float sum = 0;                                                         \
        for (int i = 0; i != dimensions; ++i) {                                \
            sum += 2 * get_inner_pressure(nodes,                               \
                                          current,                             \
                                          locator,                             \
                                          dim,                                 \
                                          ind.array[i],                        \
                                          error_flag);                         \
        }                                                                      \
        return courant_sq *                                                    \
               (sum + CAT(get_summed_surrounding_, dimensions)(                \
                              nodes, ind, current, locator, dim, error_flag)); \
    }

GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(1);
GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(2);
GET_CURRENT_SURROUNDING_WEIGHTING_TEMPLATE(3);

//----------------------------------------------------------------------------//

#define GET_FILTER_WEIGHTING_TEMPLATE(dimensions)                              \
    float CAT(get_filter_weighting_, dimensions)(                              \
            global CAT(BoundaryDataArray, dimensions) * bda,                   \
            const global FilterCoefficientsCanonical* boundary_coefficients);  \
    float CAT(get_filter_weighting_, dimensions)(                              \
            global CAT(BoundaryDataArray, dimensions) * bda,                   \
            const global FilterCoefficientsCanonical* boundary_coefficients) { \
        float sum = 0;                                                         \
        for (int i = 0; i != dimensions; ++i) {                                \
            BoundaryData bd       = bda->array[i];                             \
            FilterReal filt_state = bd.filter_memory.array[0];                 \
            sum += filt_state /                                                \
                   boundary_coefficients[bd.coefficient_index].b[0];           \
        }                                                                      \
        return courant_sq * sum;                                               \
    }

GET_FILTER_WEIGHTING_TEMPLATE(1);
GET_FILTER_WEIGHTING_TEMPLATE(2);
GET_FILTER_WEIGHTING_TEMPLATE(3);

//----------------------------------------------------------------------------//

#define GET_COEFF_WEIGHTING_TEMPLATE(dimensions)                               \
    float CAT(get_coeff_weighting_, dimensions)(                               \
            global CAT(BoundaryDataArray, dimensions) * bda,                   \
            const global FilterCoefficientsCanonical* boundary_coefficients);  \
    float CAT(get_coeff_weighting_, dimensions)(                               \
            global CAT(BoundaryDataArray, dimensions) * bda,                   \
            const global FilterCoefficientsCanonical* boundary_coefficients) { \
        float sum = 0;                                                         \
        for (int i = 0; i != dimensions; ++i) {                                \
            const global FilterCoefficientsCanonical* boundary =               \
                    boundary_coefficients + bda->array[i].coefficient_index;   \
            sum += boundary->a[0] / boundary->b[0];                            \
        }                                                                      \
        return sum * courant;                                                  \
    }

GET_COEFF_WEIGHTING_TEMPLATE(1);
GET_COEFF_WEIGHTING_TEMPLATE(2);
GET_COEFF_WEIGHTING_TEMPLATE(3);

//----------------------------------------------------------------------------//

#define BOUNDARY_TEMPLATE(dimensions)                                          \
    float CAT(boundary_, dimensions)(                                          \
            const global float* current,                                       \
            float prev_pressure,                                               \
            CondensedNode node,                                                \
            const global CondensedNode* nodes,                                 \
            int3 locator,                                                      \
            int3 dim,                                                          \
            global CAT(BoundaryDataArray, dimensions) * boundary_data,         \
            const global FilterCoefficientsCanonical* boundary_coefficients,   \
            global int* error_flag);                                           \
    float CAT(boundary_, dimensions)(                                          \
            const global float* current,                                       \
            float prev_pressure,                                               \
            CondensedNode node,                                                \
            const global CondensedNode* nodes,                                 \
            int3 locator,                                                      \
            int3 dim,                                                          \
            global CAT(BoundaryDataArray, dimensions) * boundary_data,         \
            const global FilterCoefficientsCanonical* boundary_coefficients,   \
            global int* error_flag) {                                          \
        CAT(InnerNodeDirections, dimensions)                                   \
        ind = CAT(get_inner_node_directions_, dimensions)(node.boundary_type); \
        float current_surrounding_weighting =                                  \
                CAT(get_current_surrounding_weighting_, dimensions)(           \
                        nodes, current, locator, dim, ind, error_flag);        \
        global CAT(BoundaryDataArray, dimensions)* bda =                       \
                boundary_data + node.boundary_index;                           \
        const float filter_weighting = CAT(get_filter_weighting_, dimensions)( \
                bda, boundary_coefficients);                                   \
        const float coeff_weighting = CAT(get_coeff_weighting_, dimensions)(   \
                bda, boundary_coefficients);                                   \
        const float prev_weighting = (coeff_weighting - 1) * prev_pressure;    \
        const float ret = (current_surrounding_weighting + filter_weighting +  \
                           prev_weighting) /                                   \
                          (1 + coeff_weighting);                               \
        for (int i = 0; i != dimensions; ++i) {                                \
            global BoundaryData* bd = bda->array + i;                          \
            const global FilterCoefficientsCanonical* boundary =               \
                    boundary_coefficients + bd->coefficient_index;             \
            ghost_point_pressure_update(ret,                                   \
                                        prev_pressure,                         \
                                        get_inner_pressure(nodes,              \
                                                           current,            \
                                                           locator,            \
                                                           dim,                \
                                                           ind.array[i],       \
                                                           error_flag),        \
                                        bd,                                    \
                                        boundary);                             \
        }                                                                      \
        return ret;                                                            \
    }

BOUNDARY_TEMPLATE(1);
BOUNDARY_TEMPLATE(2);
BOUNDARY_TEMPLATE(3);

//----------------------------------------------------------------------------//

#define RANGE (1)
#define ENABLE_BOUNDARIES (1)

float normal_waveguide_update(float prev_pressure,
                              const global float* current,
                              int3 dimensions,
                              int3 locator);
float normal_waveguide_update(float prev_pressure,
                              const global float* current,
                              int3 dimensions,
                              int3 locator) {
    float ret = 0;
    for (int i = 0; i != PORTS; ++i) {
        uint port_index = neighbor_index(locator, dimensions, i);
        if (port_index != no_neighbor) {
            ret += current[port_index];
        }
    }

    ret /= (PORTS / 2);
    ret -= prev_pressure;
    return ret;
}

float next_waveguide_pressure(const CondensedNode node,
                              const global CondensedNode* nodes,
                              float prev_pressure,
                              const global float* current,
                              int3 dimensions,
                              int3 locator,
                              global BoundaryDataArray1* boundary_data_1,
                              global BoundaryDataArray2* boundary_data_2,
                              global BoundaryDataArray3* boundary_data_3,
                              const global CAT(FilterCoefficients,
                                               CANONICAL_FILTER_ORDER) *
                                      boundary_coefficients,
                              global int* error_flag);
float next_waveguide_pressure(const CondensedNode node,
                              const global CondensedNode* nodes,
                              float prev_pressure,
                              const global float* current,
                              int3 dimensions,
                              int3 locator,
                              global BoundaryDataArray1* boundary_data_1,
                              global BoundaryDataArray2* boundary_data_2,
                              global BoundaryDataArray3* boundary_data_3,
                              const global CAT(FilterCoefficients,
                                               CANONICAL_FILTER_ORDER) *
                                      boundary_coefficients,
                              global int* error_flag) {
    //  find the next pressure at this node, assign it to next_pressure
    switch (popcount(node.boundary_type)) {
        //  this is inside or outside, not a boundary
        case 1:
            if (node.boundary_type & id_inside ||
                node.boundary_type & id_reentrant) {
                return normal_waveguide_update(
                        prev_pressure, current, dimensions, locator);
            } else {
#if ENABLE_BOUNDARIES
                return boundary_1(current,
                                  prev_pressure,
                                  node,
                                  nodes,
                                  locator,
                                  dimensions,
                                  boundary_data_1,
                                  boundary_coefficients,
                                  error_flag);
#endif
            }
        //  this is an edge where two boundaries meet
        case 2:
#if ENABLE_BOUNDARIES
            return boundary_2(current,
                              prev_pressure,
                              node,
                              nodes,
                              locator,
                              dimensions,
                              boundary_data_2,
                              boundary_coefficients,
                              error_flag);
#endif
        //  this is a corner where three boundaries meet
        case 3:
#if ENABLE_BOUNDARIES
            return boundary_3(current,
                              prev_pressure,
                              node,
                              nodes,
                              locator,
                              dimensions,
                              boundary_data_3,
                              boundary_coefficients,
                              error_flag);
#endif
        default: return 0;
    }
}

kernel void condensed_waveguide(global float* previous,
                                const global float* current,
                                const global CondensedNode* nodes,
                                int3 dimensions,
                                global BoundaryDataArray1* boundary_data_1,
                                global BoundaryDataArray2* boundary_data_2,
                                global BoundaryDataArray3* boundary_data_3,
                                const global CAT(FilterCoefficients,
                                                 CANONICAL_FILTER_ORDER) *
                                        boundary_coefficients,
                                global int* error_flag) {
    size_t index = get_global_id(0);

    CondensedNode node = nodes[index];
    int3 locator       = to_locator(index, dimensions);

    float prev_pressure = previous[index];
    float next_pressure = next_waveguide_pressure(node,
                                                  nodes,
                                                  prev_pressure,
                                                  current,
                                                  dimensions,
                                                  locator,
                                                  boundary_data_1,
                                                  boundary_data_2,
                                                  boundary_data_3,
                                                  boundary_coefficients,
                                                  error_flag);

    if (next_pressure < -RANGE || RANGE < next_pressure) {
        atomic_or(error_flag, id_outside_range_error);
    }
    if (isinf(next_pressure)) {
        atomic_or(error_flag, id_inf_error);
    }
    if (isnan(next_pressure)) {
        atomic_or(error_flag, id_nan_error);
    }

    previous[index] = next_pressure;
}

)"};

}  // namespace waveguide
