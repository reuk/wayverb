#pragma once

#include <tuple>
#include <functional>

template <typename T>
struct Vec3;

using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;
using Vec3i = Vec3<int>;
using Vec3b = Vec3<bool>;

template <typename T>
struct Vec3 {
    using value_type = T;
    using Vec3t = Vec3<T>;

    Vec3(T t = T())
            : x(t)
            , y(t)
            , z(t) {
    }

    Vec3(T x, T y, T z)
            : x(x)
            , y(y)
            , z(z) {
    }

    template <typename V, typename U>
    auto fold(const U & u = U(), const V & v = V()) const {
        return v(x, v(y, v(z, u)));
    }

    template <typename U>
    auto map(const U & u = U()) const {
        return Vec3<decltype(u(x))>(u(x), u(y), u(z));
    }

    template <typename U>
    auto zip(const Vec3<U> & u = Vec3<U>()) const {
        return Vec3<decltype(std::make_tuple(x, u.x))>(std::make_tuple(x, u.x),
                                                       std::make_tuple(y, u.y),
                                                       std::make_tuple(z, u.z));
    }

    template <typename U>
    auto binop(const Vec3 & rhs, const U & u = U()) const {
        return zip(rhs).map(
            [&u](const auto & i) { return u(std::get<0>(i), std::get<1>(i)); });
    }

    Vec3b operator==(const Vec3t & rhs) const {
        return binop<std::equal_to<T>>(rhs);
    }

    Vec3b operator<(const Vec3t & rhs) const {
        return binop<std::less<T>>(rhs);
    }

    Vec3b operator>(const Vec3t & rhs) const {
        return binop<std::greater<T>>(rhs);
    }

    Vec3b operator&&(const Vec3t & rhs) const {
        return binop<std::logical_and<T>>(rhs);
    }

    Vec3b operator||(const Vec3t & rhs) const {
        return binop<std::logical_or<T>>(rhs);
    }

    Vec3t operator+(const Vec3t & rhs) const {
        return binop<std::plus<T>>(rhs);
    }

    Vec3t operator-(const Vec3t & rhs) const {
        return binop<std::minus<T>>(rhs);
    }

    Vec3t operator*(const Vec3t & rhs) const {
        return binop<std::multiplies<T>>(rhs);
    }

    Vec3t operator/(const Vec3t & rhs) const {
        return binop<std::divides<T>>(rhs);
    }

    bool all() const {
        return fold<std::logical_and<T>>(true);
    }

    bool any() const {
        return fold<std::logical_or<T>>(false);
    }

    union {
        struct {
            T x, y, z;
        };
        T s[3];
    };

};
