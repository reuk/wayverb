#pragma once

#include "core/geo/box.h"
#include "core/indexing.h"
#include "core/spatial_division/range.h"

#include <memory>

namespace wayverb {
namespace core {
namespace detail {

template <size_t n,
          typename aabb_type = detail::range_t<n>,
          typename next_boundaries_type = std::array<aabb_type, (1 << n)>>
next_boundaries_type next_boundaries(const aabb_type& parent) {
    using vt = detail::range_value_t<n>;

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
    using aabb_type = detail::range_t<n>;
    using node_array = std::array<ndim_tree, (1 << n)>;

    /// Given the index of an item, and a bounding box, returns whether or not
    /// the item intersects with the box.
    using item_checker = std::function<bool(size_t, const aabb_type& aabb)>;

    ndim_tree() = default;
    ndim_tree(size_t depth,
              const item_checker& callback,
              const util::aligned::vector<size_t>& to_test,
              const aabb_type& aabb)
            : aabb_{aabb}
            , items_{compute_contained_items(callback, to_test, aabb)}
            , nodes_{compute_nodes(depth, callback, items_, aabb)} {}

    aabb_type get_aabb() const { return aabb_; }
    bool has_nodes() const { return nodes_ != nullptr; }
    const node_array& get_nodes() const { return *nodes_; }
    util::aligned::vector<size_t> get_items() const { return items_; }

    size_t get_side() const {
        return nodes_ ? 2 * nodes_->front().get_side() : 1;
    }

private:
    using next_boundaries_type = std::array<aabb_type, (1 << n)>;

    static util::aligned::vector<size_t> compute_contained_items(
            const item_checker& callback,
            const util::aligned::vector<size_t>& to_test,
            const aabb_type& aabb) {
        util::aligned::vector<size_t> ret;
        std::copy_if(begin(to_test),
                     end(to_test),
                     std::back_inserter(ret),
                     [&](auto i) { return callback(i, aabb); });
        return ret;
    }

    static std::unique_ptr<node_array> compute_nodes(
            size_t depth,
            const item_checker& callback,
            const util::aligned::vector<size_t>& to_test,
            const aabb_type& aabb) {
        if (!depth) {
            return nullptr;
        }

        const auto next = detail::next_boundaries<n>(aabb);
        auto ret = std::make_unique<std::array<ndim_tree, (1 << n)>>();
        std::transform(
                begin(next), end(next), ret->begin(), [&](const auto& i) {
                    return ndim_tree(depth - 1, callback, to_test, i);
                });
        return ret;
    }

    aabb_type aabb_;                       //  the bounding box of the node
    util::aligned::vector<size_t> items_;  //  indices of contained items
    std::unique_ptr<node_array> nodes_;    //  contained nodes
};
}  // namespace core
}  // namespace wayverb
