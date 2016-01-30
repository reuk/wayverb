#pragma once

#include "vec.h"

#include <vector>

template <typename NodeType, typename LocatorType>
class BaseMesh {
public:
    using Locator = LocatorType;
    using Node = NodeType;
    using Collection = std::vector<NodeType>;
    using size_type = typename Collection::size_type;

    BaseMesh(float spacing, const CuboidBoundary& aabb)
            : spacing(spacing)
            , aabb(aabb) {
    }

    virtual ~BaseMesh() noexcept = default;
    BaseMesh(const BaseMesh&) = default;
    BaseMesh& operator=(const BaseMesh&) = default;
    BaseMesh(BaseMesh&&) noexcept = default;
    BaseMesh& operator=(BaseMesh&&) noexcept = default;

    static constexpr int PORTS = NodeType::PORTS;

    virtual size_type compute_index(const Locator& locator) const = 0;
    virtual Locator compute_locator(size_type index) const = 0;
    virtual Locator compute_locator(const Vec3f& position) const = 0;
    virtual Vec3f compute_position(const Locator& locator) const = 0;

    virtual std::array<int, PORTS> compute_neighbors(size_type index) const = 0;
    float get_spacing() const {
        return spacing;
    }
    virtual const Collection& get_nodes() const = 0;
    const CuboidBoundary& get_aabb() const {
        return aabb;
    }

private:
    float spacing;
    CuboidBoundary aabb;
};
