#pragma once

#include "raytracer/canonical.h"

auto run_raytracer(const geo::box& box,
                   const surface<simulation_bands>& surface,
                   const model::parameters& params,
                   const raytracer::simulation_parameters& sim_params) {
    auto results = raytracer::canonical(compute_context{},
                                        geo::get_scene_data(box, surface),
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
