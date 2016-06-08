#pragma once

#include "base_mesh.h"
#include "tetrahedral_program.h"

#include "common/boundaries.h"

#include <vector>

struct TetrahedralLocator final {
    explicit TetrahedralLocator(const glm::ivec3& pos = glm::ivec3(),
                                int mod_ind = 0);
    glm::ivec3 pos;
    int mod_ind;
};

class TetrahedralMesh
        : public BaseMesh<BasicDWMProgram<4>, TetrahedralLocator> {
public:
    static const int CUBE_NODES = 8;

    TetrahedralMesh(const Boundary& boundary,
                    float spacing,
                    const glm::vec3& anchor);

    size_type compute_index(const Locator& locator) const override;
    Locator compute_locator(size_type index) const override;
    Locator compute_locator(const glm::vec3& position) const override;
    glm::vec3 compute_position(const Locator& locator) const override;

    void compute_neighbors(size_type index, cl_uint* output) const override;

    const Collection& get_nodes() const override;

private:
    TetrahedralMesh(const Boundary& b,
                    float spacing,
                    const glm::vec3& anchor,
                    float cube_side);
    Collection compute_nodes(const Boundary& boundary) const;

    float get_cube_side() const;
    const std::array<glm::vec3, CUBE_NODES>& get_scaled_cube() const;
    glm::ivec3 get_dim() const;

    static float cube_side_from_node_spacing(float spacing);
    static std::array<glm::vec3, CUBE_NODES> compute_scaled_cube(float scale);
    static const std::array<std::array<Locator, PORTS>, CUBE_NODES>
            offset_table;

    //  data members
    std::array<glm::vec3, CUBE_NODES> scaled_cube;
    glm::ivec3 dim;
    Collection nodes;
};
