#pragma once

//  please only include in .cpp files

#include <string>

namespace cl_sources {
inline std::string get_struct_definitions(size_t PORTS,
                                          size_t BIQUAD_SECTIONS) {
    return std::string("#define PORTS " + std::to_string(PORTS) + "\n" +
                       "#define BIQUAD_SECTIONS " +
                       std::to_string(BIQUAD_SECTIONS) + "\n" +
                       "#define CANONICAL_FILTER_ORDER " +
                       std::to_string(BIQUAD_SECTIONS * 2) + "\n" +
                       R"(
#define BIQUAD_ORDER 2

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

typedef struct { PortDirection array[1]; } InnerNodeDirections1;
typedef struct { PortDirection array[2]; } InnerNodeDirections2;
typedef struct { PortDirection array[3]; } InnerNodeDirections3;

#define NUM_SURROUNDING_PORTS_1 4
#define NUM_SURROUNDING_PORTS_2 2

typedef struct { PortDirection array[NUM_SURROUNDING_PORTS_1]; } SurroundingPorts1;
typedef struct { PortDirection array[NUM_SURROUNDING_PORTS_2]; } SurroundingPorts2;

typedef struct {
    ulong write_location;
    float pressure;
} InputInfo;

)");
}
}  // namespace cl_sources
