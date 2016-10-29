#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflection_processor/image_source.h"
#include "raytracer/reflection_processor/stochastic_histogram.h"
#include "raytracer/reflection_processor/visual.h"

#include "common/cl/iterator.h"
#include "common/dsp_vector_ops.h"
#include "common/geo/box.h"
#include "common/mixdown.h"
#include "common/model/parameters.h"
#include "common/pressure_intensity.h"

#include "audio_file/audio_file.h"

auto run_raytracer(const geo::box& box,
                   const surface<simulation_bands>& surface,
                   const model::parameters& params,
                   size_t image_source_order) {
    const auto voxelised = make_voxelised_scene_data(
            geo::get_scene_data(box, surface), 2, 0.1f);

    const auto directions = get_random_directions(1 << 16);

    constexpr auto max_img_src_order = 4;
    auto results = raytracer::canonical(begin(directions),
                                        end(directions),
                                        compute_context{},
                                        voxelised,
                                        params,
                                        max_img_src_order,
                                        0,
                                        true,
                                        [](auto i, auto steps) {});

    if (!results) {
        throw std::runtime_error{"raytracer failed to generate results"};
    }

    return std::move(results->aural);
}
