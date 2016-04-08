#pragma once

#include "boundaries.h"
#include "vec_forward.h"

#include <vector>

template <typename Program, typename LocatorType>
class BaseMesh {
public:
    static constexpr int PORTS = Program::PORTS;

    using Locator = LocatorType;
    using Node = typename Program::NodeStruct;
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

    virtual void compute_neighbors(size_type index, cl_uint* output) const = 0;
    std::array<cl_uint, PORTS> compute_neighbors(size_type index) const {
        std::array<cl_uint, PORTS> ret;
        compute_neighbors(index, ret.data());
        return ret;
    }
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
