#pragma once

#include <sstream>

template <typename Fun, typename... Ts>
inline void sequential_foreach(Fun f, const Ts&... args) {
    (void)std::initializer_list<int>{((void)f(args), 0)...};
}

template <typename... Ts>
inline std::ostream& to_stream(std::ostream& os, const Ts&... ts) {
    sequential_foreach([&os](const auto& i) { os << i; }, ts...);
    return os;
}

template <typename... Ts>
inline std::string build_string(const Ts&... ts) {
    std::stringstream ss;
    to_stream(ss, ts...);
    return ss.str();
}

struct Bracketer final {
    explicit Bracketer(std::ostream& os,
                       const char* open = "{",
                       const char* closed = "}");

    ~Bracketer();

private:
    std::ostream& os;
    const char* closed;
};
