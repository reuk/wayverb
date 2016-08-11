#pragma once

//  please only include in .cpp files

#include <string>

namespace cl_sources {
const std::string structs{R"(
typedef struct {
    int boundary_type;
    uint boundary_index;
} CondensedNode;

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

)"};
}  // namespace cl_sources
