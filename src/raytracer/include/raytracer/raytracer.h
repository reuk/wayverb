#pragma once

#include "raytracer/optimum_reflection_number.h"
#include "raytracer/reflector.h"

#include "core/azimuth_elevation.h"
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

#define FUNCTOR_ADAPTER(name)                        \
    template <typename T>                            \
    class name##_functor_adapter {                   \
    public:                                          \
        explicit name##_functor_adapter(T& t)        \
                : t_{t} {}                           \
                                                     \
        template <typename... Ts>                    \
        constexpr auto operator()(Ts&&... ts) {      \
            return t_.name(std::forward<Ts>(ts)...); \
        }                                            \
                                                     \
    private:                                         \
        T& t_;                                       \
    };                                               \
                                                     \
    struct make_##name##_functor_adapter final {     \
        template <typename T>                        \
        constexpr auto operator()(T& t) const {      \
            return name##_functor_adapter<T>{t};     \
        }                                            \
    };

FUNCTOR_ADAPTER(process)
FUNCTOR_ADAPTER(get_processor)
FUNCTOR_ADAPTER(get_results)
FUNCTOR_ADAPTER(accumulate)
FUNCTOR_ADAPTER(get_group_processor)

////////////////////////////////////////////////////////////////////////////////

template <typename A, typename B, size_t... Ix>
void zip_apply(A&& a, B&& b, std::index_sequence<Ix...>) {
    (void)std::initializer_list<int>{
            ((void)(std::get<Ix>(a)(std::get<Ix>(b))), 0)...};
}

template <typename A, typename B>
void zip_apply(A&& a, B&& b) {
    zip_apply(std::forward<A>(a),
              std::forward<B>(b),
              std::make_index_sequence<std::tuple_size<A>{}>{});
}

////////////////////////////////////////////////////////////////////////////////

/// This could be WAY more generic but my deadline is rly soon so maybe another
/// time.
template <typename Engine>
class random_direction_generator_iterator final {
public:
    /// 'Random access' is misleading: you get a random value every time, so if
    /// you read back and forwards over the same region you'll get different
    /// values each time. *shrug*
    using iterator_category = std::random_access_iterator_tag;
    using value_type = glm::vec3;
    using difference_type = std::ptrdiff_t;
    using pointer = glm::vec3*;
    using reference = glm::vec3&;

    constexpr explicit random_direction_generator_iterator(difference_type pos,
                                                           Engine& engine)
            : engine_{&engine}
            , pos_{pos} {}

    constexpr random_direction_generator_iterator(
            const random_direction_generator_iterator&) = default;
    constexpr random_direction_generator_iterator(
            random_direction_generator_iterator&&) noexcept = default;
    constexpr random_direction_generator_iterator& operator=(
            const random_direction_generator_iterator&) = default;
    constexpr random_direction_generator_iterator& operator=(
            random_direction_generator_iterator&&) noexcept = default;

    value_type operator*() const { return core::random_unit_vector(*engine_); }

    constexpr random_direction_generator_iterator& operator++() {
        ++pos_;
        return *this;
    }

    constexpr random_direction_generator_iterator& operator--() {
        --pos_;
        return *this;
    }

    constexpr random_direction_generator_iterator operator++(int) {
        return random_direction_generator_iterator{pos_++};
    }

    constexpr random_direction_generator_iterator operator--(int) {
        return random_direction_generator_iterator{pos_--};
    }

    constexpr random_direction_generator_iterator operator+(
            difference_type n) const {
        auto ret = *this;
        return ret += n;
    }

    constexpr random_direction_generator_iterator operator-(
            difference_type n) const {
        auto ret = *this;
        return ret -= n;
    }

    constexpr random_direction_generator_iterator& operator+=(
            difference_type n) {
        pos_ += n;
        return *this;
    }

    constexpr random_direction_generator_iterator& operator-=(
            difference_type n) {
        pos_ -= n;
        return *this;
    }

    constexpr difference_type operator-(
            const random_direction_generator_iterator& rhs) {
        return pos_ - rhs.pos_;
    }

    constexpr bool operator==(const random_direction_generator_iterator& rhs) {
        return pos_ == rhs.pos_;
    }

    constexpr bool operator!=(const random_direction_generator_iterator& rhs) {
        return !operator==(rhs);
    }

    constexpr bool operator<(const random_direction_generator_iterator& rhs) {
        return pos_ < rhs.pos_;
    }
    constexpr bool operator<=(const random_direction_generator_iterator& rhs) {
        return pos_ <= rhs.pos_;
    }
    constexpr bool operator>(const random_direction_generator_iterator& rhs) {
        return pos_ > rhs.pos_;
    }
    constexpr bool operator>=(const random_direction_generator_iterator& rhs) {
        return pos_ >= rhs.pos_;
    }

private:
    /// Dereference has to be const so this is mutable
    Engine* engine_ = nullptr;
    difference_type pos_ = 0;
};

template <typename Engine>
constexpr auto make_random_direction_generator_iterator(
        typename random_direction_generator_iterator<Engine>::difference_type
                pos,
        Engine& engine) {
    return random_direction_generator_iterator<Engine>{pos, engine};
}

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

    auto processors = util::apply_each(
            util::map(make_get_processor_functor_adapter{},
                      std::forward<Callbacks>(callbacks)),
            std::tie(cc, source, receiver, environment, voxelised));

    using return_type = decltype(util::apply_each(
            util::map(make_get_results_functor_adapter{}, processors)));

    constexpr auto segment_size = 1 << 13;
    const auto reflection_depth =
            compute_optimum_reflection_number(voxelised.get_scene_data());

    const auto run_segment = [&](auto b, auto e) {
        const auto num_directions = std::distance(b, e);

        reflector ref{cc, receiver, make_ray_iterator(b), make_ray_iterator(e)};

        auto group_processors = util::apply_each(
                util::map(make_get_group_processor_functor_adapter{},
                          processors),
                std::make_tuple(num_directions));

        for (auto i = 0ul; i != reflection_depth; ++i) {
            const auto reflections = ref.run_step(buffers);
            const auto b = begin(reflections);
            const auto e = end(reflections);
            util::call_each(
                    util::map(make_process_functor_adapter{}, group_processors),
                    std::tie(b, e, buffers, i, reflection_depth));
        }

        zip_apply(util::map(make_accumulate_functor_adapter{}, processors),
                  group_processors);
    };

    const auto groups = std::distance(b_direction, e_direction) / segment_size;

    auto it = b_direction;
    for (auto group = 0; it + segment_size <= e_direction;
         it += segment_size, ++group) {
        run_segment(it, it + segment_size);

        per_step_callback(group, groups);

        if (!keep_going) {
            return std::experimental::optional<return_type>{};
        }
    }

    if (it != e_direction) {
        run_segment(it, e_direction);
    }

    return std::experimental::make_optional(util::apply_each(
            util::map(make_get_results_functor_adapter{}, processors)));
}

}  // namespace raytracer
}  // namespace wayverb
