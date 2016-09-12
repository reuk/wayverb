#pragma once

#include <iterator>
#include <type_traits>

template <typename It, typename Mapper>
class mapping_iterator_adapter final {
public:
    using iterator_category =
            typename std::iterator_traits<It>::iterator_category;
    using value_type =
            std::decay_t<decltype(std::declval<Mapper>()(*std::declval<It>()))>;
    using difference_type = typename std::iterator_traits<It>::difference_type;
    using pointer = value_type*;
    using reference = value_type&;

    constexpr mapping_iterator_adapter() = default;
    constexpr explicit mapping_iterator_adapter(It it)
            : it_(std::move(it)) {}

    constexpr explicit mapping_iterator_adapter(It it, Mapper mapper)
            : it_(std::move(it))
            , mapper_(std::move(mapper)) {}

    template <class U, class V>
    constexpr mapping_iterator_adapter(
            const mapping_iterator_adapter<U, V>& other)
            : it_(other.it_)
            , mapper_(other.mapper_) {}

    template <class U, class V>
    void swap(mapping_iterator_adapter<U, V>& other) noexcept {
        using std::swap;
        swap(it_, other.it_);
        swap(mapper_, other.mapper_);
    }

    template <class U, class V>
    constexpr mapping_iterator_adapter& operator=(
            mapping_iterator_adapter<U, V> other) {
        swap(other);
        return *this;
    }

    constexpr It base() const { return it_; }

    constexpr reference operator*() const { return mapper_(*it_); }
    constexpr pointer operator->() const { return &operator*(); }

    constexpr reference operator[](difference_type n) const {
        return mapper_(it_[n]);
    }

    constexpr mapping_iterator_adapter& operator++() {
        ++it_;
        return *this;
    }
    constexpr mapping_iterator_adapter& operator--() {
        --it_;
        return *this;
    }

    constexpr mapping_iterator_adapter operator++(int) {
        return mapping_iterator_adapter{it_++, mapper_};
    }
    constexpr mapping_iterator_adapter operator--(int) {
        return mapping_iterator_adapter{it_--, mapper_};
    }

    constexpr mapping_iterator_adapter operator+(difference_type n) const {
        auto ret{*this};
        return ret += n;
    }
    constexpr mapping_iterator_adapter operator-(difference_type n) const {
        auto ret{*this};
        return ret -= n;
    }

    constexpr mapping_iterator_adapter& operator+=(difference_type n) {
        it_ += n;
        return *this;
    }

    constexpr mapping_iterator_adapter& operator-=(difference_type n) {
        it_ -= n;
        return *this;
    }

private:
    It it_;
    Mapper mapper_;

    template <typename U, typename V>
    friend class mapping_iterator_adapter;
};

// comparisons ---------------------------------------------------------------//

template <class A, class B, class C, class D>
constexpr bool operator==(const mapping_iterator_adapter<A, B>& lhs,
                          const mapping_iterator_adapter<C, D>& rhs) {
    return lhs.base() == rhs.base();
}

template <class A, class B, class C, class D>
constexpr bool operator!=(const mapping_iterator_adapter<A, B>& lhs,
                          const mapping_iterator_adapter<C, D>& rhs) {
    return lhs.base() != rhs.base();
}

template <class A, class B, class C, class D>
constexpr bool operator<(const mapping_iterator_adapter<A, B>& lhs,
                         const mapping_iterator_adapter<C, D>& rhs) {
    return lhs.base() < rhs.base();
}

template <class A, class B, class C, class D>
constexpr bool operator<=(const mapping_iterator_adapter<A, B>& lhs,
                          const mapping_iterator_adapter<C, D>& rhs) {
    return lhs.base() <= rhs.base();
}

template <class A, class B, class C, class D>
constexpr bool operator>(const mapping_iterator_adapter<A, B>& lhs,
                         const mapping_iterator_adapter<C, D>& rhs) {
    return lhs.base() > rhs.base();
}

template <class A, class B, class C, class D>
constexpr bool operator>=(const mapping_iterator_adapter<A, B>& lhs,
                          const mapping_iterator_adapter<C, D>& rhs) {
    return lhs.base() >= rhs.base();
}

// non-member arithmetic ops -------------------------------------------------//

template <class U, class V>
constexpr mapping_iterator_adapter<U, V> operator+(
        typename mapping_iterator_adapter<U, V>::difference_type n,
        const mapping_iterator_adapter<U, V>& it) {
    return it + n;
}

template <class U, class V>
constexpr mapping_iterator_adapter<U, V> operator-(
        typename mapping_iterator_adapter<U, V>::difference_type n,
        const mapping_iterator_adapter<U, V>& it) {
    return it - n;
}

template <class U, class V>
constexpr typename mapping_iterator_adapter<U, V>::difference_type operator-(
        const mapping_iterator_adapter<U, V>& a,
        const mapping_iterator_adapter<U, V>& b) {
    return a.base() - b.base();
}

// make iterators quickly ----------------------------------------------------//

template <class U, class V>
constexpr mapping_iterator_adapter<U, V> make_mapping_iterator_adapter(
        U it, V mapper) {
    return mapping_iterator_adapter<U, V>{std::move(it), std::move(mapper)};
}
