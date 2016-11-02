#pragma once

#include "raytracer/canonical.h"

auto run_raytracer(
        const wayverb::core::geo::box& box,
        const wayverb::core::surface<wayverb::core::simulation_bands>& surface,
        const wayverb::core::model::parameters& params,
        const wayverb::raytracer::simulation_parameters& sim_params) {
    auto results = wayverb::raytracer::canonical(
            wayverb::core::compute_context{},
            wayverb::core::geo::get_scene_data(box, surface),
            params,
            sim_params,
            0,
            true,
            [](auto i, auto steps) {});

    if (!results) {
        throw std::runtime_error{"raytracer failed to generate results"};
    }

    return std::move(results->aural);
}
