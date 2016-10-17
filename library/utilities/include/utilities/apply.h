#pragma once

#include <initializer_list>
#include <tuple>
#include <utility>

template <typename Callback, typename Collection, size_t... Ix>
constexpr auto apply(Callback&& callback,
           const Collection& c,
           std::index_sequence<Ix...>) {
    return callback(std::get<Ix>(c)...);
}

/// Given a callable thing and a tuple of arguments, call the thing using the
/// tuple contents as arguments.
template <typename Callback, typename Collection>
constexpr auto apply(Callback&& callback, const Collection& c) {
    return apply(std::forward<Callback>(callback),
                 c,
                 std::make_index_sequence<std::tuple_size<Collection>{}>{});
}

//----------------------------------------------------------------------------//

template <typename Callbacks, typename Collection, size_t... Ix>
constexpr auto apply_each(Callbacks&& callbacks,
                const Collection& c,
                std::index_sequence<Ix...>) {
    return std::make_tuple(apply(std::get<Ix>(callbacks), c)...);
}

/// Given a tuple of callable things and a tuple of arguments, call each thing
/// with the same set of arguments.
/// Returns a tuple of results.
/// This function is a bit like `map`, but where map applies the same function
/// to a collection, this applies a collection of functions to the same thing.
template <typename Callbacks, typename Collection>
constexpr auto apply_each(Callbacks&& callbacks, const Collection& c) {
    return apply_each(std::forward<Callbacks>(callbacks),
                      c,
                      std::make_index_sequence<std::tuple_size<Callbacks>{}>{});
}

//----------------------------------------------------------------------------//

template <typename Callbacks, typename Collection, size_t... Ix>
void call_each(Callbacks&& callbacks,
               const Collection& c,
               std::index_sequence<Ix...>) {
    (void)std::initializer_list<int>{
            ((void)apply(std::get<Ix>(callbacks), c), 0)...};
}

/// Given a tuple of callable things and a tuple of arguments, call each thing
/// with the same set of arguments.
/// Doesn't return anything, which means that individual applications can return
/// void.
template <typename Callbacks, typename Collection>
void call_each(Callbacks&& callbacks, const Collection& c) {
    call_each(std::forward<Callbacks>(callbacks),
              c,
              std::make_index_sequence<std::tuple_size<Callbacks>{}>{});
}

