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

    Vec3(T t = T()) noexcept : x(t), y(t), z(t) {
    }

    Vec3(const T& x, const T& y, const T& z) noexcept : x(x), y(y), z(z) {
    }

    template <typename U>
    Vec3(const Vec3<U>& u) noexcept : x(u.x), y(u.y), z(u.z) {
    }

    Vec3(const Vec3& u) noexcept : x(u.x), y(u.y), z(u.z) {
    }

    Vec3& operator=(const Vec3& rhs) noexcept {
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;
        return *this;
    }

    template <typename V, typename U>
    auto fold(const U& u = U(), const V& v = V()) const {
        return v(x, v(y, v(z, u)));
    }

    template <typename U>
    auto map(const U& u = U()) const {
        return make_vec(u(x), u(y), u(z));
    }

    template <typename U>
    void for_each(const U& u = U()) const {
        u(x);
        u(y);
        u(z);
    }

    template <typename... U>
    auto zip(const U&... u) const {
        return make_vec(std::make_tuple(x, u.x...),
                        std::make_tuple(y, u.y...),
                        std::make_tuple(z, u.z...));
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
        return Vec3t(y, z, x) * Vec3<U>(rhs.z, rhs.x, rhs.y) -
               Vec3t(z, x, y) * Vec3<U>(rhs.y, rhs.z, rhs.x);
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

    T x, y, z;
};

template <typename T>
Vec3<T> make_vec(const T& x, const T& y, const T& z) {
    return Vec3<T>(x, y, z);
}

template <typename T>
std::ostream& operator<<(std::ostream& strm, const Vec3<T>& obj) {
    return strm << "(" << obj.x << ", " << obj.y << ", " << obj.z << ")";
}
