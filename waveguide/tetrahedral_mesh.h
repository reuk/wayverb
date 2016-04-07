#pragma once

#include "base_mesh.h"
#include "boundaries.h"
#include "tetrahedral_program.h"
#include "vec.h"

#include <vector>

struct TetrahedralLocator final {
    TetrahedralLocator(const Vec3i& pos = Vec3i(), int mod_ind = 0);
    Vec3i pos;
    int mod_ind;
};

class TetrahedralMesh
    : public BaseMesh<BasicDWMProgram<4>, TetrahedralLocator> {
public:
    static const int CUBE_NODES = 8;

    TetrahedralMesh(const Boundary& boundary,
                    float spacing,
                    const Vec3f& anchor);

    size_type compute_index(const Locator& locator) const override;
    Locator compute_locator(size_type index) const override;
    Locator compute_locator(const Vec3f& position) const override;
    Vec3f compute_position(const Locator& locator) const override;

    void compute_neighbors(size_type index, cl_uint* output) const override;

    const Collection& get_nodes() const override;

private:
    TetrahedralMesh(const Boundary& b,
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
