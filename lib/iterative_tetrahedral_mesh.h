#pragma once

#include "vec.h"
#include "cl_structs.h"
#include "boundaries.h"

#include <vector>

class IterativeTetrahedralMesh {
public:
    using size_type = std::vector<UnlinkedTetrahedralNode>::size_type;

    class Locator {
    public:
        Locator(const Vec3i & pos = Vec3i(), int mod_ind = 0);
        Vec3i pos;
        int mod_ind;
    };

    IterativeTetrahedralMesh(const Boundary & boundary,
                             float cube_side);

    size_type get_index(const Locator & locator) const;
    Locator get_locator(size_type index) const;
    Vec3f get_position(const Locator & locator) const;
    std::vector<int> get_neighbors(size_type index) const;

    const float spacing;
    const std::vector<Vec3f> scaled_cube;
    Vec3i dim;
    std::vector<UnlinkedTetrahedralNode> nodes;
};
