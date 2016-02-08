#pragma once

#include "vec.h"
#include "cl_structs.h"
#include "boundaries.h"
#include "base_mesh.h"

#include <vector>

class RectangularMesh : public BaseMesh<6, Vec3i> {
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

    template <int BITS>
    size_type compute_num_boundary() const {
        return std::accumulate(
            get_nodes().begin(),
            get_nodes().end(),
            0u,
            [](auto i, const auto& j) {
                return i + ((j.bt != id_reentrant && popcount(j.bt) == BITS)
                                ? 1
                                : 0);
            });
    }

private:
    Vec3i dim;
    Collection nodes;
};
