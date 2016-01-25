#pragma once

#include "vec.h"
#include "cl_structs.h"
#include "boundaries.h"

#include <vector>

class IterativeTetrahedralMesh final {
public:
    using Node = KNode;
    using Collection = std::vector<Node>;
    using size_type = Collection::size_type;
    struct Locator {
        Locator(const Vec3i& pos = Vec3i(), int mod_ind = 0);
        Vec3i pos;
        int mod_ind;
    };

    static const int PORTS = Node::PORTS;
    static const int CUBE_NODES = 8;

    IterativeTetrahedralMesh(const Boundary& boundary,
                             float spacing,
                             const Vec3f& anchor /* = Vec3f()*/);

    size_type get_index(const Locator& locator) const;
    Locator get_locator(size_type index) const;
    Locator get_locator(const Vec3f& position) const;
    Vec3f get_position(const Locator& locator) const;

    std::array<int, PORTS> get_neighbors(size_type index) const;

    float get_cube_side() const;
    const std::vector<Vec3f>& get_scaled_cube() const;
    const CuboidBoundary& get_boundary() const;
    Vec3i get_dim() const;

    const Collection& get_nodes() const;
    float get_spacing() const;

    static float cube_side_from_node_spacing(float spacing);

private:
    static const std::array<std::array<Locator, PORTS>, CUBE_NODES>
        offset_table;

    float cube_side;
    std::vector<Vec3f> scaled_cube;
    CuboidBoundary boundary;
    Vec3i dim;

    Collection nodes;
    float spacing;

    Collection get_nodes(const Boundary& boundary) const;
    std::vector<Vec3f> compute_scaled_cube() const;
};
