#pragma once

#include "raytracer/cl_structs.h"
#include "raytracer/random_directions.h"
#include "raytracer/raytracer_program.h"
#include "raytracer/results.h"

#include "common/aligned/map.h"
#include "common/cl_include.h"
#include "common/hrtf_utils.h"
#include "common/scene_data.h"
#include "common/sinc.h"

#include <array>
#include <cmath>
#include <numeric>
#include <set>
#include <vector>
#include <random>

#include <experimental/optional>

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

/// Filter and mix down each channel of the input data.
/// Optionally, normalize all channels, trim the tail, and scale the amplitude.
aligned::vector<aligned::vector<float>> process(
        aligned::vector<aligned::vector<aligned::vector<float>>>& data,
        float sr,
        bool do_normalize,
        float lo_cutoff,
        bool do_trim_tail,
        float volumme_scale);

VolumeType attenuation_for_distance(float distance);

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

/// Get the number of necessary reflections for a given min amplitude.
int compute_optimum_reflection_number(float min_amp, float max_reflectivity);

/*
class raytracer final {
public:
    raytracer(const cl::Context&, const cl::Device&);

    using PerStepCallback = std::function<void()>;

    results run(const CopyableSceneData& scene_data,
                const glm::vec3& micpos,
                const glm::vec3& source,
                size_t rays,
                size_t reflections,
                size_t num_image_source,
                std::atomic_bool& keep_going,
                const PerStepCallback& callback);

    results run(const CopyableSceneData& scene_data,
                const glm::vec3& micpos,
                const glm::vec3& source,
                const aligned::vector<cl_float3>& directions,
                size_t reflections,
                size_t num_image_source,
                std::atomic_bool& keep_going,
                const PerStepCallback& callback);

private:
    using kernel_type =
            decltype(std::declval<raytracer_program>().get_raytrace_kernel());

    cl::CommandQueue queue;
    kernel_type kernel;
};
*/

class raytracer final {
public:
    raytracer(const cl::Context&, const cl::Device&);

    using PerStepCallback = std::function<void()>;

    std::experimental::optional<results> run(
            const CopyableSceneData& scene_data,
            const glm::vec3& source,
            const glm::vec3& receiver,
            size_t rays,
            size_t reflections,
            size_t image_source,
            std::atomic_bool& keep_going,
            const PerStepCallback& callback);

private:
    cl::Context context;
    cl::Device device;
};

}  // namespace raytracer
