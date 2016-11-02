#pragma once

#include "core/scene_data.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace wayverb {
namespace raytracer {

constexpr auto min_element(double x) { return x; }
constexpr auto min_element(float x) { return x; }

template <typename T>
double min_absorption(const T& t) {
    return min_element(t.absorption);
}

template <typename It>
double min_absorption(It begin, It end) {
    if (begin == end) {
        throw std::runtime_error{"can't find min absorption of empty vector"};
    }
    return std::accumulate(begin + 1,
                           end,
                           min_absorption(*begin),
                           [](const auto& i, const auto& j) {
                               using std::min;
                               return min(i, min_absorption(j));
                           });
}

/// Get the number of necessary reflections for a given min amplitude.
inline size_t compute_optimum_reflection_number(double absorption) {
    return std::ceil(-3 / std::log10(1 - absorption));
}

template <typename It>
size_t compute_optimum_reflection_number(It begin, It end) {
    return compute_optimum_reflection_number(min_absorption(begin, end));
}

/// Get the number of necessary reflections for a given scene.
template <typename Vertex, typename Surface>
size_t compute_optimum_reflection_number(
        const core::generic_scene_data<Vertex, Surface>& scene) {
    return compute_optimum_reflection_number(scene.get_surfaces().begin(),
                                             scene.get_surfaces().end());
}

}  // namespace raytracer
}  // namespace wayverb
