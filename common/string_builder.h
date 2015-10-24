#pragma once

#include <sstream>

inline void build_string(std::stringstream &ss) { }

template <typename T, typename... Ts>
void build_string(std::stringstream &ss, const T &t, const Ts&... ts) {
    ss << t;
    build_string(ss, ts...);
}

template <typename... Ts>
std::string build_string(const Ts &... ts) {
    std::stringstream ss;
    build_string(ss, ts...);
    return ss.str();
}
