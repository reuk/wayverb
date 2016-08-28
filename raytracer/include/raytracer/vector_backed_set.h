#pragma once

#include "common/aligned/vector.h"
#include "common/stl_wrappers.h"

namespace raytracer {

template <class T,
          class Comp = std::less<T>,
          class Alloc = aligned::allocator<T>>
class vector_backed_set final {
public:
    vector_backed_set() = default;
    explicit vector_backed_set(Comp c, Alloc a = Alloc{})
            : comp_(std::move(c))
            , data_(std::move(a)) {}

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
    std::vector<T, Alloc> data_;
};

}  // namespace raytracer
