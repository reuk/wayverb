#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/histogram.h"
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

aligned::vector<aligned::vector<float>> run_attenuation(
        const compute_context& cc,
        const model::receiver_settings& receiver,
        const aligned::vector<impulse>& input,
        double output_sample_rate,
        double speed_of_sound,
        double acoustic_impedance,
        double max_seconds);

}  // namespace raytracer
