#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/image_source/exact.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

#include "core/cl/iterator.h"
#include "core/geo/box.h"
#include "core/mixdown.h"
#include "core/pressure_intensity.h"

template <typename T>
auto run_exact_img_src(const wayverb::core::geo::box& box,
                       const T& absorption,
                       const glm::vec3& source,
                       const glm::vec3& receiver,
                       const wayverb::core::environment& env,
                       float simulation_time) {
    auto ret = wayverb::raytracer::image_source::find_impulses(
            box,
            source,
            receiver,
            absorption,
            simulation_time * env.speed_of_sound);

    //  Correct for distance travelled.
    for (auto& it : ret) {
        it.volume *= wayverb::core::pressure_for_distance(
                it.distance, env.acoustic_impedance);
    }
    return ret;
}

template <size_t Bands>
auto run_exact_img_src(const wayverb::core::geo::box& box,
                       const wayverb::core::surface<Bands>& surface,
                       const glm::vec3& source,
                       const glm::vec3& receiver,
                       const wayverb::core::environment& env,
                       float simulation_time) {
    return run_exact_image_src(
            box, surface.absorption, source, receiver, env, simulation_time);
}

auto run_fast_img_src(
        const wayverb::core::geo::box& box,
        const wayverb::core::surface<wayverb::core::simulation_bands>& surface,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const wayverb::core::environment& env,
        bool flip_phase) {
    const auto voxelised = make_voxelised_scene_data(
            wayverb::core::geo::get_scene_data(box, surface), 2, 0.1f);

    const auto directions = wayverb::core::get_random_directions(1 << 13);
    return wayverb::raytracer::image_source::run(
            begin(directions),
            end(directions),
            wayverb::core::compute_context{},
            voxelised,
            source,
            receiver,
            env,
            flip_phase);
}
