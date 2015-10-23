#pragma once

#include <sstream>

template <typename T>
void build_string(std::stringstream &ss, T &&t) {
    ss << t;
}

template <typename T, typename... Ts>
void build_string(std::stringstream &ss, T &&t, Ts... ts) {
    ss << t;
    build_string(ss, ts...);
}

template <typename... Ts>
std::string build_string(Ts &&... ts) {
    std::stringstream ss;
    build_string(ss, ts...);
    return ss.str();
}
