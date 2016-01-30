#pragma once

#include "vec.h"
#include "cl_structs.h"
#include "boundaries.h"
#include "base_mesh.h"

#include <vector>

class RectangularMesh : public BaseMesh<RectNode, Vec3i> {
public:
    RectangularMesh(const Boundary& boundary,
                    float spacing,
                    const Vec3f& anchor);

    size_type compute_index(const Locator& locator) const override;
    Locator compute_locator(const size_type index) const override;
    Locator compute_locator(const Vec3f& position) const override;
    Vec3f compute_position(const Locator& locator) const override;

    const Collection& get_nodes() const override;

    std::array<int, PORTS> compute_neighbors(size_type index) const override;
    Collection compute_nodes(const Boundary& boundary) const;

    Vec3i get_dim() const;

private:
    Vec3i dim;
    Collection nodes;
};
