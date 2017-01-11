#pragma once

#include "utilities/aligned/vector.h"

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
        throw std::runtime_error{"Can't find min absorption of empty vector."};
    }
    return std::accumulate(begin + 1,
                           end,
                           min_absorption(*begin),
                           [](const auto& i, const auto& j) {
                               using std::min;
                               return min(i, min_absorption(j));
                           });
}

/// Get the number of necessary reflections for a given min absorption.
inline size_t compute_optimum_reflection_number(double absorption) {
    return std::ceil(-6 / std::log10(1 - absorption));
}

template <typename It>
size_t compute_optimum_reflection_number(It begin, It end) {
    return compute_optimum_reflection_number(min_absorption(begin, end));
}

/// Get the number of necessary reflections for a given scene.
template <typename Vertex, typename Surface>
size_t compute_optimum_reflection_number(
        const core::generic_scene_data<Vertex, Surface>& scene) {
    std::vector<bool> used_surfaces_hash(scene.get_surfaces().size(), false);
    for (const auto& tri : scene.get_triangles()) {
        if (0 <= tri.surface && tri.surface < used_surfaces_hash.size()) {
            used_surfaces_hash[tri.surface] = true;
        }
    }

    util::aligned::vector<Surface> used_surfaces;
    used_surfaces.reserve(used_surfaces_hash.size());
    for (size_t i = 0; i != used_surfaces_hash.size(); ++i) {
        if (used_surfaces_hash[i]) {
            used_surfaces.emplace_back(scene.get_surfaces()[i]);
        }
    }

    return compute_optimum_reflection_number(std::begin(used_surfaces),
                                             std::end(used_surfaces));
}

}  // namespace raytracer
}  // namespace wayverb
