#pragma once

#include <functional>
#include <numeric>
#include <cmath>
#include <ostream>
#include <array>

#define VEC_OP(sym, functor)                                       \
    template <typename U>                                          \
    auto operator sym(const Vec3<U>& rhs) const {                  \
        return apply<std::functor<std::common_type_t<T, U>>>(rhs); \
    }                                                              \
                                                                   \
    template <typename U>                                          \
    auto operator sym(const U& rhs) const {                        \
        return operator sym(Vec3<U>(rhs));                         \
    }

template <typename T>
struct Vec3;

using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;
using Vec3i = Vec3<int>;
using Vec3b = Vec3<bool>;

template <typename T>
Vec3<T> make_vec(const T& x, const T& y, const T& z);

template <typename T>
struct Vec3 {
    using value_type = T;
    using Vec3t = Vec3<T>;

    Vec3(T t = T())
            : s({{t, t, t}}) {
    }

    Vec3(const T& x, const T& y, const T& z)
            : s({{x, y, z}}) {
    }

    template <typename U>
    Vec3(const Vec3<U>& u)
            : s({{static_cast<T>(u.s[0]),
                  static_cast<T>(u.s[1]),
                  static_cast<T>(u.s[2])}}) {
    }

    template <typename V, typename U>
    auto fold(const U& u = U(), const V& v = V()) const {
        return std::accumulate(s.begin(), s.end(), u, v);
    }

    template <typename U>
    auto map(const U& u = U()) const {
        return make_vec(u(s[0]), u(s[1]), u(s[2]));
    }

    template <typename... U>
    auto zip(const U&... u) const {
        return make_vec(std::make_tuple(s[0], u.s[0]...),
                        std::make_tuple(s[1], u.s[1]...),
                        std::make_tuple(s[2], u.s[2]...));
    }

    template <typename U, typename I>
    auto apply(const Vec3<I>& rhs, const U& u = U()) const {
        return zip(rhs).map(
            [&u](const auto& i) { return u(std::get<0>(i), std::get<1>(i)); });
    }

    template <typename U, typename I, typename J>
    auto apply(const Vec3<I>& a, const Vec3<J>& b, const U& u = U()) const {
        return zip(a, b).map([&u](const auto& i) {
            return u(std::get<0>(i), std::get<1>(i), std::get<2>(i));
        });
    }

    T sum() const {
        return fold<std::plus<T>>(0);
    }

    T product() const {
        return fold<std::multiplies<T>>(1);
    }

    T mag_squared() const {
        return dot(*this);
    }

    T mag() const {
        return sqrt(mag_squared());
    }

    template <typename U>
    auto dot(const Vec3<U>& rhs) const {
        return (*this * rhs).sum();
    }

    template <typename U>
    auto cross(const Vec3<U>& rhs) const {
        return Vec3t(s[1], s[2], s[0]) * Vec3<U>(rhs.s[2], rhs.s[0], rhs.s[1]) -
               Vec3t(s[2], s[0], s[1]) * Vec3<U>(rhs.s[1], rhs.s[2], rhs.s[0]);
    }

    VEC_OP(+, plus);
    VEC_OP(-, minus);
    VEC_OP(*, multiplies);
    VEC_OP(/, divides);
    VEC_OP(%, modulus);

    VEC_OP(==, equal_to);
    VEC_OP(!=, not_equal_to);
    VEC_OP(>, greater);
    VEC_OP(<, less);
    VEC_OP(>=, greater_equal);
    VEC_OP(<=, less_equal);

    VEC_OP(&&, logical_and);
    VEC_OP(||, logical_or);

    bool all() const {
        return fold<std::logical_and<T>>(true);
    }

    bool any() const {
        return fold<std::logical_or<T>>(false);
    }

    std::array<T, 3> s;
};

template <typename T>
Vec3<T> make_vec(const T& x, const T& y, const T& z) {
    return Vec3<T>(x, y, z);
}

template <typename T>
std::ostream& operator<<(std::ostream& strm, const Vec3<T>& obj) {
    return strm << "(" << obj.s[0] << ", " << obj.s[1] << ", " << obj.s[2]
                << ")";
}
