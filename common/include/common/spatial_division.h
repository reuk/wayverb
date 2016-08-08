#pragma once

#include "common/box.h"
#include "common/stl_wrappers.h"

#include <memory>

/// A generic interface for spatial division algorithms (octree, quadtree)
template <size_t dimensions>
class ndim_tree final {
public:
    using node_array = std::array<ndim_tree, (1 << dimensions)>;

    /// Given the index of an item, and a bounding box, returns whether or not
    /// the item intersects with the box.
    using item_checker =
            std::function<bool(size_t, const box<dimensions>& aabb)>;

    ndim_tree() = default;
    ndim_tree(size_t depth,
              const item_checker& callback,
              const aligned::vector<size_t>& to_test,
              const box<dimensions>& aabb)
            : aabb(aabb)
            , items(compute_contained_items(callback, to_test, aabb))
            , nodes(compute_nodes(depth, callback, items, aabb)) {}

    box<dimensions> get_aabb() const { return aabb; }
    bool has_nodes() const { return nodes != nullptr; }
    const node_array& get_nodes() const { return *nodes; }
    aligned::vector<size_t> get_items() const { return items; }

    size_t get_side() const {
        return nodes ? 2 * nodes->front().get_side() : 1;
    }

private:
    static std::array<box<dimensions>, (1 << dimensions)> next_boundaries(
            const box<dimensions>& parent);

    static aligned::vector<size_t> compute_contained_items(
            const item_checker& callback,
            const aligned::vector<size_t>& to_test,
            const box<dimensions>& aabb) {
        aligned::vector<size_t> ret;
        proc::copy_if(to_test, std::back_inserter(ret), [&](auto i) {
            return callback(i, aabb);
        });
        return ret;
    }

    static std::unique_ptr<node_array> compute_nodes(
            size_t depth,
            const item_checker& callback,
            const aligned::vector<size_t>& to_test,
            const box<dimensions>& aabb) {
        if (!depth) {
            return nullptr;
        }

        const auto next = next_boundaries(aabb);
        std::array<ndim_tree, (1<<dimensions)> ret;
        proc::transform(next, ret.begin(), [&](const auto& i) {
            return ndim_tree(depth - 1, callback, to_test, i);
        });
        return std::make_unique<std::array<ndim_tree, (1 << dimensions)>>(
                std::move(ret));
    }

    box<dimensions> aabb;               //  the bounding box of the node
    aligned::vector<size_t> items;      //  indices of contained items
    std::unique_ptr<node_array> nodes;  // contained nodes
};

using quadtree = ndim_tree<2>;
using octree = ndim_tree<3>;

octree octree_from_scene_data(const copyable_scene_data& scene_data,
                              size_t depth,
                              float padding);
