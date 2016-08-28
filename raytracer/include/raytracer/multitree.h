#pragma once

#include "raytracer/recursive_set_adapter.h"
#include "raytracer/vector_backed_set.h"

#include <set>

namespace raytracer {

template <class T, class Comp = std::less<T>>
using recursive_set = recursive_set_adapter<std::set, T, Comp>;

template <class T, class Comp = std::less<T>>
using recursive_vector_set = recursive_set_adapter<vector_backed_set, T, Comp>;

//----------------------------------------------------------------------------//

/// A tree node holds a single item, and optionally a collection of
/// following items.
template <typename T>
struct multitree final {
    explicit multitree(T item)
            : item(std::move(item)) {}

    const T item;
    using branches_type = recursive_vector_set<multitree<T>>;
    branches_type branches;
};

template <typename T>
constexpr bool operator<(const multitree<T>& a, const multitree<T>& b) {
    return a.item < b.item;
}

template <typename T, typename It>
void add_path(multitree<T>& tree, It begin, It end) {
    if (begin != end) {
        const auto it{tree.branches.insert(multitree<T>{*begin}).first};
        add_path(*it, std::next(begin), end);
    }
}

}//namespace raytracer
