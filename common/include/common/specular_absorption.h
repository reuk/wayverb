#pragma once

template <typename T>
struct specular_absorption_extraction_trait final {
    static const auto& run(const T& s) { return s; }
};

template<> struct specular_absorption_extraction_trait<surface> final {
    static const auto& run(const surface& s) { return s.specular_absorption; }
};

template <typename T>
auto get_specular_absorption(const T& t) {
    return specular_absorption_extraction_trait<T>::run(t);
}

template <typename T>
using specular_absorption_t =
        decltype(get_specular_absorption(std::declval<T>()));
