#pragma once

#include "common/cl/scene_structs.h"

#include <iterator>
#include <type_traits>

/// An iterator adapter that is designed to allow iterating over a single 'band'
/// of a collection of cl_ vectors.
///
/// For example, given a std::vector<cl_float4>, you can use this adapter to
/// iterate over only the x component of each cl_float4.

template <typename It>
class cl_type_iterator final {
public:
    using iterator_category =
            typename std::iterator_traits<It>::iterator_category;
    using value_type = std::remove_all_extents_t<decltype(
            std::declval<typename std::iterator_traits<It>::value_type>().s)>;
    using difference_type = typename std::iterator_traits<It>::difference_type;
    using pointer = value_type*;
    using reference = value_type&;

    constexpr cl_type_iterator() = default;
    constexpr explicit cl_type_iterator(It it, size_t index)
            : it(it)
            , index(index) {}
    template <class U>
    constexpr cl_type_iterator(const cl_type_iterator<U>& other)
            : it(other.it)
            , index(other.index) {}

    template <class U>
    void swap(cl_type_iterator<U>& other) noexcept {
        using std::swap;
        swap(it, other.it);
        swap(index, other.index);
    }

    template <class U>
    constexpr cl_type_iterator& operator=(cl_type_iterator<U>& other) {
        swap(other);
        return *this;
    }

    constexpr It base() const { return it; }

    reference operator*() { return *operator->(); }
    constexpr reference operator*() const { return *operator->(); }
    pointer operator->() { return it->s + index; }
    constexpr pointer operator->() const { return it->s + index; }

    constexpr reference operator[](difference_type n) const {
        return it[n]->s[index];
    }

    constexpr cl_type_iterator& operator++() {
        ++it;
        return *this;
    }
    constexpr cl_type_iterator& operator--() {
        --it;
        return *this;
    }

    constexpr cl_type_iterator operator++(int) {
        return cl_type_iterator{it++};
    }
    constexpr cl_type_iterator operator--(int) {
        return cl_type_iterator{it--};
    }

    constexpr cl_type_iterator operator+(difference_type n) const {
        auto ret{*this};
        return ret += n;
    }
    constexpr cl_type_iterator operator-(difference_type n) const {
        auto ret{*this};
        return ret -= n;
    }

    constexpr cl_type_iterator& operator+=(difference_type n) { it += n; }

    constexpr cl_type_iterator& operator-=(difference_type n) { it -= n; }

private:
    It it;
    size_t index{0};
};

// comparisons ---------------------------------------------------------------//

template <class Ita, class Itb>
constexpr bool operator==(const cl_type_iterator<Ita>& lhs,
                          const cl_type_iterator<Itb>& rhs) {
    return lhs.base() == rhs.base();
}

template <class Ita, class Itb>
constexpr bool operator!=(const cl_type_iterator<Ita>& lhs,
                          const cl_type_iterator<Itb>& rhs) {
    return lhs.base() != rhs.base();
}

template <class Ita, class Itb>
constexpr bool operator<(const cl_type_iterator<Ita>& lhs,
                         const cl_type_iterator<Itb>& rhs) {
    return lhs.base() < rhs.base();
}

template <class Ita, class Itb>
constexpr bool operator<=(const cl_type_iterator<Ita>& lhs,
                          const cl_type_iterator<Itb>& rhs) {
    return lhs.base() <= rhs.base();
}

template <class Ita, class Itb>
constexpr bool operator>(const cl_type_iterator<Ita>& lhs,
                         const cl_type_iterator<Itb>& rhs) {
    return lhs.base() > rhs.base();
}

template <class Ita, class Itb>
constexpr bool operator>=(const cl_type_iterator<Ita>& lhs,
                          const cl_type_iterator<Itb>& rhs) {
    return lhs.base() >= rhs.base();
}

// non-member arithmetic ops -------------------------------------------------//

template <class It>
constexpr cl_type_iterator<It> operator+(
        typename cl_type_iterator<It>::difference_type n,
        const cl_type_iterator<It>& it) {
    return it + n;
}

template <class It>
constexpr cl_type_iterator<It> operator-(
        typename cl_type_iterator<It>::difference_type n,
        const cl_type_iterator<It>& it) {
    return it - n;
}

// make iterators quickly ----------------------------------------------------//

template <class It>
constexpr cl_type_iterator<It> make_cl_type_iterator(It it, size_t index) {
    return cl_type_iterator<It>(it, index);
}
