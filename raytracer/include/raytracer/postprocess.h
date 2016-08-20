#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/results.h"

#include "common/cl_common.h"
#include "common/hrtf_utils.h"
#include "common/pressure_intensity.h"
#include "common/receiver_settings.h"
#include "common/stl_wrappers.h"

namespace raytracer {

/// Sum impulses ocurring at the same (sampled) time and return a vector in
/// which each subsequent item refers to the next sample of an impulse
/// response.
template <typename im>  //  an impulse has a .volume and a .time
aligned::vector<aligned::vector<float>> flatten_impulses(
        const aligned::vector<im>& impulse,
        double samplerate,
        double acoustic_impedance) {
    const auto MAX_TIME_LIMIT = 20.0f;
    // Find the index of the final sample based on time and samplerate
    const auto maxtime = std::min(
            proc::max_element(impulse,
                              [](auto i, auto j) { return i.time < j.time; })
                    ->time,
            MAX_TIME_LIMIT);

    const auto MAX_SAMPLE = round(maxtime * samplerate) + 1;

    //  Create somewhere to store the results.
    aligned::vector<aligned::vector<float>> flattened(
            detail::components_v<volume_type>,
            aligned::vector<float>(MAX_SAMPLE, 0));

    //  For each impulse, calculate its index, then add the impulse's volumes
    //  to the volumes already in the output array.
    for (const auto& i : impulse) {
        const auto SAMPLE = round(i.time * samplerate);
        if (SAMPLE < MAX_SAMPLE) {
            for (auto j = 0u; j != flattened.size(); ++j) {
                const auto intensity = i.volume.s[j];
                const auto pressure = intensity_to_pressure(
                        intensity, static_cast<float>(acoustic_impedance));
                flattened[j][SAMPLE] += pressure;
            }
        }
    }

    return flattened;
}

/// Maps flattenimpulses over a vector of input vectors.
template <typename im>
aligned::vector<aligned::vector<aligned::vector<float>>> flatten_impulses(
        const aligned::vector<aligned::vector<im>>& impulse,
        double samplerate,
        double acoustic_impedance) {
    return map_to_vector(impulse, [=](const auto& i) {
        return flatten_impulses(i, samplerate, acoustic_impedance);
    });
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

template <typename im>
aligned::vector<float> flatten_filter_and_mixdown(
        const aligned::vector<im>& input,
        double output_sample_rate,
        double acoustic_impedance) {
    return multiband_filter_and_mixdown(
            flatten_impulses(input, output_sample_rate, acoustic_impedance),
            output_sample_rate);
}

aligned::vector<aligned::vector<float>> run_attenuation(
        const compute_context& cc,
        const model::ReceiverSettings& receiver,
        const results& input,
        double output_sample_rate, double acoustic_impedance);

aligned::vector<aligned::vector<float>> run_attenuation(
        const compute_context& cc,
        const model::ReceiverSettings& receiver,
        const aligned::vector<impulse>& input,
        double output_sample_rate,
        double speed_of_sound, double acoustic_impedance);

}  // namespace raytracer
