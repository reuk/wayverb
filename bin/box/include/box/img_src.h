#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/image_source/exact.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

#include "core/cl/iterator.h"
#include "core/geo/box.h"
#include "core/mixdown.h"
#include "core/model/parameters.h"
#include "core/pressure_intensity.h"

template <size_t Bands>
auto run_exact_img_src(const core::geo::box& box,
                       const core::surface<Bands>& surface,
                       const core::model::parameters& params,
                       float simulation_time,
                       bool flip_phase) {
    auto ret = raytracer::image_source::find_impulses(
            box,
            params.source,
            params.receiver,
            surface,
            simulation_time * params.speed_of_sound,
            flip_phase);

    //  Correct for distance travelled.
    for (auto& it : ret) {
        it.volume *= core::pressure_for_distance(it.distance,
                                                 params.acoustic_impedance);
    }
    return ret;
}

auto run_fast_img_src(const core::geo::box& box,
                      const core::surface<core::simulation_bands>& surface,
                      const core::model::parameters& params,
                      bool flip_phase) {
    const auto voxelised = make_voxelised_scene_data(
            core::geo::get_scene_data(box, surface), 2, 0.1f);

    const auto directions = core::get_random_directions(1 << 13);
    return raytracer::image_source::run(begin(directions),
                                        end(directions),
                                        core::compute_context{},
                                        voxelised,
                                        params,
                                        flip_phase);
}
