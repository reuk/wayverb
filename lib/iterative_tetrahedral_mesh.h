#pragma once

#include "vec.h"
#include "cl_structs.h"
#include "boundaries.h"

#include <vector>
#include <array>

class IterativeTetrahedralMesh {
public:
    using size_type = std::vector<KNode>::size_type;
    struct Locator {
        Locator(const Vec3i& pos = Vec3i(), int mod_ind = 0);
        Vec3i pos;
        int mod_ind;
    };

    static const int PORTS = 4;
    static const int CUBE_NODES = 8;

    IterativeTetrahedralMesh(const Boundary& boundary,
                             float spacing,
                             const Vec3f& anchor /* = Vec3f()*/);
    virtual ~IterativeTetrahedralMesh() noexcept = default;

    size_type get_index(const Locator& locator) const;
    Locator get_locator(size_type index) const;
    Locator get_locator(const Vec3f& position) const;
    std::array<int, PORTS> get_neighbors(size_type index) const;

    const float cube_side;
    const std::vector<Vec3f> scaled_cube;
    const CuboidBoundary boundary;
    const Vec3i dim;

    const std::vector<KNode>& get_nodes() const;
    float get_spacing() const;

    static float cube_side_from_node_spacing(float spacing);

private:
    Vec3f get_position(const Locator& locator) const;
    static const std::array<std::array<Locator, PORTS>, CUBE_NODES>
        offset_table;

    CuboidBoundary get_adjusted_boundary(const CuboidBoundary& min_boundary,
                                         const Vec3f& anchor) const;

    const std::vector<KNode> nodes;
    float spacing;

    Vec3i get_dim() const;
    std::vector<KNode> get_nodes(const Boundary& boundary) const;
    std::vector<Vec3f> get_scaled_cube() const;
};
