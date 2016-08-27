#pragma once

#include "common/aligned/vector.h"
#include "common/cl/include.h"
#include "common/mapping_iterator_adapter.h"
#include "common/stl_wrappers.h"

#include <experimental/optional>
#include <memory>
#include <set>

namespace raytracer {

template <class T, class Comp = std::less<T>>
class vector_backed_set final {
public:
    vector_backed_set() = default;
    explicit vector_backed_set(Comp c)
            : comp_(std::move(c)) {}

    auto begin() { return data_.begin(); }
    const auto begin() const { return data_.begin(); }
    const auto cbegin() const { return data_.cbegin(); }

    auto end() { return data_.end(); }
    const auto end() const { return data_.end(); }
    const auto cend() const { return data_.cend(); }

    auto find(const T& t) const {
        const auto it{proc::lower_bound(data_, t, comp_)};
        if (it != data_.end() && values_equal(t, *it)) {
            return it;
        }
        return data_.end();
    }

    auto insert(T t) {
        const auto it{proc::lower_bound(data_, t, comp_)};
        if (it != data_.end() && values_equal(t, *it)) {
            return std::make_pair(it, false);
        }
        return std::make_pair(data_.insert(it, std::move(t)), true);
    }

    constexpr auto key_comp() const { return comp_; }
    constexpr auto value_comp() const { return comp_; }

    auto size() const { return data_.size(); }

private:
    constexpr bool values_not_equal(const T& a, const T& b) const {
        return comp_(a, b) || comp_(b, a);
    }
    constexpr bool values_equal(const T& a, const T& b) const {
        return !values_not_equal(a, b);
    }

    Comp comp_;
    std::vector<T> data_;
};

//----------------------------------------------------------------------------//

template <template <class...> class Set,
          typename T,
          typename Comp = std::less<T>>
class recursive_set_adapter final {
public:
    recursive_set_adapter() = default;
    explicit recursive_set_adapter(Comp c)
            : data_(comp(std::move(c))) {}

    auto begin() { return make_iterator(data_.begin()); }
    const auto begin() const { return make_iterator(data_.begin()); }
    const auto cbegin() const { return make_iterator(data_.cbegin()); }

    auto end() { return make_iterator(data_.end()); }
    const auto end() const { return make_iterator(data_.end()); }
    const auto cend() const { return make_iterator(data_.cend()); }

    auto find(const T& t) const {
        const auto it{proc::lower_bound(
                data_, t, [&](const std::unique_ptr<T>& a, const T& b) {
                    return key_comp()(*a, b);
                })};
        if (it == data_.end() || values_not_equal(t, *(*it))) {
            return make_iterator(data_.end());
        }
        return make_iterator(it);
    }

    auto insert(T t) {
        const auto pair{data_.insert(std::make_unique<T>(std::move(t)))};
        return std::make_pair(make_iterator(pair.first), pair.second);
    }

    auto key_comp() const { return data_.key_comp().get_comp(); }
    auto value_comp() const { return data_.key_comp().get_comp(); }

    auto size() const { return data_.size(); }

private:
    constexpr bool values_not_equal(const T& a, const T& b) const {
        return key_comp()(a, b) || key_comp()(b, a);
    }

    class deref final {
    public:
        template <typename U>
        auto& operator()(U& t) const {
            return *t;
        }
    };

    template <typename It>
    static auto make_iterator(It it) {
        return make_mapping_iterator_adapter(std::move(it), deref{});
    }

    class comp final {
    public:
        comp() = default;
        comp(Comp comp)
                : comp_(std::move(comp)) {}

        bool operator()(const std::unique_ptr<T>& a,
                        const std::unique_ptr<T>& b) const {
            return comp_(*a, *b);
        }

        Comp get_comp() const { return comp_; }

    private:
        Comp comp_;
    };

    Set<std::unique_ptr<T>, comp> data_;
};

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
            : item_(std::move(item)) {}

    const T item_;
    using branches = recursive_vector_set<multitree<T>>;
    branches branches_;

    template <typename It>
    void add_path(It begin, It end) {
        if (begin != end) {
            const auto it{branches_.insert(multitree<T>{*begin}).first};
            it->add_path(std::next(begin), end);
        }
    }
};

template <typename T>
constexpr bool operator<(const multitree<T>& a, const multitree<T>& b) {
    return a.item_ < b.item_;
}

//----------------------------------------------------------------------------//

/// Each item in the tree references an intersected triangle, which may or
/// may not be visible from the receiver.
struct path_element final {
    cl_ulong index;
    bool visible;
};

/// Need this because we'll be storing path_elements in a set.
constexpr bool operator<(const path_element& a, const path_element& b) {
    return a.index < b.index;
}

//----------------------------------------------------------------------------//

multitree<path_element>::branches construct_image_source_tree(
        const aligned::vector<aligned::vector<path_element>>& paths);

}  // namespace raytracer
