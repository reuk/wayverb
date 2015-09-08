#pragma once

#include "cl_structs.h"
#include "boundaries.h"

#include <vector>

class BVHMesh {
public:
    BVHMesh(const std::vector<Triangle> & triangles,
            const std::vector<Vec3f> & vertices);

private:
    const std::vector<Triangle> triangles;
    const std::vector<Vec3f> vertices;
    const CuboidBoundary boundary;
};
