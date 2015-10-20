#pragma once

#include "vec.h"
#include "cl_structs.h"
#include "boundaries.h"

#include <vector>
#include <array>

class IterativeTetrahedralMesh {
public:
    using size_type = std::vector<Node>::size_type;
    struct Locator {
        Locator(const Vec3i & pos = Vec3i(), int mod_ind = 0);
        Vec3i pos;
        int mod_ind;
    };

    static const int PORTS = 4;
    static const int CUBE_NODES = 8;

    IterativeTetrahedralMesh(const Boundary & boundary, float spacing);
    virtual ~IterativeTetrahedralMesh() noexcept = default;

    size_type get_index(const Locator & locator) const;
    Locator get_locator(size_type index) const;
    Vec3f get_position(const Locator & locator) const;
    std::array<int, PORTS> get_neighbors(size_type index) const;

    const CuboidBoundary boundary;
    const float cube_side;
    const std::vector<Vec3f> scaled_cube;
    const Vec3i dim;
    std::vector<Node> nodes;

    static float cube_side_from_node_spacing(float spacing);

private:
    static const std::array<std::array<Locator, PORTS>, CUBE_NODES>
        offset_table;

    Vec3i get_dim() const;
    std::vector<Node> get_nodes(const Boundary & boundary) const;
    std::vector<Vec3f> get_scaled_cube() const;
};
