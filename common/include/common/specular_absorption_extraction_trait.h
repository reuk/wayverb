#pragma once

template <typename T>
struct specular_absorption_extraction_trait;

template<> struct specular_absorption_extraction_trait<surface> final {
    static const auto& run(const surface& s) { return s.specular_absorption; }
};

template<> struct specular_absorption_extraction_trait<float> final {
    static const auto& run(const float& s) { return s; }
};

template<> struct specular_absorption_extraction_trait<double> final {
    static const auto& run(const double& s) { return s; }
};

template <typename T>
auto get_specular_absorption(const T& t) {
    return specular_absorption_extraction_trait<T>::run(t);
}
