#pragma once

#include "common/aligned/set.h"

namespace raytracer {

/// Super-basic set that holds unique_ptrs
template <typename T, typename compare = std::less<T>>
class unique_ptr_set final {
public:
    unique_ptr_set()
            : unique_ptr_set(compare()) {}
    explicit unique_ptr_set(const compare& comp)
            : set(comparator(comp)) {}

    void clear() { set.clear(); }

    //  don't like these because they require dynamic memory allocation :(
    void erase(const T& t) { set.erase(make_item(t)); }
    void insert(T&& t) { set.insert(make_item(std::forward<T>(t))); }
    void insert(const T& t) { set.insert(make_item(t)); }

    auto find(const T& t) { return set.find(make_item(t)); }
    auto find(const T& t) const { return set.find(make_item(t)); }

    void swap(unique_ptr_set& other) noexcept {
        using std::swap;
        swap(set, other.set);
    }

    auto begin() { return set.begin(); }
    auto begin() const { return set.begin(); }
    auto cbegin() const { return set.cbegin(); }

    auto end() { return set.end(); }
    auto end() const { return set.end(); }
    auto cend() const { return set.cend(); }

    auto rbegin() { return set.rbegin(); }
    auto rbegin() const { return set.rbegin(); }
    auto crbegin() const { return set.crbegin(); }

    auto rend() { return set.rend(); }
    auto rend() const { return set.rend(); }
    auto crend() const { return set.crend(); }

    auto get_comp() const { return set.value_comp().get_comp(); }

private:
    auto make_item(T&& t) const {
        return std::make_unique<T>(std::forward<T>(t));
    }
    auto make_item(const T& t) const { return std::make_unique<T>(t); }

    class comparator final {
    public:
        comparator()
                : comparator(compare()) {}
        explicit comparator(const compare& comp)
                : comp(comp) {}
        constexpr bool operator()(const std::unique_ptr<T>& a,
                                  const std::unique_ptr<T>& b) const {
            return comp(*a, *b);
        }

        compare get_comp() const { return comp; }

    private:
        compare comp;
    };

    aligned::set<std::unique_ptr<T>, comparator> set;
};

}  // namespace raytracer
