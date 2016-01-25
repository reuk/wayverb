#pragma once

#include "vec.h"
#include "cl_structs.h"
#include "boundaries.h"

#include <vector>

class RectangularMesh final {
public:
    using Node = RectNode;
    using Collection = std::vector<Node>;
    using size_type = Collection::size_type;
    using Locator = Vec3i;

    static const int PORTS = Node::PORTS;

    RectangularMesh(const Boundary& boundary,
                    float spacing,
                    const Vec3f& anchor);

    size_type get_index(const Locator& locator) const;
    Locator get_locator(const size_type index) const;
    Locator get_locator(const Vec3f& position) const;
    Vec3f get_position(const Locator& locator) const;

    std::array<int, PORTS> get_neighbors(size_type index) const;

    const Collection& get_nodes() const;
    const CuboidBoundary& get_aabb() const;
    float get_spacing() const;
    Vec3i get_dim() const;

private:
    Collection get_nodes(const Boundary& boundary) const;

    float spacing;
    CuboidBoundary aabb;
    Vec3i dim;
    Collection nodes;
};
