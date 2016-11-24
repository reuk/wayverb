#pragma once

#include <string>

namespace util {

template <typename T>
struct named_value final {
    named_value(std::string name, T value)
            : name{std::move(name)}, value{std::move(value)} {}

    std::string name;
    T value;
};

template <typename T>
auto make_named_value(std::string name, T value) {
    return named_value<T>{std::move(name), std::move(value)};
}

template <typename Callback, typename T>
auto map(Callback&& callback, const named_value<T>& t) {
    return make_named_value(t.name, callback(t.value));
}

}  // namespace util
