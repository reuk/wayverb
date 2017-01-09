#pragma once

#include "raytracer/raytracer.h"
#include "raytracer/simulation_parameters.h"

#include "core/spatial_division/voxelised_scene_data.h"

namespace wayverb {
namespace raytracer {

std::tuple<reflection_processor::make_image_source,
           reflection_processor::make_directional_histogram,
           reflection_processor::make_visual>
make_canonical_callbacks(const simulation_parameters& params,
                         size_t visual_items);

template <typename Histogram>
struct simulation_results final {
    util::aligned::vector<impulse<core::simulation_bands>> image_source;
    Histogram stochastic;
};

template <typename Histogram>
auto make_simulation_results(
        util::aligned::vector<impulse<core::simulation_bands>> image_source,
        Histogram stochastic) {
    return simulation_results<Histogram>{std::move(image_source),
                                         std::move(stochastic)};
}

template <typename Histogram>
struct canonical_results final {
    simulation_results<Histogram> aural;
    util::aligned::vector<util::aligned::vector<reflection>> visual;
};

template <typename Histogram>
auto make_canonical_results(
        simulation_results<Histogram> aural,
        util::aligned::vector<util::aligned::vector<reflection>> visual) {
    return canonical_results<Histogram>{std::move(aural), std::move(visual)};
}

template <typename Callback>
auto canonical(
        const core::compute_context& cc,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                scene,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const simulation_parameters& sim_params,
        size_t visual_items,
        const std::atomic_bool& keep_going,
        Callback&& callback) {
    std::default_random_engine engine{std::random_device{}()};

    auto tup = run(
            make_random_direction_generator_iterator(0, engine),
            make_random_direction_generator_iterator(sim_params.rays, engine),
            cc,
            scene,
            source,
            receiver,
            environment,
            keep_going,
            std::forward<Callback>(callback),
            make_canonical_callbacks(sim_params, visual_items));
    return tup ? std::experimental::make_optional(make_canonical_results(
                         make_simulation_results(std::move(std::get<0>(*tup)),
                                                 std::move(std::get<1>(*tup))),
                         std::move(std::get<2>(*tup))))
               : std::experimental::nullopt;
}

}  // namespace raytracer
}  // namespace wayverb
