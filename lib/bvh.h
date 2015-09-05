#pragma once

#include "cl_structs.h"
#include "boundaries.h"

#include <vector>

class BVHMesh {
public:
    BVHMesh(const std::vector<Triangle> & triangles,
            const std::vector<cl_float3> & vertices);
private:
    const std::vector<Triangle> triangles;
    const std::vector<cl_float3> vertices;
    const CuboidBoundary boundary;
};
