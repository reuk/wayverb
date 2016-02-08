#pragma once

#include "vec.h"
#include "cl_structs.h"

#include <vector>

template <int P, typename LocatorType>
class BaseMesh {
public:
    static constexpr int PORTS = P;

    using Locator = LocatorType;
    using Node = NodeStruct<PORTS>;
    using Collection = std::vector<Node>;
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

    template <typename T>
    static auto popcount(T t) {
        return std::bitset<sizeof(T) * 8>(t).count();
    }

private:
    float spacing;
    CuboidBoundary aabb;
};
