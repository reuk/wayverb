#pragma once

//  please only include in .cpp files

#include <string>

namespace cl_sources {
inline std::string get_struct_definitions(size_t BIQUAD_SECTIONS) {
    return std::string("#define BIQUAD_SECTIONS " +
                       std::to_string(BIQUAD_SECTIONS) + "\n" +
                       "#define CANONICAL_FILTER_ORDER " +
                       std::to_string(BIQUAD_SECTIONS * 2) + "\n" +
                       R"(
#define BIQUAD_ORDER 2

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
    uint coefficient_index;
} BoundaryData;

typedef struct { BoundaryData array[1]; } BoundaryDataArray1;
typedef struct { BoundaryData array[2]; } BoundaryDataArray2;
typedef struct { BoundaryData array[3]; } BoundaryDataArray3;

#define NUM_SURROUNDING_PORTS_1 4
#define NUM_SURROUNDING_PORTS_2 2

typedef struct { PortDirection array[NUM_SURROUNDING_PORTS_1]; } SurroundingPorts1;
typedef struct { PortDirection array[NUM_SURROUNDING_PORTS_2]; } SurroundingPorts2;

)");
}
}  // namespace cl_sources
