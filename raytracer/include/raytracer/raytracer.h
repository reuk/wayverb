#pragma once

#include "attenuator_program.h"
#include "cl_structs.h"
#include "raytracer_program.h"

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

class Results final {
public:
    using map_type = aligned::map<aligned::vector<cl_ulong>, Impulse>;

    Results(map_type&& image_source,
            aligned::vector<aligned::vector<Impulse>>&& diffuse,
            const glm::vec3& receiver,
            const glm::vec3& source);
    /// Raytraces are calculated in relation to a specific microphone position.
    /// This is a struct to keep the impulses and mic position together, because
    /// you'll probably never need one without the other.
    class Selected {
    public:
        Selected(const aligned::vector<Impulse>& impulses,
                 const glm::vec3& receiver,
                 const glm::vec3& source);

        aligned::vector<Impulse> get_impulses() const;
        glm::vec3 get_receiver() const;
        glm::vec3 get_source() const;

    private:
        aligned::vector<Impulse> impulses;
        glm::vec3 receiver;
        glm::vec3 source;
    };

    Selected get_diffuse() const;
    Selected get_image_source(bool remove_direct) const;
    Selected get_all(bool remove_direct) const;

private:
    aligned::vector<Impulse> get_diffuse_impulses() const;
    aligned::vector<Impulse> get_image_source_impulses(
            bool remove_direct) const;

    map_type image_source;
    aligned::vector<aligned::vector<Impulse>> diffuse;
    glm::vec3 receiver;
    glm::vec3 source;
};

aligned::vector<cl_float3> get_random_directions(size_t num);

class Raytracer final {
public:
    Raytracer(const cl::Context&, const cl::Device&);

    using PerStepCallback = std::function<void()>;

    Results run(const CopyableSceneData& scene_data,
                const glm::vec3& micpos,
                const glm::vec3& source,
                size_t rays,
                size_t reflections,
                size_t num_image_source,
                std::atomic_bool& keep_going,
                const PerStepCallback& callback);

    Results run(const CopyableSceneData& scene_data,
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

/// Class for parallel HRTF attenuation of raytrace results.
class HrtfAttenuator final {
public:
    HrtfAttenuator(const cl::Context&, const cl::Device&);

    /// Attenuate some raytrace results.
    /// The outer vector corresponds to separate channels, the inner vector
    /// contains the impulses, each of which has a time and an 8-band volume.
    aligned::vector<AttenuatedImpulse> process(const Results::Selected& results,
                                               const glm::vec3& direction,
                                               const glm::vec3& up,
                                               const glm::vec3& position,
                                               HrtfChannel channel);

    const std::array<std::array<std::array<cl_float8, 180>, 360>, 2>&
    get_hrtf_data() const;

private:
    using kernel_type =
            decltype(std::declval<attenuator_program>().get_hrtf_kernel());

    cl::CommandQueue queue;
    kernel_type kernel;

    cl::Buffer cl_hrtf;
};

/// Class for parallel Speaker attenuation of raytrace results.
class MicrophoneAttenuator final {
public:
    MicrophoneAttenuator(const cl::Context&, const cl::Device&);

    /// Attenuate some raytrace results.
    /// The outer vector corresponds to separate channels, the inner vector
    /// contains the impulses, each of which has a time and an 8-band volume.
    aligned::vector<AttenuatedImpulse> process(const Results::Selected& results,
                                               const glm::vec3& pointing,
                                               float shape,
                                               const glm::vec3& position);

private:
    using kernel_type = decltype(
            std::declval<attenuator_program>().get_microphone_kernel());

    cl::CommandQueue queue;
    kernel_type kernel;
};
}  // namespace raytracer
