#pragma once

#include "raytracer/attenuator.h"
#include "raytracer/cl/structs.h"
#include "raytracer/histogram.h"

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/cl/common.h"
#include "common/cl/iterator.h"
#include "common/mixdown.h"
#include "common/model/receiver.h"
#include "common/pressure_intensity.h"
#include "common/scene_data.h"

#include "utilities/map_to_vector.h"

namespace raytracer {

constexpr auto min_element(double x) { return x; }
constexpr auto min_element(float x) { return x; }

template <typename T>
double min_absorption(const T& t) {
    return min_element(get_absorption(t));
}

template <typename It>
double min_absorption(It begin, It end) {
    if (begin == end) {
        throw std::runtime_error("can't find min absorption of empty vector");
    }
    return std::accumulate(begin + 1,
                           end,
                           min_absorption(*begin),
                           [](const auto& i, const auto& j) {
                               using std::min;
                               return min(i, min_absorption(j));
                           });
}

/// Get the number of necessary reflections for a given min amplitude.
size_t compute_optimum_reflection_number(double absorption);

template <typename It>
size_t compute_optimum_reflection_number(It begin, It end) {
    return compute_optimum_reflection_number(min_absorption(begin, end));
}

/// Get the number of necessary reflections for a given scene.
template <typename Vertex, typename Surface>
size_t compute_optimum_reflection_number(
        const generic_scene_data<Vertex, Surface>& scene) {
    return compute_optimum_reflection_number(scene.get_surfaces().begin(),
                                             scene.get_surfaces().end());
}

/// Recursively check a collection of impulses for the earliest non-zero time of
/// an impulse.
template <typename T>
inline auto find_predelay(const T& ret) {
    return std::accumulate(ret.begin() + 1,
                           ret.end(),
                           findPredelay(ret.front()),
                           [](auto a, const auto& b) {
                               auto pd = findPredelay(b);
                               if (a == 0) {
                                   return pd;
                               }
                               if (pd == 0) {
                                   return a;
                               }
                               return std::min(a, pd);
                           });
}

//----------------------------------------------------------------------------//

template <typename InputIt>
auto postprocess(InputIt b,
                 InputIt e,
                 const glm::vec3& position,
                 double speed_of_sound,
                 double sample_rate,
                 double max_seconds) {
    const auto make_iterator{[&](auto it) {
        return make_histogram_iterator(std::move(it), speed_of_sound);
    }};
    auto hist{histogram(make_iterator(b),
                        make_iterator(e),
                        sample_rate,
                        max_seconds,
                        sinc_sum_functor{})};
    return multiband_filter_and_mixdown(
            begin(hist), end(hist), sample_rate, [](auto it, auto index) {
                return make_cl_type_iterator(std::move(it), index);
            });
}

template <typename InputIt, typename Method>
auto postprocess(InputIt b,
                 InputIt e,
                 const Method& method,
                 const glm::vec3& position,
                 double speed_of_sound,
                 double sample_rate,
                 double max_seconds) {
    const auto make_iterator{[&](auto it) {
        return make_attenuator_iterator(std::move(it), method, position);
    }};
    return postprocess(make_iterator(b),
                       make_iterator(e),
                       position,
                       speed_of_sound,
                       sample_rate,
                       max_seconds);
}

template <typename InputIt, typename AttenuatorIt>
auto postprocess(InputIt b_input,
                 InputIt e_input,
                 AttenuatorIt b_attenuator,
                 AttenuatorIt e_attenuator,
                 const glm::vec3& position,
                 double speed_of_sound,
                 double sample_rate,
                 double max_seconds) {
    return map_to_vector(b_attenuator, e_attenuator, [&](const auto& i) {
        return postprocess(b_input,
                           e_input,
                           i,
                           position,
                           speed_of_sound,
                           sample_rate,
                           max_seconds);
    });
}

template <typename It>
auto run_attenuation(It b,
                     It e,
                     const model::receiver& receiver,
                     double speed_of_sound,
                     double sample_rate,
                     double max_seconds) {
    const auto run{[&](auto tag) {
        return postprocess(b,
                           e,
                           get_begin(receiver, tag),
                           get_end(receiver, tag),
                           receiver.position,
                           speed_of_sound,
                           sample_rate,
                           max_seconds);
    }};

    switch (receiver.mode) {
        case model::receiver::mode::microphones:
            return run(model::receiver::mode_t<
                       model::receiver::mode::microphones>{});

        case model::receiver::mode::hrtf:
            return run(model::receiver::mode_t<model::receiver::mode::hrtf>{});
    }
}

}  // namespace raytracer
