#pragma once

#include "raytracer/attenuator.h"
#include "raytracer/cl/structs.h"
#include "raytracer/histogram.h"
#include "raytracer/results.h"

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/cl/common.h"
#include "common/hrtf_utils.h"
#include "common/map_to_vector.h"
#include "common/model/receiver_settings.h"
#include "common/pressure_intensity.h"
#include "common/stl_wrappers.h"

namespace raytracer {

constexpr auto min(double x) {
    return x;
}

constexpr auto min(float x) {
    return x;
}

template <typename T>
double min_absorption(const T& t) {
    return min(get_specular_absorption(t));
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

template <typename It>
aligned::vector<aligned::vector<float>> attenuate_microphone(
        const model::receiver_settings& receiver,
        double speed_of_sound,
        double sample_rate,
        double max_seconds,
        It begin,
        It end) {
    return map_to_vector(receiver.microphones, [&](const auto& i) {
        const auto processed{attenuate(
                microphone{get_pointing(i.orientable, receiver.position),
                           i.shape},
                receiver.position,
                begin,
                end)};
        return multiband_filter_and_mixdown(dirac_histogram(processed.begin(),
                                                            processed.end(),
                                                            speed_of_sound,
                                                            sample_rate,
                                                            max_seconds),
                                            sample_rate);
    });
}

template <typename It>
aligned::vector<aligned::vector<float>> attenuate_hrtf(
        const model::receiver_settings& receiver,
        double speed_of_sound,
        double sample_rate,
        double max_seconds,
        It begin,
        It end) {
    const auto channels = {hrtf::channel::left, hrtf::channel::right};
    return map_to_vector(channels, [&](const auto& i) {
        const auto processed{
                attenuate(hrtf{get_pointing(receiver.hrtf, receiver.position),
                               glm::vec3{0, 1, 0},
                               i},
                          receiver.position,
                          begin,
                          end)};
        return multiband_filter_and_mixdown(dirac_histogram(processed.begin(),
                                                            processed.end(),
                                                            speed_of_sound,
                                                            sample_rate,
                                                            max_seconds),
                                            sample_rate);
    });
}

template <typename It>
aligned::vector<aligned::vector<float>> run_attenuation(
        const model::receiver_settings& receiver,
        double speed_of_sound,
        double sample_rate,
        double max_seconds,
        It begin,
        It end) {
    switch (receiver.mode) {
        case model::receiver_settings::mode::microphones:
            return attenuate_microphone(receiver,
                                        speed_of_sound,
                                        sample_rate,
                                        max_seconds,
                                        begin,
                                        end);
        case model::receiver_settings::mode::hrtf:
            return attenuate_hrtf(receiver,
                                  speed_of_sound,
                                  sample_rate,
                                  max_seconds,
                                  begin,
                                  end);
    }
}

}  // namespace raytracer
