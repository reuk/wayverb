#pragma once

#include <initializer_list>
#include <tuple>
#include <utility>

template <typename Callback, typename Collection, size_t... Ix>
auto apply(Callback&& callback,
           const Collection& c,
           std::index_sequence<Ix...>) {
    return callback(std::get<Ix>(c)...);
}

template <typename Callback, typename Collection>
auto apply(Callback&& callback, const Collection& c) {
    return apply(std::forward<Callback>(callback),
                 c,
                 std::make_index_sequence<std::tuple_size<Collection>{}>{});
}

template <typename Callbacks, typename Collection, size_t... Ix>
void call_each(Callbacks&& callbacks,
               const Collection& c,
               std::index_sequence<Ix...>) {
    (void)std::initializer_list<int>{
            ((void)apply(std::get<Ix>(callbacks), c), 0)...};
}

template <typename Callbacks, typename Collection>
void call_each(Callbacks&& callbacks, const Collection& c) {
    call_each(std::forward<Callbacks>(callbacks),
              c,
              std::make_index_sequence<std::tuple_size<Callbacks>{}>{});
}

