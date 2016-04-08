#pragma once

#include "reduce.h"

constexpr int popcount(size_t t) {
    int ret = 0;
    for (; t; t &= t - 1) {
        ++ret;
    }
    return ret;
}

template <typename Flag>
struct FlagSet {
    constexpr explicit FlagSet(Flag flag)
            : flag(flag) {
    }
    template <typename T>
    constexpr auto operator()(T t) const {
        return t & flag;
    }

private:
    Flag flag;
};

template <typename T, typename A>
constexpr bool any_flags_set(T t, const A& arr) {
    return any(arr, FlagSet<T>(t));
}

template <typename T, typename A>
constexpr bool all_flags_set(T t, const A& arr) {
    return all(arr, FlagSet<T>(t));
}
