#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/results.h"

#include "common/cl/common.h"
#include "common/hrtf_utils.h"
#include "common/model/receiver_settings.h"
#include "common/pressure_intensity.h"
#include "common/stl_wrappers.h"

namespace raytracer {

/// Get the number of necessary reflections for a given min amplitude.
size_t compute_optimum_reflection_number(float absorption);

/// Get the number of necessary reflections for a given scene.
size_t compute_optimum_reflection_number(
        const aligned::vector<surface>& surfaces);
size_t compute_optimum_reflection_number(const scene_data& scene);

template <typename It>
aligned::vector<volume_type> convert_to_histogram(It begin,
                                                  It end,
                                                  float histogram_frequency,
                                                  float max_seconds) {
    if (begin == end) {
        return aligned::vector<volume_type>{};
    }
    const auto max_time_in_input{
            std::max_element(begin, end, [](auto i, auto j) {
                return i.time < j.time;
            })->time};
    const auto max_time{std::min(max_time_in_input, max_seconds)};

    const size_t output_size = std::round(max_time * histogram_frequency) + 1;

    //  Build the histogram by looking up the time of each input, and
    //  then adding its energy to the appropriate histogram bin.
    aligned::vector<volume_type> ret{output_size, make_volume_type(0)};
    for (auto i{begin}; i != end; ++i) {
        const auto bin{std::round(i->time * histogram_frequency)};
        if (bin < ret.size()) {
            ret[bin] += i->volume;
        }
    }

    return ret;
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

/// The base case of the findPredelay recursion.
template <typename T>
inline auto find_predelay_base(const T& t) {
    return t.time;
}

template <>
inline auto find_predelay(const attenuated_impulse& i) {
    return find_predelay_base(i);
}

template <>
inline auto find_predelay(const impulse& i) {
    return find_predelay_base(i);
}

/// Recursively subtract a time value from the time fields of a collection of
/// impulses.
template <typename T>
inline void fix_predelay(T& ret, float seconds) {
    for (auto& i : ret) {
        fixPredelay(i, seconds);
    }
}

template <typename T>
inline void fix_predelay_base(T& ret, float seconds) {
    ret.time = ret.time > seconds ? ret.time - seconds : 0;
}

/// The base case of the fixPredelay recursion.
template <>
inline void fix_predelay(attenuated_impulse& ret, float seconds) {
    fix_predelay_base(ret, seconds);
}

template <>
inline void fix_predelay(impulse& ret, float seconds) {
    fix_predelay_base(ret, seconds);
}

/// Fixes predelay by finding and then removing predelay.
template <typename T>
inline void fix_predelay(T& ret) {
    auto predelay = findPredelay(ret);
    fixPredelay(ret, predelay);
}

template <typename It>  /// It is an iterator through impulse types
aligned::vector<float> flatten_filter_and_mixdown(It begin,
                                                  It end,
                                                  float output_sample_rate,
                                                  float max_seconds) {
    return multiband_filter_and_mixdown(
            convert_to_histogram(begin, end, output_sample_rate, max_seconds),
            output_sample_rate);
}

aligned::vector<aligned::vector<float>> run_attenuation(
        const compute_context& cc,
        const model::receiver_settings& receiver,
        const aligned::vector<impulse>& input,
        double output_sample_rate,
        double speed_of_sound,
        double acoustic_impedance,
        double max_seconds);

}  // namespace raytracer
