#pragma once

template <typename T>
constexpr auto identity(T&& t) -> decltype(std::forward<T>(t)) {
    return std::forward<T>(t);
}

struct identity_functor final {
    template <typename T>
    constexpr auto operator()(T&& t) -> decltype(std::forward<T>(t)) {
        return std::forward<T>(t);
    }
};
