#pragma once

#include <sstream>

template <typename Fun, typename... Ts>
void sequential_foreach(Fun f, const Ts &... args) {
    (void)std::initializer_list<int>{((void)f(args), 0)...};
}

template <typename... Ts>
std::string build_string(const Ts &... ts) {
    std::stringstream ss;
    sequential_foreach([&ss](auto i) { ss << i; }, ts...);
    return ss.str();
}
