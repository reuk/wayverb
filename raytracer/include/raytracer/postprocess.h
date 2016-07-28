#pragma once

#include "raytracer/cl_structs.h"
#include "raytracer/results.h"

#include "common/receiver_settings.h"

namespace raytracer {

/// Sum impulses ocurring at the same (sampled) time and return a vector in
/// which each subsequent item refers to the next sample of an impulse
/// response.
aligned::vector<aligned::vector<float>> flatten_impulses(
        const aligned::vector<AttenuatedImpulse>& impulse, float samplerate);

/// Maps flattenImpulses over a vector of input vectors.
aligned::vector<aligned::vector<aligned::vector<float>>> flatten_impulses(
        const aligned::vector<aligned::vector<AttenuatedImpulse>>& impulse,
        float samplerate);

/// Recursively check a collection of Impulses for the earliest non-zero time of
/// an impulse.
template <typename T>
inline float find_predelay(const T& ret) {
    return accumulate(ret.begin() + 1,
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
template <>
inline float find_predelay(const AttenuatedImpulse& i) {
    return i.time;
}

/// Recursively subtract a time value from the time fields of a collection of
/// Impulses.
template <typename T>
inline void fix_predelay(T& ret, float seconds) {
    for (auto& i : ret) {
        fixPredelay(i, seconds);
    }
}

/// The base case of the fixPredelay recursion.
template <>
inline void fix_predelay(AttenuatedImpulse& ret, float seconds) {
    ret.time = ret.time > seconds ? ret.time - seconds : 0;
}

/// Fixes predelay by finding and then removing predelay.
template <typename T>
inline void fix_predelay(T& ret) {
    auto predelay = findPredelay(ret);
    fixPredelay(ret, predelay);
}

aligned::vector<float> flatten_filter_and_mixdown(
        const aligned::vector<AttenuatedImpulse>& input,
        double output_sample_rate);

aligned::vector<aligned::vector<float>> run_attenuation(
        const compute_context& cc,
        const model::ReceiverSettings& receiver,
        const results& input,
        double output_sample_rate);

aligned::vector<aligned::vector<float>> run_attenuation(
        const compute_context&cc,
        const model::ReceiverSettings& receiver,
        const aligned::vector<Impulse>& input,
        double output_sample_rate);

}  // namespace raytracer
