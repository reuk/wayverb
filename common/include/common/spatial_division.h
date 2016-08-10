#pragma once

#include "common/geo/box.h"
#include "common/stl_wrappers.h"
#include "common/indexing.h"

#include <memory>

namespace detail {

template <size_t n,
          typename aabb_type = util::detail::range_t<n>,
          typename next_boundaries_type = std::array<aabb_type, (1 << n)>>
next_boundaries_type next_boundaries(const aabb_type& parent) {
    using vt = util::detail::range_value_t<n>;

    const auto c = centre(parent);

    const auto root_aabb = aabb_type(parent.get_min(), c);
    const auto d = dimensions(root_aabb);

    next_boundaries_type ret;

    for (size_t i = 0; i != ret.size(); ++i) {
        const auto relative = indexing::relative_position<n>(i);
        ret[i] = root_aabb + d * vt{relative};
    }

    return ret;
}

}  // namespace detail

/// A generic interface for spatial division algorithms (octree, quadtree)
template <size_t n>
class ndim_tree final {
public:
    using aabb_type = util::detail::range_t<n>;
    using node_array = std::array<ndim_tree, (1 << n)>;

    /// Given the index of an item, and a bounding box, returns whether or not
    /// the item intersects with the box.
    using item_checker = std::function<bool(size_t, const aabb_type& aabb)>;

    ndim_tree() = default;
    ndim_tree(size_t depth,
              const item_checker& callback,
              const aligned::vector<size_t>& to_test,
              const aabb_type& aabb)
            : aabb(aabb)
            , items(compute_contained_items(callback, to_test, aabb))
            , nodes(compute_nodes(depth, callback, items, aabb)) {}

    aabb_type get_aabb() const { return aabb; }
    bool has_nodes() const { return nodes != nullptr; }
    const node_array& get_nodes() const { return *nodes; }
    aligned::vector<size_t> get_items() const { return items; }

    size_t get_side() const {
        return nodes ? 2 * nodes->front().get_side() : 1;
    }

private:
    using next_boundaries_type = std::array<aabb_type, (1 << n)>;

    static aligned::vector<size_t> compute_contained_items(
            const item_checker& callback,
            const aligned::vector<size_t>& to_test,
            const aabb_type& aabb) {
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
            const aabb_type& aabb) {
        if (!depth) {
            return nullptr;
        }

        const auto next = detail::next_boundaries<n>(aabb);
        auto ret = std::make_unique<std::array<ndim_tree, (1 << n)>>();
        proc::transform(next, ret->begin(), [&](const auto& i) {
            return ndim_tree(depth - 1, callback, to_test, i);
        });
        return ret;
    }

    aabb_type aabb;                     //  the bounding box of the node
    aligned::vector<size_t> items;      //  indices of contained items
    std::unique_ptr<node_array> nodes;  //  contained nodes
};

using bintree = ndim_tree<1>;
using quadtree = ndim_tree<2>;
using octree = ndim_tree<3>;

octree octree_from_scene_data(const copyable_scene_data& scene_data,
                              size_t depth,
                              float padding);
