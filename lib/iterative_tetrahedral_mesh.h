#pragma once

#include "vec.h"
#include "cl_structs.h"
#include "boundaries.h"
#include "base_mesh.h"

#include <vector>

struct TetrahedralLocator final {
    TetrahedralLocator(const Vec3i& pos = Vec3i(), int mod_ind = 0);
    Vec3i pos;
    int mod_ind;
};

class IterativeTetrahedralMesh : public BaseMesh<KNode, TetrahedralLocator> {
public:
    static const int CUBE_NODES = 8;

    IterativeTetrahedralMesh(const Boundary& boundary,
                             float spacing,
                             const Vec3f& anchor);

    size_type compute_index(const Locator& locator) const override;
    Locator compute_locator(size_type index) const override;
    Locator compute_locator(const Vec3f& position) const override;
    Vec3f compute_position(const Locator& locator) const override;

    std::array<int, PORTS> compute_neighbors(size_type index) const override;

    const Collection& get_nodes() const override;

private:
    IterativeTetrahedralMesh(const Boundary& b,
                             float spacing,
                             const Vec3f& anchor,
                             float cube_side);
    Collection compute_nodes(const Boundary& boundary) const;

    float get_cube_side() const;
    const std::array<Vec3f, CUBE_NODES>& get_scaled_cube() const;
    Vec3i get_dim() const;

    static float cube_side_from_node_spacing(float spacing);
    static std::array<Vec3f, CUBE_NODES> compute_scaled_cube(float scale);
    static const std::array<std::array<Locator, PORTS>, CUBE_NODES>
        offset_table;

    //  data members
    std::array<Vec3f, CUBE_NODES> scaled_cube;
    Vec3i dim;
    Collection nodes;
};
