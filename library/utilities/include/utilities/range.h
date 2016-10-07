#pragma once

#include <numeric>

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

    constexpr auto get_min() const { return min_; }
    constexpr auto get_max() const { return max_; }

    /// mutators

    constexpr range& operator+=(const value_type& v) {
        min_ += v;
        max_ += v;
        return *this;
    }

    constexpr range& operator-=(const value_type& v) {
        min_ -= v;
        max_ -= v;
        return *this;
    }

    constexpr range& pad(const value_type& v) {
        min_ -= v;
        max_ += v;
        maintain_invariant();
        return *this;
    }

private:
    void maintain_invariant() {
        const auto a{min_};
        const auto b{max_};

        using std::min;
        using std::max;

        min_ = min(a, b);
        max_ = max(a, b);
    }

    value_type min_;
    value_type max_;
};

template <typename t>
range<t> make_range(const t& a, const t& b) {
    return range<t>{a, b};
}

//----------------------------------------------------------------------------//

template <typename t>
constexpr bool operator==(const range<t>& a, const range<t>& b) {
    return std::make_tuple(a.get_min(), a.get_max()) ==
           std::make_tuple(b.get_min(), b.get_max());
}

template <typename t>
constexpr bool operator!=(const range<t>& a, const range<t>& b) {
    return !(a == b);
}

template <typename t>
inline range<t> operator+(const range<t>& a, const t& b) {
    auto ret{a};
    return ret += b;
}

template <typename t>
inline range<t> operator-(const range<t>& a, const t& b) {
    auto ret{a};
    return ret -= b;
}

template <typename t>
inline auto padded(const range<t>& a, const t& b) {
    auto ret{a};
    return ret.pad(b);
}

template <typename t, typename u>
inline auto padded(const range<t>& a, const u& b) {
    return padded(a, t(b));
}

template <typename t>
inline auto inside(const range<t>& a, const t& b) {
    return glm::all(glm::lessThan(a.get_min(), b)) &&
           glm::all(glm::lessThan(b, a.get_max()));
}

template <typename t>
inline auto centre(const range<t>& a) {
    return (a.get_min() + a.get_max()) * t{0.5};
}

template <typename t>
inline auto dimensions(const range<t>& a) {
    return a.get_max() - a.get_min();
}

template <typename T, typename U, typename V>
constexpr auto map(T x, range<U> in, range<V> out) {
    return (((x - in.get_min()) * dimensions(out)) / dimensions(in)) +
           out.get_min();
}

//----------------------------------------------------------------------------//

template <typename It, typename T>
inline auto accumulate_min(It begin, It end, T starting_value) {
    return std::accumulate(
            begin, end, starting_value, [](const auto& a, const auto& b) {
                return min(a, b);
            });
}

template <typename It, typename T>
inline auto accumulate_max(It begin, It end, T starting_value) {
    return std::accumulate(
            begin, end, starting_value, [](const auto& a, const auto& b) {
                return max(a, b);
            });
}

template <typename It>
inline auto enclosing_range(It begin, It end) {
    if (begin == end) {
        throw std::runtime_error("can't minmax empty range");
    }
    return make_range(accumulate_min(begin + 1, end, *begin),
                      accumulate_max(begin + 1, end, *begin));
}

