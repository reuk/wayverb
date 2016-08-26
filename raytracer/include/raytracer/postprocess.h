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
size_t compute_optimum_reflection_number(float min_amp, float max_reflectivity);

template <typename im>
aligned::vector<volume_type> compute_multiband_signal(
        const aligned::vector<im>& impulse,
        double samplerate,
        float acoustic_impedance) {
    // Find the index of the final sample based on time and samplerate
    const auto max_time{std::min(
            proc::max_element(impulse,
                              [](auto i, auto j) { return i.time < j.time; })
                    ->time,
            20.0f)};

    const auto max_sample = round(max_time * samplerate) + 1;

    //  kutruff2009 p. 52
    //  With some assumptions (the wavefronts are incoherent), the total energy
    //  at some point can be calculated just by adding the energies
    //  (intensities) of the components.
    aligned::vector<volume_type> ret(max_sample, make_volume_type(0));
    for (const auto& i : impulse) {
        const auto sample{round(i.time * samplerate)};
        if (sample < ret.size()) {
            ret[sample] += i.volume;
        }
    }

    //  However, we need to convert to pressures at some point, so we'll do it
    //  here.

    //  TODO run tests on this
    //  for (auto& i : ret) {
    //      i = intensity_to_pressure(i, acoustic_impedance);
    //  }

    return ret;
}

template <typename im>
aligned::vector<aligned::vector<volume_type>> compute_multiband_signal(
        const aligned::vector<aligned::vector<im>>& impulse,
        double samplerate,
        double acoustic_impedance) {
    return map_to_vector(impulse, [=](const auto& i) {
        return compute_multiband_signal(i, samplerate, acoustic_impedance);
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
            compute_multiband_signal(
                    input, output_sample_rate, acoustic_impedance),
            output_sample_rate);
}

aligned::vector<aligned::vector<float>> run_attenuation(
        const compute_context& cc,
        const model::ReceiverSettings& receiver,
        const results& input,
        double output_sample_rate,
        double acoustic_impedance);

aligned::vector<aligned::vector<float>> run_attenuation(
        const compute_context& cc,
        const model::ReceiverSettings& receiver,
        const aligned::vector<impulse>& input,
        double output_sample_rate,
        double speed_of_sound,
        double acoustic_impedance);

}  // namespace raytracer
