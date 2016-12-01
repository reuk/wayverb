#pragma once

#include "raytracer/optimum_reflection_number.h"
#include "raytracer/reflector.h"

#include "core/cl/common.h"
#include "core/cl/geometry.h"
#include "core/environment.h"
#include "core/nan_checking.h"
#include "core/pressure_intensity.h"
#include "core/spatial_division/scene_buffers.h"
#include "core/spatial_division/voxelised_scene_data.h"

#include "raytracer/reflection_processor/image_source.h"
#include "raytracer/reflection_processor/stochastic_histogram.h"
#include "raytracer/reflection_processor/visual.h"

#include "utilities/apply.h"
#include "utilities/map.h"

#include <experimental/optional>
#include <iostream>

namespace wayverb {
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

template <typename It, typename PerStepCallback, typename Callbacks>
auto run(
        It b_direction,
        It e_direction,
        const core::compute_context& cc,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                voxelised,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const std::atomic_bool& keep_going,
        PerStepCallback&& per_step_callback,
        Callbacks&& callbacks) {
    const core::scene_buffers buffers{cc.context, voxelised};

    const auto make_ray_iterator = [&](auto it) {
        return util::make_mapping_iterator_adapter(
                std::move(it), [&](const auto& i) {
                    return core::geo::ray{source, i};
                });
    };

    const auto num_directions = std::distance(b_direction, e_direction);
    auto processors = util::apply_each(std::forward<Callbacks>(callbacks),
                                       std::tie(cc,
                                                source,
                                                receiver,
                                                environment,
                                                voxelised,
                                                num_directions));

    using return_type = decltype(util::apply_each(
            util::map(make_get_results_functor_adapter{}, processors)));

    reflector ref{cc,
                  receiver,
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
        util::call_each(util::map(make_process_functor_adapter{}, processors),
                        std::tie(b, e, buffers, i, reflection_depth));

        per_step_callback(i, reflection_depth);
    }

    return std::experimental::make_optional(util::apply_each(
            util::map(make_get_results_functor_adapter{}, processors)));
}

}  // namespace raytracer
}  // namespace wayverb
