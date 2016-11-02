#pragma once

#include "core/recursive_vector.h"

#include <set>

namespace raytracer {

/// A tree node holds a single item, and optionally a collection of
/// following items.
template <typename T>
struct multitree final {
    explicit multitree(T item = T{})
            : item(std::move(item)) {}

    T item;
    using branches_type = core::recursive_vector_backed_set<multitree<T>>;
    branches_type branches;
};

template <typename T>
constexpr bool operator<(const multitree<T>& a, const multitree<T>& b) {
    return a.item < b.item;
}

template <typename T, typename It>
void add_path(multitree<T>& tree, It begin, It end) {
    if (begin != end) {
        const auto it = tree.branches.insert(multitree<T>{*begin}).first;
        add_path(*it, std::next(begin), end);
    }
}

namespace detail {

/// The trick here is that the callback can be a stateful object...
template <typename T, typename Callback>
void traverse_multitree(const multitree<T>& tree, const Callback& callback) {
    const auto next = callback(tree.item);
    for (const auto& i : tree.branches) {
        detail::traverse_multitree(i, next);
    }
}

}  // namespace detail

template <typename T, typename Callback>
void traverse_multitree(const multitree<T>& tree, const Callback& callback) {
    for (const auto& i : tree.branches) {
        detail::traverse_multitree(i, callback);
    }
}

}  // namespace raytracer
