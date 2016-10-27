#pragma once

#include "utilities/tuple_like.h"

#include <initializer_list>
#include <tuple>
#include <utility>

template <typename Func, typename... Ts>
constexpr auto apply_params(Func&& func, Ts&&... ts) {
    return func(std::forward<Ts>(ts)...);
}

template <typename Func, typename T, size_t... Ix>
constexpr auto apply(Func&& func, T&& t, std::index_sequence<Ix...>) {
    return apply_params(std::forward<Func>(func),
                        tuple_like_getter<Ix>(std::forward<T>(t))...);
}

/// Given a callable thing and a tuple of arguments, call the thing using the
/// tuple contents as arguments.
template <typename Callback, typename Collection>
constexpr auto apply(Callback&& callback, Collection&& c) {
    return apply(std::forward<Callback>(callback),
                 std::forward<Collection>(c),
                 std::make_index_sequence<
                         tuple_like_size_v<decay_const_ref_t<Collection>>>{});
}

////////////////////////////////////////////////////////////////////////////////

template <typename Callbacks, typename Collection, size_t... Ix>
constexpr auto apply_each(Callbacks&& callbacks,
                          const Collection& c,
                          std::index_sequence<Ix...>) {
    return std::make_tuple(apply(tuple_like_getter<Ix>(callbacks), c)...);
}

/// Given a tuple of callable things and a tuple of arguments, call each thing
/// with the same set of arguments.
/// Returns a tuple of results.
/// This function is a bit like `map`, but where map applies the same function
/// to a collection, this applies a collection of functions to the same thing.
template <typename Callbacks, typename Collection>
constexpr auto apply_each(Callbacks&& callbacks, const Collection& c) {
    return apply_each(
            std::forward<Callbacks>(callbacks),
            c,
            std::make_index_sequence<
                    tuple_like_size_v<decay_const_ref_t<Callbacks>>>{});
}

template <typename Callbacks, size_t... Ix>
constexpr auto apply_each(Callbacks&& callbacks, std::index_sequence<Ix...>) {
    return std::make_tuple(tuple_like_getter<Ix>(callbacks)()...);
}

template <typename Callbacks>
constexpr auto apply_each(Callbacks&& callbacks) {
    return apply_each(
            std::forward<Callbacks>(callbacks),
            std::make_index_sequence<
                    tuple_like_size_v<decay_const_ref_t<Callbacks>>>{});
}

////////////////////////////////////////////////////////////////////////////////

template <typename Callbacks, typename Collection, size_t... Ix>
void call_each(Callbacks&& callbacks,
               const Collection& c,
               std::index_sequence<Ix...>) {
    (void)std::initializer_list<int>{
            ((void)apply(tuple_like_getter<Ix>(callbacks), c), 0)...};
}

/// Given a tuple of callable things and a tuple of arguments, call each thing
/// with the same set of arguments.
/// Doesn't return anything, which means that individual applications can return
/// void.
template <typename Callbacks, typename Collection>
void call_each(Callbacks&& callbacks, const Collection& c) {
    call_each(std::forward<Callbacks>(callbacks),
              c,
              std::make_index_sequence<
                      tuple_like_size_v<decay_const_ref_t<Callbacks>>>{});
}

template <typename Callbacks, size_t... Ix>
void call_each(Callbacks&& callbacks, std::index_sequence<Ix...>) {
    (void)std::initializer_list<int>{
            ((void)tuple_like_getter<Ix>(callbacks)(), 0)...};
}

template <typename Callbacks>
void call_each(Callbacks&& callbacks) {
    call_each(std::forward<Callbacks>(callbacks),
              std::make_index_sequence<
                      tuple_like_size_v<decay_const_ref_t<Callbacks>>>{});
}
