#pragma once

#include <initializer_list>
#include <utility>

template <typename Fun, typename... Ts>
void sequential_foreach(Fun&& f, Ts&&... args) {
    (void)std::initializer_list<int>{
            ((void)std::forward<Fun>(f)(std::forward<Ts>(args)), 0)...};
}
