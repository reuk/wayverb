#pragma once

template <typename T>
struct named_value final {
    const char* name;
    T value;
};

template <typename T>
constexpr auto make_named_value(const char* name, T value) {
    return named_value<T>{name, std::move(value)};
}

template <typename T, typename Callback>
constexpr auto map(const named_value<T>& t, const Callback& callback) {
    return make_named_value(t.name, callback(t.value));
}