#pragma once

#include "vec.h"
#include "boundaries.h"
#include "base_mesh.h"
#include "rectangular_program.h"

#include <vector>

class RectangularMesh : public BaseMesh<RectangularProgram, Vec3i> {
public:
    RectangularMesh(const RectangularProgram& program,
                    cl::CommandQueue& queue,
                    const Boundary& boundary,
                    float spacing,
                    const Vec3f& anchor);

    using CondensedNode = RectangularProgram::CondensedNodeStruct;

    size_type compute_index(const Locator& locator) const override;
    Locator compute_locator(const size_type index) const override;
    Locator compute_locator(const Vec3f& position) const override;
    Vec3f compute_position(const Locator& locator) const override;

    const Collection& get_nodes() const override;

    void compute_neighbors(size_type index, cl_uint* output) const override;
    Collection compute_nodes(const Boundary& boundary,
                             const RectangularProgram& program,
                             cl::CommandQueue& queue) const;

    std::vector<CondensedNode> get_condensed_nodes() const;

    Vec3i get_dim() const;

    template <int BITS>
    size_type compute_num_boundary() const {
        return std::accumulate(
            get_nodes().begin(),
            get_nodes().end(),
            0u,
            [](auto i, const auto& j) {
                return i +
                       ((j.boundary_type != RectangularProgram::id_reentrant &&
                         popcount(j.boundary_type) == BITS)
                            ? 1
                            : 0);
            });
    }

private:
    Vec3i dim;
    Collection nodes;
};
