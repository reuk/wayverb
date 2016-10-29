#pragma once

#include "raytracer/optimum_reflection_number.h"
#include "raytracer/reflector.h"

#include "common/cl/common.h"
#include "common/cl/geometry.h"
#include "common/model/parameters.h"
#include "common/nan_checking.h"
#include "common/pressure_intensity.h"
#include "common/spatial_division/scene_buffers.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "raytracer/reflection_processor/image_source.h"
#include "raytracer/reflection_processor/stochastic_histogram.h"
#include "raytracer/reflection_processor/visual.h"

#include "utilities/apply.h"
#include "utilities/map.h"

#include <experimental/optional>
#include <iostream>

namespace raytracer {

template <typename T>
class process_functor_adapter final {
public:
    explicit process_functor_adapter(T& t)
            : t_{t} {}

    template <typename... Ts>
    constexpr auto operator()(Ts&&... ts) {
        return t_.process(std::forward<Ts>(ts)...);
    }

private:
    T& t_;
};

struct make_process_functor_adapter final {
    template <typename T>
    constexpr auto operator()(T& t) const {
        return process_functor_adapter<T>{t};
    }
};

template <typename T>
class get_results_functor_adapter final {
public:
    explicit get_results_functor_adapter(T& t)
            : t_{t} {}

    template <typename... Ts>
    constexpr auto operator()(Ts&&... ts) {
        return t_.get_results(std::forward<Ts>(ts)...);
    }

private:
    T& t_;
};

struct make_get_results_functor_adapter final {
    template <typename T>
    constexpr auto operator()(T& t) const {
        return get_results_functor_adapter<T>{t};
    }
};

////////////////////////////////////////////////////////////////////////////////

/// parameters:
///     b_direction:        first iterator to a range of ray directions
///     e_direction:        last iterator to a range of ray directions
///     cc:                 the compute context + device to use
///     voxelised:          a 3D model with surfaces
///     params:             general scene parameters
///     keep_going:         whether or not to continue calculating
///     per_step_callback:  Will be called every step with the current step
///                         number and the total number of steps.
///     callbacks:          A tuple of callbacks which can be called with
///                         simulation parameters to construct processing
///                         objects.
///                         Each processing object should have a 'process'
///                         and a 'get_results' method.
template <typename It, typename PerStepCallback, typename Callbacks>
auto run(It b_direction,
         It e_direction,
         const compute_context& cc,
         const voxelised_scene_data<cl_float3, surface<simulation_bands>>&
                 voxelised,
         const model::parameters& params,
         const std::atomic_bool& keep_going,
         PerStepCallback&& per_step_callback,
         Callbacks&& callbacks) {
    const scene_buffers buffers{cc.context, voxelised};

    const auto make_ray_iterator = [&](auto it) {
        return make_mapping_iterator_adapter(std::move(it), [&](const auto& i) {
            return geo::ray{params.source, i};
        });
    };

    const auto num_directions = std::distance(b_direction, e_direction);
    auto processors =
            apply_each(std::forward<Callbacks>(callbacks),
                       std::tie(cc, params, voxelised, num_directions));

    using return_type = decltype(
            apply_each(map(make_get_results_functor_adapter{}, processors)));

    reflector ref{cc,
                  params.receiver,
                  make_ray_iterator(b_direction),
                  make_ray_iterator(e_direction)};

    for (auto i = 0ul,
              reflection_depth = compute_optimum_reflection_number(
                      voxelised.get_scene_data());
         i != reflection_depth;
         ++i) {
        if (!keep_going) {
            return std::experimental::optional<return_type>{};
        }

        const auto reflections = ref.run_step(buffers);
        const auto b = begin(reflections);
        const auto e = end(reflections);
        call_each(map(make_process_functor_adapter{}, processors),
                  std::tie(b, e, buffers, i, reflection_depth));

        per_step_callback(i, reflection_depth);
    }

    return std::experimental::make_optional(
            apply_each(map(make_get_results_functor_adapter{}, processors)));
}

////////////////////////////////////////////////////////////////////////////////

std::tuple<reflection_processor::make_image_source,
           reflection_processor::make_directional_histogram,
           reflection_processor::make_visual>
make_canonical_callbacks(size_t max_image_source_order, size_t visual_items);

template <typename Histogram>
struct aural_results final {
    aligned::vector<impulse<simulation_bands>> image_source;
    Histogram stochastic;
};

template <typename Histogram>
auto make_aural_results(aligned::vector<impulse<simulation_bands>> image_source,
                        Histogram stochastic) {
    return aural_results<Histogram>{std::move(image_source),
                                    std::move(stochastic)};
}

template <typename Histogram>
struct canonical_results final {
    aural_results<Histogram> aural;
    aligned::vector<aligned::vector<reflection>> visual;
};

template <typename Histogram>
auto make_canonical_results(
        aural_results<Histogram> aural,
        aligned::vector<aligned::vector<reflection>> visual) {
    return canonical_results<Histogram>{std::move(aural), std::move(visual)};
}

template <typename It, typename Callback>
auto canonical(
        It b_direction,
        It e_direction,
        const compute_context& cc,
        const voxelised_scene_data<cl_float3, surface<simulation_bands>>& scene,
        const model::parameters& params,
        size_t max_image_source_order,
        size_t visual_items,
        const std::atomic_bool& keep_going,
        Callback&& callback) {
    auto tup =
            run(std::move(b_direction),
                std::move(e_direction),
                cc,
                scene,
                params,
                keep_going,
                std::forward<Callback>(callback),
                make_canonical_callbacks(max_image_source_order, visual_items));
    return tup ? std::experimental::make_optional(make_canonical_results(
                         make_aural_results(std::move(std::get<0>(*tup)),
                                            std::move(std::get<1>(*tup))),
                         std::move(std::get<2>(*tup))))
               : std::experimental::nullopt;
}

}  // namespace raytracer
