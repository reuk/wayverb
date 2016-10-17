#pragma once

#include <initializer_list>
#include <utility>

template <typename Fun, typename... Ts>
void sequential_foreach(Fun&& f, const Ts&... args) {
    (void)std::initializer_list<int>{((void)std::forward<Fun>(f)(args), 0)...};
}
