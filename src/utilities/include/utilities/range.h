#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <type_traits>

namespace util {

template <typename t>
class range final {
public:
    using value_type = t;

    /// setters/getters

    constexpr range()
            : min_{0}
            , max_{0} {}

    constexpr range(value_type a, value_type b)
            : min_{std::move(a)}
            , max_{std::move(b)} {
        maintain_invariant();
    }

    template <typename V>
    constexpr range(const range<V>& r)
            : min_{r.get_min()}
            , max_{r.get_max()} {}

    template <typename V>
    constexpr range(range<V>&& r) noexcept
            : min_{std::move(r.min_)}
            , max_{std::move(r.max_)} {}

    constexpr auto get_min() const { return min_; }
    constexpr auto get_max() const { return max_; }

    /// mutators

    template <typename V>
    constexpr range& operator+=(const V& v) {
        min_ += v;
        max_ += v;
        return *this;
    }

    template <typename V>
    constexpr range& operator-=(const V& v) {
        min_ -= v;
        max_ -= v;
        return *this;
    }

    template <typename V>
    constexpr range& operator*=(const V& v) {
        min_ *= v;
        max_ *= v;
        maintain_invariant();
        return *this;
    }

    template <typename V>
    constexpr range& operator/=(const V& v) {
        min_ /= v;
        max_ /= v;
        maintain_invariant();
        return *this;
    }

    template <typename V>
    constexpr range& pad(const V& v) {
        min_ -= v;
        max_ += v;
        maintain_invariant();
        return *this;
    }

private:
    constexpr void maintain_invariant() {
        const auto a = min_;
        const auto b = max_;

        using std::min;
        using std::max;

        min_ = min(a, b);
        max_ = max(a, b);
    }

    value_type min_;
    value_type max_;
};

template <typename t>
constexpr range<t> make_range(const t& a, const t& b) {
    return range<t>{a, b};
}

////////////////////////////////////////////////////////////////////////////////

template <typename A, typename B>
constexpr bool operator==(const range<A>& a, const range<B>& b) {
    return std::make_tuple(a.get_min(), a.get_max()) ==
           std::make_tuple(b.get_min(), b.get_max());
}

template <typename A, typename B>
constexpr bool operator!=(const range<A>& a, const range<B>& b) {
    return !(a == b);
}

template <typename A, typename B>
constexpr auto operator+(const range<A>& a, const B& b) {
    range<std::common_type_t<A, B>> ret{a};
    return ret += b;
}

template <typename A, typename B>
constexpr auto operator-(const range<A>& a, const B& b) {
    range<std::common_type_t<A, B>> ret{a};
    return ret -= b;
}

template <typename A, typename B>
constexpr auto operator*(const range<A>& a, const B& b) {
    range<std::common_type_t<A, B>> ret{a};
    return ret *= b;
}

template <typename A, typename B>
constexpr auto operator/(const range<A>& a, const B& b) {
    range<std::common_type_t<A, B>> ret{a};
    return ret /= b;
}

////////////////////////////////////////////////////////////////////////////////

template <typename A, typename B>
constexpr auto padded(const range<A>& a, const B& b) {
    range<std::common_type_t<A, B>> ret{a};
    return ret.pad(b);
}

template <typename t>
constexpr auto centre(const range<t>& a) {
    return (a.get_min() + a.get_max()) * t{0.5};
}

template <typename t>
constexpr auto dimensions(const range<t>& a) {
    return a.get_max() - a.get_min();
}

template <typename T, typename U, typename V>
constexpr auto map(T x, range<U> in, range<V> out) {
    return (((x - in.get_min()) * dimensions(out)) / dimensions(in)) +
           out.get_min();
}

template <typename T, typename U>
constexpr auto inside(const T& t, const range<U>& r) {
    return r.get_min() <= t && t < r.get_max();
}

template <typename T, typename U>
constexpr auto clamp(const T& t, const range<U>& r) {
    using std::min;
    using std::max;
    return max(r.get_min(), min(r.get_max(), t));
}

////////////////////////////////////////////////////////////////////////////////

template <typename It, typename T>
constexpr auto accumulate_min(It begin, It end, T starting_value) {
    return std::accumulate(
            begin, end, starting_value, [](const auto& a, const auto& b) {
                return min(a, b);
            });
}

template <typename It, typename T>
constexpr auto accumulate_max(It begin, It end, T starting_value) {
    return std::accumulate(
            begin, end, starting_value, [](const auto& a, const auto& b) {
                return max(a, b);
            });
}

template <typename It>
constexpr auto enclosing_range(It begin, It end) {
    if (begin == end) {
        throw std::runtime_error("Can't minmax empty range.");
    }
    return make_range(accumulate_min(begin + 1, end, *begin),
                      accumulate_max(begin + 1, end, *begin));
}

}  // namespace util
